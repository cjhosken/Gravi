///
/// @file Tabs.cpp
/// @brief Tabbed interface components for scene management


#include <QTabWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMouseEvent>
#include <QFormLayout>
#include <QLabel>
#include <QSplitter>
#include <QToolButton>
#include <QDir>
#include <QMessageBox>
#include <QProgressDialog>
#include <QButtonGroup>
#include <QLineEdit>
#include <QTimer>
#include <QThread>
#include <pxr/usd/usd/tokens.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usdGeom/xformCache.h>
#include <pxr/usd/usdRender/settings.h>
#include <pxr/imaging/hd/renderSettings.h>

#include "graviRenderSettings.h"
#include "RenderSettings.h"

#include "Tabs.h"


//-------------------------------------------------------------------------
// Construction / Destruction
//-------------------------------------------------------------------------

RenderTab::RenderTab(HdWidget* _hydraWidget) {
    const auto renderLayout = new QVBoxLayout(this);

    // Render buttons layout (horizontal)
    const auto renderButtonsLayout = new QHBoxLayout();

    // 2. Define or get the render settings prim properly
    pxr::SdfPath renderSettingsPath("/Gravi/RenderSettings");

    // Check if prim already exists
    if (auto existingPrim = _hydraWidget->getStage()->GetPrimAtPath(renderSettingsPath)) {
        m_renderSettings = pxr::GraviRenderSettings(existingPrim);
    } else {
        m_renderSettings = pxr::GraviRenderSettings::Define(_hydraWidget->getStage(), renderSettingsPath);
        auto samples = m_renderSettings.CreateConvergedSamplesPerPixelAttr(pxr::VtValue(pxr::defaultConvergedSamplesPerPixel));

        auto bounces = m_renderSettings.CreateBouncesAttr(pxr::VtValue(pxr::defaultBounces));

        auto frameRange = m_renderSettings.CreateFrameRangeAttr(pxr::VtValue(pxr::GfVec2i(pxr::defaultStartFrame, pxr::defaultEndFrame)));

        auto lightStepSize = m_renderSettings.CreateLightStepSizeAttr(pxr::VtValue(pxr::defaultLightStepSize));

        auto minLightStepSize = m_renderSettings.CreateMinLightStepSizeAttr(pxr::VtValue(pxr::defaultMinLightStepSize));
        auto maxLightStepSize = m_renderSettings.CreateMaxLightStepSizeAttr(pxr::VtValue(pxr::defaultMaxLightStepSize));

        auto maxLightDistance = m_renderSettings.CreateMaxLightDistanceAttr(pxr::VtValue(pxr::defaultMaxLightDistance));
        auto maxLightSteps = m_renderSettings.CreateMaxLightStepsAttr(pxr::VtValue(pxr::defaultMaxLightSteps));
    }

    // Render image button
    const auto renderButton = new QPushButton("Render image", this);
    renderButton->setStyleSheet("QPushButton { background-color: #2A2D32; color: white; padding: 8px; }"
                               "QPushButton:hover { background-color: #3A3D42; }");

    // Render animation button
    auto *renderAnimButton = new QPushButton("Render animation", this);
    renderAnimButton->setStyleSheet("QPushButton { background-color: #2A2D32; color: white; padding: 8px; }"
                                   "QPushButton:hover { background-color: #3A3D42; }");

    renderButtonsLayout->addWidget(renderButton);
    renderButtonsLayout->addWidget(renderAnimButton);

    renderLayout->addLayout(renderButtonsLayout);

    auto *settingsLabel = new QLabel("Render Settings:", this);
    settingsLabel->setStyleSheet("color: white; margin-top: 10px;");
    renderLayout->addWidget(settingsLabel);

    // Width control
    auto *widthLabel = new QLabel("Width:", this);
    widthLabel->setStyleSheet("color: #AAAAAA; margin-top: 5px;");
    renderLayout->addWidget(widthLabel);

    m_widthSpin = new QSpinBox(this);
    m_widthSpin->setRange(1, 16384);
    m_widthSpin->setStyleSheet("background-color: #2A2D32; color: white;");
    renderLayout->addWidget(m_widthSpin);

    // Height control
    auto *heightLabel = new QLabel("Height:", this);
    heightLabel->setStyleSheet("color: #AAAAAA; margin-top: 5px;");
    renderLayout->addWidget(heightLabel);

    m_heightSpin = new QSpinBox(this);
    m_heightSpin->setRange(1, 16384);
    m_heightSpin->setStyleSheet("background-color: #2A2D32; color: white;");
    renderLayout->addWidget(m_heightSpin);

    // Start Frame control
    auto *startFrameLabel = new QLabel("Start Frame:", this);
    startFrameLabel->setStyleSheet("color: #AAAAAA; margin-top: 5px;");
    renderLayout->addWidget(startFrameLabel);

    m_startFrameSpin = new QSpinBox(this);
    m_startFrameSpin->setRange(-100000, 100000);
    m_startFrameSpin->setStyleSheet("background-color: #2A2D32; color: white;");
    renderLayout->addWidget(m_startFrameSpin);

    // End Frame control
    auto *endFrameLabel = new QLabel("End Frame:", this);
    endFrameLabel->setStyleSheet("color: #AAAAAA; margin-top: 5px;");
    renderLayout->addWidget(endFrameLabel);

    m_endFrameSpin = new QSpinBox(this);
    m_endFrameSpin->setRange(-100000, 100000);
    m_endFrameSpin->setStyleSheet("background-color: #2A2D32; color: white;");
    renderLayout->addWidget(m_endFrameSpin);

    // Samples control
    auto *samplesLabel = new QLabel("Samples:", this);
    samplesLabel->setStyleSheet("color: #AAAAAA;");
    renderLayout->addWidget(samplesLabel);

    m_samplesSpin = new QSpinBox(this);
    m_samplesSpin->setRange(1, 100000);
    m_samplesSpin->setStyleSheet("background-color: #2A2D32; color: white;");
    renderLayout->addWidget(m_samplesSpin);

    // Bounces control
    auto *bouncesLabel = new QLabel("Bounces:", this);
    bouncesLabel->setStyleSheet("color: #AAAAAA; margin-top: 5px;");
    renderLayout->addWidget(bouncesLabel);

    m_bouncesSpin = new QSpinBox(this);
    m_bouncesSpin->setRange(1, 100000);
    m_bouncesSpin->setStyleSheet("background-color: #2A2D32; color: white;");
    renderLayout->addWidget(m_bouncesSpin);

    // Bounces control
    auto *lightStepSizeLabel = new QLabel("Light Step Size:", this);
    lightStepSizeLabel->setStyleSheet("color: #AAAAAA; margin-top: 5px;");
    renderLayout->addWidget(lightStepSizeLabel);

    m_lightStepSizeSpin = new QDoubleSpinBox(this);
    m_lightStepSizeSpin->setDecimals(3);
    m_lightStepSizeSpin->setRange(0.001, 10000);
    m_lightStepSizeSpin->setStyleSheet("background-color: #2A2D32; color: white;");
    renderLayout->addWidget(m_lightStepSizeSpin);

    // Bounces control
    auto *minLightStepSizeLabel = new QLabel("Min Light Step Size:", this);
    minLightStepSizeLabel->setStyleSheet("color: #AAAAAA; margin-top: 5px;");
    renderLayout->addWidget(minLightStepSizeLabel);

    m_minLightStepSizeSpin = new QDoubleSpinBox(this);
    m_minLightStepSizeSpin->setDecimals(3);
    m_minLightStepSizeSpin->setRange(0.001, 10000);
    m_minLightStepSizeSpin->setStyleSheet("background-color: #2A2D32; color: white;");
    renderLayout->addWidget(m_minLightStepSizeSpin);

    auto *maxLightStepSizeLabel = new QLabel("Max Light Steps Size:", this);
    maxLightStepSizeLabel->setStyleSheet("color: #AAAAAA; margin-top: 5px;");
    renderLayout->addWidget(maxLightStepSizeLabel);

    m_maxLightStepSizeSpin = new QDoubleSpinBox(this);
    m_maxLightStepSizeSpin->setDecimals(3);
    m_maxLightStepSizeSpin->setRange(0.001, 10000);
    m_maxLightStepSizeSpin->setStyleSheet("background-color: #2A2D32; color: white;");
    renderLayout->addWidget(m_maxLightStepSizeSpin);

    auto *maxLightStepsLabel = new QLabel("Max Light Steps:", this);
    maxLightStepsLabel->setStyleSheet("color: #AAAAAA; margin-top: 5px;");
    renderLayout->addWidget(maxLightStepsLabel);

    m_maxLightStepsSpin = new QSpinBox(this);
    m_maxLightStepsSpin->setRange(1, 1000000);
    m_maxLightStepsSpin->setStyleSheet("background-color: #2A2D32; color: white;");
    renderLayout->addWidget(m_maxLightStepsSpin);

    auto *maxLightDistanceLabel = new QLabel("Max Light Distance:", this);
    maxLightDistanceLabel->setStyleSheet("color: #AAAAAA; margin-top: 5px;");
    renderLayout->addWidget(maxLightDistanceLabel);

    m_maxLightDistanceSpin = new QDoubleSpinBox(this);
    m_maxLightDistanceSpin->setDecimals(3);
    m_maxLightDistanceSpin->setRange(0.001, 100000);
    m_maxLightDistanceSpin->setStyleSheet("background-color: #2A2D32; color: white;");
    renderLayout->addWidget(m_maxLightDistanceSpin);


    auto *outputLabel = new QLabel("Output Path:", this);
    outputLabel->setStyleSheet("color: white; margin-top: 10px;");
    renderLayout->addWidget(outputLabel);

    m_outputPath = new QLineEdit(this);
    m_outputPath->setPlaceholderText("Enter save path...");
    m_outputPath->setStyleSheet("background-color: #2A2D32; color: white;");
    renderLayout->addWidget(m_outputPath);

    renderLayout->addStretch();

    // Now the connection code will work safely
    connect(m_widthSpin, QOverload<int>::of(&QSpinBox::valueChanged), [=](int value) {
        pxr::GfVec2i currentRes;
        m_renderSettings.GetResolutionAttr().Get(&currentRes);
        m_renderSettings.GetResolutionAttr().Set(pxr::GfVec2i(value, currentRes[1]));
    });

    connect(m_heightSpin, QOverload<int>::of(&QSpinBox::valueChanged), [=](int value) {
        pxr::GfVec2i currentRes;
        m_renderSettings.GetResolutionAttr().Get(&currentRes);
        m_renderSettings.GetResolutionAttr().Set(pxr::GfVec2i(currentRes[0], value));
    });

    connect(m_startFrameSpin, QOverload<int>::of(&QSpinBox::valueChanged), [=](int value) {
        pxr::GfVec2i currentFrames;
        m_renderSettings.GetFrameRangeAttr().Get(&currentFrames);
        m_renderSettings.GetFrameRangeAttr().Set(pxr::GfVec2i(value, currentFrames[1]));
    });

    connect(m_endFrameSpin, QOverload<int>::of(&QSpinBox::valueChanged), [=](int value) {
        pxr::GfVec2i currentFrames;
        m_renderSettings.GetFrameRangeAttr().Get(&currentFrames);
        m_renderSettings.GetFrameRangeAttr().Set(pxr::GfVec2i(currentFrames[0], value));
    });

    connect(m_samplesSpin, QOverload<int>::of(&QSpinBox::valueChanged), [=](int value) {
        m_renderSettings.GetConvergedSamplesPerPixelAttr().Set(value);
    });

    connect(m_bouncesSpin, QOverload<int>::of(&QSpinBox::valueChanged), [=](int value) {
        m_renderSettings.GetBouncesAttr().Set(value);
    });

    connect(m_lightStepSizeSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
        m_renderSettings.GetLightStepSizeAttr().Set(value);
    });

    connect(m_minLightStepSizeSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
        m_renderSettings.GetMinLightStepSizeAttr().Set(value);
    });

    connect(m_maxLightStepSizeSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
        m_renderSettings.GetMaxLightStepSizeAttr().Set(value);
    });

    connect(m_maxLightDistanceSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
        m_renderSettings.GetMaxLightDistanceAttr().Set(value);
    });

    connect(m_maxLightStepsSpin, QOverload<int>::of(&QSpinBox::valueChanged), [=](int value) {
        m_renderSettings.GetMaxLightStepsAttr().Set(value);
    });

    connect(m_outputPath, &QLineEdit::textChanged, [=](const QString &text) {
        m_renderSettings.GetOutputPathAttr().Set(text.toStdString());
    });

    // --------------------- //
    // RENDER BUTTON ACTIONS //
    // --------------------- //

    auto render = [=](int _frame, QString _outputPath)
    {

        if (_outputPath.contains('#')) {
            const auto padding = _outputPath.count('#');
            _outputPath.replace(QString(padding, '#'), QString("%1").arg(_frame, static_cast<int>(padding), 10, QChar('0')));
        }

        bool success = _hydraWidget->renderToDisk(_outputPath, _frame, m_widthSpin->value());

        qDebug() << "sucess; " << success;

    };

    connect(renderButton, &QPushButton::clicked, [=]() {

    // Perform render
    const double frame = _hydraWidget->getCurrentTime();

        QMessageBox* popup = new QMessageBox();
    popup->setIcon(QMessageBox::Information);
        popup->setStandardButtons(QMessageBox::NoButton);
    popup->setWindowTitle("Rendering");
    popup->setText("Rendering frame " + QString::number(frame) + "...");
    popup->setModal(false);  // Make it non-modal if you want to interact with other windows
    popup->show();
        QCoreApplication::processEvents();

        QThread::sleep(1);
    render(static_cast<int>(frame), m_outputPath->text());
        popup->close();
        popup->deleteLater();
});

    connect(renderAnimButton, &QPushButton::clicked, [=]() {
        QMessageBox* popup = new QMessageBox();
        popup->setIcon(QMessageBox::Information);
        popup->setStandardButtons(QMessageBox::NoButton);
        popup->setWindowTitle("Rendering");
        popup->setText("Rendering frame " + QString::number(m_startFrameSpin->value()) + "...");
        popup->setModal(false);  // Make it non-modal if you want to interact with other windows
        popup->show();

        QPushButton* cancelButton = popup->addButton(QMessageBox::Cancel);

        QThread::sleep(1);

        bool cancelled = false;
        for (int frame = m_startFrameSpin->value(); frame <= m_endFrameSpin->value(); ++frame) {
            popup->setText("Rendering frame " + QString::number(frame) + "...");
            QCoreApplication::processEvents();

            if (popup->clickedButton() == cancelButton) {
                cancelled = true;
                break;
            }

            render(frame, m_outputPath->text());

            QCoreApplication::processEvents();
            if (popup->clickedButton() == cancelButton) {
                cancelled = true;
                break;
            }
        }
        popup->close();
        popup->deleteLater();
    });

    loadRenderSettings(_hydraWidget);
}

