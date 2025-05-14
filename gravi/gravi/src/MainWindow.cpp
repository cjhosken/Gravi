///
/// @file MainWindow.cpp
/// @brief A basic mainwindow class for the applcation

#include <QMenuBar>
#include <QAction>
#include <QFileDialog>
#include <QMouseEvent>
#include <QSplitter>
#include <QDir>
#include <QButtonGroup>
#include <QToolButton>
#include <pxr/usd/usd/tokens.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/primRange.h>

#include "HdWidget.h"
#include "TimelineWidget.h"
#include "Tabs.h"

#include "MainWindow.h"

//-------------------------------------------------------------------------
// Construction / Destruction
//-------------------------------------------------------------------------

MainWindow::MainWindow(QWidget *_parent)
    : QMainWindow(_parent)
    , m_width(1280)
    , m_height(720)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    resize(m_width, m_height);

    // Create central widget - main container
    auto *centralWidget = new QWidget();
    centralWidget->setObjectName("centralWidget");
    setCentralWidget(centralWidget);

    // Main vertical layout (title bar + content)
    auto *windowLayout = new QVBoxLayout(centralWidget);
    windowLayout->setContentsMargins(0, 0, 0, 0);
    windowLayout->setSpacing(0);

    // Custom title bar (draggable)
    m_titleBar = new QWidget();
    m_titleBar->setObjectName("titleBar");
    m_titleBar->setFixedHeight(55);
    m_titleBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // Title bar layout
    auto *titleLayout = new QHBoxLayout(m_titleBar);
    titleLayout->setContentsMargins(10, 0, 10, 0);
    titleLayout->setSpacing(15);

    // Logo button with image icon
    auto *logoButton = new QToolButton();
    logoButton->setObjectName("logoButton");
    logoButton->setIcon(QIcon(":/icons/logo")); // Replace with your actual image path
    logoButton->setIconSize(QSize(24, 24)); // Set appropriate size for your logo
    logoButton->setFixedSize(40, 40); // Square button
    logoButton->setStyleSheet(
        "QToolButton { border: none; background: transparent; }"
        "QToolButton:hover { background-color: #3E4450; border-radius: 4px; }"
        "QToolButton::menu-indicator { image: none; }"  // This hides the arrow
    );

    auto *logoMenu = new QMenu(this);
    logoMenu->setStyleSheet(
        "QMenu {"
        "   background-color: #2D2F33;"
        "   color: white;"
        "   border: 1px solid #444;"
        "}"
        "QMenu::item:selected {"
        "   background-color: #3E4450;"
        "}"

    );

    // Add actions to the menu
    QAction *openAction = logoMenu->addAction("Open");
    QAction *saveAction = logoMenu->addAction("Save");

    connect(openAction, &QAction::triggered, this, &MainWindow::onOpenAction);
    connect(saveAction, &QAction::triggered, this, &MainWindow::onSaveAction);

    logoButton->setMenu(logoMenu);
    logoButton->setPopupMode(QToolButton::InstantPopup);

    auto *titleLabel = new QLabel("Gravi");
    titleLabel->setStyleSheet("color: white; font-weight: bold; font-size: 22px;");

    // Spacer to push buttons to the right
    auto *spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    // Window control buttons - square style
    auto *minButton = new QPushButton();
    minButton->setFixedSize(30, 30);
    minButton->setText("─");
    minButton->setStyleSheet(
        "QPushButton {"
        "   border: none;"
        "   background-color: transparent;"
        "   color: white;"
        "   font-size: 16px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #3E4450;"
        "   border-radius: 4px;"
        "}"
    );

    auto *closeButton = new QPushButton();
    closeButton->setFixedSize(30, 30);
    closeButton->setText("×");
    closeButton->setStyleSheet(
        "QPushButton {"
        "   border: none;"
        "   background-color: transparent;"
        "   color: white;"
        "   font-size: 18px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #e81123;" // Red hover for close button
        "   border-radius: 4px;"
        "}"
    );

    // Connect buttons
    connect(minButton, &QPushButton::clicked, this, &MainWindow::showMinimized);
    connect(closeButton, &QPushButton::clicked, this, &MainWindow::close);

    // Add widgets to title bar
    titleLayout->addWidget(logoButton);
    titleLayout->addWidget(titleLabel);
    titleLayout->addItem(spacer);

    // Add menu bar items here if needed
    // titleLayout->addWidget(m_menuBar);

    titleLayout->addWidget(minButton);
    titleLayout->addWidget(closeButton);

    windowLayout->addWidget(m_titleBar);

    // Main content area (app bar + splitter)
    auto *contentWidget = new QWidget();
    auto *contentLayout = new QHBoxLayout(contentWidget);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    // Central area with splitter (HdWidget + resizable sidebar)
    auto* splitter = new QSplitter(Qt::Horizontal);
    splitter->setHandleWidth(3);
    splitter->setStyleSheet("QSplitter::handle { background-color: #3E4450; }");

    // Fixed-width app bar (50px)
    auto *appBar = new QWidget();
    appBar->setFixedWidth(55);
    appBar->setStyleSheet("background-color: #1F2124;");

    auto*appBarLayout = new QVBoxLayout(appBar);
    appBarLayout->setContentsMargins(0, 10, 0, 10);
    appBarLayout->setAlignment(Qt::AlignHCenter);
    appBarLayout->setSpacing(10);

    // Create the add button
    auto *addButton = new QToolButton(appBar);
    addButton->setText("+");
    addButton->setFixedSize(40, 40);
    addButton->setStyleSheet(R"(
        QToolButton {
            background-color: transparent;
            color: white;
            border: none;
            border-radius: 5px;
            font-size: 20px;
            font-weight: bold;
        }
        QToolButton:hover {
            background-color: #3A3D42;
        }
        QToolButton::menu-indicator { image: none; }
    )");
    addButton->setPopupMode(QToolButton::InstantPopup);

    // Create the dropdown menu
    auto *addMenu = new QMenu(addButton);
    addMenu->setStyleSheet(R"(
        QMenu {
            background-color: #2A2D32;
            border: 1px solid #3A3D42;
            padding: 5px;
        }
        QMenu::item {
            color: white;
            padding: 5px 20px;
        }
        QMenu::item:selected {
            background-color: #3A3D42;
        }
    )");

    // Add menu actions
    QAction *addWellAction = addMenu->addAction("Add Gravity Well");
    connect(addWellAction, &QAction::triggered, this, &MainWindow::createGravityWell);

    // Set the menu to the button
    addButton->setMenu(addMenu);

    // Add button to app bar with spacer
    appBarLayout->addWidget(addButton);
    appBarLayout->addStretch();  // Pushes the button to the top
    splitter->addWidget(appBar);

    // -------
    // VIEWPORT
    // -------

    m_hydraWidget = new HdWidget(contentWidget);
    m_hydraWidget->setBackgroundColor(23, 25, 27);
    splitter->addWidget(m_hydraWidget);

    auto *gridLayout = new QGridLayout(m_hydraWidget);
    gridLayout->setContentsMargins(5, 5, 5, 5);  // Small margin
    gridLayout->setSpacing(5);

    auto *wireframeModeButton = new QPushButton(m_hydraWidget);
    wireframeModeButton->setIcon(QIcon(":/icons/wireframeMode"));
    wireframeModeButton->setIconSize(QSize(24, 24));
    wireframeModeButton->setFixedSize(40, 40);
    wireframeModeButton->setCheckable(true);
    wireframeModeButton->setProperty("viewMode", HdWidget::WIREFRAME);

    auto *solidModeButton = new QPushButton(m_hydraWidget);
    solidModeButton->setIcon(QIcon(":/icons/solidMode"));
    solidModeButton->setIconSize(QSize(24, 24));
    solidModeButton->setFixedSize(40, 40);
    solidModeButton->setCheckable(true);
    solidModeButton->setProperty("viewMode", HdWidget::SOLID);

    solidModeButton->setChecked(true); // Set solid mode to be the default.

    auto *renderModeButton = new QPushButton(m_hydraWidget);
    renderModeButton->setIcon(QIcon(":/icons/renderMode"));
    renderModeButton->setIconSize(QSize(24, 24));
    renderModeButton->setFixedSize(40, 40);
    renderModeButton->setCheckable(true);
    renderModeButton->setProperty("viewMode", HdWidget::RENDER);

    auto *viewModeGroup = new QButtonGroup(m_hydraWidget);
    viewModeGroup->addButton(wireframeModeButton);
    viewModeGroup->addButton(solidModeButton);
    viewModeGroup->addButton(renderModeButton);
    viewModeGroup->setExclusive(true);

    QString activeButtonStyle = R"(
        QPushButton {
            border: none;
            background: transparent;
        }
        QPushButton:checked {
            background-color: rgba(255, 255, 255, 50); /* Slightly highlighted */
            border-radius: 5px;
        }
    )";

    wireframeModeButton->setStyleSheet(activeButtonStyle);
    solidModeButton->setStyleSheet(activeButtonStyle);
    renderModeButton->setStyleSheet(activeButtonStyle);

    connect(viewModeGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this, &MainWindow::setViewMode);

    // -------
    // SETTINGS
    // -------

    auto *settingsButton = new QPushButton(m_hydraWidget);
    settingsButton->setIcon(QIcon(":/icons/view"));
    settingsButton->setIconSize(QSize(24, 24));
    settingsButton->setFixedSize(40, 40);
    settingsButton->setStyleSheet("QPushButton {border: none; background: transparent; color: white;}");

    m_dropdownWidget = new QWidget(m_hydraWidget);
    m_dropdownWidget->setWindowFlags(Qt::Popup);
    m_dropdownWidget->setFixedWidth(200);
    m_dropdownWidget->setStyleSheet("QWidget { background: #333; border: 1px solid #555; }");
    m_dropdownWidget->hide();
    m_dropdownExpanded = true;

    auto *dropdownLayout = new QVBoxLayout(m_dropdownWidget);
    dropdownLayout->setContentsMargins(10, 10, 10, 10);
    dropdownLayout->setSpacing(8);

    auto *camerasLabel = new QLabel("Cameras:", m_dropdownWidget);
    camerasLabel->setStyleSheet("QLabel { color: white; border: none; }");
    dropdownLayout->addWidget(camerasLabel);

    // Add cameras combo box
    m_camerasCombo = new QComboBox(m_dropdownWidget);
    m_camerasCombo->addItems({"Camera 1", "Camera 2", "Camera 3"});
    m_camerasCombo->setStyleSheet(R"(
        QComboBox {
            background-color: #444;
            color: white;
            border: none;
            padding: 5px;
        }
        QComboBox QAbstractItemView {
            background-color: #444;
            color: white;
            border: none;
            selection-background-color: #555;
            selection-color: white;
        }
        QComboBox::drop-down {
            border: none;
            background: transparent;
        }
    )");
    m_camerasCombo->view()->parentWidget()->setStyleSheet("background-color: #2A2C30;");
    dropdownLayout->addWidget(m_camerasCombo);

    // Add checkboxes
    m_showGridCheck = new QCheckBox("Show Grid", m_dropdownWidget);
    m_showGridCheck->setStyleSheet(R"(
    QCheckBox {
        color: white;
        border: none;
    }
)");
    m_showGridCheck->setChecked(true);
    dropdownLayout->addWidget(m_showGridCheck);

    m_showWellsCheck = new QCheckBox("Show Wells", m_dropdownWidget);
    m_showWellsCheck->setStyleSheet(R"(
    QCheckBox {
        color: white;
        border: none;
    }
)");
    m_showWellsCheck->setChecked(true);
    dropdownLayout->addWidget(m_showWellsCheck);

    // Connect signals
    connect(settingsButton, &QPushButton::clicked, this, [this, settingsButton]() {
        if (m_dropdownWidget->isHidden()) {
            // Show dropdown below the button

            QPoint pos = settingsButton->mapToGlobal(QPoint(0, settingsButton->height()));
            m_dropdownWidget->move(pos);
            m_dropdownWidget->show();
        } else {
            m_dropdownWidget->hide();
        }
    });

    connect(m_showGridCheck, &QCheckBox::toggled, this, [this](bool checked) {
        m_hydraWidget->setGridVisibility(checked);
    });

    connect(m_showWellsCheck, &QCheckBox::toggled, this, [this](bool checked) {
        m_hydraWidget->setWellVisibility(checked);
    });

    m_aovView = new QComboBox(m_hydraWidget);
    m_aovView->setFixedSize(120, 30);  // Adjust size as needed
    m_aovView->setStyleSheet(R"(
    QComboBox {
        background-color: #2A2C30;
        color: white;
        border: 1px solid transparent;
        border-radius: 5px;
        padding: 5px;
    }
    QComboBox QFrame{
       background: transparent;
    }

    QComboBox::drop-down {
        border: none;
    }
    QComboBox QAbstractItemView {
        background-color: #2A2C30;
        border: none;
        outline: 0px;
        padding: 0px;
        margin: 0;
    }
    QScrollBar:vertical {
        background: #2A2C30;
        width: 8px;
        margin: 0px;
    }
    QScrollBar::handle:vertical {
        background: #555;
        border-radius: 4px;
    }
    QScrollBar::add-line:vertical,
    QScrollBar::sub-line:vertical {
        background: none;
        height: 0px;
    }
)");
    m_aovView->view()->parentWidget()->setStyleSheet("background-color: #2A2C30;");

    m_timelineWidget = new TimelineWidget(this);
    m_timelineWidget->setFrameRange(0, 240); // Example 10 sec at 24fps

    connect(m_timelineWidget, &TimelineWidget::currentFrameChanged,this, [this](int frame) { m_hydraWidget->setCurrentTime(frame); });

    // Add to layout
    gridLayout->addWidget(m_timelineWidget, 1, 0, 1, 8);

    // Add buttons to grid layout
    // Row 0: Top buttons
    gridLayout->addWidget(m_timelineWidget, 1, 0, 1, 8);
    gridLayout->addWidget(wireframeModeButton, 0, 2, Qt::AlignCenter | Qt::AlignTop);  // Center column
    gridLayout->addWidget(solidModeButton, 0, 3, Qt::AlignCenter | Qt::AlignTop);      // Center column
    gridLayout->addWidget(renderModeButton, 0, 4, Qt::AlignCenter | Qt::AlignTop);     // Center column
    gridLayout->addWidget(m_aovView, 0, 6, Qt::AlignRight | Qt::AlignTop);  // Next to settings button

    gridLayout->addWidget(settingsButton, 0, 7, Qt::AlignRight | Qt::AlignTop);  // Top-right

    // Configure row and column stretching.

    gridLayout->setRowStretch(0, 1);
    gridLayout->setRowStretch(1, 0);

    gridLayout->setColumnStretch(0, 1);
    gridLayout->setColumnStretch(1, 1);
    gridLayout->setColumnStretch(2, 0);
    gridLayout->setColumnStretch(3, 0);
    gridLayout->setColumnStretch(4, 0);
    gridLayout->setColumnStretch(5, 1);
    gridLayout->setColumnStretch(6, 0);
    gridLayout->setColumnStretch(7, 0);

    // -------
    // SIDEBAR
    // -------

    m_sideBar = new Tabs(m_hydraWidget);
    m_sideBar->setGeometry(QRect(0, 0, 215, m_height));
    m_sideBar->setMinimumWidth(215);
    m_sideBar->setStyleSheet("background-color: #1F2124;");
    splitter->addWidget(m_sideBar);

    // Add tabs to tab widget

    // Set stretch factors (HdWidget gets priority)
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setStretchFactor(2, 0);

    contentLayout->addWidget(splitter, 1); // Takes remaining space
    windowLayout->addWidget(contentWidget, 1); // Takes remaining space below title bar

    // Make title bar draggable
    m_titleBar->installEventFilter(this);

    // Style sheet
    setStyleSheet(R"(
        #centralWidget { background-color: #252525; }
    #titleBar { background-color: #222428; }

    /* Wells tab styling */
    #wellsTab, #wellsSplitter, #wellsList, #wellProperties {
        background-color: #252525;
    }

    #wellsList {
        color: white;
        background-color: #333333;
        border: 1px solid #444444;
    }

    #propertyLabel, QLabel {
        color: white;
    }

    QDoubleSpinBox, QDoubleSpinBox QAbstractItemView {
        color: white;
        background-color: #333333;
        border: 1px solid #444444;
    }

    QDoubleSpinBox::up-button, QDoubleSpinBox::down-button {
        background-color: #444444;
        border: 1px solid #555555;
    }
)");

    m_hydraWidget->setCameraComboBox(m_camerasCombo);
}

