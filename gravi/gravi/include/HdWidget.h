/// @file HdWidget.h
/// @brief Qt Widget for Hydra Imaging Engine rendering
///
/// This class implements a Qt OpenGL widget that integrates the Hydra Imaging Engine,
/// providing functionality for:
/// - USD stage loading and rendering
/// - Camera controls and view manipulation
/// - Renderer plugin management
/// - Custom OpenGL drawing (grids, gravity wells)
/// - Time-based animation control
///
/// @author Christopher Hosken
/// @version 1.0
/// @date 07/05/2025 Pre-release cleanup
/// @ingroup Gravi

#ifndef HDWIDGET_H_
#define HDWIDGET_H_

#include <QComboBox>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <pxr/usdImaging/usdImagingGL/engine.h>
#include <pxr/usd/usd/stage.h>

//-----------------------------------------------------------------------------
/// @class HdWidget
/// @brief Qt Widget for rendering USD stages using Hydra Imaging Engine
///
/// Provides a Qt OpenGL widget that integrates USD stage rendering with:
/// - Interactive camera controls
/// - Multiple view modes (Wireframe, Solid, Render)
/// - Custom OpenGL overlays
/// - Renderer plugin management
//-----------------------------------------------------------------------------
class HdWidget final : public QOpenGLWidget, protected QOpenGLFunctions
{
  Q_OBJECT

public:
  //-------------------------------------------------------------------------
  /// @name Construction / Destruction
  //-------------------------------------------------------------------------
  /// @{

  /// @brief Constructs an empty HdWidget
  /// @param _parent Parent Qt widget
  explicit HdWidget(QWidget* _parent = nullptr);

  /// @brief Constructs an HdWidget with USD stage
  /// @param _parent Parent Qt widget
  /// @param _stagePath Path to USD file to load
  HdWidget(QWidget* _parent, const std::string& _stagePath);

  /// @brief Destructor - cleans up OpenGL resources and USD stage
  ~HdWidget() override;

  /// @}

  //-------------------------------------------------------------------------
  /// @name View Modes
  //-------------------------------------------------------------------------
  /// @{

  /// @brief View modes supported by the widget
  enum ViewMode {
    WIREFRAME = 0,  ///< Wireframe rendering mode
    SOLID = 1,      ///< Solid shaded mode
    RENDER = 2      ///< Full render mode
  };
  Q_ENUM(ViewMode)

  /// @}

  //-------------------------------------------------------------------------
  /// @name Stage Operations
  //-------------------------------------------------------------------------
  /// @{

  /// @brief Loads a USD stage from file
  /// @param _stagePath Path to USD file
  void setStage(const std::string& _stagePath);

  /// @brief Saves current stage to disk
  /// @param _savePath Destination file path
  void saveStage(const std::string& _savePath) const;

  /// @brief Gets current USD stage
  /// @return Shared pointer to current stage (may be null)
  pxr::UsdStageRefPtr getStage() { return m_stage; }

  /// @}

  //-------------------------------------------------------------------------
  /// @name Camera Controls
  //-------------------------------------------------------------------------
  /// @{

  /// @brief Sets camera field of view
  /// @param _fov Field of view in degrees (1-179)
  void setFov(float _fov);

  /// @brief Sets camera clipping planes
  /// @param _nearClip Near clipping plane distance (must be > 0)
  /// @param _farClip Far clipping plane distance (must be > nearClip)
  void setClipping(float _nearClip, float _farClip);

  /// @brief Sets background color
  /// @param _r Red component (0-255)
  /// @param _g Green component (0-255)
  /// @param _b Blue component (0-255)
  void setBackgroundColor(int _r, int _g, int _b);

  /// @brief Gets camera view matrix
  /// @return 4x4 view transformation matrix
  [[nodiscard]] pxr::GfMatrix4d getViewMatrix() const;

  /// @brief Gets camera world position
  /// @return 3D camera position vector in world space
  [[nodiscard]] pxr::GfVec3d getCameraPosition() const;

  /// @brief Gets list of cameras in stage
  /// @return Vector of camera prim paths (empty if no cameras found)
  pxr::TfTokenVector getStageCameras() const;

  /// @brief Sets active camera
  /// @param _cameraPath Path to camera prim
  /// @note If camera doesn't exist, falls back to default perspective
  void setActiveCamera(const pxr::TfToken& _cameraPath);

