///
/// @file TimelineWidget.cpp
/// @brief Timeline control widget for animation playback

#include <QHBoxLayout>

#include "TimelineWidget.h"

//-------------------------------------------------------------------------
// Construction / Initialization
//-------------------------------------------------------------------------

TimelineWidget::TimelineWidget(QWidget *_parent)
    : QWidget(_parent) {
    setObjectName("TimelineWidget");
    setupUI();

    // Default frame range
    setFrameRange(m_minFrame, m_maxFrame);

    // Timer setup
    m_animationTimer.setInterval(static_cast<int>(1000/m_fps));
    connect(&m_animationTimer, &QTimer::timeout,
            this, &TimelineWidget::handleTimerTimeout);
}

//-------------------------------------------------------------------------
// Timeline Control
//-------------------------------------------------------------------------

int TimelineWidget::currentFrame() const {
    return m_timeSlider->value();
}

int TimelineWidget::frameRange() const {
    return m_maxFrame - m_minFrame;
}

void TimelineWidget::setFrameRange(int minFrame, int maxFrame) {
    m_minFrame = minFrame;
    m_maxFrame = maxFrame;
    m_timeSlider->setRange(minFrame, maxFrame);
    m_startFrameInput->setValue(minFrame);
    m_endFrameInput->setValue(maxFrame);
    updateFrameLabel();
}

void TimelineWidget::setFps(const double _fps)
{
    m_fps = _fps;
    m_animationTimer.setInterval(static_cast<int>(1000/m_fps));
}

void TimelineWidget::setCurrentFrame(int _frame)
{
    _frame = qBound(m_minFrame, _frame, m_maxFrame);
    m_timeSlider->setValue(_frame);
    Q_EMIT currentFrameChanged(_frame);  // Emit signal when frame changes
}

void TimelineWidget::togglePlayback(const bool _play)
{
    if (_play) {
        m_animationTimer.start();
        m_playButton->setText("▐▐");
    } else {
        m_animationTimer.stop();
        m_playButton->setText("▶");
    }
}

//-------------------------------------------------------------------------
// Public Slots
//-------------------------------------------------------------------------

void TimelineWidget::handleSliderChanged(const int _value)
{
    updateFrameLabel();
    Q_EMIT currentFrameChanged(_value);  // Also emit when slider changes
}

void TimelineWidget::handleTimerTimeout()
{
    int nextFrame = currentFrame() + 1;
    if (nextFrame > m_maxFrame) {
        nextFrame = m_minFrame;
    }
    setCurrentFrame(nextFrame);
}

void TimelineWidget::updateRangeFromInputs()
{
    int newStart = m_startFrameInput->value();
    int newEnd = m_endFrameInput->value();

    if (newStart >= newEnd) {
        // Keep values valid
        m_startFrameInput->setValue(m_minFrame);
        m_endFrameInput->setValue(m_maxFrame);
        return;
    }

    setFrameRange(newStart, newEnd);
}

//-------------------------------------------------------------------------
// Private Methods
//-------------------------------------------------------------------------

void TimelineWidget::setupUI()
{
    // Main layout
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 5, 10, 5);

    // Start frame input
    m_startFrameInput = new QSpinBox(this);
    m_startFrameInput->setRange(-10000, 10000);
    m_startFrameInput->setValue(m_minFrame);
    m_startFrameInput->setFixedWidth(60);

    // End frame input
    m_endFrameInput = new QSpinBox(this);
    m_endFrameInput->setRange(-10000, 10000);
    m_endFrameInput->setValue(m_maxFrame);
    m_endFrameInput->setFixedWidth(60);

    // Navigation buttons
    m_prevButton = new QPushButton("<", this);
    m_prevButton->setFixedSize(30, 25);

    m_nextButton = new QPushButton(">", this);
    m_nextButton->setFixedSize(30, 25);

    // Frame label
    m_frameLabel = new QLabel("0", this);
    m_frameLabel->setAlignment(Qt::AlignCenter);
    m_frameLabel->setMinimumWidth(50);

    // Time slider
    m_timeSlider = new QSlider(Qt::Horizontal, this);
    m_timeSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // Play button
    m_playButton = new QPushButton("▶", this);
    m_playButton->setCheckable(true);
    m_playButton->setFixedSize(60, 25);

    // Assemble UI
    layout->addWidget(m_playButton);
    layout->addWidget(m_startFrameInput);
    layout->addWidget(m_prevButton);
    layout->addWidget(m_frameLabel);
    layout->addWidget(m_timeSlider);
    layout->addWidget(m_nextButton);
    layout->addWidget(m_endFrameInput);

    // Connections
    connect(m_timeSlider, &QSlider::valueChanged,
            this, &TimelineWidget::handleSliderChanged);

    connect(m_playButton, &QPushButton::toggled,
            this, &TimelineWidget::togglePlayback);

    connect(m_prevButton, &QPushButton::clicked, [this]() {
        setCurrentFrame(currentFrame() - 1);
    });

    connect(m_nextButton, &QPushButton::clicked, [this]() {
        setCurrentFrame(currentFrame() + 1);
    });

    // Connect range inputs
    connect(m_startFrameInput, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &TimelineWidget::updateRangeFromInputs);
    connect(m_endFrameInput, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &TimelineWidget::updateRangeFromInputs);

    // Style
    setStyleSheet(R"(
    TimelineWidget {
        border-radius: 8px;
        background-color: #222;
        padding: 8px;
    }

    QLabel {
        color: white;
    }

    QPushButton {
        background-color: #333;
        color: white;
        border: 1px solid #555;
        border-radius: 4px;
        padding: 4px;
    }

    QPushButton:hover {
        background-color: #444;
    }

    QPushButton:pressed {
        background-color: #555;
    }

    QPushButton:checked {
        background-color: #2d88ff;
        color: white;
        border: 1px solid #2d88ff;
    }

    QSpinBox {
        background-color: #333;
        color: white;
        border: 1px solid #555;
        border-radius: 4px;
        padding: 2px 4px;
    }

    QSpinBox::up-button,
    QSpinBox::down-button {
        background: transparent;
        subcontrol-origin: border;
        width: 10px;
        border: none;
    }

    QSlider::groove:horizontal {
        background: #444;
        height: 4px;
        border-radius: 2px;
    }

    QSlider::handle:horizontal {
        background: white;
        width: 12px;
        margin: -6px 0;
        border-radius: 6px;
    }
)");
}

void TimelineWidget::updateFrameLabel() const
{
    m_frameLabel->setText(QString("%1/%2")
                         .arg(currentFrame())
                         .arg(m_maxFrame));
}