//-------------------------------------------------------------------------
// Event Handling
//-------------------------------------------------------------------------

void MainWindow::showEvent(QShowEvent *_event)
{
    QMainWindow::showEvent(_event);

    // We call this after initializing the Hydra Widget OpenGL Context.
    pxr::TfTokenVector plugins = HdWidget::getRendererPlugins();
    updateAOVMenu();
    updateCameraMenu();
}

void MainWindow::resizeEvent(QResizeEvent *event) { QMainWindow::resizeEvent(event); }

bool MainWindow::eventFilter(QObject *_obj, QEvent *_event)
{
    // Since we've removed the default application bar, we have to manually consider moving the application.
    if (_obj == m_titleBar)
    {
        const auto *mouseEvent = dynamic_cast<QMouseEvent*>(_event);

        if (_event->type() == QEvent::MouseButtonPress)
        {
            if (mouseEvent->button() == Qt::LeftButton)
            {
                m_dragPosition = mouseEvent->globalPosition() - frameGeometry().topLeft();
                return true;
            }
        } else if (_event->type() == QEvent::MouseMove)
        {
            if (mouseEvent->buttons() & Qt::LeftButton && !m_dragPosition.isNull())
            {
                const auto pos = mouseEvent->globalPosition() - m_dragPosition;
                move(static_cast<int>(pos.x()), static_cast<int>(pos.y()));
                return true;
            }
        }
    }

    return QMainWindow::eventFilter(_obj, _event);
}

