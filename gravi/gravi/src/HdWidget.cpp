///
/// @file HdWidget.cpp
/// @brief A Qt Widget that renders the UsdImagingGLEngine

#include <iostream>

#include <QDateTime>
#include <QWheelEvent>
#include <QTimer>
#include <QApplication>
#include <QOpenGLFunctions>
#include <QOpenGLContext>
#include <QOpenGLPaintDevice>
#include <QMessageBox>
#include <QPainter>
#include <pxr/imaging/hdSt/renderDelegate.h>
#include <pxr/imaging/hd/renderIndex.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usdLux/sphereLight.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/imaging/hd/renderBuffer.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/imaging/hio/image.h>
#include <pxr/usdImaging/usdAppUtils/frameRecorder.h>
#include <pxr/usdImaging/usdAppUtils/camera.h>

#include "gravityWell.h"

#include "HdWidget.h"

QMatrix4x4 static GfMatrixToQMatrix4(const pxr::GfMatrix4f& _mat) {
    return {
        _mat[0][0], _mat[1][0], _mat[2][0], _mat[3][0],
        _mat[0][1], _mat[1][1], _mat[2][1], _mat[3][1],
        _mat[0][2], _mat[1][2], _mat[2][2], _mat[3][2],
        _mat[0][3], _mat[1][3], _mat[2][3], _mat[3][3]
    };
}

//-------------------------------------------------------------------------
// Construction / Destruction
//-------------------------------------------------------------------------

HdWidget::HdWidget(QWidget* _parent) :
    QOpenGLWidget(_parent),
    m_stage(pxr::UsdStage::CreateInMemory("UsdGraviScene"))
{
    if (m_stage) {
        pxr::UsdGeomXform::Define(m_stage, pxr::SdfPath("/Gravi"));
        m_stage->SetDefaultPrim(m_stage->GetPrimAtPath(pxr::SdfPath("/Gravi")));
        m_stage->SetMetadata(pxr::TfToken("upAxis"), "Y"),
        m_stage->SetMetadata(pxr::TfToken("metersPerUnit"), 1.0);
    }

    setFocusPolicy(Qt::StrongFocus);
    auto* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &HdWidget::updateGL);
    timer->start(16); // ~60 FPS (1000ms / 60 ≈ 16.67ms)
}

HdWidget::HdWidget(QWidget* _parent, const std::string &_stagePath) : HdWidget(_parent) { m_stage = pxr::UsdStage::Open(_stagePath); }

HdWidget::~HdWidget() {
    m_engine->StopRenderer();
    delete m_engine;
}

//-------------------------------------------------------------------------
// Stage Operations
//-------------------------------------------------------------------------

void HdWidget::setStage(const std::string& _stagePath)
{
    m_stage = pxr::UsdStage::Open(_stagePath);
    adjustMetaData();
    update();
}

void HdWidget::saveStage(const std::string& _savePath) const { m_stage->Export(_savePath); }

//-------------------------------------------------------------------------
// Camera Controls
//-------------------------------------------------------------------------

void HdWidget::setFov(float const _fov) { m_fov = _fov; update(); }

void HdWidget::setClipping(float const _nearClip, float const _farClip)
{
    m_nearClip = _nearClip;
    m_farClip = _farClip;
    update();
}

void HdWidget::setBackgroundColor(const int _r, const int _g, const int _b)
{
    m_backgroundColor = pxr::GfVec4f(
            static_cast<float>(_r)/255.0f,
            static_cast<float>(_g)/255.0f,
            static_cast<float>(_b)/255.0f, 1.0f
        );
}

pxr::GfMatrix4d HdWidget::getViewMatrix() const
{
    // We get the camera position.
    const pxr::GfVec3d cameraPos = getCameraPosition();

    // And we return a matrix that looks at the positional offset.
    // This matrix is used for viewport rendering.
    return pxr::GfMatrix4d(1.0).SetLookAt(
            cameraPos,
            m_cameraLook,
            pxr::GfVec3d(0, 1, 0)
            );
}

