#include "recordthread.h"
#include <alsa/asoundlib.h>
#include <errno.h>
#include <string.h>

RecordThread::RecordThread(QObject *parent)
    : QThread(parent), m_stopFlag(false)
{
}

void RecordThread::stopRecording()
{
    m_stopFlag = true;
}

void RecordThread::run()
{
    snd_pcm_t *pcm_handle = nullptr;
    snd_pcm_hw_params_t *params = nullptr;
    int rc;
    FILE *output = nullptr;
    int16_t *buffer = nullptr;
    bool success = false;
    QString errorMsg;

    // 打开输出文件
    output = fopen(RECORD_FILE, "wb");
    if (!output) {
        errorMsg = QString("无法创建录音文件: %1").arg(strerror(errno));
        emit recordFinished(false, errorMsg);
        return;
    }

    // 打开PCM捕获设备
    rc = snd_pcm_open(&pcm_handle, PCM_DEVICE, SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0) {
        errorMsg = QString("无法打开PCM设备: %1").arg(snd_strerror(rc));
        fclose(output);
        emit recordFinished(false, errorMsg);
        return;
    }

    // 初始化硬件参数结构体
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(pcm_handle, params);

    // 设置访问模式
    rc = snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (rc < 0) {
        errorMsg = QString("设置访问模式失败: %1").arg(snd_strerror(rc));
        snd_pcm_close(pcm_handle);
        fclose(output);
        emit recordFinished(false, errorMsg);
        return;
    }

    // 设置采样格式
    rc = snd_pcm_hw_params_set_format(pcm_handle, params, SAMPLE_FORMAT);
    if (rc < 0) {
        errorMsg = QString("设置采样格式失败: %1").arg(snd_strerror(rc));
        snd_pcm_close(pcm_handle);
        fclose(output);
        emit recordFinished(false, errorMsg);
        return;
    }

    // 设置声道数
    rc = snd_pcm_hw_params_set_channels(pcm_handle, params, CHANNELS);
    if (rc < 0) {
        errorMsg = QString("设置声道数失败: %1").arg(snd_strerror(rc));
        snd_pcm_close(pcm_handle);
        fclose(output);
        emit recordFinished(false, errorMsg);
        return;
    }

    // 设置采样率
    rc = snd_pcm_hw_params_set_rate(pcm_handle, params, SAMPLE_RATE, 0);
    if (rc < 0) {
        errorMsg = QString("设置采样率失败: %1").arg(snd_strerror(rc));
        snd_pcm_close(pcm_handle);
        fclose(output);
        emit recordFinished(false, errorMsg);
        return;
    }

    // 应用硬件参数
    rc = snd_pcm_hw_params(pcm_handle, params);
    if (rc < 0) {
        errorMsg = QString("应用硬件参数失败: %1").arg(snd_strerror(rc));
        snd_pcm_close(pcm_handle);
        fclose(output);
        emit recordFinished(false, errorMsg);
        return;
    }

    // 分配音频缓冲区
    buffer = (int16_t *)malloc(BUFFER_FRAMES * CHANNELS * sizeof(int16_t));
    if (!buffer) {
        errorMsg = "内存分配失败";
        snd_pcm_close(pcm_handle);
        fclose(output);
        emit recordFinished(false, errorMsg);
        return;
    }

    // 不限时长录制，直到按下停止按钮
    int totalFramesRead = 0;
    int lastSecond = -1;

    // 无限循环读取音频数据，直到停止标志被设置
    while (!m_stopFlag) {
        rc = snd_pcm_readi(pcm_handle, buffer, BUFFER_FRAMES);
        if (rc == -EPIPE) {
            // 缓冲区溢出，重新准备设备
            snd_pcm_prepare(pcm_handle);
            continue;
        } else if (rc < 0) {
            errorMsg = QString("读取数据失败: %1").arg(snd_strerror(rc));
            break;
        }

        // 将数据写入文件
        fwrite(buffer, sizeof(int16_t), rc * CHANNELS, output);
        
        // 累加读取的帧数
        totalFramesRead += rc;
        
        // 发送进度信号（每秒发送一次）
        int currentSeconds = totalFramesRead / SAMPLE_RATE;
        if (currentSeconds != lastSecond) {
            emit recordProgress(currentSeconds);
            lastSecond = currentSeconds;
        }
    }

    if (!m_stopFlag) {
        success = true;
        errorMsg = QString("录音完成！文件保存为 %1").arg(RECORD_FILE);
    } else {
        errorMsg = "录音已停止";
    }

    if (buffer) free(buffer);
    if (output) fclose(output);
    if (pcm_handle) {
        snd_pcm_drain(pcm_handle);
        snd_pcm_close(pcm_handle);
    }

    emit recordFinished(success, errorMsg);
}


