#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

// 两个互斥锁
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

// 共享资源
int resource1 = 0;
int resource2 = 0;

// 线程1：先获取mutex1，再获取mutex2
void* thread1_func(void *arg)
{
    int thread_id = *(int *)arg;
    
    printf("线程%d: 尝试获取mutex1...\n", thread_id);
    pthread_mutex_lock(&mutex1);
    printf("线程%d: ✓ 获得mutex1，持有资源1\n", thread_id);
    resource1 = 100;
    
    // 模拟一些操作
    sleep(1);
    
    printf("线程%d: 尝试获取mutex2...\n", thread_id);
    pthread_mutex_lock(&mutex2);  // 如果线程2已经持有mutex2，这里会阻塞
    printf("线程%d: ✓ 获得mutex2，持有资源2\n", thread_id);
    resource2 = 200;
    
    printf("线程%d: 访问资源1=%d, 资源2=%d\n", thread_id, resource1, resource2);
    
    pthread_mutex_unlock(&mutex2);
    pthread_mutex_unlock(&mutex1);
    
    printf("线程%d: 释放所有锁，完成\n", thread_id);
    return NULL;
}

// 线程2：先获取mutex2，再获取mutex1（与线程1顺序相反，导致死锁）
void* thread2_func(void *arg)
{
    int thread_id = *(int *)arg;
    
    printf("线程%d: 尝试获取mutex2...\n", thread_id);
    pthread_mutex_lock(&mutex2);
    printf("线程%d: ✓ 获得mutex2，持有资源2\n", thread_id);
    resource2 = 300;
    
    // 模拟一些操作
    sleep(1);
    
    printf("线程%d: 尝试获取mutex1...\n", thread_id);
    pthread_mutex_lock(&mutex1);  // 如果线程1已经持有mutex1，这里会阻塞
    printf("线程%d: ✓ 获得mutex1，持有资源1\n", thread_id);
    resource1 = 400;
    
    printf("线程%d: 访问资源1=%d, 资源2=%d\n", thread_id, resource1, resource2);
    
    pthread_mutex_unlock(&mutex1);
    pthread_mutex_unlock(&mutex2);
    
    printf("线程%d: 释放所有锁，完成\n", thread_id);
    return NULL;
}

int main()
{
    pthread_t t1, t2;
    int thread_id1 = 1;
    int thread_id2 = 2;
    int ret;
    
    printf("=== 死锁演示案例 ===\n\n");
    printf("说明：\n");
    printf("1. 线程1先获取mutex1，再获取mutex2\n");
    printf("2. 线程2先获取mutex2，再获取mutex1\n");
    printf("3. 如果两个线程同时执行，会导致死锁\n");
    printf("4. 程序会在这里挂起，需要手动终止（Ctrl+C）\n\n");
    
    ret = pthread_create(&t1, NULL, thread1_func, &thread_id1);
    if(ret != 0){
        fprintf(stderr, "创建线程1失败: %s\n", strerror(ret));
        return 1;
    }
    
    // 稍微延迟，让两个线程几乎同时开始
    usleep(100000);  // 100毫秒
    
    ret = pthread_create(&t2, NULL, thread2_func, &thread_id2);
    if(ret != 0){
        fprintf(stderr, "创建线程2失败: %s\n", strerror(ret));
        pthread_join(t1, NULL);
        return 1;
    }
    
    printf("所有线程已创建，开始执行...\n");
    printf("观察：程序可能会因为死锁而挂起\n\n");
    
    // 设置超时，避免程序永远挂起
    sleep(5);
    printf("\n如果程序还在运行，说明可能发生了死锁\n");
    printf("解决死锁的方法：\n");
    printf("1. 所有线程按相同顺序获取锁（先mutex1，再mutex2）\n");
    printf("2. 使用pthread_mutex_trylock()尝试加锁，失败时释放已持有的锁\n");
    printf("3. 使用超时机制，避免无限等待\n");
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    
    return 0;
}