  /// @}

  //-------------------------------------------------------------------------
  /// @name Rendering Controls
  //-------------------------------------------------------------------------
  /// @{

  /// @brief Sets current view mode
  /// @param _mode One of WIREFRAME, SOLID or RENDER
  void setRenderMode(ViewMode _mode);

  /// @brief Shows/hides grid overlay
  /// @param _visible Grid visibility flag
  void setGridVisibility(bool _visible);

  /// @brief Shows/hides gravity wells
  /// @param _visible Wells visibility flag
  void setWellVisibility(bool _visible);

  /// @}

  //-------------------------------------------------------------------------
  /// @name Time Controls
  //-------------------------------------------------------------------------
  /// @{

  /// @brief Sets current time code
  /// @param _value Time in frames
  void setCurrentTime(int _value);

  /// @brief Gets current time code
  /// @return Current time in frames
  double getCurrentTime() const;

  /// @}

  //-------------------------------------------------------------------------
  /// @name Renderer Controls
  //-------------------------------------------------------------------------
  /// @{

  /// @brief Gets current renderer plugin
  /// @return Token identifying current renderer (empty if none active)
  [[nodiscard]] pxr::TfToken getCurrentRendererPlugin() const;

  /// @brief Sets renderer plugin
  /// @param _id Token identifying desired renderer
  void setCurrentRendererPlugin(const pxr::TfToken& _id);

  /// @brief Gets available renderer plugins
  /// @return Vector of available renderer tokens (empty if none found)
  [[nodiscard]] static pxr::TfTokenVector getRendererPlugins();

  /// @brief Gets available AOVs for current renderer
  /// @return Vector of available AOV tokens (empty if none found)
  [[nodiscard]] pxr::TfTokenVector getRendererAOVs() const;

  /// @brief Sets current AOV
  /// @param _id Token identifying desired AOV
  void setCurrentRendererAOV(const pxr::TfToken& _id);

  /// @}

  //-------------------------------------------------------------------------
  /// @name Special Operations
  //-------------------------------------------------------------------------
  /// @{

  /// @brief Creates a gravity well prim in the current stage
  void createGravityWell() const;

  /// @brief Renders current frame to disk
  /// @param _fileName Output file name (extension determines format)
  /// @param _frame Frame number to render
  /// @param _width Output image width (height calculated from aspect ratio)
  /// @return true if render succeeded, false otherwise
  bool renderToDisk(const QString& _fileName, int _frame, int _width) const;

  /// @brief Gets stage meters per unit
  /// @return Meters per unit value (default 1.0)
  double getStageMPU() const;

  /// @brief Gets stage up axis
  /// @return Up axis as string ("X", "Y" or "Z")
  std::string getStageUpAxis() const;

  /// @brief Sets camera combo box reference and populates it
  /// @param _cams Camera selection combo box (must outlive this widget)
  void setCameraComboBox(QComboBox* _cams);

  /// @}

protected:
  //-------------------------------------------------------------------------
  /// @name QOpenGLWidget Overrides
  //-------------------------------------------------------------------------
  /// @{

  /// @brief Initializes OpenGL context and resources
  void initializeGL() override;

  /// @brief Main rendering function
  void paintGL() override;

  /// @brief Handles widget resize events
  /// @param _w New width in pixels
  /// @param _h New height in pixels
  void resizeGL(int _w, int _h) override;

  /// @brief Triggers a paintGL update. Used with a timer to show UsdImagingGLEngine viewport draws.
  void updateGL();

  /// @}

  //-------------------------------------------------------------------------
  /// @name Event Handlers
  //-------------------------------------------------------------------------
  /// @{

  /// @brief Handles mouse press events for camera interaction
  void mousePressEvent(QMouseEvent* _event) override;

  /// @brief Handles mouse move events for camera interaction
  void mouseMoveEvent(QMouseEvent* _event) override;

  /// @brief Handles mouse wheel events for zooming
  void wheelEvent(QWheelEvent* _event) override;

  /// @brief Handles key press events for camera controls
  void keyPressEvent(QKeyEvent* _event) override;

  /// @}

private:
  //-------------------------------------------------------------------------
  /// @name Private Methods
  //-------------------------------------------------------------------------
  /// @{