//-------------------------------------------------------------------------
// UI Action Handlers
//-------------------------------------------------------------------------

void MainWindow::onOpenAction()
{
    QString assetsPath = QDir::homePath();
    const std::string fileName = QFileDialog::getOpenFileName(this, "Open USD File", assetsPath, "USD Files (*.usd *.usda *.usdc *.usdz)").toStdString();
    if (!fileName.empty())
    {
        m_hydraWidget->setStage(fileName);
        updateCameraMenu();
        updateWellsMenu();
        m_sideBar->m_renderTab->loadRenderSettings(m_hydraWidget);
    }
}

void MainWindow::onSaveAction() {
    QString assetsPath = QDir::homePath();
    const std::string fileName = QFileDialog::getSaveFileName(this, "Save USD File", assetsPath, "USD Files (*.usd *.usda *.usdc *.usdz)").toStdString();

    if (!fileName.empty())
    {
        m_hydraWidget->saveStage(fileName);
    }
}

void MainWindow::onRendererAOVSelected(int _index) const { m_hydraWidget->setCurrentRendererAOV(pxr::TfToken(m_aovView->currentText().toStdString())); }

void MainWindow::onCameraSelected(int _index) const { m_hydraWidget->setActiveCamera(pxr::TfToken(m_camerasCombo->currentText().toStdString())); }

