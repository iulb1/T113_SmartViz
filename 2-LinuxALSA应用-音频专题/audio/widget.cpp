#include "widget.h"
#include "ui_widget.h"
#include <QFileInfo>
#include <QVBoxLayout>

// 录音文件名
#define RECORD_FILE "record.pcm"

// ========== Widget 实现 ==========
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , m_recordThread(nullptr)
    , m_playThread(nullptr)
    , m_waveformWidget(nullptr)
    , m_currentSeconds(0)
    , m_totalSeconds(0)
{
    ui->setupUi(this);
    
    // 创建波形动画widget
    m_waveformWidget = new WaveformWidget(this);
    QVBoxLayout *waveformLayout = new QVBoxLayout(ui->waveformWidget);
    waveformLayout->setContentsMargins(0, 0, 0, 0);
    waveformLayout->addWidget(m_waveformWidget);
    
    // 创建时间更新定时器
    m_timeTimer = new QTimer(this);
    connect(m_timeTimer, &QTimer::timeout, this, &Widget::updateTimeDisplay);
    
    updateUI(false, false);
    ui->labelTime->setText("00:00");
    ui->progressBar->setValue(0);
    
    // 隐藏鼠标光标
    setCursor(Qt::BlankCursor);
    QApplication::setOverrideCursor(Qt::BlankCursor);
}

Widget::~Widget()
{
    // 停止所有线程
    if (m_recordThread && m_recordThread->isRunning()) {
        m_recordThread->stopRecording();
        m_recordThread->wait();
    }
    if (m_playThread && m_playThread->isRunning()) {
        m_playThread->stopPlaying();
        m_playThread->wait();
    }
    
    if (m_timeTimer) {
        m_timeTimer->stop();
    }
    if (m_waveformWidget) {
        m_waveformWidget->stopAnimation();
    }
    
    delete m_recordThread;
    delete m_playThread;
    delete ui;
}

void Widget::on_btnRecord_clicked()
{
    if (m_recordThread && m_recordThread->isRunning()) {
        return;
    }

    // 清理旧的线程对象（如果存在）
    if (m_recordThread) {
        if (m_recordThread->isRunning()) {
            m_recordThread->stopRecording();
            m_recordThread->wait();
        }
        delete m_recordThread;
        m_recordThread = nullptr;
    }

    // 创建并启动录制线程
    m_recordThread = new RecordThread(this);
    connect(m_recordThread, &RecordThread::recordFinished, 
            this, &Widget::onRecordFinished);
    connect(m_recordThread, &RecordThread::recordProgress,
            this, &Widget::onRecordProgress);
    connect(m_recordThread, &RecordThread::finished, 
            this, [this]() {
                if (m_recordThread) {
                    m_recordThread->deleteLater();
                    m_recordThread = nullptr;
                }
            });
    
    // 启动录制效果
    m_currentSeconds = 0;
    m_waveformWidget->startAnimation();
    m_timeTimer->start(100); // 每100ms更新一次时间显示
    
    // 设置进度条为不确定状态（不限时长录制）
    ui->progressBar->setMaximum(0);
    ui->progressBar->setValue(0);
    
    m_recordThread->start();
    updateUI(true, false);
    ui->labelStatus->setText("正在录制...");
    ui->labelTime->setText("00:00");
}

void Widget::on_btnStopRecord_clicked()
{
    if (m_recordThread && m_recordThread->isRunning()) {
        m_recordThread->stopRecording();
        m_waveformWidget->stopAnimation();
        m_timeTimer->stop();
        ui->labelStatus->setText("正在停止录制...");
    }
}

void Widget::on_btnPlay_clicked()
{
    if (m_playThread && m_playThread->isRunning()) {
        return;
    }

    // 检查文件是否存在
    QString recordFile = RECORD_FILE;
    if (!QFileInfo::exists(recordFile)) {
        ui->labelStatus->setText("错误：录音文件不存在，请先录制！");
        return;
    }

    // 清理旧的线程对象（如果存在）
    if (m_playThread) {
        if (m_playThread->isRunning()) {
            m_playThread->stopPlaying();
            m_playThread->wait();
        }
        delete m_playThread;
        m_playThread = nullptr;
    }

    // 创建并启动播放线程
    m_playThread = new PlayThread(recordFile, this);
    connect(m_playThread, &PlayThread::playFinished, 
            this, &Widget::onPlayFinished);
    connect(m_playThread, &PlayThread::playProgress,
            this, &Widget::onPlayProgress);
    connect(m_playThread, &PlayThread::finished, 
            this, [this]() {
                if (m_playThread) {
                    m_playThread->deleteLater();
                    m_playThread = nullptr;
                }
            });
    
    // 初始化播放状态
    m_currentSeconds = 0;
    m_totalSeconds = 0;
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(100);
    ui->labelTime->setText("00:00");
    
    m_playThread->start();
    updateUI(false, true);
    ui->labelStatus->setText("正在播放...");
}

void Widget::on_btnStopPlay_clicked()
{
    if (m_playThread && m_playThread->isRunning()) {
        m_playThread->stopPlaying();
        ui->progressBar->setValue(0);
        ui->labelStatus->setText("正在停止播放...");
    }
}

void Widget::onRecordFinished(bool success, const QString &message)
{
    m_waveformWidget->stopAnimation();
    m_timeTimer->stop();
    updateUI(false, false);
    ui->labelStatus->setText(message);
    
    // 恢复进度条为确定状态
    ui->progressBar->setMaximum(100);
    ui->progressBar->setValue(0);
}

void Widget::onPlayFinished(bool success, const QString &message)
{
    updateUI(false, false);
    ui->labelStatus->setText(message);
    ui->progressBar->setValue(100);
}

void Widget::onRecordProgress(int seconds)
{
    m_currentSeconds = seconds;
    ui->labelTime->setText(formatTime(seconds));
    ui->progressBar->setMaximum(0);  // 不确定状态（显示动画）
}

void Widget::onPlayProgress(int currentMs, int totalMs)
{
    if (totalMs > 0 && currentMs >= 0) {
        int progress = qBound(0, (currentMs * 100) / totalMs, 100);
        ui->progressBar->setValue(progress);
        
        m_currentSeconds = currentMs / 1000;
        m_totalSeconds = totalMs / 1000;
        ui->labelTime->setText(formatTime(m_currentSeconds));
    }
}

void Widget::updateTimeDisplay()
{
    // 录制时实时更新时间显示
    if (m_recordThread && m_recordThread->isRunning()) {
        ui->labelTime->setText(formatTime(m_currentSeconds));
    }
}

QString Widget::formatTime(int seconds)
{
    int minutes = seconds / 60;
    int secs = seconds % 60;
    return QString("%1:%2").arg(minutes, 2, 10, QChar('0'))
                          .arg(secs, 2, 10, QChar('0'));
}

void Widget::updateUI(bool recording, bool playing)
{
    ui->btnRecord->setEnabled(!recording && !playing);
    ui->btnStopRecord->setEnabled(recording);
    ui->btnPlay->setEnabled(!recording && !playing);
    ui->btnStopPlay->setEnabled(playing);
}

