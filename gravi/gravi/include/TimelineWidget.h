/// @file TimelineWidget.h
/// @brief Timeline control widget for animation playback
///
/// This class provides a complete timeline interface with:
/// - Frame scrubbing controls
/// - Playback management
/// - Frame range configuration
/// - Time display
///
/// Features include:
/// - Play/pause functionality
/// - Frame-by-frame navigation
/// - Adjustable frame rate
/// - Customizable frame range
///
/// @author Christopher Hosken
/// @version 1.0
/// @date 07/05/2025 Pre-release cleanup
/// @ingroup Gravi

#ifndef TIMELINE_WIDGET_H_
#define TIMELINE_WIDGET_H_

#include <QSlider>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QSpinBox>

//-----------------------------------------------------------------------------
/// @class TimelineWidget
/// @brief Interactive timeline for animation control
///
/// Provides a Qt widget for managing animation playback with:
/// - Visual timeline scrubber
/// - Play/pause controls
/// - Frame navigation buttons
/// - Configurable frame range
/// - Real-time frame display
//-----------------------------------------------------------------------------
class TimelineWidget final : public QWidget
{
  Q_OBJECT

public:
  //-------------------------------------------------------------------------
  /// @name Construction / Initialization
  //-------------------------------------------------------------------------
  /// @{

  /// @brief Constructs a timeline widget
  /// @param _parent Parent widget (default nullptr)
  explicit TimelineWidget(QWidget* _parent = nullptr);

  /// @}

  //-------------------------------------------------------------------------
  /// @name Timeline Control
  //-------------------------------------------------------------------------
  /// @{

  /// @brief Gets current frame number
  /// @return Current frame index (between minFrame and maxFrame)
  [[nodiscard]] int currentFrame() const;

  /// @brief Gets the current frame range
  /// @return Number of frames in timeline (maxFrame - minFrame)
  [[nodiscard]] int frameRange() const;

  /// @brief Sets the frame range
  /// @param _minFrame Starting frame number (inclusive)
  /// @param _maxFrame Ending frame number (inclusive)
  /// @note Will clamp current frame if outside new range
  void setFrameRange(int _minFrame, int _maxFrame);

  /// @brief Sets playback frames per second
  /// @param _fps Frames per second value (must be > 0)
  /// @note Affects playback speed when animation is active
  void setFps(double _fps);

  /// @brief Sets the current frame
  /// @param _frame Frame number to set (will be clamped to valid range)
  /// @note Emits currentFrameChanged signal if frame changes
  void setCurrentFrame(int _frame);

  /// @brief Toggles playback state
  /// @param _play True to start playback, false to pause
  /// @note Starts/stops the internal animation timer
  void togglePlayback(bool _play);

  /// @}

Q_SIGNALS:
  //-------------------------------------------------------------------------
  /// @name Signals
  //-------------------------------------------------------------------------
  /// @{

  /// @brief Signal emitted when frame changes. Definition can be found src/MainWindow.cpp.
  /// @param _frame New frame number (guaranteed to be within current range)
  void currentFrameChanged(int _frame);

  /// @}

public Q_SLOTS:
  //-------------------------------------------------------------------------
  /// @name Public Slots
  //-------------------------------------------------------------------------
  /// @{

  /// @brief Handles slider position changes
  /// @param _value New slider position (converted to frame number)
  /// @note Updates current frame and display
  void handleSliderChanged(int _value);

  /// @brief Handles animation timer updates
  /// @note Advances frame during playback, wraps at end of range
  void handleTimerTimeout();

  /// @brief Updates frame range from input fields
  /// @note Reads values from startFrameInput and endFrameInput
  void updateRangeFromInputs();

  /// @}

private:
  //-------------------------------------------------------------------------
  /// @name Private Methods
  //-------------------------------------------------------------------------
  /// @{

  /// @brief Initializes UI components
  /// @note Creates and arranges all child widgets
  void setupUI();

  /// @brief Updates frame number display
  /// @note Synchronizes frameLabel with current frame
  void updateFrameLabel() const;

  /// @}

  //-------------------------------------------------------------------------
  // UI Components
  //-------------------------------------------------------------------------
  QSlider* m_timeSlider;         ///< Main timeline slider (horizontal)
  QPushButton* m_playButton;     ///< Play/pause toggle button
  QPushButton* m_prevButton;     ///< Previous frame button (single step)
  QPushButton* m_nextButton;     ///< Next frame button (single step)
  QLabel* m_frameLabel;          ///< Current frame display (text)
  QSpinBox* m_startFrameInput;   ///< Start frame input (spin box)
  QSpinBox* m_endFrameInput;     ///< End frame input (spin box)
  QTimer m_animationTimer;       ///< Playback timer (QTimer)

  //-------------------------------------------------------------------------
  // Timeline State
  //-------------------------------------------------------------------------
  int m_minFrame = 0;            ///< Minimum frame number (inclusive)
  int m_maxFrame = 100;          ///< Maximum frame number (inclusive)
  double m_fps = 24.0;           ///< Frames per second (default 24)
};

#endif // TIMELINE_WIDGET_H_