void RenderTab::loadRenderSettings(HdWidget* _hydraWidget) {
    pxr::SdfPath renderSettingsPath("/Gravi/RenderSettings");

    // Check if prim already exists
    if (auto existingPrim = _hydraWidget->getStage()->GetPrimAtPath(renderSettingsPath)) {
        m_renderSettings = pxr::GraviRenderSettings(existingPrim);
    } else {
        m_renderSettings = pxr::GraviRenderSettings::Define(_hydraWidget->getStage(), renderSettingsPath);
        auto samples = m_renderSettings.CreateConvergedSamplesPerPixelAttr(pxr::VtValue(pxr::defaultConvergedSamplesPerPixel));

        auto bounces = m_renderSettings.CreateBouncesAttr(pxr::VtValue(pxr::defaultBounces));

        auto frameRange = m_renderSettings.CreateFrameRangeAttr(pxr::VtValue(pxr::GfVec2i(pxr::defaultStartFrame, pxr::defaultEndFrame)));

        auto lightStepSize = m_renderSettings.CreateLightStepSizeAttr(pxr::VtValue(pxr::defaultLightStepSize));

        auto minLightStepSize = m_renderSettings.CreateMinLightStepSizeAttr(pxr::VtValue(pxr::defaultMinLightStepSize));
        auto maxLightStepSize = m_renderSettings.CreateMaxLightStepSizeAttr(pxr::VtValue(pxr::defaultMaxLightStepSize));

        auto maxLightDistance = m_renderSettings.CreateMaxLightDistanceAttr(pxr::VtValue(pxr::defaultMaxLightDistance));
        auto maxLightSteps = m_renderSettings.CreateMaxLightStepsAttr(pxr::VtValue(pxr::defaultMaxLightSteps));
    }

    if (!m_renderSettings) {
        qWarning() << "Failed to get or create RenderSettings prim";
        return;
    }

    // Block all signals while loading
    bool oldState = this->blockSignals(true);

    // Load resolution
    pxr::GfVec2i resolution(1920, 1080);
    m_renderSettings.GetResolutionAttr().Get(&resolution);
    m_widthSpin->setValue(resolution[0]);
    m_heightSpin->setValue(resolution[1]);

    // Helper lambda to load attributes
    auto loadIntAttribute = [&](const char* attrName, QSpinBox* widget, int defaultValue) {
        int value = defaultValue;
        pxr::UsdAttribute attr = m_renderSettings.GetPrim().GetAttribute(pxr::TfToken(attrName));
        if (attr && attr.Get(&value)) {
            widget->blockSignals(true);
            widget->setValue(value);
            widget->blockSignals(false);
        } else {
            qWarning() << "Failed to load" << attrName << "- using default:" << defaultValue;
            widget->setValue(defaultValue);
        }
    };

    auto loadDoubleAttribute = [&](const char* attrName, QDoubleSpinBox* widget, double defaultValue) {
        double value = defaultValue;
        pxr::UsdAttribute attr = m_renderSettings.GetPrim().GetAttribute(pxr::TfToken(attrName));
        if (attr && attr.Get(&value)) {
            widget->blockSignals(true);
            widget->setValue(value);
            widget->blockSignals(false);
        } else {
            qWarning() << "Failed to load" << attrName << "- using default:" << defaultValue;
            widget->setValue(defaultValue);
        }
    };

    // Load custom attributes
    loadIntAttribute("convergedSamplesPerPixel", m_samplesSpin, pxr::defaultConvergedSamplesPerPixel);
    loadIntAttribute("bounces", m_bouncesSpin, pxr::defaultBounces);
    loadDoubleAttribute("lightStepSize", m_lightStepSizeSpin, pxr::defaultLightStepSize);
    loadDoubleAttribute("minLightStepSize", m_minLightStepSizeSpin, pxr::defaultMinLightStepSize);
    loadDoubleAttribute("maxLightStepSize", m_maxLightStepSizeSpin, pxr::defaultMaxLightStepSize);
    loadDoubleAttribute("maxLightDistance", m_maxLightDistanceSpin, pxr::defaultMaxLightDistance);
    loadIntAttribute("maxLightSteps", m_maxLightStepsSpin, pxr::defaultMaxLightSteps);


    // Load output path
    std::string outputPath = pxr::defaultOutputPath;
    if (m_renderSettings.GetOutputPathAttr().Get(&outputPath)) {
        m_outputPath->setText(QString::fromStdString(outputPath));
    }

    pxr::GfVec2i frames(pxr::defaultStartFrame, pxr::defaultEndFrame);
    m_renderSettings.GetFrameRangeAttr().Get(&frames);
    m_startFrameSpin->setValue(frames[0]);
    m_endFrameSpin->setValue(frames[1]);

    // Restore signal blocking
    this->blockSignals(oldState);

    // Force UI update
    this->update();
}

