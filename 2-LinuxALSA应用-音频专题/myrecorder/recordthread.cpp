#include "recordthread.h"

QMutex *RecordThread::s_recordMutex = nullptr;

RecordThread::RecordThread(QObject *parent)   : QThread(parent)
{
    m_stopRecordMark = false;
}

RecordThread::RecordThread()
{

}

void RecordThread::run()
{
    //线程一启动就尝试获得锁，如果无法获取锁，线程就结束
    if(s_recordMutex){

        if(s_recordMutex->tryLock()){
            //获取到锁，返回true能够跑到这里执行
            qDebug() << "获取到锁，正常进入录制流程";
            emit recordStarted(this);
        }else{
            //无法获取到锁，结束线程
            qDebug() << "无法获取到锁,退出线程";
            return;
        }
    }
    snd_pcm_t *pcm_handle;          // PCM设备句柄
    snd_pcm_hw_params_t *params;    // 硬件参数结构体
    int rc;                         // 返回值
    FILE *output;                   // 输出文件指针
    int16_t *buffer;                // 音频缓冲区


    // 打开输出文件
    output = fopen("record.pcm", "wb");
    if (!output) {
        perror("fopen failed");
    }
    //config the alsa devices
    // 打开PCM捕获设备
    rc = snd_pcm_open(&pcm_handle, PCM_DEVICE, SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0) {
        fprintf(stderr, "无法打开PCM设备: %s\n", snd_strerror(rc));
        fclose(output);
    }

    // 初始化硬件参数结构体
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(pcm_handle, params);

    // 设置访问模式：交错模式（左右声道数据交替存储）
    rc = snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (rc < 0) {
        fprintf(stderr, "设置访问模式失败: %s\n", snd_strerror(rc));
    }
    // 设置采样格式
    rc = snd_pcm_hw_params_set_format(pcm_handle, params, SAMPLE_FORMAT);
    if (rc < 0) {
        fprintf(stderr, "设置采样格式失败: %s\n", snd_strerror(rc));
    }
    // 设置声道数
    rc = snd_pcm_hw_params_set_channels(pcm_handle, params, CHANNELS);
    if (rc < 0) {
        fprintf(stderr, "设置声道数失败: %s\n", snd_strerror(rc));
    }
    // 设置采样率
    rc = snd_pcm_hw_params_set_rate(pcm_handle, params, SAMPLE_RATE, 0);
    if (rc < 0) {
        fprintf(stderr, "设置采样率失败: %s\n", snd_strerror(rc));
    }
    // 应用硬件参数到设备
    rc = snd_pcm_hw_params(pcm_handle, params);
    if (rc < 0) {
        fprintf(stderr, "应用硬件参数失败: %s\n", snd_strerror(rc));
    }
    // 分配音频缓冲区（帧数 × 声道数 × 采样大小）
    buffer = (int16_t *)malloc(BUFFER_FRAMES * CHANNELS * sizeof(int16_t));
    if (!buffer) {
        fprintf(stderr, "内存分配失败\n");
    }

    qDebug() << "开始录制";
    // 循环读取音频数据
    while (!m_stopRecordMark) {
        rc = snd_pcm_readi(pcm_handle, buffer, BUFFER_FRAMES);
        if (rc == -EPIPE) {
            // 缓冲区溢出，重新准备设备
            fprintf(stderr, "缓冲区溢出，重新准备设备...\n");
            snd_pcm_prepare(pcm_handle);
            continue;
        } else if (rc < 0) {
            fprintf(stderr, "读取数据失败: %s\n", snd_strerror(rc));
            break;
        } else if (rc != BUFFER_FRAMES) {
            fprintf(stderr, "短读：预期%d帧，实际读取%d帧\n", BUFFER_FRAMES, rc);
        }

        // 将数据写入文件
        fwrite(buffer, sizeof(int16_t), rc * CHANNELS, output);
    }

    qDebug() << "录制完成";
    // 释放资源
    free(buffer);
    fclose(output);
    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);

    //释放锁
    if(s_recordMutex){
        s_recordMutex->unlock();//释放锁，以便其他线程可以拿到锁
        qDebug() << "线程结束，释放了锁";
    }
}

void RecordThread::startRecording()
{
    m_stopRecordMark = false;
}

void RecordThread::stopRecording()
{
    m_stopRecordMark = true;
}

void RecordThread::setRecordMutex(QMutex *mutex)
{
    s_recordMutex = mutex;
}
