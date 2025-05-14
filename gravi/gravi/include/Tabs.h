/// @file Tabs.h
/// @brief Tabbed interface components for scene management
///
/// This file contains the tabbed interface widgets for:
/// - Render settings and camera management
/// - Scene outliner and gravity well properties
/// - USD stage hierarchy visualization
///
/// @author Christopher Hosken
/// @version 1.0
/// @date 07/05/2025 Pre-release cleanup
/// @ingroup Gravi

#ifndef TABS_H
#define TABS_H

#include <QListWidget>
#include <QSpinBox>

#include "gravityWell.h"
#include "graviRenderSettings.h"
#include "HdWidget.h"

//-----------------------------------------------------------------------------
/// @class RenderTab
/// @brief Tab panel for render settings and camera management
///
/// Provides UI controls for:
/// - Camera selection and management
/// - Renderer settings configuration
/// - Viewport display options
//-----------------------------------------------------------------------------
class RenderTab : public QWidget
{
public:
  //-------------------------------------------------------------------------
  /// @name Construction / Destruction
  //-------------------------------------------------------------------------
  /// @{

  /// @brief Constructs a render tab panel
  /// @param _hydraWidget Associated Hydra rendering widget (must not be null)
  explicit RenderTab(HdWidget* _hydraWidget);

  /// @brief Default destructor
  ~RenderTab() override = default;

  /// @brief Loads render settings from Hydra widget
  /// @param _hydraWidget Source widget containing render settings
  void loadRenderSettings(HdWidget* _hydraWidget);

  /// @}

private:
  //-------------------------------------------------------------------------
  // Render Settings Controls
  //-------------------------------------------------------------------------
  QSpinBox* m_widthSpin;              ///< Output width control (pixels)
  QSpinBox* m_heightSpin;             ///< Output height control (pixels)
  QSpinBox* m_samplesSpin;            ///< Sample count control
  QSpinBox* m_bouncesSpin;            ///< Light bounce count control
  QSpinBox* m_startFrameSpin;         ///< Animation start frame control
  QSpinBox* m_endFrameSpin;           ///< Animation end frame control
  QDoubleSpinBox* m_lightStepSizeSpin; ///< Primary light step size control
  QDoubleSpinBox* m_minLightStepSizeSpin; ///< Minimum light step size control
  QDoubleSpinBox* m_maxLightStepSizeSpin; ///< Maximum light step size control
  QDoubleSpinBox* m_maxLightDistanceSpin; ///< Maximum light distance control
  QSpinBox* m_maxLightStepsSpin;      ///< Maximum light steps control
  QLineEdit* m_outputPath;            ///< Render output path control

  pxr::GraviRenderSettings m_renderSettings; ///< Current render settings state
};

//-----------------------------------------------------------------------------
/// @class OutlinerTab
/// @brief Tab panel for scene hierarchy and properties
///
/// Provides UI controls for:
/// - USD stage hierarchy visualization
/// - Gravity well management
/// - Prim selection and property editing
//-----------------------------------------------------------------------------
class OutlinerTab : public QWidget
{
public:
  //-------------------------------------------------------------------------
  /// @name Construction / Destruction
  //-------------------------------------------------------------------------
  /// @{

  /// @brief Constructs an outliner tab panel
  /// @param _hydraWidget Associated Hydra rendering widget (must not be null)
  explicit OutlinerTab(HdWidget* _hydraWidget);

  /// @brief Default destructor
  ~OutlinerTab() override = default;

  /// @}

  //-------------------------------------------------------------------------
  /// @name Outliner Operations
  //-------------------------------------------------------------------------
  /// @{

  /// @brief Clears the outliner contents
  /// @note Removes all items from the outliner list
  void clearOutliner();

  /// @brief Updates outliner with stage contents
  /// @param _stage USD stage to visualize (may be null)
  void updateOutliner(const pxr::UsdStagePtr& _stage);

  /// @}

private:
  //-------------------------------------------------------------------------
  // Private Members
  //-------------------------------------------------------------------------
  HdWidget* m_hydraWidget;            ///< Associated rendering widget
  QWidget* m_wellProperties;          ///< Gravity well properties panel
  pxr::GravityWell* m_activeWell;     ///< Currently selected gravity well (may be null)
  QListWidget* m_wellsList;           ///< List of gravity wells
  std::vector<pxr::GravityWell*> m_wells;  ///< Collection of well primitives
  std::string m_upAxis = "Y";         ///< Scene up axis ("X", "Y" or "Z")
  double m_mpu = 1.0;                 ///< Meters per unit value
};

//-----------------------------------------------------------------------------
/// @class Tabs
/// @brief Container for all tabbed interface panels
///
/// Manages the tabbed interface containing:
/// - Render settings tab
/// - Scene outliner tab
/// - Additional management panels
//-----------------------------------------------------------------------------
class Tabs : public QWidget
{
public:
  //-------------------------------------------------------------------------
  /// @name Construction / Destruction
  //-------------------------------------------------------------------------
  /// @{

  /// @brief Constructs the tab container
  /// @param _hydraWidget Associated Hydra rendering widget (must not be null)
  explicit Tabs(HdWidget* _hydraWidget);

  /// @brief Default destructor
  ~Tabs() override = default;

  /// @}

  //-------------------------------------------------------------------------
  // Public Members
  //-------------------------------------------------------------------------
  RenderTab* m_renderTab;    ///< Render settings tab instance (owned)
  OutlinerTab* m_outlinerTab; ///< Scene outliner tab instance (owned)
};

#endif // TABS_H