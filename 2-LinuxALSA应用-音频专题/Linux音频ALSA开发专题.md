# Linux音频ALSA开发专题



## 一、音频框架概述与 ALSA 来源

### 1.1 Linux 音频框架演进

Linux 系统音频架构经历了从 OSS（Open Sound System）到 ALSA（Advanced Linux Sound Architecture）的发展。OSS 曾是 Linux 主流音频驱动框架，但存在闭源组件、扩展性差等问题；ALSA 作为新一代音频架构，于 2001 年被引入 Linux 内核，现已成为标准音频子系统，提供更完善的功能、更低的延迟和更好的硬件兼容性。

### 1.2 ALSA 的核心优势

- **模块化设计**：支持声卡驱动、中间层 API、应用层工具的分层架构；
- **多设备支持**：同时管理多个声卡和音频设备；
- **低延迟音频**：满足实时音频处理需求（如嵌入式语音交互）；
- **兼容性**：兼容 OSS 接口，支持多种音频格式和采样率。

### 1.3 ALSA 的组成结构

ALSA 主要包含三层：

<img src="G:\202507\pics\alsa.png" style="zoom:50%;" /> 

- **内核层**：声卡驱动程序、音频核心模块，负责硬件交互；

  ~~~shell
  # 查看ALSA在/proc中的信息
  ls -la /proc/asound/
  # 查看声卡信息
  cat /proc/asound/cards
  # 查看设备列表
  cat /proc/asound/devices
  # 查看PCM设备
  cat /proc/asound/pcm
  # 查看版本信息
  cat /proc/asound/version
  ~~~

- **中间层**：alsa-lib 库，封装内核接口，提供统一的应用层 API；

  ~~~c
  // 分配硬件参数结构体
  snd_pcm_hw_params_alloca(snd_pcm_hw_params_t **params);
  // 初始化参数为设备支持的默认值
  snd_pcm_hw_params_any(snd_pcm_t *pcm_handle, snd_pcm_hw_params_t *params);
  // 设置访问模式（如交错模式）
  snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
  // 设置采样格式（如16位小端）
  snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_S16_LE);
  // 设置声道数
  snd_pcm_hw_params_set_channels(pcm_handle, params, channels);
  // 设置采样率（最后一个参数为允许的误差，0表示严格匹配）
  snd_pcm_hw_params_set_rate(pcm_handle, params, rate, 0);
  
  // 应用硬件参数到设备
  snd_pcm_hw_params(pcm_handle, params);
  ~~~

- **应用层**：命令行工具（如 aplay、arecord）和应用程序，基于 alsa-lib 开发。

  ~~~shell
  # 使用aplay查看播放设备（ALSA工具层）
  aplay -l
  # 使用arecord查看录音设备
  arecord -l
  # 录制音频并保存为PCM文件（16kHz、单声道、16位采样）
  arecord -D hw:0,0 -f S16_LE -r 16000 -c 1 record.pcm
  # 录制为WAV文件
  arecord -D hw:0,0 -f S16_LE -r 16000 -c 1 -t wav record.wav
  # 播放PCM文件
  aplay -D hw:0,0 -f S16_LE -r 16000 -c 1 record.pcm
  # 播放WAV文件（自动识别参数）
  aplay record.wav
  ~~~

  

## 二、ALSA 关键 API 函数

初始化开发流程

![](G:\202507\pics\初始化.png) 

### **2.1 关键API总结表**

