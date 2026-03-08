/**
 * 音频播放模块实现
 */

#include "audio_playback.h"
#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <unistd.h>
#include <errno.h>

// 根据位深度获取ALSA格式
static snd_pcm_format_t get_alsa_format(unsigned int bit_depth) {
    switch (bit_depth) {
        case 16:
            return SND_PCM_FORMAT_S16_LE;
        case 24:
            return SND_PCM_FORMAT_S24_LE;
        case 32:
            return SND_PCM_FORMAT_S32_LE;
        default:
            fprintf(stderr, "警告: 不支持的位深度 %u，使用16位\n", bit_depth);
            return SND_PCM_FORMAT_S16_LE;
    }
}

// 播放线程函数
static void *playback_thread_func(void *arg) {
    audio_playback_t *playback = (audio_playback_t *)arg;
    snd_pcm_t *pcm_handle = NULL;
    snd_pcm_hw_params_t *params = NULL;
    int16_t *buffer = NULL;
    int rc;
    
    printf("播放线程: 启动\n");
    
    // 打开PCM设备
    rc = snd_pcm_open(&pcm_handle, playback->params.device,
                      SND_PCM_STREAM_PLAYBACK, 0);
    if (rc < 0) {
        fprintf(stderr, "播放线程: 无法打开设备 %s: %s\n",
                playback->params.device, snd_strerror(rc));
        return NULL;
    }
    
    // 分配参数结构
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(pcm_handle, params);
    
    // 设置参数
    snd_pcm_hw_params_set_access(pcm_handle, params,
                                  SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_format_t format = get_alsa_format(playback->params.bit_depth);
    snd_pcm_hw_params_set_format(pcm_handle, params, format);
    snd_pcm_hw_params_set_channels(pcm_handle, params, playback->params.channels);
    snd_pcm_hw_params_set_rate(pcm_handle, params, playback->params.sample_rate, 0);
    
    // 设置低延迟参数：period_size和buffer_size
    snd_pcm_uframes_t period_size = playback->params.buffer_frames;
    snd_pcm_uframes_t buffer_size = period_size * 16;  // buffer_size = 16 * period_size，提供更多缓冲
    
    int dir = 0;
    rc = snd_pcm_hw_params_set_period_size_near(pcm_handle, params, &period_size, &dir);
    if (rc < 0) {
        fprintf(stderr, "播放线程: 设置period_size失败: %s\n", snd_strerror(rc));
        goto cleanup;
    }
    
    dir = 0;
    rc = snd_pcm_hw_params_set_buffer_size_near(pcm_handle, params, &buffer_size);
    if (rc < 0) {
        fprintf(stderr, "播放线程: 设置buffer_size失败: %s\n", snd_strerror(rc));
        goto cleanup;
    }
    
    // 应用参数
    rc = snd_pcm_hw_params(pcm_handle, params);
    if (rc < 0) {
        fprintf(stderr, "播放线程: 设置参数失败: %s\n", snd_strerror(rc));
        goto cleanup;
    }
    
    // 验证实际设置的参数
    unsigned int actual_rate;
    snd_pcm_uframes_t actual_period_size, actual_buffer_size;
    snd_pcm_hw_params_get_rate(params, &actual_rate, NULL);
    snd_pcm_hw_params_get_period_size(params, &actual_period_size, NULL);
    snd_pcm_hw_params_get_buffer_size(params, &actual_buffer_size);
    printf("播放线程: 实际参数 - 采样率: %u, period_size: %lu, buffer_size: %lu\n",
           actual_rate, (unsigned long)actual_period_size, (unsigned long)actual_buffer_size);
    
    // 分配缓冲区（使用buffer_frames而不是固定的PLAYBACK_BUFFER_SIZE）
    size_t buffer_samples = playback->params.buffer_frames * playback->params.channels;
    buffer = (int16_t *)malloc(buffer_samples * sizeof(int16_t));
    if (!buffer) {
        fprintf(stderr, "播放线程: 内存分配失败\n");
        goto cleanup;
    }
    
    printf("播放线程: 开始播放\n");
    
    // 等待缓冲区有足够数据再开始播放（避免下溢）
    size_t min_samples = buffer_samples * 8;  // 至少需要8倍的数据，提供更多缓冲
    printf("播放线程: 等待缓冲区积累数据（至少需要 %zu 样本）...\n", min_samples);
    while (playback->running && ring_buffer_readable(playback->input_buffer) < min_samples) {
        usleep(10000);  // 等待10ms
    }
    printf("播放线程: 缓冲区数据充足，开始播放\n");
    
    // 播放循环
    while (playback->running) {
        // 检查可用数据量，动态调整读取大小
        size_t available = ring_buffer_readable(playback->input_buffer);
        
        // 如果数据不足，等待更多数据（避免下溢）
        if (available < buffer_samples) {
            usleep(5000);  // 等待5ms
            continue;
        }
        
        // 从环形缓冲区读取（使用与录制线程匹配的块大小）
        size_t samples = ring_buffer_read(playback->input_buffer, buffer,
                                              buffer_samples);
        if (samples == 0) {
            continue;
        }
        
        // 确保samples是channels的整数倍
        if (samples % playback->params.channels != 0) {
            fprintf(stderr, "播放线程: 警告 - 样本数(%zu)不是通道数(%u)的整数倍\n", 
                    samples, playback->params.channels);
            continue;
        }
        
        // 写入ALSA设备
        snd_pcm_sframes_t frames = samples / playback->params.channels;
        if (frames <= 0) {
            fprintf(stderr, "播放线程: 警告 - 无效的帧数: %ld\n", (long)frames);
            continue;
        }
        
        rc = snd_pcm_writei(pcm_handle, buffer, frames);
        
        if (rc == -EPIPE) {
            fprintf(stderr, "播放线程: 缓冲区下溢，重新准备\n");
            snd_pcm_prepare(pcm_handle);
            // 下溢后等待更多数据再继续
            usleep(10000);
            continue;
        } else if (rc < 0) {
            fprintf(stderr, "播放线程: 写入失败: %s\n", snd_strerror(rc));
            break;
        } else if (rc != frames) {
            fprintf(stderr, "播放线程: 短写：预期%ld帧，实际%ld帧\n", (long)frames, (long)rc);
        }
        
        // 减少打印频率，避免输出过多
        static int print_count = 0;
        if (++print_count % 10 == 0) {
            printf("播放线程: 播放 %zu 样本 (%ld 帧, 可用: %zu)\n", 
                   samples, (long)frames, ring_buffer_readable(playback->input_buffer));
        }
    }
    
    printf("播放线程: 停止播放\n");
    
cleanup:
    if (buffer) free(buffer);
    if (pcm_handle) {
        snd_pcm_drain(pcm_handle);
        snd_pcm_close(pcm_handle);
    }
    
    return NULL;
}

// 初始化播放模块
int audio_playback_init(audio_playback_t *playback, ring_buffer_t *input_buffer,
                        const playback_params_t *params) {
    playback->input_buffer = input_buffer;
    playback->params = *params;
    playback->running = false;
    
    return 0;
}

// 启动播放线程
int audio_playback_start(audio_playback_t *playback) {
    if (playback->running) {
        return -1;
    }
    
    playback->running = true;
    int rc = pthread_create(&playback->thread, NULL, playback_thread_func, playback);
    if (rc != 0) {
        playback->running = false;
        return -1;
    }
    
    return 0;
}

// 停止播放线程
void audio_playback_stop(audio_playback_t *playback) {
    playback->running = false;
}

// 等待播放线程结束
void audio_playback_join(audio_playback_t *playback) {
    if (playback->running) {
        pthread_join(playback->thread, NULL);
    }
}

// 清理资源
void audio_playback_cleanup(audio_playback_t *playback) {
    audio_playback_stop(playback);
    audio_playback_join(playback);
}

