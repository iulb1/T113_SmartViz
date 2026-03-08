/**
 * 线程安全环形缓冲区
 * 用于音频数据在多个线程间的传递
 */

#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <pthread.h>
#include <stdint.h>
#include <stddef.h>

// 环形缓冲区结构
typedef struct {
    int16_t *buffer;           // 数据缓冲区
    size_t size;               // 缓冲区大小（必须是2的幂次）
    volatile size_t write_pos;  // 写入位置
    volatile size_t read_pos;   // 读取位置
    volatile size_t count;      // 当前数据量
    pthread_mutex_t mutex;      // 互斥锁
    pthread_cond_t not_full;    // 不满条件
    pthread_cond_t not_empty;   // 不空条件
} ring_buffer_t;

// 初始化环形缓冲区
int ring_buffer_init(ring_buffer_t *rb, size_t size);

// 销毁环形缓冲区
void ring_buffer_destroy(ring_buffer_t *rb);

// 写入数据（阻塞）
size_t ring_buffer_write(ring_buffer_t *rb, const int16_t *data, size_t count);

// 读取数据（阻塞）
size_t ring_buffer_read(ring_buffer_t *rb, int16_t *data, size_t count);

// 获取可读数据量
size_t ring_buffer_readable(ring_buffer_t *rb);

// 获取可写空间
size_t ring_buffer_writable(ring_buffer_t *rb);

// 清空缓冲区
void ring_buffer_clear(ring_buffer_t *rb);

#endif // RING_BUFFER_H










