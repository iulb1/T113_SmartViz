#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

// 递归锁
pthread_mutex_t g_mutex;

// 共享资源
int g_resource = 0;

// 函数B：可能被函数A调用，也需要访问资源
void function_b(int depth)
{
    printf("  function_b(深度%d): 尝试加锁...\n", depth);
    pthread_mutex_lock(&g_mutex);  // 递归加锁，不会死锁
    
    printf("  function_b(深度%d): ✓ 获得锁\n", depth);
    g_resource += depth;
    printf("  function_b(深度%d): 访问资源，resource=%d\n", depth, g_resource);
    
    usleep(200000);  // 200毫秒
    
    pthread_mutex_unlock(&g_mutex);
    printf("  function_b(深度%d): 释放锁\n", depth);
}

// 函数A：调用函数B，两者都需要访问同一资源
void function_a(int depth)
{
    printf("function_a(深度%d): 尝试加锁...\n", depth);
    pthread_mutex_lock(&g_mutex);  // 第一次加锁
    
    printf("function_a(深度%d): ✓ 获得锁\n", depth);
    g_resource += depth * 10;
    printf("function_a(深度%d): 访问资源，resource=%d\n", depth, g_resource);
    
    // 调用函数B，B内部也需要加锁
    if(depth > 0){
        function_b(depth - 1);  // 递归调用
    }
    
    usleep(200000);  // 200毫秒
    
    pthread_mutex_unlock(&g_mutex);
    printf("function_a(深度%d): 释放锁\n", depth);
}

// 线程函数
void* thread_func(void *arg)
{
    printf("线程开始工作\n");
    
    // 调用函数A，A会调用B，两者都需要加锁
    function_a(2);
    
    printf("线程完成，最终resource=%d\n", g_resource);
    return NULL;
}

int main()
{
    pthread_t t1;
    pthread_mutexattr_t attr;
    int ret;
    
    printf("=== 递归锁（PTHREAD_MUTEX_RECURSIVE）演示 ===\n\n");
    printf("说明：\n");
    printf("1. 使用递归锁，允许同一线程多次加锁\n");
    printf("2. 函数A调用函数B，两者都需要访问同一资源\n");
    printf("3. 递归锁可以避免死锁\n");
    printf("4. 每次加锁计数+1，解锁计数-1，计数为0时真正释放锁\n\n");
    
    // 初始化递归锁
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    ret = pthread_mutex_init(&g_mutex, &attr);
    if(ret != 0){
        fprintf(stderr, "初始化递归锁失败: %s\n", strerror(ret));
        pthread_mutexattr_destroy(&attr);
        return 1;
    }
    pthread_mutexattr_destroy(&attr);
    printf("递归锁初始化成功\n\n");
    
    ret = pthread_create(&t1, NULL, thread_func, NULL);
    if(ret != 0){
        fprintf(stderr, "创建线程失败: %s\n", strerror(ret));
        pthread_mutex_destroy(&g_mutex);
        return 1;
    }
    
    pthread_join(t1, NULL);
    
    printf("\n=== 执行完成 ===\n");
    printf("最终resource值: %d\n", g_resource);
    printf("\n递归锁特点：\n");
    printf("- 同一线程可以多次加锁，不会死锁\n");
    printf("- 适合函数可能被递归调用的场景\n");
    printf("- 适合复杂模块化代码\n");
    printf("- 性能略低于普通锁\n");
    
    pthread_mutex_destroy(&g_mutex);
    return 0;
}
