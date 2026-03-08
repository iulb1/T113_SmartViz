#ifndef RECORDTHREAD_H
#define RECORDTHREAD_H
#include <QThread>
#include <alsa/asoundlib.h>
#include <QDebug>
#include <QMutex>
#include <QElapsedTimer>


#define PCM_DEVICE "hw:audiocodec"  // 开发板音频设备名
#define SAMPLE_RATE 16000           // 采样率
#define CHANNELS 1                  // 声道数
#define SAMPLE_FORMAT SND_PCM_FORMAT_S16_LE  // 采样格式：16位小端
#define RECORD_SECONDS 3            // 录制时长（秒）
#define BUFFER_FRAMES 1024          // 每次读取的帧数

class RecordThread : public QThread
{
    Q_OBJECT

public:
    explicit RecordThread(QObject *parent = nullptr);
    RecordThread();

    void run() override;
    void startRecording();
    void stopRecording();

    static void setRecordMutex(QMutex *mutex);

signals:
    void recordStarted(RecordThread *thread);
    void updateTime(int time);

private:
    bool m_stopRecordMark;
    static QMutex *s_recordMutex;
    QElapsedTimer *m_recordTime;
};

#endif // RECORDTHREAD_H