| 功能       | 函数                             | 说明                 |
| :--------- | :------------------------------- | :------------------- |
| 打开设备   | snd_pcm_open()                   | 打开PCM设备          |
| 分配参数   | snd_pcm_hw_params_malloc()       | 分配硬件参数结构     |
| 初始化参数 | snd_pcm_hw_params_any()          | 初始化参数为默认值   |
| 设置格式   | snd_pcm_hw_params_set_format()   | 设置采样格式         |
| 设置采样率 | snd_pcm_hw_params_set_rate()     | 设置采样率           |
| 设置声道   | snd_pcm_hw_params_set_channels() | 设置声道数           |
| 应用参数   | snd_pcm_hw_params()              | 应用硬件参数         |
| 准备PCM    | snd_pcm_prepare()                | 准备PCM设备          |
| 写入数据   | snd_pcm_writei()                 | 写入音频数据（播放） |
| 读取数据   | snd_pcm_readi()                  | 读取音频数据（录音） |
| 排空缓冲区 | snd_pcm_drain()                  | 排空播放缓冲区       |
| 关闭设备   | snd_pcm_close()                  | 关闭PCM设备          |

### 2.2 设置硬件参数

~~~c

int snd_pcm_open(snd_pcm_t **pcmp, const char *name, 
                 snd_pcm_stream_t stream, int mode);
/**
 * 打开PCM
 * 这是使用ALSA进行音频操作的第一步，必须先打开PCM设备才能进行后续操作
 * M句柄指针的指针（二级指针）
 *   - 函数成功后会在此处返回PCM设备句柄
 *   - 后续所有PCM操作都需要使用这个句柄
 *   - 使用完后必须调用snd_pcm_close()关闭
 * 
 * @param name PCM设备名称字符串
 *   常用设备名称：
 *   - "default": 使用系统默认PCM设备（推荐，最常用）
 *   - "hw:0,0": 直接访问硬件设备0的子设备0（无插件，低延迟）
 *   - "hw:0,1": 硬件设备0的子设备1
 *   - "plughw:0,0": 使用插件访问硬件设备（自动转换格式）
 *   - "dmix": 使用dmix插件（允许多个应用同时播放）
 *   - "dsnoop": 使用dsnoop插件（允许多个应用同时录音）
 *   - "/dev/snd/pcmC0D0p": 直接使用设备文件路径
 * 
 * @param stream 数据流方向
 *   - SND_PCM_STREAM_PLAYBACK: 播放模式（输出音频）
 *   - SND_PCM_STREAM_CAPTURE: 录音模式（输入音频）
 * 
 * @param mode 打开模式（位掩码，可以组合使用）
 *   常用模式：
 *   - 0: 阻塞模式（默认，推荐）
 *   - SND_PCM_NONBLOCK: 非阻塞模式（读写操作立即返回）
 *   - SND_PCM_ASYNC: 异步模式（使用信号通知）
 *   组合示例：
 *   - SND_PCM_NONBLOCK | SND_PCM_ASYNC: 非阻塞+异步
 * 
 * @return 返回值
 *   - 成功返回0
 *   - 失败返回负数错误码：
 *     * -ENOENT: 设备不存在
 *     * -EBUSY: 设备被占用
 *     * -EACCES: 权限不足
 *     * -ENODEV: 设备不可用
 *     * 可以使用snd_strerror()将错误码转换为错误信息
 * 
 * @note 重要提示：
 *   1. 打开设备后必须调用snd_pcm_close()关闭，否则会资源泄漏
 *   2. "default"设备会自动选择最佳可用设备，推荐使用
 *   3. "hw:X,Y"格式直接访问硬件，性能最好但需要精确匹配格式
 *   4. "plughw:X,Y"会自动进行格式转换，更灵活但性能略低
 *   5. 非阻塞模式下，读写操作可能返回-EAGAIN
 * 
 * @example 使用示例：
 *   snd_pcm_t *pcm_handle;
 *   int err;
 *   
 *   // 打开默认设备用于播放
 *   if ((err = snd_pcm_open(&pcm_handle, "default", 
 *                           SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
 *       fprintf(stderr, "无法打开PCM设备: %s\n", snd_strerror(err));
 *       return -1;
 *   }
 *   
 *   // ... 使用pcm_handle进行后续操作 ...
 *   
 *   // 使用完后关闭
 *   snd_pcm_close(pcm_handle);
 */
