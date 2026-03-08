#ifndef WAVEFORMWIDGET_H
#define WAVEFORMWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QtMath>
#include <QTimer>

class WaveformWidget : public QWidget
{
    Q_OBJECT
public:
    explicit WaveformWidget(QWidget *parent = nullptr);

    void paintEvent(QPaintEvent *event);
    void startAnimation();
    void stopAnimation();
private slots:
    void updateUI();
signals:

private:
    int m_animationData;
    QTimer *m_timer;
    bool m_active;
};

#endif // WAVEFORMWIDGET_H
