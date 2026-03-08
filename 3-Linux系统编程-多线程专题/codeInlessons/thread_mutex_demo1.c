#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

//定一个全局互斥锁
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;//使用静态初始化的方式

int g_shareData = 0;

void* thread2_func(void *params)
{
    int i = 0;
    int thread2_data = 100;
    static int s_data  = 10;
    printf("thread2变量是我的局部变量：%d\n",thread2_data);
    printf("静态局部变量s_data：%d\n",s_data);
    for(i = 0; i < 3; ++i){
        //`int pthread_mutex_lock(pthread_mutex_t *mutex);`
        pthread_mutex_lock(&mutex);
        g_shareData++;
        printf("这是线程2打印第%d次,模拟播放线程,g_shareData地址%p:数据：%d\n",i+1,&g_shareData,g_shareData);
        //int pthread_mutex_unlock(pthread_mutex_t *mutex);
        pthread_mutex_unlock(&mutex);
        sleep(1);
    }
}

void* thread1_func(void *params)
{
    int i = 0;
    //printf("thread2变量是我的局部变量：%d\n",thread2_data);
    // printf("静态局部变量s_data：%d\n",s_data);
    for(i = 0; i < 3; ++i){
        pthread_mutex_lock(&mutex);
        g_shareData++;
        printf("这是线程1打印第%d次，模拟录制线程,g_shareData地址%p:,数据%d\n",i+1,&g_shareData,g_shareData);
        pthread_mutex_unlock(&mutex);
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

    //初始化互斥锁
    // 初始化互斥锁
    //int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
	//动态初始化一个互斥锁
    //pthread_mutex_init(&mutex, NULL);

    //2 Linux如何创建并启动一个线程
    //int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);
    ret = pthread_create(&t1,NULL,thread1_func,NULL);
    ret = pthread_create(&t2,NULL,thread2_func,NULL);
    if(ret == 0){
        printf("线程创建成功\n");
    }
	while(1){
        pthread_mutex_lock(&mutex);
        g_shareData++;
        printf("这是主线程打印第%d次，模拟UI线程,g_shareData地址%p: 数据%d:\n",a++,&g_shareData,g_shareData);
        pthread_mutex_unlock(&mutex);
		sleep(1);	
        
	}
	//pthread_mutex_destroy(&mutex);
    //等待线程退出int pthread_join(pthread_t thread, void **retval);
    pthread_join(t1, NULL);
    return 0;
}