int snd_pcm_open(snd_pcm_t **pcmp, const char *name, 
                 snd_pcm_stream_t stream, int mode);
~~~



~~~c
/**
 * 分配硬件参数结构体内存
 * @param params 指向硬件参数指针的指针，函数会分配内存并返回
 * @return 成功返回0，失败返回负数错误码
 */
int snd_pcm_hw_params_malloc(snd_pcm_hw_params_t **params);

/**
 * 初始化硬件参数为PCM设备支持的所有可能值,默认值
 * @param pcm PCM设备句柄
 * @param params 硬件参数结构体指针（需先通过malloc分配）
 * @return 成功返回0，失败返回负数错误码
 */
int snd_pcm_hw_params_any(snd_pcm_t *pcm, snd_pcm_hw_params_t *params);

/**
 * 设置PCM访问模式（数据在内存中的组织方式）
 * @param pcm PCM设备句柄
 * @param params 硬件参数结构体指针
 * @param access 访问模式：
 *   - SND_PCM_ACCESS_RW_INTERLEAVED: 交错模式（左右声道交替存储）
 *   - SND_PCM_ACCESS_RW_NONINTERLEAVED: 非交错模式（左右声道分开存储）
 *   - SND_PCM_ACCESS_MMAP_INTERLEAVED: 内存映射交错模式
 *   - SND_PCM_ACCESS_MMAP_NONINTERLEAVED: 内存映射非交错模式
 * @return 成功返回0，失败返回负数错误码
 */
int snd_pcm_hw_params_set_access(snd_pcm_t *pcm, 
                                  snd_pcm_hw_params_t *params,
                                  snd_pcm_access_t access);

/**
 * 设置PCM采样格式（位深度和字节序）
 * @param pcm PCM设备句柄
 * @param params 硬件参数结构体指针
 * @param format 采样格式：
 *   - SND_PCM_FORMAT_S16_LE: 16位有符号整数，小端
 *   - SND_PCM_FORMAT_S16_BE: 16位有符号整数，大端
 *   - SND_PCM_FORMAT_S24_LE: 24位有符号整数，小端
 *   - SND_PCM_FORMAT_S32_LE: 32位有符号整数，小端
 *   - SND_PCM_FORMAT_FLOAT_LE: 32位浮点数，小端
 *   - 等等...
 * @return 成功返回0，失败返回负数错误码
 */
int snd_pcm_hw_params_set_format(snd_pcm_t *pcm,
                                  snd_pcm_hw_params_t *params,
                                  snd_pcm_format_t format);

/**
 * 设置采样率
 * @param pcm PCM设备句柄
 * @param params 硬件参数结构体指针
 * @param val 采样率值（如44100, 48000等），函数可能会修改此值以匹配硬件支持的值
 * @param dir 方向参数：
 *   - 0: 精确匹配或向上取整
 *   - 1: 向下取整
 *   - -1: 最接近的值
 * @return 成功返回0，失败返回负数错误码
 * @note 使用snd_pcm_hw_params_set_rate_near()可以自动选择最接近的值
 */
int snd_pcm_hw_params_set_rate(snd_pcm_t *pcm,
                                snd_pcm_hw_params_t *params,
                                unsigned int val, int dir);

/**
 * 设置声道数
 * @param pcm PCM设备句柄
 * @param params 硬件参数结构体指针
 * @param val 声道数：
 *   - 1: 单声道
 *   - 2: 立体声
 *   - 更多声道（如5.1, 7.1等）
 * @return 成功返回0，失败返回负数错误码
 */
int snd_pcm_hw_params_set_channels(snd_pcm_t *pcm,
                                    snd_pcm_hw_params_t *params,
                                    unsigned int val);

/**
 * 应用硬件参数到PCM设备
 * 此函数会将之前设置的所有硬件参数应用到实际的PCM设备
 * @param pcm PCM设备句柄
 * @param params 硬件参数结构体指针（包含所有已设置的参数）
 * @return 成功返回0，失败返回负数错误码
 * @note 调用此函数后，硬件参数才会真正生效
 */