pxr::GfVec3d HdWidget::getCameraPosition() const
{
    // We obtain the camera position using the Horizontal Coordinate System.
    // (Φ, θ, r) are used to find a position in (x, y, z) space.

    // Azimuth is the horizontal rotation around the center point.
    const pxr::GfQuatd quatAzimuth(cos(m_azimuth / 2.0f), 0.0f, sin(m_azimuth / 2.0f), 0.0f);

    // Elevation is the vertical rotation around the center point.
    const pxr::GfQuatd quatElevation(cos(m_elevation / 2.0f), sin(m_elevation / 2.0f), 0.0f, 0.0f);

    // We combine the azimuth and elevation into a rotation matrix to rotate a point.
    // This point is originally (0, 0, distance).
    const pxr::GfMatrix4d rotationMatrix = pxr::GfMatrix4d(1.0).SetRotate(quatAzimuth * quatElevation);
    const auto direction = pxr::GfVec3d(0.0f, 0.0f, -m_cameraZoom);

    // We then get the rotated point in 3d space.
    const pxr::GfVec3d rotatedDirection = rotationMatrix.Transform(direction);

    // A positional offset is added to the position in case the user has tried to pan.
    return rotatedDirection + m_cameraLook;
}

pxr::TfTokenVector HdWidget::getStageCameras() const
{
    pxr::TfTokenVector cameras;

    pxr::UsdPrimRange range = m_stage->Traverse();
    for (const pxr::UsdPrim& prim : range) {
        if (prim.IsA<pxr::UsdGeomCamera>()) {
            cameras.push_back(prim.GetPrimPath().GetToken());
        }
    }

    return cameras;
}

void HdWidget::setActiveCamera(const pxr::TfToken &_cameraPath)  { m_camera = _cameraPath; }

//-------------------------------------------------------------------------
// Rendering Controls
//-------------------------------------------------------------------------

void HdWidget::setRenderMode(const ViewMode _mode) {
    m_viewMode = _mode;

    if (_mode == SOLID || _mode == WIREFRAME) {
        setCurrentRendererPlugin(pxr::TfToken("HdStormRendererPlugin"));
    }
    if (_mode == RENDER) {
        setCurrentRendererPlugin(pxr::TfToken("HdGraviRendererPlugin"));
    }
}

void HdWidget::setGridVisibility(const bool _visible)  { m_enableGrid = _visible; update(); }

void HdWidget::setWellVisibility(const bool _visible) { m_showWells = _visible; update(); }

//-------------------------------------------------------------------------
// Time Controls
//-------------------------------------------------------------------------

void HdWidget::setCurrentTime(const int _value)  { m_time = _value; update(); }

double HdWidget::getCurrentTime() const { return m_time.GetValue(); }

//-------------------------------------------------------------------------
// Renderer Controls
//-------------------------------------------------------------------------

pxr::TfToken HdWidget::getCurrentRendererPlugin() const { return m_engine->GetCurrentRendererId(); }

void HdWidget::setCurrentRendererPlugin(pxr::TfToken const &_id) { m_engine->SetRendererPlugin(_id); update(); }

pxr::TfTokenVector HdWidget::getRendererPlugins() { return pxr::UsdImagingGLEngine::GetRendererPlugins(); }

pxr::TfTokenVector HdWidget::getRendererAOVs() const { return m_engine->GetRendererAovs(); }

void HdWidget::setCurrentRendererAOV(pxr::TfToken const &_id) { m_engine->SetRendererAov(_id); update(); }

//-------------------------------------------------------------------------
// Special Operations
//-------------------------------------------------------------------------

