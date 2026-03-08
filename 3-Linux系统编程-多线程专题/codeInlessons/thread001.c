#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void* thread2_func(void *params)
{
    int i = 0;
    for(i = 0; i < 3; ++i){
        printf("这是线程2打印第%d次,模拟播放线程\n",i+1);
        sleep(1);
    }
}

void* thread1_func(void *params)
{
    int i = 0;
    for(i = 0; i < 3; ++i){
        printf("这是线程1打印第%d次，模拟录制线程\n",i+1);
        sleep(1);
    }
}

int main()
{
    //1 Linux如何表示一个线程
    int a = 1;
    int b;
    char c;
    double d;
    int ret = -1;

    pthread_t t1;
    pthread_t t2;


    //2 Linux如何创建并启动一个线程
    //int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);
    ret = pthread_create(&t1,NULL,thread1_func,NULL);
    ret = pthread_create(&t2,NULL,thread2_func,NULL);
    if(ret == 0){
        printf("线程创建成功\n");
    }
	while(1){
        printf("这是主线程打印第%d次，模拟UI线程\n",a++);
		sleep(1);	
	}
    //等待线程退出int pthread_join(pthread_t thread, void **retval);
    pthread_join(t1, NULL);
    return 0;
}