int snd_pcm_hw_params(snd_pcm_t *pcm, snd_pcm_hw_params_t *params);

/**
 * 释放硬件参数结构体内存
 * @param params 硬件参数结构体指针（由malloc分配的内存）
 * @note 使用完硬件参数后必须调用此函数释放内存
 */
void snd_pcm_hw_params_free(snd_pcm_hw_params_t *params);
~~~

### 2.3 设置软件参数

~~~c
/**
 * 分配软件参数结构体内存
 * @param params 指向软件参数指针的指针，函数会分配内存并返回
 * @return 成功返回0，失败返回负数错误码
 */
int snd_pcm_sw_params_malloc(snd_pcm_sw_params_t **params);

/**
 * 获取PCM设备当前的软件参数
 * @param pcm PCM设备句柄
 * @param params 软件参数结构体指针（需先通过malloc分配）
 * @return 成功返回0，失败返回负数错误码
 * @note 此函数会将当前设备的软件参数填充到params中
 */
int snd_pcm_sw_params_current(snd_pcm_t *pcm, snd_pcm_sw_params_t *params);

/**
 * 设置启动阈值（start threshold）
 * 当缓冲区中可用数据达到此阈值时，PCM设备才会开始播放
 * @param pcm PCM设备句柄
 * @param params 软件参数结构体指针
 * @param val 启动阈值（帧数）：
 *   - 通常设置为period_size或更大
 *   - 设置为缓冲区大小可以避免启动时的延迟
 * @return 成功返回0，失败返回负数错误码
 * @note 较大的值可以减少启动延迟，但会增加初始延迟
 */
int snd_pcm_sw_params_set_start_threshold(snd_pcm_t *pcm,
                                           snd_pcm_sw_params_t *params,
                                           snd_pcm_uframes_t val);

/**
 * 设置最小可用空间阈值（avail_min）
 * 当可用空间达到此阈值时，才会触发中断或唤醒等待的线程
 * @param pcm PCM设备句柄
 * @param params 软件参数结构体指针
 * @param val 最小可用空间阈值（帧数）：
 *   - 通常设置为period_size
 *   - 较小的值可以提高响应性，但会增加CPU占用
 *   - 较大的值可以减少中断次数，但可能增加延迟
 * @return 成功返回0，失败返回负数错误码
 * @note 此参数影响中断频率和延迟
 */
int snd_pcm_sw_params_set_avail_min(snd_pcm_t *pcm,
                                    snd_pcm_sw_params_t *params,
                                    snd_pcm_uframes_t val);

/**
 * 应用软件参数到PCM设备
 * 此函数会将之前设置的所有软件参数应用到实际的PCM设备
 * @param pcm PCM设备句柄
 * @param params 软件参数结构体指针（包含所有已设置的参数）
 * @return 成功返回0，失败返回负数错误码
 * @note 调用此函数后，软件参数才会真正生效
 */
int snd_pcm_sw_params(snd_pcm_t *pcm, snd_pcm_sw_params_t *params);

/**
 * 释放软件参数结构体内存
 * @param params 软件参数结构体指针（由malloc分配的内存）
 * @note 使用完软件参数后必须调用此函数释放内存
 */
void snd_pcm_sw_params_free(snd_pcm_sw_params_t *params);
~~~

### 2.4 数据读写函数

