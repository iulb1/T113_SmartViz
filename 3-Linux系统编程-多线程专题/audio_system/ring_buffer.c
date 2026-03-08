/**
 * 线程安全环形缓冲区实现
 */

#include "ring_buffer.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// 初始化环形缓冲区，4096个样本数
int ring_buffer_init(ring_buffer_t *rb, size_t size) {
    // 确保size是2的幂次
    size_t actual_size = 1;
    while (actual_size < size) {
        actual_size <<= 1;
    }
    
    rb->buffer = (int16_t *)malloc(actual_size * sizeof(int16_t));
    if (!rb->buffer) {
        return -1;
    }
    
    rb->size = actual_size;
    rb->write_pos = 0;
    rb->read_pos = 0;
    rb->count = 0;
    
    // 线程同步相关：锁和条件变量的初始化
    pthread_mutex_init(&rb->mutex, NULL);
    pthread_cond_init(&rb->not_full, NULL);
    pthread_cond_init(&rb->not_empty, NULL);
    
    return 0;
}

// 销毁环形缓冲区
void ring_buffer_destroy(ring_buffer_t *rb) {
    if (rb->buffer) {
        free(rb->buffer);
        rb->buffer = NULL;
    }
    
    pthread_mutex_destroy(&rb->mutex);
    pthread_cond_destroy(&rb->not_full);
    pthread_cond_destroy(&rb->not_empty);
}

// 获取可读数据量
size_t ring_buffer_readable(ring_buffer_t *rb) {
    return rb->count;
}

// 获取可写空间

size_t ring_buffer_writable(ring_buffer_t *rb) {
    return rb->size - rb->count ;  
}

// 写入数据（阻塞）
size_t ring_buffer_write(ring_buffer_t *rb, const int16_t *data, size_t count) {
    pthread_mutex_lock(&rb->mutex);
    
    // 等待有足够空间
    while (ring_buffer_writable(rb) < count) {
        pthread_cond_wait(&rb->not_full, &rb->mutex);
    }
    
    // 写入数据,  加入-环形缓冲区的常见优化方式，在实时音频处理中能提升性能。
    size_t written = 0;
    size_t mask = rb->size - 1;  // 位运算优化 2048 = 0b100000000000，2047 = 0b011111111111（11 个 1）
    
    for (size_t i = 0; i < count; i++) {
        rb->buffer[rb->write_pos] = data[i];
        rb->write_pos = (rb->write_pos + 1) & mask; //此时 & mask 等价于 % size. 性能：位运算 & 比取模 % 快
        written++;
    }
    
    rb->count += written;
    
    // 通知读取线程
    pthread_cond_signal(&rb->not_empty);
    pthread_mutex_unlock(&rb->mutex);
    
    return written;
}

// 读取数据（阻塞）
size_t ring_buffer_read(ring_buffer_t *rb, int16_t *data, size_t count) {
    pthread_mutex_lock(&rb->mutex);
    
    // 等待有足够数据
    while (rb->count < count) {
        pthread_cond_wait(&rb->not_empty, &rb->mutex);
    }
    
    // 读取数据
    size_t read = 0;
    size_t mask = rb->size - 1;  // 位运算优化
    
    for (size_t i = 0; i < count; i++) {
        data[i] = rb->buffer[rb->read_pos];
        rb->read_pos = (rb->read_pos + 1) & mask;
        read++;
    }
    
    rb->count -= read;
    
    // 通知写入线程
    pthread_cond_signal(&rb->not_full);
    pthread_mutex_unlock(&rb->mutex);
    
    return read;
}

// 清空缓冲区
void ring_buffer_clear(ring_buffer_t *rb) {
    pthread_mutex_lock(&rb->mutex);
    rb->write_pos = 0;
    rb->read_pos = 0;
    rb->count = 0;
    pthread_cond_broadcast(&rb->not_full);
    pthread_mutex_unlock(&rb->mutex);
}