void MainWindow::updateAOVMenu()
{
    const pxr::TfTokenVector aovs = m_hydraWidget->getRendererAOVs();
    m_aovView->clear();

    for (const auto& aov : aovs)
    {
        m_aovView->addItem(QString::fromStdString(aov.GetString()));
    }

    m_aovView->setCurrentIndex(0);

    connect(m_aovView, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onRendererAOVSelected);
}

void MainWindow::setViewMode(const QAbstractButton* _button) const
{
    if (!_button || !m_hydraWidget) return;

    bool ok = false;
    int modeValue = _button->property("viewMode").toInt(&ok);

    if (ok)
    {
        const auto mode = static_cast<HdWidget::ViewMode>(modeValue);
        m_hydraWidget->setRenderMode(mode);
    }
}

void MainWindow::createGravityWell() const { m_hydraWidget->createGravityWell(); updateWellsMenu(); }

void MainWindow::updateCameraMenu()
{
    m_camerasCombo->clear();
    m_camerasCombo->addItem("perspective");;
    // Populate the combo box
    for (const pxr::TfToken& camera : m_hydraWidget->getStageCameras())
    {
        m_camerasCombo->addItem(QString::fromStdString(camera.GetString()));
    }
    m_camerasCombo->setCurrentIndex(0);
    connect(m_camerasCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onCameraSelected);
}

void MainWindow::updateWellsMenu() const
{
    m_sideBar->m_outlinerTab->clearOutliner();
    pxr::UsdStageRefPtr stage = m_hydraWidget->getStage();
    m_sideBar->m_outlinerTab->updateOutliner(stage);
}



