#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

#define PCM_DEVICE "hw:audiocodec"  // 开发板音频设备名
#define SAMPLE_RATE 16000           // 采样率
#define CHANNELS 1                  // 声道数
#define SAMPLE_FORMAT SND_PCM_FORMAT_S16_LE  // 采样格式：16位小端
#define RECORD_SECONDS 5            // 录制时长（秒）
#define BUFFER_FRAMES 1024          // 每次读取的帧数

int main() {
    
    snd_pcm_t *pcm_handle;          // PCM设备句柄
    snd_pcm_hw_params_t *params;    // 硬件参数结构体
    int rc;                         // 返回值
    FILE *output;                   // 输出文件指针
    int16_t *buffer;                // 音频缓冲区
    int loops;                      // 循环次数

    // 打开输出文件
    output = fopen("record.pcm", "wb");
    if (!output) {
        perror("fopen failed");
        return EXIT_FAILURE;
    }

    // 打开PCM捕获设备
    rc = snd_pcm_open(&pcm_handle, PCM_DEVICE, SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0) {
        fprintf(stderr, "无法打开PCM设备: %s\n", snd_strerror(rc));
        fclose(output);
        return EXIT_FAILURE;
    }

    // 初始化硬件参数结构体
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(pcm_handle, params);

    // 设置访问模式：交错模式（左右声道数据交替存储）
    rc = snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (rc < 0) {
        fprintf(stderr, "设置访问模式失败: %s\n", snd_strerror(rc));
        goto cleanup;
    }

    // 设置采样格式
    rc = snd_pcm_hw_params_set_format(pcm_handle, params, SAMPLE_FORMAT);
    if (rc < 0) {
        fprintf(stderr, "设置采样格式失败: %s\n", snd_strerror(rc));
        goto cleanup;
    }

    // 设置声道数
    rc = snd_pcm_hw_params_set_channels(pcm_handle, params, CHANNELS);
    if (rc < 0) {
        fprintf(stderr, "设置声道数失败: %s\n", snd_strerror(rc));
        goto cleanup;
    }

    // 设置采样率
    rc = snd_pcm_hw_params_set_rate(pcm_handle, params, SAMPLE_RATE, 0);
    if (rc < 0) {
        fprintf(stderr, "设置采样率失败: %s\n", snd_strerror(rc));
        goto cleanup;
    }

    // 应用硬件参数到设备
    rc = snd_pcm_hw_params(pcm_handle, params);
    if (rc < 0) {
        fprintf(stderr, "应用硬件参数失败: %s\n", snd_strerror(rc));
        goto cleanup;
    }

    // 分配音频缓冲区（帧数 × 声道数 × 采样大小）
    buffer = (int16_t *)malloc(BUFFER_FRAMES * CHANNELS * sizeof(int16_t));
    if (!buffer) {
        fprintf(stderr, "内存分配失败\n");
        goto cleanup;
    }

    // 计算循环次数：总采样数 / 每次读取帧数
    loops = SAMPLE_RATE * RECORD_SECONDS / BUFFER_FRAMES;

    // 循环读取音频数据
    for (int i = 0; i < loops; ++i) {
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

    // 释放资源
    free(buffer);
cleanup:
    fclose(output);
    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);

    printf("录音完成！文件保存为 record.pcm\n");
    return EXIT_SUCCESS;
}