void HdWidget::createGravityWell() const {
    if (!m_stage) {
        qWarning() << "Cannot create GravityWell - no valid USD stage";
        return;
    }

    // Base path for the gravity well
    pxr::SdfPath basePath("/Gravi/GWell");

    // Find a unique path
    pxr::SdfPath gravityWellPath = basePath;
    int counter = 1;
    while (m_stage->GetPrimAtPath(gravityWellPath)) {
        gravityWellPath = pxr::SdfPath(basePath.GetString() + "_" + std::to_string(counter++));
    }

    // Create the GravityWell prim
    pxr::GravityWell gravityWell;
    try {
        gravityWell = pxr::GravityWell::Define(m_stage, gravityWellPath);
        if (!gravityWell) {
            throw std::runtime_error("Failed to define GravityWell prim");
        }
    } catch (const std::exception& e) {
        qCritical() << "Error creating GravityWell:" << e.what();
        return;
    }

    // Create a transform operations stack
    pxr::UsdGeomXformable xformable(gravityWell.GetPrim());

    // Clear any existing transform operations
    xformable.ClearXformOpOrder();

    // Add transform operations (in the order you want them applied)
    // Typically: translate, then rotate, then scale
    auto translateOp = xformable.AddTranslateOp(pxr::UsdGeomXformOp::PrecisionDouble);
    auto rotateOp = xformable.AddRotateXYZOp();
    auto scaleOp = xformable.AddScaleOp();

    // Set default attributes with proper typing
    auto force = gravityWell.CreateForceAttr(pxr::VtValue(-0.98f));
    force.Set(-0.98f);
}

bool HdWidget::renderToDisk(const QString& _fileName, const int _frame, const int _width) const {
    if (!m_stage) {
        std::cerr << "Error: Invalid USD stage." << std::endl;
        return false;
    }

    // Initialize frame recorder
    pxr::UsdAppUtilsFrameRecorder recorder(pxr::TfToken("HdGraviRendererPlugin"), true);
    recorder.SetImageWidth(_width);
    recorder.SetActiveRenderSettingsPrimPath(pxr::SdfPath("/Gravi/RenderSettings"));

    pxr::UsdGeomCamera renderCamera;

    if (m_camera != "perspective") {
        // Use existing camera
        renderCamera = UsdAppUtilsGetCameraAtPath(m_stage, pxr::SdfPath(m_camera.GetText()));
        if (!renderCamera) {
            std::cerr << "Error: Camera not found in stage: " << m_camera.GetText() << std::endl;
            return false;
        }
    } else {
        QMessageBox* popup = new QMessageBox(QMessageBox::Information,
                            "Error",
                            "Unable to render using perspect view! Please render through a camera.",
                            QMessageBox::NoButton);
        popup->setModal(true); popup->show();QCoreApplication::processEvents();
        return false;
    }

    QCoreApplication::processEvents();

    bool success = recorder.Record(m_stage, renderCamera, _frame, _fileName.toStdString());

    if (!success) {
        std::cerr << "Error: Failed to record frame." << std::endl;
    }
    return success;
}

double HdWidget::getStageMPU() const { return m_mpu; }

std::string HdWidget::getStageUpAxis() const { return m_upAxis; }

void HdWidget::setCameraComboBox(QComboBox *_cams) { m_camerasCombo = _cams; }

//-------------------------------------------------------------------------
// QOpenGLWidget Overrides
//-------------------------------------------------------------------------

void HdWidget::initializeGL()
{
    initializeOpenGLFunctions();
    setupCustomGL();

    m_engine = new pxr::UsdImagingGLEngine();
    pxr::TfTokenVector plugins = getRendererPlugins();

    if (m_stage) {
        adjustMetaData();
    }

    update();
}

