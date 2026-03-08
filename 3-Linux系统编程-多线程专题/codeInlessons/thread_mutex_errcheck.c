#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

// 检错锁
pthread_mutex_t g_mutex;

// 共享资源
int g_counter = 0;

// 线程函数：演示检错锁的错误检测
void* thread_func(void *arg)
{
    int thread_id = *(int *)arg;
    int ret;
    
    printf("线程%d开始工作\n", thread_id);
    
    // 第一次加锁
    
    printf("线程%d: 第一次加锁...\n", thread_id);
    if(thread_id == 2){
         ret = pthread_mutex_unlock(&g_mutex);
         if(ret == EPERM)
        {
        	printf("无权解锁ret = %d\n",ret);
    	}
    }
    ret = pthread_mutex_lock(&g_mutex);
    if(ret != 0){
        fprintf(stderr, "线程%d加锁失败: %s\n", thread_id, strerror(ret));
        
        return NULL;
    }
    printf("线程%d: ✓ 获得锁\n", thread_id);
    
    g_counter++;
    printf("线程%d: 访问资源，counter=%d\n", thread_id, g_counter);
    
    // 尝试第二次加锁（错误操作）
    if(thread_id == 1){
        printf("线程%d: 尝试第二次加锁（错误操作）...\n", thread_id);
        ret = pthread_mutex_lock(&g_mutex);
        if(ret == EDEADLK){
            printf("线程%d: ✗ 检测到死锁风险！错误码=EDEADLK\n", thread_id);
            printf("线程%d: 同一线程重复加锁会导致死锁\n", thread_id);
          
        } else if(ret != 0){
            printf("线程%d: ✗ 加锁失败，错误码=%d: %s\n", thread_id, ret, strerror(ret));
        } else {
            printf("线程%d: ✓ 第二次加锁成功（不应该发生）\n", thread_id);
            pthread_mutex_unlock(&g_mutex);
        }
    }
    
    usleep(500000);  // 500毫秒
    
    // 解锁
    printf("线程%d: 释放锁\n", thread_id);
    ret = pthread_mutex_unlock(&g_mutex);
    if(ret != 0){
        fprintf(stderr, "线程%d解锁失败: %s\n", thread_id, strerror(ret));
    }
    
    printf("线程%d完成\n", thread_id);
    return NULL;
}

int main()
{
    pthread_t t1, t2;
    int thread_id1 = 1;
    int thread_id2 = 2;
    pthread_mutexattr_t attr;
    int ret;
    
    printf("=== 检错锁（PTHREAD_MUTEX_ERRORCHECK）演示 ===\n\n");
    printf("说明：\n");
    printf("1. 使用检错锁，可以检测锁使用错误\n");
    printf("2. 同一线程重复加锁会返回EDEADLK错误\n");
    printf("3. 其他线程对已加锁的锁解锁会返回EPERM错误\n");
    printf("4. 适合开发和调试阶段使用\n\n");
    
    // 初始化检错锁
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    ret = pthread_mutex_init(&g_mutex, &attr);
    if(ret != 0){
        fprintf(stderr, "初始化检错锁失败: %s\n", strerror(ret));
        pthread_mutexattr_destroy(&attr);
        return 1;
    }
    pthread_mutexattr_destroy(&attr);
    printf("检错锁初始化成功\n\n");
    
    ret = pthread_create(&t1, NULL, thread_func, &thread_id1);
    if(ret != 0){
        fprintf(stderr, "创建线程1失败: %s\n", strerror(ret));
        pthread_mutex_destroy(&g_mutex);
        return 1;
    }
    
    usleep(100000);  // 100毫秒，让线程1先执行
    
    ret = pthread_create(&t2, NULL, thread_func, &thread_id2);
    if(ret != 0){
        fprintf(stderr, "创建线程2失败: %s\n", strerror(ret));
        pthread_join(t1, NULL);
        pthread_mutex_destroy(&g_mutex);
        return 1;
    }
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    
    printf("\n=== 执行完成 ===\n");
    printf("最终counter值: %d\n", g_counter);
    printf("\n检错锁特点：\n");
    printf("- 检测错误操作，帮助发现锁使用错误\n");
    printf("- 同一线程重复加锁返回EDEADLK错误\n");
    printf("- 其他线程错误解锁返回EPERM错误\n");
    printf("- 适合开发和调试阶段\n");
    printf("- 生产环境可切换为普通锁以提高性能\n");
    
    pthread_mutex_destroy(&g_mutex);
    return 0;
}