  /// @brief Adjusts stage metadata for correct display
  /// @note Sets up axis and metersPerUnit if not properly configured
  void adjustMetaData() const;

  /// @brief Initializes custom OpenGL resources (grid, wells)
  /// @throws std::runtime_error if shaders fail to compile
  void setupCustomGL();

  /// @brief Renders custom OpenGL content (grid, wells)
  /// @param _view Current view matrix
  /// @param _proj Current projection matrix
  void paintCustomGL(const pxr::GfMatrix4d& _view, const pxr::GfMatrix4d& _proj);

  /// @brief Draws grid overlay
  /// @param _view Current view matrix
  /// @param _proj Current projection matrix
  void drawGrid(const QMatrix4x4& _view, const QMatrix4x4& _proj);

  /// @brief Draws gravity well indicators
  /// @param _view Current view matrix
  /// @param _proj Current projection matrix
  void drawGravityWell(const QMatrix4x4& _view, const QMatrix4x4& _proj);

  /// @}

  //-------------------------------------------------------------------------
  // USD Members
  //-------------------------------------------------------------------------
  pxr::UsdStageRefPtr m_stage;                  ///< Current USD stage
  mutable pxr::UsdImagingGLEngine* m_engine;    ///< Hydra imaging engine
  pxr::TfToken m_camera = pxr::TfToken("perspective"); ///< Active camera path
  pxr::UsdTimeCode m_time;                      ///< Current time code

  //-------------------------------------------------------------------------
  // Camera State
  //-------------------------------------------------------------------------
  QPoint m_lastMousePosition;           ///< Last mouse position for interaction
  float m_fov = 45.0f;                 ///< Field of view in degrees
  float m_nearClip = 0.001f;           ///< Near clipping plane
  float m_farClip = 1000.0f;           ///< Far clipping plane
  float m_zoomSpeed = 1.0f;            ///< Camera zoom sensitivity
  float m_rotationSpeed = 0.01f;       ///< Camera rotation sensitivity
  float m_panSpeed = 0.01f;            ///< Camera pan sensitivity
  pxr::GfVec3d m_cameraLook = {0.0, 0.0, 0.0};  ///< Camera look-at point
  double m_cameraZoom = 10.0;          ///< Current zoom level
  float m_azimuth = 0.0;               ///< Horizontal rotation angle (radians)
  float m_elevation = 0.0;             ///< Vertical rotation angle (radians)
  double m_sceneScale = 100.0;         ///< Scene scale factor
  pxr::GfVec4f m_backgroundColor = pxr::GfVec4f(0.0f, 0.0f, 0.0f, 1.0f); ///< Background color (RGBA)

  //-------------------------------------------------------------------------
  // View State
  //-------------------------------------------------------------------------
  ViewMode m_viewMode = SOLID;         ///< Current view mode
  bool m_enableGrid = true;            ///< Grid visibility flag
  bool m_showWells = true;             ///< Wells visibility flag

  //-------------------------------------------------------------------------
  // OpenGL Resources
  //-------------------------------------------------------------------------
  QOpenGLVertexArrayObject m_gridVAO;  ///< Grid vertex array
  QOpenGLBuffer m_gridVBO{QOpenGLBuffer::VertexBuffer};  ///< Grid vertex buffer
  QOpenGLVertexArrayObject m_wellVAO;  ///< Well vertex array
  QOpenGLBuffer m_wellVBO{QOpenGLBuffer::VertexBuffer};  ///< Well vertex buffer
  QOpenGLBuffer m_wellEBO{QOpenGLBuffer::IndexBuffer};   ///< Well index buffer
  int m_wellIndexCount = 0;            ///< Number of well indices
  QOpenGLShaderProgram* m_gridShader = nullptr;  ///< Grid shader program
  QOpenGLShaderProgram* m_wellShader = nullptr;  ///< Well shader program

  //-------------------------------------------------------------------------
  // UI Components
  //-------------------------------------------------------------------------
  QComboBox* m_camerasCombo = nullptr; ///< Camera selection combo box
  mutable double m_mpu = 1.0;          ///< Meters per unit
  mutable std::string m_upAxis = "Y";  ///< Scene up axis
};

#endif // HDWIDGET_H_