void HdWidget::paintGL() {
    // We clear the buffer every frame
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(m_backgroundColor[0], m_backgroundColor[1], m_backgroundColor[2], m_backgroundColor[3]);
    if (!m_stage) return;

    // Set up camera light
    pxr::GlfSimpleLight cameraLight;
    pxr::GfVec4f lightPos(0, 0, 0, 1);  // Will be transformed by view matrix

    // Configure the camera light properties
    cameraLight.SetPosition(pxr::GfVec4f(lightPos));
    cameraLight.SetDiffuse(pxr::GfVec4f(1.0f, 1.0f, 1.0f, 1.0f));  // White light
    cameraLight.SetAmbient(pxr::GfVec4f(0.1f, 0.1f, 0.1f, 1.0f));   // Low ambient
    cameraLight.SetSpecular(pxr::GfVec4f(1.0f, 1.0f, 1.0f, 1.0f));  // Full specular
    cameraLight.SetTransform(getViewMatrix().GetInverse());

    pxr::GlfSimpleLightVector lights;
    lights.push_back(cameraLight);

    pxr::GlfSimpleMaterial material;
    material.SetAmbient(pxr::GfVec4f(0.2f, 0.2f, 0.2f, 1.0f));
    material.SetDiffuse(pxr::GfVec4f(0.8f, 0.8f, 0.8f, 1.0f));
    material.SetSpecular(pxr::GfVec4f(0.2f, 0.2f, 0.2f, 1.0f));
    material.SetShininess(1.0f);

    pxr::UsdImagingGLRenderParams params;
    if (m_viewMode == SOLID) {
        params.enableLighting = true;
        params.enableSceneLights = false;
        params.enableSceneMaterials = true;
        params.forceRefresh = true;
        params.drawMode = pxr::UsdImagingGLDrawMode::DRAW_GEOM_SMOOTH;
    }
    if (m_viewMode == WIREFRAME) {
        params.enableLighting = false;
        params.drawMode = pxr::UsdImagingGLDrawMode::DRAW_WIREFRAME;
    }
    if (m_viewMode == RENDER) {
        params.enableLighting = true;
        params.enableSceneLights = true;
        params.enableSceneMaterials = true;
    }
    params.forceRefresh = true;
    params.frame = m_time;
    //params.clearColor = m_backgroundColor;

    glViewport(0, 0, width(), height());
    m_engine->SetRenderBufferSize(pxr::GfVec2i(width(), height()));
    m_engine->SetRenderViewport(pxr::GfVec4i(0, 0, width(), height()));

    pxr::GfMatrix4d viewMatrix(1.0);
    pxr::GfMatrix4d projMatrix(1.0);

    if (m_camera != "perspective") {
        pxr::SdfPath camPath(m_camera.GetText());
        if (!m_stage->GetPrimAtPath(camPath)) {
            std::cerr << "Camera does not exist in stage: " << camPath << std::endl;
            return;
        }
        m_engine->SetCameraPath(camPath);

        auto cameraPrim = pxr::UsdGeomCamera::Get(m_stage, camPath);
        auto camera = cameraPrim.GetCamera(m_time);

        viewMatrix = camera.GetTransform().GetInverse();
        projMatrix = camera.GetFrustum().ComputeProjectionMatrix();
    }
    else {
        pxr::GfFrustum frustum;
        viewMatrix = getViewMatrix();

        frustum.SetPerspective(m_fov,
                   static_cast<float>(width()) / static_cast<float>(height()),
                   m_nearClip,
                   m_farClip);

        projMatrix =  frustum.ComputeProjectionMatrix();
        m_engine->SetCameraState(viewMatrix, projMatrix);
    }

    pxr::GfMatrix4d invViewMatrix = viewMatrix.GetInverse();
    constexpr pxr::GfVec3d pos(0, 0, 0);
    const pxr::GfVec3d transformedLightPos = invViewMatrix.Transform(pos);
    cameraLight.SetPosition(pxr::GfVec4f(static_cast<float>(transformedLightPos[0]), static_cast<float>(transformedLightPos[1]), static_cast<float>(transformedLightPos[2]), 1));
    lights[0] = cameraLight;  // Update the light in the vector

    // Set the lights and material for rendering
    m_engine->SetLightingState(lights, material, pxr::GfVec4f(0, 0, 0, 1.0f));

    paintCustomGL(viewMatrix, projMatrix);
    m_engine->Render(m_stage->GetPseudoRoot(), params);
}