~~~c
/**
 * 向PCM设备写入音频数据（播放）
 * 将音频数据从应用程序缓冲区写入PCM设备的播放缓冲区
 * 
 * @param pcm PCM设备句柄（必须是SND_PCM_STREAM_PLAYBACK模式打开）
 * @param buffer 音频数据缓冲区指针
 *   - 数据格式必须与设置的硬件参数一致（格式、声道数等）
 *   - 对于交错模式：数据排列为 L R L R L R ...
 *   - 对于非交错模式：需要提供多个缓冲区指针
 * @param size 要写入的帧数（不是字节数）
 *   - 1帧 = 1个采样点 × 声道数
 *   - 例如：16位立体声，1帧 = 2字节 × 2声道 = 4字节
 * 
 * @return 成功返回实际写入的帧数（通常等于size）
 *         失败返回负数错误码：
 *   - -EPIPE: 发生underrun（缓冲区欠载，数据不足）
 *   - -ESTRPIPE: 设备被挂起（需要调用snd_pcm_prepare()恢复）
 *   - -EAGAIN: 设备暂时不可用（非阻塞模式）
 * 
 * @note 
 *   - 函数是阻塞的，会等待直到有足够空间写入数据
 *   - 如果返回的帧数小于请求的size，说明发生了错误
 *   - 发生underrun后需要调用snd_pcm_prepare()重新准备设备
 *   - 缓冲区大小 = period_size × periods
 */
snd_pcm_sframes_t snd_pcm_writei(snd_pcm_t *pcm, 
                                   const void *buffer, 
                                   snd_pcm_uframes_t size);

/**
 * 从PCM设备读取音频数据（录音）
 * 从PCM设备的录音缓冲区读取音频数据到应用程序缓冲区
 * 
 * @param pcm PCM设备句柄（必须是SND_PCM_STREAM_CAPTURE模式打开）
 * @param buffer 音频数据缓冲区指针（用于存储读取的数据）
 * @param size 要操作的采样数
 * 
 * @return 成功返回实际读取的帧数（通常等于size）
 *         失败返回负数错误码：
 *   - -EPIPE: 发生overrun（缓冲区溢出，数据丢失）
 *   - -ESTRPIPE: 设备被挂起（需要调用snd_pcm_prepare()恢复）
 *   - -EAGAIN: 设备暂时不可用（非阻塞模式）
 * 
 * @note 
 *   - 函数是阻塞的，会等待直到有足够数据可读
 *   - 如果返回的帧数小于请求的size，说明发生了错误
 *   - 发生overrun后需要调用snd_pcm_prepare()重新准备设备
 *   - 应该定期检查可用数据量，避免overrun
 */
snd_pcm_sframes_t snd_pcm_readi(snd_pcm_t *pcm, 
                                 void *buffer, 
                                 snd_pcm_uframes_t size);
/*
读取逻辑：
    比如要录制10s的音频，那么数据总量是：
    	采样率*录制时间  
    
    真实音频数据大小：采样率 x 时间 x 位深度 x 声道数   一整个池子的水
    处理帧：帧大小(字节) = 采样数 × 位深度 × 声道数     一桶的水，桶的大小被
*/
~~~

### 2.5 使用实例

~~~c
// 硬件参数设置示例
snd_pcm_hw_params_t *hw_params;
snd_pcm_hw_params_malloc(&hw_params);  // 1. 分配内存
snd_pcm_hw_params_any(pcm_handle, hw_params);  // 2. 初始化为默认值

// 3. 设置各种参数
snd_pcm_hw_params_set_access(pcm_handle, hw_params, 
                              SND_PCM_ACCESS_RW_INTERLEAVED);  // 交错模式
snd_pcm_hw_params_set_format(pcm_handle, hw_params, 
                              SND_PCM_FORMAT_S16_LE);  // 16位小端
unsigned int rate = 44100;
snd_pcm_hw_params_set_rate_near(pcm_handle, hw_params, &rate, 0);  // 采样率
snd_pcm_hw_params_set_channels(pcm_handle, hw_params, 2);  // 立体声

snd_pcm_hw_params(pcm_handle, hw_params);  // 4. 应用参数
snd_pcm_hw_params_free(hw_params);  // 5. 释放内存

// 软件参数设置示例
snd_pcm_sw_params_t *sw_params;
snd_pcm_sw_params_malloc(&sw_params);  // 1. 分配内存
snd_pcm_sw_params_current(pcm_handle, sw_params);  // 2. 获取当前参数

