/**
 * 音频处理系统主程序
 * 直通模式：录制 → 播放（低延迟实时监听）
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "ring_buffer.h"
#include "audio_record.h"
#include "audio_playback.h"

// 全局变量
ring_buffer_t g_buffer;  // 单个环形缓冲区（直通模式）
audio_record_t g_record;
audio_playback_t g_playback;
volatile int g_running = 1;

// 信号处理函数
void signal_handler(int sig) {
    printf("\n收到信号 %d，准备退出...\n", sig);
    g_running = 0;
}

int main() {
    int rc;
    
    // 注册信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    printf("=== 音频处理系统（直通模式） ===\n");
    printf("架构: 录制线程 → 环形缓冲区 → 播放线程（低延迟实时监听）\n\n");
    
    // 初始化环形缓冲区（平衡延迟和稳定性）
    // 2048样本在16kHz下约128ms延迟，提供足够的缓冲以避免杂音
    printf("初始化环形缓冲区（2048样本，约128ms延迟）...\n");
    rc = ring_buffer_init(&g_buffer, 2048);
    if (rc < 0) {
        fprintf(stderr, "初始化缓冲区失败\n");
        return EXIT_FAILURE;
    }
    
    // 初始化录制模块（平衡延迟和稳定性）
    printf("初始化录制模块（优化配置）...\n");
    record_params_t record_params = {
        .device = "hw:audiocodec",
        .sample_rate = 16000,
        .channels = 1,
        .bit_depth = 16,          // 16位深度
        .buffer_frames = 128     // 128帧（约8ms延迟）
    };
    audio_record_init(&g_record, &g_buffer, &record_params);
    
    // 初始化播放模块（平衡延迟和稳定性）
    printf("初始化播放模块（优化配置）...\n");
    playback_params_t playback_params = {
        .device = "hw:audiocodec",
        .sample_rate = 16000,
        .channels = 1,
        .bit_depth = 16,          // 16位深度
        .buffer_frames = 128     // 128帧（约8ms延迟）
    };
    audio_playback_init(&g_playback, &g_buffer, &playback_params);
    
    // 启动所有线程
    printf("\n启动所有线程...\n");
    audio_record_start(&g_record);
    usleep(1000000);  // 等待1000ms，让录制先积累足够的数据
    
    audio_playback_start(&g_playback);
    
    printf("系统运行中（直通模式，低延迟），按 Ctrl+C 退出...\n\n");
    
    // 主循环
    while (g_running) {
        sleep(1);
    }
    
    // 停止所有线程
    printf("\n停止所有线程...\n");
    audio_record_stop(&g_record);
    audio_playback_stop(&g_playback);
    
    // 等待线程结束
    audio_record_join(&g_record);
    audio_playback_join(&g_playback);
    
    // 清理资源
    printf("清理资源...\n");
    audio_record_cleanup(&g_record);
    audio_playback_cleanup(&g_playback);
    ring_buffer_destroy(&g_buffer);
    
    printf("程序退出\n");
    return EXIT_SUCCESS;
}