void HdWidget::resizeGL(int const _w, int const _h)
{
    glViewport(0, 0, _w, _h);
    m_engine->SetRenderViewport(pxr::GfVec4i(0, 0, _w, _h));
    m_engine->SetRenderBufferSize(pxr::GfVec2i(_w, _h));
    update();
}

void HdWidget::updateGL() {
    // By default, you can only see engine render updates when certain buttons or keys are pressed.
    // To solve this, we make a custom updateGL function that runs every timer tick.
    // This gives a much more "active" viewport.
    update();
    QApplication::processEvents(); // We call processEvents so that the UI doesnt freeze up.
    // We couldn't place this inside the update() because a recursive call would be made an the app would crash.
}

//-------------------------------------------------------------------------
// Event Handlers
//-------------------------------------------------------------------------

void HdWidget::mousePressEvent(QMouseEvent* _event) { m_lastMousePosition = _event->pos(); }

void HdWidget::mouseMoveEvent(QMouseEvent* _event)
{
    const QPoint delta = _event->pos() - m_lastMousePosition;

    if (_event->buttons() & Qt::LeftButton) {
        m_elevation += static_cast<float>(delta.y()) * m_rotationSpeed;
        m_azimuth -= static_cast<float>(delta.x()) * m_rotationSpeed;

        const float limit = M_PI_2 - m_rotationSpeed;

        m_elevation = std::clamp(m_elevation, -limit, limit);

        if (m_camera != "perspective") {
            m_camera = pxr::TfToken("perspective");
            m_camerasCombo->setCurrentIndex(0);
        }
    }
    else if (_event->buttons() & Qt::MiddleButton) {
        pxr::GfMatrix4d const inverseViewMatrix = getViewMatrix().GetInverse();
        pxr::GfVec3d panDelta(
            static_cast<float>(-delta.x()) * m_panSpeed,
            static_cast<float>(delta.y()) * m_panSpeed,  // Inverting Y for the correct direction
            0.0
        );

        panDelta = inverseViewMatrix.TransformDir(panDelta);
        m_cameraLook += panDelta;

        if (m_camera != "perspective") {
            m_camera = pxr::TfToken("perspective");
            m_camerasCombo->setCurrentIndex(0);
        }

    }
    else if (_event->buttons() & Qt::RightButton) {

    }

    m_lastMousePosition = _event->pos();

    update();
}

void HdWidget::wheelEvent(QWheelEvent *_event)
{
    const int delta = _event->angleDelta().y();

    const auto zoomFactor = static_cast<float>(log(m_cameraZoom + 1.0f));

    if (delta < 0) {
        m_cameraZoom += zoomFactor * m_zoomSpeed;
    } else {
        m_cameraZoom -= zoomFactor * m_zoomSpeed;
    }

    m_cameraZoom = std::max(1.0, m_cameraZoom);

    if (m_camera != "perspective") {
        m_camera = pxr::TfToken("perspective");
        m_camerasCombo->setCurrentIndex(0);

    }

    update();
}

void HdWidget::keyPressEvent(QKeyEvent *_event)
{
    QWidget::keyPressEvent(_event);
    if (_event->key() == Qt::Key_H) {
        m_cameraLook = pxr::GfVec3d(0, 0, 0);
    }
    update();
}

//-------------------------------------------------------------------------
// Private Methods
//-------------------------------------------------------------------------

