#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

// 线程函数
void* thread_func(void *params)
{
    int thread_id = *(int *)params;
    int i = 0;
    
    printf("线程%d开始执行\n", thread_id);
    
    // 获取线程ID
    pthread_t tid = pthread_self();
    printf("线程%d的ID: %lu\n", thread_id, (unsigned long)tid);
    
    for(i = 0; i < 3; ++i){
        printf("线程%d: 工作第%d次\n", thread_id, i+1);
        sleep(1);
    }
    
    printf("线程%d执行完成\n", thread_id);
    return NULL;
}


int main()
{
	pthread_t t1, t2;
	pthread_attr_t attr1, attr2;

	int ret = -1;
    int thread_id1 = 1;
    int thread_id2 = 2;

    size_t stacksize = 0;
    
    printf("=== 线程属性设置演示 ===\n\n");
    
    // ========== 获取默认栈大小 ==========
	//int pthread_attr_init(pthread_attr_t *attr);
    pthread_attr_init(&attr1);
    ret = pthread_attr_getstacksize(&attr1, &stacksize);
    if(ret == 0){
        printf("默认栈大小: %zu 字节 (%.2f MB)\n", stacksize, stacksize / (1024.0 * 1024.0));
    }
    printf("\n");
    
    // ========== 线程1：设置自定义栈大小 ==========
    // 设置栈大小为1MB
    stacksize = 1024 * 1024;  // 1MB
    ret = pthread_attr_setstacksize(&attr1, stacksize);
    if(ret == 0){
        printf("线程1: 栈大小设置为 %zu 字节 (1MB)\n", stacksize);
    }
    

    ret = pthread_attr_getstacksize(&attr1, &stacksize);
    if(ret == 0){
        printf("修改后栈大小: %zu 字节 (%.2f MB)\n", stacksize, stacksize / (1024.0 * 1024.0));
    }
    printf("\n");

	  // 创建线程1（使用自定义栈大小）
    ret = pthread_create(&t1, &attr1, thread_func, &thread_id1);
    if(ret == 0){
        printf("线程1创建成功\n");
    }    

	  // ========== 线程2：设置为分离状态 ==========
    pthread_attr_init(&attr2);
    
    // 设置为分离状态（不需要join）
    ret = pthread_attr_setdetachstate(&attr2, PTHREAD_CREATE_DETACHED);
    if(ret == 0){
        printf("线程2: 设置为分离状态（自动清理）\n");
    }
    
    // 创建线程2（分离线程）
    ret = pthread_create(&t2, &attr2, thread_func, &thread_id2);
    if(ret == 0){
        printf("线程2创建成功\n");
    }
    
    printf("\n主线程: 等待线程1完成...\n");
    // 等待线程1（线程1是可joinable的）
    pthread_join(t1, NULL);
    printf("线程1已退出\n");
    
    // 线程2是分离的，不需要join，会自动清理
    printf("主线程: 等待3秒，观察分离线程2的输出...\n");
    sleep(3);
    
    // 清理属性对象
    pthread_attr_destroy(&attr1);
    pthread_attr_destroy(&attr2);
    
    printf("主线程退出\n");


	return 0;
}