//-------------------------------------------------------------------------
// Construction / Destruction
//-------------------------------------------------------------------------

OutlinerTab::OutlinerTab(HdWidget* _hydraWidget) {
    auto *wellsLayout = new QVBoxLayout(this);
    auto *wellsSplitter = new QSplitter(Qt::Horizontal, this);

    m_hydraWidget = _hydraWidget;

    m_wellsList = new QListWidget();
    m_wellsList->setStyleSheet("color: white;");
    m_wellsList->setSelectionMode(QAbstractItemView::SingleSelection);

    m_wellProperties = new QWidget();
    m_wellProperties->setHidden(true);

    auto *propertiesLayout = new QVBoxLayout(m_wellProperties);
    auto *propertyLabel = new QLabel("Well Properties");
    auto *formLayout = new QFormLayout();

    auto *forceSpinBox = new QDoubleSpinBox();
    forceSpinBox->setDecimals(3);
    forceSpinBox->setRange(-1000.0, 1000.0);

    connect(forceSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=](const double _value) {
        if (m_activeWell) {
            m_activeWell->GetForceAttr().Set(static_cast<float>(_value));
        }
    });

    auto *xPosSpinBox = new QDoubleSpinBox();
    xPosSpinBox->setDecimals(3);
    xPosSpinBox->setRange(-1000, 1000);

    auto *yPosSpinBox = new QDoubleSpinBox();
    yPosSpinBox->setDecimals(3);
    yPosSpinBox->setRange(-1000, 1000);

    auto *zPosSpinBox = new QDoubleSpinBox();
    zPosSpinBox->setDecimals(3);
    zPosSpinBox->setRange(-1000, 1000);

    auto updatePosition = [=]()
    {
        if (m_activeWell)
        {
            std::string up = m_hydraWidget->getStageUpAxis();
            double mpu = m_hydraWidget->getStageMPU();

            const pxr::GfVec3d position(
                xPosSpinBox->value() / mpu,
                yPosSpinBox->value() / mpu,
                zPosSpinBox->value() / mpu
            );

            pxr::GfMatrix4d xform(1.0); // Identity matrix

            // Apply rotation based on upAxis
            if (up == "X") {
                // Rotate -90 degrees around Y to make X up
                xform = pxr::GfMatrix4d().SetRotate(pxr::GfRotation(pxr::GfVec3d::YAxis(), 90)) * xform;
            } else if (up == "Z") {
                // Rotate 90 degrees around X to make Z up
                xform = pxr::GfMatrix4d().SetRotate(pxr::GfRotation(pxr::GfVec3d::XAxis(), 90)) * xform;
            }

            pxr::GfVec3d transformedPosition = xform.Transform(position);

            pxr::UsdGeomXformable xformable(m_activeWell->GetPrim());
            auto translateOp = xformable.GetXformOp(pxr::UsdGeomXformOp::TypeTranslate);
            if (!translateOp)
            {
                xformable.ClearXformOpOrder();
                translateOp = xformable.AddTranslateOp();
            }
            translateOp.Set(transformedPosition);
        }
    };

    connect(xPosSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, updatePosition);
    connect(yPosSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, updatePosition);
    connect(zPosSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, updatePosition);

    connect(m_wellsList, &QListWidget::currentItemChanged, [=](const QListWidgetItem* _current, QListWidgetItem* _previous)
    {
        if (_current)
        {
            const int index = m_wellsList->row(_current);
            m_activeWell = m_wells.at(index);

            if (m_activeWell)
            {
                m_wellProperties->setVisible(true);

                float force;
                if (m_activeWell->GetForceAttr().Get(&force))
                {
                    forceSpinBox->setValue(force);
                }

                static pxr::UsdGeomXformCache xformCache;
                const pxr::GfMatrix4d transform = xformCache.GetLocalToWorldTransform(m_activeWell->GetPrim());
                pxr::GfVec3d position = transform.ExtractTranslation();
                xPosSpinBox->setValue(position[0]);
                yPosSpinBox->setValue(position[1]);
                zPosSpinBox->setValue(position[2]);
            }
        } else
        {
            m_activeWell = nullptr;
            m_wellProperties->setHidden(true);
        }
    });

    auto *deleteButton = new QPushButton("Delete", this);
    deleteButton->setStyleSheet("QPushButton { background-color: #4A2D32; color: white; padding: 8px; }"
                              "QPushButton:hover { background-color: #5A3D42; }"
                              "QPushButton:disabled { background-color: #2A2D32; color: #555555; }");

    connect(deleteButton, &QPushButton::clicked, [=]()
    {
        QListWidgetItem *currentItem = m_wellsList->currentItem();
        if (!currentItem) return;

        int index = m_wellsList->row(currentItem);
        if (index >= 0 && index < m_wells.size())
        {
            if (m_wells[index])
            {
                if (const pxr::UsdPrim prim = m_wells[index]->GetPrim())
                {
                    prim.GetStage()->RemovePrim(prim.GetPath());
                }
            }

            delete m_wells[index];
            m_wells.erase(m_wells.begin() + index);
            delete m_wellsList->takeItem(index);

            m_wellsList->clearSelection();
            m_activeWell = nullptr;
            m_wellProperties->setVisible(false);
        }
    });

    formLayout->addRow("Force:", forceSpinBox);
    formLayout->addRow("X Position:", xPosSpinBox);
    formLayout->addRow("Y Position:", yPosSpinBox);
    formLayout->addRow("Z Position:", zPosSpinBox);
    formLayout->addRow("", deleteButton);

    propertiesLayout->addWidget(propertyLabel);
    propertiesLayout->addLayout(formLayout);
    propertiesLayout->addStretch();

    wellsSplitter->addWidget(m_wellsList);
    wellsSplitter->addWidget(m_wellProperties);
    wellsSplitter->setStretchFactor(1, 1);

    wellsLayout->addWidget(wellsSplitter);
}

