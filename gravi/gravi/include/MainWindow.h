/// @file MainWindow.h
/// @brief Main application window containing all UI elements
///
/// This class implements the primary application window that hosts:
/// - Viewport rendering via HdWidget
/// - Menu bars and tool controls
/// - Timeline and playback controls
/// - Camera and renderer selection
/// - Scene management controls
///
/// @author Christopher Hosken
/// @version 1.0
/// @date 07/05/2025 Pre-release cleanup
/// @ingroup Gravi

#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <QMainWindow>
#include <QVBoxLayout>
#include <QStackedLayout>
#include <QPoint>
#include <QCheckBox>

#include "TimelineWidget.h"
#include "HdWidget.h"
#include "Tabs.h"

//-----------------------------------------------------------------------------
/// @class MainWindow
/// @brief Primary application window hosting all UI components
///
/// QMainWindow subclass that serves as the root container for:
/// - USD scene viewport rendering (via HdWidget)
/// - Menu bars and toolbar controls
/// - Timeline and animation controls
/// - Camera and render settings management
/// - Scene navigation and manipulation tools
//-----------------------------------------------------------------------------
class MainWindow final : public QMainWindow
{
  Q_OBJECT

public:
  //-------------------------------------------------------------------------
  /// @name Construction / Destruction
  //-------------------------------------------------------------------------
  /// @{

  /// @brief Constructs the main application window
  /// @param _parent Parent widget (default nullptr)
  explicit MainWindow(QWidget* _parent = nullptr);

  /// @brief Default destructor
  ~MainWindow() override = default;

  /// @}

private:
  //-------------------------------------------------------------------------
  // UI Components
  //-------------------------------------------------------------------------
  QWidget* m_root;                        ///< Root container widget
  QVBoxLayout* m_toolBarLayout;           ///< Main toolbar layout (vertical)
  QVBoxLayout* m_sideBarLayout;           ///< Sidebar control layout (vertical)
  QStackedLayout* m_viewportLayout;       ///< Viewport stack layout
  HdWidget* m_hydraWidget;                ///< Hydra rendering widget
  QPushButton* m_loadButton;              ///< Scene load button
  QMenuBar* m_menuBar;                    ///< Main menu bar
  Tabs* m_sideBar;                        ///< Sidebar tab control
  QMenu* m_fileMenu;                      ///< File operations menu
  QMenu* m_renderMenu;                    ///< Render settings menu
  QMenu* m_aovMenu;                       ///< AOV selection menu
  QWidget* m_viewportOverlayWidget;       ///< Viewport overlay container
  TimelineWidget* m_timelineWidget;       ///< Animation timeline widget

  //-------------------------------------------------------------------------
  // Interaction State
  //-------------------------------------------------------------------------
  QPointF m_dragPosition;                 ///< Window drag position storage (screen coordinates)
  QWidget* m_titleBar;                    ///< Custom title bar widget
  QComboBox* m_aovView;                   ///< AOV selection dropdown

  //-------------------------------------------------------------------------
  // Camera/View Controls
  //-------------------------------------------------------------------------
  QWidget* m_dropdownWidget;              ///< Camera controls container
  QComboBox* m_camerasCombo;              ///< Camera selection dropdown
  QCheckBox* m_showGridCheck;             ///< Grid visibility toggle
  QCheckBox* m_showWellsCheck;            ///< Gravity wells visibility toggle
  QPushButton* m_collapseButton;          ///< Control panel collapse button
  bool m_dropdownExpanded = true;         ///< Panel expansion state flag

  //-------------------------------------------------------------------------
  // Window State
  //-------------------------------------------------------------------------
  bool m_isFullscreen = false;            ///< Fullscreen mode flag
  int m_width = 1280;                     ///< Window width storage (pixels)
  int m_height = 720;                     ///< Window height storage (pixels)

  //-------------------------------------------------------------------------
  /// @name Event Handling
  //-------------------------------------------------------------------------
  /// @{

  /// @brief Handles window show events
  /// @param _event Show event details
  void showEvent(QShowEvent* _event) override;

  /// @brief Handles window resize events
  /// @param _event Resize event details
  void resizeEvent(QResizeEvent* _event) override;

  /// @brief Filters events for custom handling
  /// @param _obj Object generating the event
  /// @param _event Event details
  /// @return true if event was handled, false otherwise
  bool eventFilter(QObject* _obj, QEvent* _event) override;

  /// @}

  //-------------------------------------------------------------------------
  /// @name UI Action Handlers
  //-------------------------------------------------------------------------
  /// @{

  /// @brief Handles scene open action
  /// @note Shows file dialog and loads selected USD stage
  void onOpenAction();

  /// @brief Handles scene save action
  /// @note Saves current USD stage to disk
  void onSaveAction();

  /// @brief Processes AOV selection changes
  /// @param _index Selected AOV index
  void onRendererAOVSelected(int _index) const;

  /// @brief Processes camera selection changes
  /// @param _index Selected camera index
  void onCameraSelected(int _index) const;

  /// @brief Updates AOV menu options from current renderer
  /// @note Populates menu with available AOVs from active renderer
  void updateAOVMenu();

  /// @brief Sets viewport rendering mode
  /// @param _button Source button triggering the change
  /// @note Mode is determined from button's associated data
  void setViewMode(const QAbstractButton* _button) const;

  /// @brief Creates new gravity well prim in current stage
  /// @note Adds gravity well at scene origin by default
  void createGravityWell() const;

  /// @brief Updates camera selection menu from current stage
  /// @note Populates dropdown with all cameras found in stage
  void updateCameraMenu();

  /// @brief Updates gravity wells display state
  /// @note Synchronizes menu state with current visibility setting
  void updateWellsMenu() const;

  /// @}
};

#endif // MAINWINDOW_H_