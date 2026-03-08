#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTimer>
#include "recordthread.h"
#include "playthread.h"
#include "waveformwidget.h"
#include <QDebug>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_btnRecord_clicked();
    void on_btnStopRecord_clicked();
    void on_btnPlay_clicked();
    void on_btnStopPlay_clicked();
    void onRecordFinished(bool success, const QString &message);
    void onPlayFinished(bool success, const QString &message);
    void onRecordProgress(int seconds);
    void onPlayProgress(int currentMs, int totalMs);

private:
    Ui::Widget *ui;
    RecordThread *m_recordThread;
    PlayThread *m_playThread;
    WaveformWidget *m_waveformWidget;
    QTimer *m_timeTimer;
    int m_currentSeconds;
    int m_totalSeconds;
    
    void updateUI(bool recording, bool playing);
    QString formatTime(int seconds);
    void updateTimeDisplay();
};
#endif // WIDGET_H
