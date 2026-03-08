#include "waveformwidget.h"
#include <QRandomGenerator>
#include <QPainter>
#include <QtMath>

WaveformWidget::WaveformWidget(QWidget *parent)
    : QWidget(parent)
    , m_animationStep(0)
    , m_isActive(false)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &WaveformWidget::updateAnimation);
    
    // 初始化振幅数组（用于绘制波形）
    m_amplitudes.resize(50);
    generateRandomAmplitudes();
}

void WaveformWidget::startAnimation()
{
    m_isActive = true;
    m_timer->start(50); // 每50ms更新一次
}

void WaveformWidget::stopAnimation()
{
    m_isActive = false;
    m_timer->stop();
    update(); // 清除波形
}

void WaveformWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    if (!m_isActive) {
        // 非活动状态，绘制静态波形
        painter.setPen(QPen(QColor(200, 200, 200), 2));
        int centerY = height() / 2;
        int barWidth = width() / m_amplitudes.size();
        
        for (int i = 0; i < m_amplitudes.size(); ++i) {
            int x = i * barWidth + barWidth / 2;
            int barHeight = 10;
            painter.drawLine(x, centerY - barHeight / 2, x, centerY + barHeight / 2);
        }
        return;
    }
    
    // 活动状态，绘制动态波形
    int centerY = height() / 2;
    int barWidth = width() / m_amplitudes.size();
    int maxHeight = height() * 0.4;
    
    // 生成动态振幅
    for (int i = 0; i < m_amplitudes.size(); ++i) {
        // 使用正弦波和随机值生成动态效果
        double phase = (m_animationStep * 0.1) + (i * 0.3);
        double amplitude = qSin(phase) * 0.5 + 0.5;
        amplitude += QRandomGenerator::global()->bounded(0.3);
        amplitude = qMin(amplitude, 1.0);
        
        int barHeight = static_cast<int>(amplitude * maxHeight);
        int x = i * barWidth + barWidth / 2;
        
        // 绘制波形条
        QColor color(255, 107, 107, 200); // 红色半透明
        painter.setPen(QPen(color, 3));
        painter.drawLine(x, centerY - barHeight / 2, x, centerY + barHeight / 2);
    }
}

void WaveformWidget::updateAnimation()
{
    m_animationStep++;
    update();
}

void WaveformWidget::generateRandomAmplitudes()
{
    for (int i = 0; i < m_amplitudes.size(); ++i) {
        m_amplitudes[i] = QRandomGenerator::global()->bounded(20, 80);
    }
}