snd_pcm_uframes_t period_size;
snd_pcm_hw_params_get_period_size(hw_params, &period_size, NULL);
snd_pcm_sw_params_set_avail_min(pcm_handle, sw_params, period_size);  // 设置最小可用空间

snd_pcm_sw_params(pcm_handle, sw_params);  // 3. 应用参数
snd_pcm_sw_params_free(sw_params);  // 4. 释放内存

// 数据写入示例
char *buffer = malloc(period_size * 2 * 2);  // 分配缓冲区（16位×2声道）
snd_pcm_sframes_t frames_written = snd_pcm_writei(pcm_handle, buffer, period_size);
if (frames_written < 0) {
    // 处理错误
    if (frames_written == -EPIPE) {
        // 发生underrun，需要重新准备
        snd_pcm_prepare(pcm_handle);
    }
}
~~~

配置检测

~~~c
void print_hw_params(snd_pcm_hw_params_t *params) {
    unsigned int rate, channels, periods;
    snd_pcm_uframes_t period_size, buffer_size;
    snd_pcm_format_t format;
    snd_pcm_access_t access;
    
    // 获取采样率
    snd_pcm_hw_params_get_rate(params, &rate, NULL);
    printf("采样率: %u Hz\n", rate);
    
    // 获取声道数
    snd_pcm_hw_params_get_channels(params, &channels);
    printf("声道数: %u\n", channels);
    
    // 获取格式
    snd_pcm_hw_params_get_format(params, &format);
    printf("格式: %s\n", snd_pcm_format_name(format));
    
    // 获取访问模式
    snd_pcm_hw_params_get_access(params, &access);
    printf("访问模式: %d\n", access);
    
    // 获取period大小
    snd_pcm_hw_params_get_period_size(params, &period_size, NULL);
    printf("Period大小: %lu 帧\n", period_size);
    
    // 获取period数量
    snd_pcm_hw_params_get_periods(params, &periods, NULL);
    printf("Period数量: %u\n", periods);
    
    // 获取缓冲区大小
    snd_pcm_hw_params_get_buffer_size(params, &buffer_size);
    printf("缓冲区大小: %lu 帧\n", buffer_size);
}
~~~



## 三、实战开发：音频录制与播放

### 3.1 音频录制实战

#### 功能说明

实现 16kHz 采样率、单声道、16 位采样的音频录制，保存为 PCM 原始文件。

#### 代码流程

1. 打开 PCM 捕获设备；

2. 配置硬件参数（采样率、声道数、采样格式）；

3. 分配缓冲区，循环读取音频数据；

4. 将数据写入文件，完成后释放资源。

    <img src="G:\202507\pics\音频录制流程2.jpg" style="zoom:80%;" />  

#### 完整代码

```c
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
```

#### 编译与运行

```bash
# 编译（需链接alsa库）
 arm-openwrt-linux-gcc alsa_recordo004.c 
 -I /home/zhiwan/t113/Tina-Linux/out/t113-zhiwan_v1/staging_dir/target/usr/include                                                -L /home/zhiwan/t113/Tina-Linux/out/t113-zhiwan_v1/staging_dir/target/usr/lib/ -lasound -o alsa04          
# 推送并运行
adb push alsa04 /root
# 开发板上命令
cd /root
./alsa04

#播放测试是否录制成功-开发板上运行
aplay -D hw:0,0 -f S16_LE -r 16000 -c 1 record.pcm
```

### 3.2 音频播放实战

#### 功能说明

播放上述录制的 PCM 文件，参数与录制时一致。

<img src="G:\202507\pics\音频读取.jpg"  />  

#### 代码流程

1. 打开 PCM 播放设备；
2. 配置与录制相同的硬件参数；
3. 读取 PCM 文件数据，循环写入播放设备；
4. 完成后释放资源。

#### 完整代码

