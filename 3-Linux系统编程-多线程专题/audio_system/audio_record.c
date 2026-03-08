/**
 * 音频录制模块实现
 */

#include "audio_record.h"
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

// 录制线程函数
static void *record_thread_func(void *arg) {
    audio_record_t *record = (audio_record_t *)arg;
    snd_pcm_t *pcm_handle = NULL;
    snd_pcm_hw_params_t *params = NULL;
    int16_t *buffer = NULL;
    int rc;
    
    printf("录制线程: 启动\n");
    
    // 打开PCM设备
    rc = snd_pcm_open(&pcm_handle, record->params.device, 
                      SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0) {
        fprintf(stderr, "录制线程: 无法打开设备 %s: %s\n", 
                record->params.device, snd_strerror(rc));
        return NULL;
    }
    
    // 分配参数结构
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(pcm_handle, params);
    
    // 设置参数
    snd_pcm_hw_params_set_access(pcm_handle, params, 
                                  SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_format_t format = get_alsa_format(record->params.bit_depth);
    snd_pcm_hw_params_set_format(pcm_handle, params, format);
    snd_pcm_hw_params_set_channels(pcm_handle, params, record->params.channels);
    snd_pcm_hw_params_set_rate(pcm_handle, params, record->params.sample_rate, 0);
    
    // 设置低延迟参数：period_size和buffer_size
    snd_pcm_uframes_t period_size = record->params.buffer_frames;
    snd_pcm_uframes_t buffer_size = period_size * 16;  // buffer_size = 16 * period_size，提供更多缓冲
    
    int dir = 0;
    rc = snd_pcm_hw_params_set_period_size_near(pcm_handle, params, &period_size, &dir);
    if (rc < 0) {
        fprintf(stderr, "录制线程: 设置period_size失败: %s\n", snd_strerror(rc));
        goto cleanup;
    }
    
    dir = 0;
    rc = snd_pcm_hw_params_set_buffer_size_near(pcm_handle, params, &buffer_size);
    if (rc < 0) {
        fprintf(stderr, "录制线程: 设置buffer_size失败: %s\n", snd_strerror(rc));
        goto cleanup;
    }
    
    // 应用参数
    rc = snd_pcm_hw_params(pcm_handle, params);
    if (rc < 0) {
        fprintf(stderr, "录制线程: 设置参数失败: %s\n", snd_strerror(rc));
        goto cleanup;
    }
    
    // 验证实际设置的参数
    unsigned int actual_rate;
    snd_pcm_uframes_t actual_period_size, actual_buffer_size;
    snd_pcm_hw_params_get_rate(params, &actual_rate, NULL);
    snd_pcm_hw_params_get_period_size(params, &actual_period_size, NULL);
    snd_pcm_hw_params_get_buffer_size(params, &actual_buffer_size);
    printf("录制线程: 实际参数 - 采样率: %u, period_size: %lu, buffer_size: %lu\n",
           actual_rate, (unsigned long)actual_period_size, (unsigned long)actual_buffer_size);
    
    // 分配缓冲区
    buffer = (int16_t *)malloc(record->params.buffer_frames * 
                               record->params.channels * sizeof(int16_t));
    if (!buffer) {
        fprintf(stderr, "录制线程: 内存分配失败\n");
        goto cleanup;
    }
    
    printf("录制线程: 开始录制\n");
    
    // 录制循环
    while (record->running) {
        rc = snd_pcm_readi(pcm_handle, buffer, record->params.buffer_frames);
        
        if (rc == -EPIPE) {
            fprintf(stderr, "录制线程: 缓冲区溢出，重新准备\n");
            snd_pcm_prepare(pcm_handle);
            continue;
        } else if (rc < 0) {
            fprintf(stderr, "录制线程: 读取失败: %s\n", snd_strerror(rc));
            break;
        }
        
        // 写入环形缓冲区
        size_t samples = rc * record->params.channels;
        if (samples > 0) {
            size_t written = ring_buffer_write(record->output_buffer, buffer, samples);
            if (written != samples) {
                fprintf(stderr, "录制线程: 警告 - 写入不完整: 预期%zu，实际%zu\n", 
                        samples, written);
            }
        }
        
        printf("录制线程: 录制 %zu 样本 (rc=%d, channels=%u)\n", 
               samples, rc, record->params.channels);
    }
    
    printf("录制线程: 停止录制\n");
    
cleanup:
    if (buffer) free(buffer);
    if (pcm_handle) {
        snd_pcm_drain(pcm_handle);
        snd_pcm_close(pcm_handle);
    }
    
    return NULL;
}

// 初始化录制模块
int audio_record_init(audio_record_t *record, ring_buffer_t *output_buffer,
                      const record_params_t *params) {
    record->output_buffer = output_buffer;
    record->params = *params;
    record->running = false;
    
    return 0;
}

// 启动录制线程
int audio_record_start(audio_record_t *record) {
    if (record->running) {
        return -1;  // 已经在运行
    }
    
    record->running = true;
    int rc = pthread_create(&record->thread, NULL, record_thread_func, record);
    if (rc != 0) {
        record->running = false;
        return -1;
    }
    
    return 0;
}

// 停止录制线程
void audio_record_stop(audio_record_t *record) {
    record->running = false;
}

// 等待录制线程结束
void audio_record_join(audio_record_t *record) {
    if (record->running) {
        pthread_join(record->thread, NULL);
    }
}

// 清理资源
void audio_record_cleanup(audio_record_t *record) {
    audio_record_stop(record);
    audio_record_join(record);
}