void HdWidget::adjustMetaData() const
{
    // Most USD files come with an upAxis or metersPerUnit (MPU)
    // We need to make sure that the application scales these correctly.
    // We use 1 Unit = 1 Meter for simplicity.
    auto upAxis = pxr::TfToken("Y");
    double metersPerUnit = 1.0;

    // Get the metadata values
    m_stage->GetMetadata(pxr::TfToken("upAxis"), &upAxis);
    m_stage->GetMetadata(pxr::TfToken("metersPerUnit"), &metersPerUnit);

    // Create a transform matrix that combines both rotation and scale
    pxr::GfMatrix4d xform(1.0); // Identity matrix

    // Apply rotation based on upAxis
    if (upAxis == pxr::TfToken("X")) {
        // Rotate -90 degrees around Y to make X up
        xform = pxr::GfMatrix4d().SetRotate(pxr::GfRotation(pxr::GfVec3d::YAxis(), -90)) * xform;
    } else if (upAxis == pxr::TfToken("Z")) {
        // Rotate 90 degrees around X to make Z up
        xform = pxr::GfMatrix4d().SetRotate(pxr::GfRotation(pxr::GfVec3d::XAxis(), -90)) * xform;
    }
    // No rotation needed if upAxis is Y

    // Apply scaling based on metersPerUnit
    double scaleFactor = metersPerUnit;
    xform = pxr::GfMatrix4d().SetScale(pxr::GfVec3d(scaleFactor)) * xform;

    // Apply the combined transform to the engine
    // Depending on which scene, we open, we have to reset the engine so that it clears the previous scene index.
    // This also makes setting the Root Transform of the engine easier.
    const pxr::TfToken currentRenderer = m_engine->GetCurrentRendererId(); // Storing the active renderer so that we don't revert to default on engine reset.
    m_engine = new pxr::UsdImagingGLEngine();
    m_engine->SetRootTransform(xform);
    m_engine->SetRendererPlugin(currentRenderer);

    m_mpu = metersPerUnit;
    m_upAxis = upAxis.GetString();
}

void HdWidget::setupCustomGL() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_POINT_SPRITE);

    m_wellShader = new QOpenGLShaderProgram(this);
    m_wellShader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/WellShader.vert");
    m_wellShader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/WellShader.frag");
    m_wellShader->link();

    m_gridShader = new QOpenGLShaderProgram(this);
    m_gridShader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/GridShader.vert");
    m_gridShader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/GridShader.frag");
    m_gridShader->link();

    // Create grid geometry
    constexpr int gridSize = 20; // Number of lines in each direction
    constexpr float gridStep = 1.0f; // Distance between grid lines
    constexpr float halfSize = (gridSize * gridStep) / 2;

    std::vector<float> vertices;

    // Generate grid lines in X direction (along Z axis)
    for (int i = 0; i <= gridSize; ++i) {
        float z = -halfSize + static_cast<float>(i) * gridStep;
        // Line along X axis
        vertices.push_back(-halfSize); vertices.push_back(0.0f); vertices.push_back(z);
        vertices.push_back(halfSize); vertices.push_back(0.0f); vertices.push_back(z);
    }

    // Generate grid lines in Z direction (along X axis)
    for (int j = 0; j <= gridSize; ++j) {
        float x = -halfSize + static_cast<float>(j) * gridStep;
        // Line along Z axis
        vertices.push_back(x); vertices.push_back(0.0f); vertices.push_back(-halfSize);
        vertices.push_back(x); vertices.push_back(0.0f); vertices.push_back(halfSize);
    }

    // Create and bind VAO/VBO
    m_gridVAO.create();
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_gridVAO);

    m_gridVBO.create();
    m_gridVBO.bind();
    m_gridVBO.allocate(vertices.data(), static_cast<int>(vertices.size() * sizeof(float)));

    // Configure vertex attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

    m_gridVBO.release();
}


void HdWidget::paintCustomGL(const pxr::GfMatrix4d& _view, const pxr::GfMatrix4d& _proj) {
    // Enable depth testing for wells
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Enable blending for the fresnel effect
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const QMatrix4x4 projection = GfMatrixToQMatrix4(pxr::GfMatrix4f(_proj));
    QMatrix4x4 view = GfMatrixToQMatrix4(pxr::GfMatrix4f(_view));

    if (m_enableGrid && m_viewMode != RENDER) {
        drawGrid(view, projection);
    }

    if (m_showWells && m_viewMode != RENDER) {
        drawGravityWell(view, projection);
    }
}

