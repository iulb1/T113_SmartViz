#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

pthread_cond_t g_cond;
pthread_mutex_t g_mutex;
int g_data = 0;
int g_ready = 0;

void* wait_thread(void *arg) {
    int thread_id = *(int *)arg;
    struct timespec timeout;
    int ret;
    
    printf("线程 %d: 使用超时等待条件...\n", thread_id);
    
    pthread_mutex_lock(&g_mutex);
    
    // 设置超时时间（3秒后）
    clock_gettime(CLOCK_MONOTONIC, &timeout);
    timeout.tv_sec += 3;
    
    while (g_ready == 0) {
        ret = pthread_cond_timedwait(&g_cond, &g_mutex, &timeout);
        if (ret == ETIMEDOUT) {
            printf("线程 %d: 等待超时\n", thread_id);
            pthread_mutex_unlock(&g_mutex);
            return NULL;
        }
    }
    
    printf("线程 %d: 条件满足，data = %d\n", thread_id, g_data);
    pthread_mutex_unlock(&g_mutex);
    
    return NULL;
}

int main() {
    pthread_t thread;
    int thread_id = 1;
    int ret;
    pthread_condattr_t attr;
    
    printf("=== 动态初始化示例（时钟属性） ===\n");
    
    // 初始化互斥锁
    pthread_mutex_init(&g_mutex, NULL);
    
    // 初始化条件变量属性
    ret = pthread_condattr_init(&attr);
    if (ret != 0) {
        fprintf(stderr, "初始化属性失败: %s\n", strerror(ret));
        return EXIT_FAILURE;
    }
 
    ret = pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    if (ret == 0) {
        printf("设置时钟类型为 CLOCK_MONOTONIC\n");
    } else {
        printf("系统不支持设置时钟类型，使用默认 CLOCK_REALTIME\n");
    }
   
    // 使用属性初始化条件变量
    ret = pthread_cond_init(&g_cond, &attr);
    if (ret != 0) {
        fprintf(stderr, "初始化条件变量失败: %s\n", strerror(ret));
        pthread_condattr_destroy(&attr);
        return EXIT_FAILURE;
    }
    
    // 属性不再需要，可以销毁
    pthread_condattr_destroy(&attr);
    printf("动态初始化完成（使用时钟属性）\n\n");
    
    // 创建等待线程
    pthread_create(&thread, NULL, wait_thread, &thread_id);
    
    // 主线程等待1秒后触发条件
    sleep(1);
    
    pthread_mutex_lock(&g_mutex);
    g_data = 400;
    g_ready = 1;
    printf("主线程: 设置 data = %d，通知等待线程\n", g_data);
    pthread_cond_signal(&g_cond);
    pthread_mutex_unlock(&g_mutex);
    
    // 等待线程完成
    pthread_join(thread, NULL);
    
    // 清理资源
    printf("\n清理资源...\n");
    pthread_cond_destroy(&g_cond);
    pthread_mutex_destroy(&g_mutex);
    printf("资源清理完成\n");
    
    return 0;
}
