#ifndef WIDGET_H
#define WIDGET_H

#include <string.h>
#include <QWidget>
#include "recordthread.h"
#include "playthread.h"

#include <QMutex>
#include <QElapsedTimer>

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

private:
    Ui::Widget *ui;
    RecordThread *m_recordThread;
    PlayThread *m_playThread;
    QElapsedTimer *time;
    QMutex *m_recordMutex;
    char updateTime[12];
};
#endif // WIDGET_H
