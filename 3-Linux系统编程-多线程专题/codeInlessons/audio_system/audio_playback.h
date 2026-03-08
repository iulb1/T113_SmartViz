/**
 * 音频播放模块
 */

#ifndef AUDIO_PLAYBACK_H
#define AUDIO_PLAYBACK_H

#include "ring_buffer.h"
#include <stdint.h>
#include <stdbool.h>

// 播放参数
typedef struct {
    const char *device;
    unsigned int sample_rate;
    unsigned int channels;
    unsigned int bit_depth;
    size_t buffer_frames;
} playback_params_t;

// 播放控制结构
typedef struct {
    ring_buffer_t *input_buffer;  // 输入缓冲区
    playback_params_t params;
    volatile bool running;
    pthread_t thread;
} audio_playback_t;

// 初始化播放模块
int audio_playback_init(audio_playback_t *playback, ring_buffer_t *input_buffer,
                        const playback_params_t *params);

// 启动播放线程
int audio_playback_start(audio_playback_t *playback);

// 停止播放线程
void audio_playback_stop(audio_playback_t *playback);

// 等待播放线程结束
void audio_playback_join(audio_playback_t *playback);

// 清理资源
void audio_playback_cleanup(audio_playback_t *playback);

#endif // AUDIO_PLAYBACK_H









