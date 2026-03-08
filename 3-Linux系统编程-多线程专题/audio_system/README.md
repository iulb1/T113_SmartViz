# 音频处理系统

这是一个完整的多线程音频处理系统，实现了录制 → 处理 → 播放的完整流程。

## 架构

```
录制线程 → 环形缓冲区1 → 处理线程 → 环形缓冲区2 → 播放线程
```

## 编译

```bash
make
```

## 运行

```bash
./audio_system
```

按 Ctrl+C 退出。

## 模块说明

- `ring_buffer.h/c`: 线程安全环形缓冲区
- `audio_record.h/c`: 音频录制模块
- `audio_process.h/c`: 音频处理模块（增益、降噪）
- `audio_playback.h/c`: 音频播放模块
- `main.c`: 主程序

## 依赖

- ALSA库（libasound2-dev）
- pthread库

## 配置

可以在 `main.c` 中修改以下参数：
- 采样率
- 声道数
- 缓冲区大小
- 增益系数
- 降噪开关










