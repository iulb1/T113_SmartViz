#include "waveformwidget.h"

WaveformWidget::WaveformWidget(QWidget *parent) : QWidget(parent)
{
    m_animationData = 0;
    m_active = false;
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout,this,&WaveformWidget::updateUI);
}

void WaveformWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);



    this->resize(parentWidget()->width(),parentWidget()->height());

    int barCount = 50;
    int barWidth = width()/50;
    int barHight = 10;
    int centerY = height() / 2;

    if(!m_active){
        QColor color(200,200,200);
        QPen pen = QPen(color, 3);
        painter.setPen(pen);
        for(int i = 0; i < barCount; ++i){
            int x = i * barWidth + barWidth/2;
            painter.drawLine(x,centerY - barHight /2 ,x, centerY + barHight /2);
        }
    }else{
        painter.setPen(QPen(QColor(255, 107, 107, 200),2));
        for(int i = 0; i < barCount; ++i){
            int x = i * barWidth + barWidth/2;
            //sin(x+y)//x是时间因子，决定波在时间上的位置，y是空间因子，决定波每个点的位置
            double sinValue = qSin(m_animationData*0.1+i*0.3) * 0.5 + 0.5;//-1 1  -0.5 0.5     0 1
            int tmpBarHight = 80 * sinValue;
            painter.drawLine(x,centerY - tmpBarHight /2 ,x, centerY + tmpBarHight /2);
        }
    }
}

void WaveformWidget::startAnimation()
{
    m_active = true;
    m_timer->start(50);
}

void WaveformWidget::stopAnimation()
{
    m_active = false;
    m_timer->stop();
    update();
}

void WaveformWidget::updateUI()
{
    m_animationData++;
    update();
}
