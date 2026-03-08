#ifndef PLAYTHREAD_H
#define PLAYTHREAD_H

#include <QThread>
#include <QString>
#include <QElapsedTimer>
#include <alsa/asoundlib.h>

// ALSA配置常量
#define PCM_DEVICE "hw:audiocodec"
#define SAMPLE_RATE 16000
#define CHANNELS 1
#define SAMPLE_FORMAT SND_PCM_FORMAT_S16_LE
#define BUFFER_FRAMES 1024

class PlayThread : public QThread
{
    Q_OBJECT
public:
    explicit PlayThread(const QString &filename, QObject *parent = nullptr);
    void stopPlaying();
    
signals:
    void playFinished(bool success, const QString &message);
    void playProgress(int currentMs, int totalMs);  // 播放进度（毫秒）
    
protected:
    void run() override;
    
private:
    QString m_filename;
    bool m_stopFlag;
    qint64 m_fileSize;
};

#endif // PLAYTHREAD_H

