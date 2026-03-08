#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

int main() {

    snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *params;

    //1. 打开目标音频文件
    FILE *output = fopen("record.pcm", "wb");
    if (!output) {
        perror("fopen");
        return 1;
    }
    //2. 打开ALSA设备
    //int snd_pcm_open(snd_pcm_t **pcmp, const char *name, 
    //    snd_pcm_stream_t stream, int mode);
    snd_pcm_open(&pcm_handle,"default",SND_PCM_STREAM_CAPTURE,0);
    //3. 申请配置结构体 int snd_pcm_hw_params_malloc(snd_pcm_hw_params_t **params);
    snd_pcm_hw_params_malloc(&params);
    //4. 设置配置结构体
    snd_pcm_hw_params_any(pcm_handle, params);
    //5. 设置访问模式左右声道交替存储
    // int snd_pcm_hw_params_set_access(snd_pcm_t *pcm, 
    //                               snd_pcm_hw_params_t *params,
    //                               snd_pcm_access_t access);
    snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    //6. 设置采样格式
    // int snd_pcm_hw_params_set_format(snd_pcm_t *pcm,
    //                                 snd_pcm_hw_params_t *params,
    //                                 snd_pcm_format_t format);
    snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_S16_LE);
    //7. 设置采样率
    // int snd_pcm_hw_params_set_rate(snd_pcm_t *pcm,
    //                                 snd_pcm_hw_params_t *params,
    //                                 unsigned int val, int dir);
    snd_pcm_hw_params_set_rate(pcm_handle, params, 16000, 0);

    //8. 设置通道数
    // int snd_pcm_hw_params_set_channels(snd_pcm_t *pcm,
    //                                 snd_pcm_hw_params_t *params,
    //                                 unsigned int channels);
    snd_pcm_hw_params_set_channels(pcm_handle, params, 1);
    //9. 应用硬件参数
    //int snd_pcm_hw_params(snd_pcm_t *pcm, snd_pcm_hw_params_t *params);
    snd_pcm_hw_params(pcm_handle, params);
   
}