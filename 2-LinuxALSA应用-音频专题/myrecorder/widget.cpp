#include "widget.h"
#include "ui_widget.h"
#include <alsa/asoundlib.h>
#include <QDebug>


Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    m_recordMutex = new QMutex();
    RecordThread::setRecordMutex(m_recordMutex);
}
Widget::~Widget()
{
    delete ui;
}

void Widget::on_btnRecord_clicked()
{
    RecordThread *tmpRecordThread = new RecordThread(this);
    //监听录制线程的recordStarted信号，只有拿到锁的线程才被记录为m_recordThread,以方便停止录制功能控制
    //原来的方案是导致正在录制的线程管理丢失，m_recordThread被非法线程占用的问题
    connect(tmpRecordThread,&RecordThread::recordStarted,this,[this](RecordThread *thread){
         m_recordThread = thread;
    });
   tmpRecordThread->start();
}

void Widget::on_btnStopRecord_clicked()
{
    qDebug() << "停止录制按键被按下";
    m_recordThread->stopRecording();
}

void Widget::on_btnPlay_clicked()
{
    m_playThread = new PlayThread(this);
    m_playThread->startPlaying();
    m_playThread->start();
}

void Widget::on_btnStopPlay_clicked()
{
     qDebug() << "停止播放按键被按下";
    m_playThread->stopPlaying();
}
