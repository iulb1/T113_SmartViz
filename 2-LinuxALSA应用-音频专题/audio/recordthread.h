#ifndef RECORDTHREAD_H
#define RECORDTHREAD_H

#include <QThread>
#include <QString>
#include <alsa/asoundlib.h>

// ALSA配置常量
#define PCM_DEVICE "hw:audiocodec"
#define SAMPLE_RATE 16000
#define CHANNELS 1
#define SAMPLE_FORMAT SND_PCM_FORMAT_S16_LE
#define BUFFER_FRAMES 1024
#define RECORD_FILE "record.pcm"

class RecordThread : public QThread
{
    Q_OBJECT
public:
    explicit RecordThread(QObject *parent = nullptr);
    void stopRecording();
    
signals:
    void recordFinished(bool success, const QString &message);
    void recordProgress(int seconds);  // 录制进度（秒）
    
protected:
    void run() override;
    
private:
    bool m_stopFlag;
};

#endif // RECORDTHREAD_H