```c
#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

#define PCM_DEVICE "hw:audiocodec"  // 开发板音频设备名
#define SAMPLE_RATE 16000           // 采样率
#define CHANNELS 1                  // 声道数
#define SAMPLE_FORMAT SND_PCM_FORMAT_S16_LE  // 采样格式
#define BUFFER_FRAMES 1024          // 每次写入的帧数

int main() {
    snd_pcm_t *pcm_handle;          // PCM设备句柄
    snd_pcm_hw_params_t *params;    // 硬件参数结构体
    int rc;                         // 返回值
    FILE *input;                    // 输入文件指针
    int16_t *buffer;                // 音频缓冲区
    int read_frames;                // 实际读取的帧数

    // 打开PCM文件
    input = fopen("record.pcm", "rb");
    if (!input) {
        perror("fopen failed");
        return EXIT_FAILURE;
    }

    // 打开PCM播放设备
    rc = snd_pcm_open(&pcm_handle, PCM_DEVICE, SND_PCM_STREAM_PLAYBACK, 0);
    if (rc < 0) {
        fprintf(stderr, "无法打开PCM设备: %s\n", snd_strerror(rc));
        fclose(input);
        return EXIT_FAILURE;
    }

    // 初始化硬件参数结构体
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(pcm_handle, params);

    // 设置访问模式、采样格式、声道数、采样率（与录制一致）
    rc = snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (rc < 0) goto cleanup;

    rc = snd_pcm_hw_params_set_format(pcm_handle, params, SAMPLE_FORMAT);
    if (rc < 0) goto cleanup;

    rc = snd_pcm_hw_params_set_channels(pcm_handle, params, CHANNELS);
    if (rc < 0) goto cleanup;

    rc = snd_pcm_hw_params_set_rate(pcm_handle, params, SAMPLE_RATE, 0);
    if (rc < 0) goto cleanup;

    // 应用硬件参数
    rc = snd_pcm_hw_params(pcm_handle, params);
    if (rc < 0) {
        fprintf(stderr, "应用硬件参数失败: %s\n", snd_strerror(rc));
        goto cleanup;
    }

    // 分配缓冲区
    buffer = (int16_t *)malloc(BUFFER_FRAMES * CHANNELS * sizeof(int16_t));
    if (!buffer) {
        fprintf(stderr, "内存分配失败\n");
        goto cleanup;
    }

    // 循环读取文件并播放
    while ((read_frames = fread(buffer, sizeof(int16_t) * CHANNELS, BUFFER_FRAMES, input)) > 0) {
        rc = snd_pcm_writei(pcm_handle, buffer, read_frames);
        if (rc == -EPIPE) {
            // 缓冲区下溢，重新准备设备
            fprintf(stderr, "缓冲区下溢，重新准备设备...\n");
            snd_pcm_prepare(pcm_handle);
            continue;
        } else if (rc < 0) {
            fprintf(stderr, "写入数据失败: %s\n", snd_strerror(rc));
            break;
        } else if (rc != read_frames) {
            fprintf(stderr, "短写：预期%d帧，实际写入%d帧\n", read_frames, rc);
        }
    }

    // 等待播放完成
    snd_pcm_drain(pcm_handle);

    // 释放资源
    free(buffer);
cleanup:
    fclose(input);
    snd_pcm_close(pcm_handle);

    printf("播放完成！\n");
    return EXIT_SUCCESS;
}
```

#### 编译与运行

```bash
# 编译（需链接alsa库）
arm-openwrt-linux-gcc alsa_play.c -I /home/zhiwan/t113/Tina-Linux/out/t113-zhiwan_v1/staging_dir/target/usr/include -L /home/zhiwan/t113/Tina-Linux/out/t113-zhiwan_v1/staging_dir/target/usr/lib/ -lasound -o alsaplay        
# 推送并运行
adb push alsaplay /root
# 开发板上命令
cd /root
./alsaplay
```
