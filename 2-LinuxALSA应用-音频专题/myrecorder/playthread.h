#ifndef PLAYTHREAD_H
#define PLAYTHREAD_H

#include <QThread>
#include <alsa/asoundlib.h>
#include <QDebug>

#define PCM_DEVICE "hw:audiocodec"  // 开发板音频设备名
#define SAMPLE_RATE 16000           // 采样率
#define CHANNELS 1                  // 声道数
#define SAMPLE_FORMAT SND_PCM_FORMAT_S16_LE  // 采样格式：16位小端
#define BUFFER_FRAMES 1024          // 每次读取的帧数

class PlayThread : public QThread
{
    Q_OBJECT

public:
    explicit PlayThread(QObject *parent = nullptr);
    PlayThread();

    void run() override;
    void stopPlaying();
    void startPlaying();

private:
    bool stopPlayMark;
};

#endif // PLAYTHREAD_H
