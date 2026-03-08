#include "playthread.h"
#include <alsa/asoundlib.h>
#include <errno.h>
#include <string.h>
#include <QFileInfo>

PlayThread::PlayThread(const QString &filename, QObject *parent)
    : QThread(parent), m_filename(filename), m_stopFlag(false), m_fileSize(0)
{
    // 获取文件大小
    QFileInfo fileInfo(filename);
    m_fileSize = fileInfo.size();
}

void PlayThread::stopPlaying()
{
    m_stopFlag = true;
}

void PlayThread::run()
{
    snd_pcm_t *pcm_handle = nullptr;
    snd_pcm_hw_params_t *params = nullptr;
    int rc;
    FILE *input = nullptr;
    int16_t *buffer = nullptr;
    int read_frames;
    bool success = false;
    QString errorMsg;

    // 打开PCM文件
    input = fopen(m_filename.toLocal8Bit().data(), "rb");
    if (!input) {
        errorMsg = QString("无法打开文件: %1").arg(strerror(errno));
        emit playFinished(false, errorMsg);
        return;
    }

    // 打开PCM播放设备
    rc = snd_pcm_open(&pcm_handle, PCM_DEVICE, SND_PCM_STREAM_PLAYBACK, 0);
    if (rc < 0) {
        errorMsg = QString("无法打开PCM设备: %1").arg(snd_strerror(rc));
        fclose(input);
        emit playFinished(false, errorMsg);
        return;
    }

    // 初始化硬件参数结构体
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(pcm_handle, params);

    // 设置访问模式、采样格式、声道数、采样率
    rc = snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (rc < 0) {
        errorMsg = QString("设置访问模式失败: %1").arg(snd_strerror(rc));
        snd_pcm_close(pcm_handle);
        fclose(input);
        emit playFinished(false, errorMsg);
        return;
    }

    rc = snd_pcm_hw_params_set_format(pcm_handle, params, SAMPLE_FORMAT);
    if (rc < 0) {
        errorMsg = QString("设置采样格式失败: %1").arg(snd_strerror(rc));
        snd_pcm_close(pcm_handle);
        fclose(input);
        emit playFinished(false, errorMsg);
        return;
    }

    rc = snd_pcm_hw_params_set_channels(pcm_handle, params, CHANNELS);
    if (rc < 0) {
        errorMsg = QString("设置声道数失败: %1").arg(snd_strerror(rc));
        snd_pcm_close(pcm_handle);
        fclose(input);
        emit playFinished(false, errorMsg);
        return;
    }

    rc = snd_pcm_hw_params_set_rate(pcm_handle, params, SAMPLE_RATE, 0);
    if (rc < 0) {
        errorMsg = QString("设置采样率失败: %1").arg(snd_strerror(rc));
        snd_pcm_close(pcm_handle);
        fclose(input);
        emit playFinished(false, errorMsg);
        return;
    }

    // 应用硬件参数
    rc = snd_pcm_hw_params(pcm_handle, params);
    if (rc < 0) {
        errorMsg = QString("应用硬件参数失败: %1").arg(snd_strerror(rc));
        snd_pcm_close(pcm_handle);
        fclose(input);
        emit playFinished(false, errorMsg);
        return;
    }

    // 分配缓冲区
    buffer = (int16_t *)malloc(BUFFER_FRAMES * CHANNELS * sizeof(int16_t));
    if (!buffer) {
        errorMsg = "内存分配失败";
        snd_pcm_close(pcm_handle);
        fclose(input);
        emit playFinished(false, errorMsg);
        return;
    }

    // 计算总时长（毫秒）
    int bytesPerSample = sizeof(int16_t);
    int bytesPerFrame = CHANNELS * bytesPerSample;
    int bytesPerSecond = SAMPLE_RATE * bytesPerFrame;
    int totalMs = 0;
    if (m_fileSize > 0 && bytesPerSecond > 0) {
        totalMs = (m_fileSize * 1000LL) / bytesPerSecond;
    }
    
    emit playProgress(0, totalMs);
    
    // 使用写入的帧数来计算进度，考虑播放缓冲区的延迟
    int framesWritten = 0;
    int lastUpdateMs = 0;
    QElapsedTimer playTimer;
    playTimer.start();
    const int bufferDelayMs = 3000;  // 缓冲区延迟约3秒

    // 循环读取文件并播放
    while (!m_stopFlag && (read_frames = fread(buffer, bytesPerFrame, BUFFER_FRAMES, input)) > 0) {
        if (m_stopFlag) {
            if (pcm_handle) {
                snd_pcm_drop(pcm_handle);
            }
            errorMsg = "播放已停止";
            break;
        }
        
        rc = snd_pcm_writei(pcm_handle, buffer, read_frames);
        if (rc == -EPIPE) {
            snd_pcm_prepare(pcm_handle);
            continue;
        } else if (rc < 0) {
            errorMsg = QString("写入数据失败: %1").arg(snd_strerror(rc));
            break;
        }
        
        framesWritten += rc;
        
        // 计算进度：写入帧数对应的时长 - 缓冲区延迟
        int framesMs = (framesWritten * 1000LL) / SAMPLE_RATE;
        int currentMs = qMax(0, qMin(framesMs - bufferDelayMs, totalMs));
        
        // 每200ms更新一次进度
        if (currentMs - lastUpdateMs >= 200 || currentMs == 0) {
            emit playProgress(currentMs, totalMs);
            lastUpdateMs = currentMs;
        }
    }
    
    // 文件读取完成，继续更新进度直到播放完成
    if (!m_stopFlag && pcm_handle) {
        int framesMsAtReadComplete = (framesWritten * 1000LL) / SAMPLE_RATE;
        int readCompleteTime = playTimer.elapsed();
        
        while (!m_stopFlag) {
            // 每20ms检查一次停止标志
            for (int i = 0; i < 10 && !m_stopFlag; ++i) {
                QThread::msleep(20);
            }
            
            if (m_stopFlag) {
                snd_pcm_drop(pcm_handle);
                errorMsg = "播放已停止";
                break;
            }
            
            // 计算当前进度
            int elapsedSinceRead = playTimer.elapsed() - readCompleteTime;
            int currentMs = qMax(0, qMin(framesMsAtReadComplete - bufferDelayMs + elapsedSinceRead, totalMs));
            
            // 每200ms更新一次进度
            if (currentMs - lastUpdateMs >= 200 || currentMs >= totalMs) {
                emit playProgress(currentMs, totalMs);
                lastUpdateMs = currentMs;
                
                if (currentMs >= totalMs) {
                    break;
                }
            }
        }
        
        if (!m_stopFlag) {
            snd_pcm_drain(pcm_handle);
            success = true;
            errorMsg = "播放完成！";
        }
    } else {
        errorMsg = "播放已停止";
    }
    
    if (!m_stopFlag) {
        emit playProgress(totalMs, totalMs);
    }

    // 清理资源
    if (buffer) free(buffer);
    if (input) fclose(input);
    if (pcm_handle) {
        if (!m_stopFlag) {
            snd_pcm_drain(pcm_handle);
        }
        snd_pcm_close(pcm_handle);
    }

    emit playFinished(success, errorMsg);
}