//-------------------------------------------------------------------------
// Outliner Operations
//-------------------------------------------------------------------------

void OutlinerTab::clearOutliner()
{
    m_activeWell = nullptr;
    m_wellsList->clear();
    m_wells.clear();
    m_wellProperties->setVisible(false);
}


void OutlinerTab::updateOutliner(const pxr::UsdStagePtr& _stage)
{
    if (_stage) {
        // We Traverse through the scene looking for Gravity Wells.
        // Any that we find we add to the m_wells and the name to m_wellList
        const pxr::UsdPrimRange range = _stage->Traverse();
        for (const pxr::UsdPrim& prim : range)
        {
            if (prim.GetTypeName() == pxr::TfToken("GravityWell"))
            {
                QString wellName = QString::fromStdString(prim.GetPath().GetString());

                auto* well = new pxr::GravityWell(prim);

                m_wells.push_back(well);
                m_wellsList->addItem(wellName);
            }
        }
    }
}

//-------------------------------------------------------------------------
// Construction / Destruction
//-------------------------------------------------------------------------

Tabs::Tabs(HdWidget* _hydraWidget)
{
    const auto tabWidget = new QTabWidget(this);
    tabWidget->setStyleSheet("QTabWidget::pane { border: 0; }"
                             "QTabBar::tab { background: #1F2124; color: white; padding: 8px; }"
                             "QTabBar::tab:selected { background: #2A2D32; }");

    const auto sideBarLayout = new QVBoxLayout(this);
    sideBarLayout->setContentsMargins(0, 0, 0, 0);
    sideBarLayout->addWidget(tabWidget);

    m_renderTab = new RenderTab(_hydraWidget);
    tabWidget->addTab(m_renderTab, "Render");

    m_outlinerTab = new OutlinerTab(_hydraWidget);
    tabWidget->addTab(m_outlinerTab, "Outliner");
}