void HdWidget::drawGrid(const QMatrix4x4& _viewMatrix, const QMatrix4x4& _projMatrix) {
    glEnable(GL_DEPTH_TEST);
    if (!m_gridShader || !m_gridVAO.isCreated()) {
        return;
    }

    QOpenGLVertexArrayObject::Binder vaoBinder(&m_gridVAO);
    m_gridShader->bind();

    // Set shader uniforms
    m_gridShader->setUniformValue("view", _viewMatrix);
    m_gridShader->setUniformValue("proj", _projMatrix);

    // Main grid lines (darker)
    m_gridShader->setUniformValue("color", QVector4D(0.5f, 0.5f, 0.5f, 1.0f));
    glDrawArrays(GL_LINES, 0, 84); // Draw all grid lines

    m_gridShader->release();
}

void HdWidget::drawGravityWell(const QMatrix4x4& _viewMatrix, const QMatrix4x4& _projMatrix) {  // Assume this is (Projection * View)
    if (m_showWells) {
        // Enable depth testing and blending
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Traverse the stage for GravityWell prims
        for (const pxr::UsdPrim& prim : m_stage->Traverse()) {
            if (prim.GetTypeName() == pxr::TfToken("GravityWell")) {
                pxr::GravityWell well(prim);
                float force;

                if (auto forAttr = well.GetForceAttr()) {
                    forAttr.Get(&force);
                }

                float radius = std::sqrt(std::abs(force)/0.25f);

                pxr::GfVec3d position{};

                pxr::UsdGeomXformable xformable(well.GetPrim());
                if (xformable) {
                    // Get the local transformation matrix
                    pxr::GfMatrix4d localXform{};
                    bool resetXformStack;
                    xformable.GetLocalTransformation(&localXform, &resetXformStack);

                    // Apply rotation based on upAxis
                    if (m_upAxis == "X") {
                        // Rotate -90 degrees around Y to make X up
                        localXform = localXform * pxr::GfMatrix4d().SetRotate(pxr::GfRotation(pxr::GfVec3d::YAxis(), -90));
                    } else if (m_upAxis == "Z") {
                        // Rotate 90 degrees around X to make Z up
                        localXform = localXform * pxr::GfMatrix4d().SetRotate(pxr::GfRotation(pxr::GfVec3d::XAxis(), -90));
                    }

                    // Extract the translation component (last column of the matrix)
                    position = localXform.ExtractTranslation() * m_sceneScale * m_mpu;
                }

                if (radius <= 0.0) {
                    continue;
                }

                // Create VAO/VBO for this well
                QOpenGLVertexArrayObject wellVAO;
                QOpenGLBuffer wellVBO(QOpenGLBuffer::VertexBuffer);

                if (wellVAO.create()) {
                    QOpenGLVertexArrayObject::Binder vaoBinder(&wellVAO);

                    if (wellVBO.create()) {
                        wellVBO.bind();
                        QVector3D pointPosition(0, 0, 0);
                        wellVBO.allocate(&pointPosition, sizeof(QVector3D));

                        m_wellShader->bind();
                        m_wellShader->enableAttributeArray(0);
                        m_wellShader->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(QVector3D));

                        QMatrix4x4 modelMatrix;
                        modelMatrix.translate(static_cast<float>(position[0] / m_sceneScale), static_cast<float>(position[1] / m_sceneScale), static_cast<float>(position[2] / m_sceneScale));

                        m_wellShader->setUniformValue("view", _viewMatrix);
                        m_wellShader->setUniformValue("proj", _projMatrix);
                        m_wellShader->setUniformValue("model", modelMatrix);

                        // The 10 is the initial camera zoom
                        const auto pointSize = radius * m_sceneScale * 2 * 10 / m_cameraZoom;
                        glPointSize(static_cast<float>(pointSize));

                        glDrawArrays(GL_POINTS, 0, 1);

                        m_wellShader->release();
                        wellVBO.release();
                    }
                }
            }
        }

        // Reset GL state
        glDisable(GL_BLEND);
        glPointSize(1.0f);
    }
}