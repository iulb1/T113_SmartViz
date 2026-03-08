/**
 * 音频录制模块
 */

#ifndef AUDIO_RECORD_H
#define AUDIO_RECORD_H

#include "ring_buffer.h"
#include <stdint.h>
#include <stdbool.h>

// 录制参数
typedef struct {
    const char *device;
    unsigned int sample_rate;
    unsigned int channels;
    unsigned int bit_depth;
    size_t buffer_frames;
} record_params_t;

// 录制控制结构
typedef struct {
    ring_buffer_t *output_buffer;  // 输出缓冲区
    record_params_t params;
    volatile bool running;
    pthread_t thread;
} audio_record_t;

// 初始化录制模块
int audio_record_init(audio_record_t *record, ring_buffer_t *output_buffer, 
                       const record_params_t *params);

// 启动录制线程
int audio_record_start(audio_record_t *record);

// 停止录制线程
void audio_record_stop(audio_record_t *record);

// 等待录制线程结束
void audio_record_join(audio_record_t *record);

// 清理资源
void audio_record_cleanup(audio_record_t *record);

#endif // AUDIO_RECORD_H









