#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void* thread_func_detached(void *params)
{
    int thread_id = *(int *)params;
    int i = 0;
    
    printf("分离线程%d开始工作\n", thread_id);
    
    for(i = 0; i < 5; ++i){
        printf("分离线程%d: 工作第%d次\n", thread_id, i+1);
        sleep(1);
    }
    
    printf("分离线程%d完成，将自动清理资源\n", thread_id);
    return NULL;
}

int main()
{
    pthread_t t1, t2;
    int ret = -1;
    int thread_id1 = 1;
    int thread_id2 = 2;
    
    // 创建线程
    ret = pthread_create(&t1, NULL, thread_func_detached, &thread_id1);
    ret = pthread_create(&t2, NULL, thread_func_detached, &thread_id2);
    
    if(ret == 0){
 //       printf("线程创建成功\n");
    }
    
    // 设置为分离状态
    pthread_detach(t1);
    pthread_detach(t2);
//    printf("线程1和线程2已设置为分离状态，无需join\n");
    
    // 分离线程不需要join，会自动清理
//    printf("主线程等待3秒，观察分离线程的输出...\n");
//    sleep(3);
    
    printf("主线程退出\n");
//    printf("注意：分离线程会在后台继续运行直到完成\n");
//    return 0;
	pthread_exit(0);//让主线程退出，实际上进程没有结束
}
