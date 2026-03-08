#ifndef WAVEFORMWIDGET_H
#define WAVEFORMWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QVector>

class WaveformWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WaveformWidget(QWidget *parent = nullptr);
    void startAnimation();
    void stopAnimation();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void updateAnimation();

private:
    QTimer *m_timer;
    QVector<int> m_amplitudes;
    int m_animationStep;
    bool m_isActive;
    
    void generateRandomAmplitudes();
};

#endif // WAVEFORMWIDGET_H




