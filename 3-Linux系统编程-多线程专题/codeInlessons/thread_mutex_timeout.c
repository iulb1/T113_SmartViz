#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>

// 两个互斥锁
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

// 共享资源
int resource1 = 0;
int resource2 = 0;

// 计算超时时间（从现在开始，等待timeout_seconds秒）
void calculate_timeout(struct timespec *timeout, int timeout_seconds)
{
    clock_gettime(CLOCK_REALTIME, timeout);
    timeout->tv_sec += timeout_seconds;
}

// 线程1：使用超时机制获取锁
void* thread1_func(void *arg)
{
    int thread_id = *(int *)arg;
    struct timespec timeout;
    int ret;
    
    printf("线程%d开始工作\n", thread_id);
    
    // 获取第一个锁
    printf("线程%d: 尝试获取mutex1...\n", thread_id);
    ret = pthread_mutex_lock(&mutex1);
    if(ret != 0){
        fprintf(stderr, "线程%d获取mutex1失败: %s\n", thread_id, strerror(ret));
        return NULL;
    }
    printf("线程%d: ✓ 获得mutex1\n", thread_id);
    resource1 = 100;
    
    // 模拟一些操作
    sleep(1);
    
    // 使用超时机制获取第二个锁（超时时间3秒）
    printf("线程%d: 尝试获取mutex2（超时3秒）...\n", thread_id);
    calculate_timeout(&timeout, 3);  // 设置3秒超时
    
    ret = pthread_mutex_timedlock(&mutex2, &timeout);
    if(ret == ETIMEDOUT){
        printf("线程%d: ✗ 获取mutex2超时！释放mutex1避免死锁\n", thread_id);
        pthread_mutex_unlock(&mutex1);  // 释放已持有的锁，避免死锁
        printf("线程%d: 已释放mutex1，退出\n", thread_id);
        return NULL;
    } else if(ret != 0){
        fprintf(stderr, "线程%d获取mutex2失败: %s\n", thread_id, strerror(ret));
        pthread_mutex_unlock(&mutex1);
        return NULL;
    }
    
    printf("线程%d: ✓ 获得mutex2\n", thread_id);
    resource2 = 200;
    
    printf("线程%d: 访问资源1=%d, 资源2=%d\n", thread_id, resource1, resource2);
    
    pthread_mutex_unlock(&mutex2);
    pthread_mutex_unlock(&mutex1);
    
    printf("线程%d: 释放所有锁，完成\n", thread_id);
    return NULL;
}

// 线程2：长时间持有mutex2
void* thread2_func(void *arg)
{
    int thread_id = *(int *)arg;
    
    printf("线程%d开始工作\n", thread_id);
    
    printf("线程%d: 尝试获取mutex2...\n", thread_id);
    pthread_mutex_lock(&mutex2);
    printf("线程%d: ✓ 获得mutex2，将持有5秒\n", thread_id);
    resource2 = 300;
    
    // 长时间持有锁（5秒），让线程1超时
    sleep(2);
    
    printf("线程%d: 释放mutex2\n", thread_id);
    pthread_mutex_unlock(&mutex2);
    
    printf("线程%d完成\n", thread_id);
    return NULL;
}

int main()
{
    pthread_t t1, t2;
    int thread_id1 = 1;
    int thread_id2 = 2;
    int ret;
    
    printf("=== 超时机制（pthread_mutex_timedlock）演示 ===\n\n");
    printf("说明：\n");
    printf("1. 线程1使用pthread_mutex_timedlock()获取锁，超时时间3秒\n");
    printf("2. 线程2长时间持有mutex2（5秒）\n");
    printf("3. 线程1在3秒后超时，自动释放已持有的mutex1，避免死锁\n");
    printf("4. 超时机制可以防止程序无限等待\n\n");
    
    ret = pthread_create(&t1, NULL, thread1_func, &thread_id1);
    if(ret != 0){
        fprintf(stderr, "创建线程1失败: %s\n", strerror(ret));
        return 1;
    }
    
    usleep(100000);  // 100毫秒，让线程1先执行
    
    ret = pthread_create(&t2, NULL, thread2_func, &thread_id2);
    if(ret != 0){
        fprintf(stderr, "创建线程2失败: %s\n", strerror(ret));
        pthread_join(t1, NULL);
        return 1;
    }
    
    printf("所有线程已创建，开始执行...\n\n");
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    
    printf("\n=== 执行完成 ===\n");
    printf("超时机制的优势：\n");
    printf("- 避免无限等待，防止死锁\n");
    printf("- 超时后可以释放已持有的锁，避免资源占用\n");
    printf("- 适合对响应时间有要求的场景\n");
    printf("\n使用pthread_mutex_timedlock()的要点：\n");
    printf("- 超时时间使用绝对时间（struct timespec）\n");
    printf("- 使用CLOCK_REALTIME获取当前时间\n");
    printf("- 超时返回ETIMEDOUT错误码\n");
    printf("- 超时后应该释放已持有的锁，避免死锁\n");
    
    return 0;
}
