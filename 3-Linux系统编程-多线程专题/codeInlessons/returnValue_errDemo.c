#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

// 方式2：传递结构体参数
typedef struct {
    int thread_id;
    char name[32];
    int count;
} thread_param_t;


// 方式1：传递整数参数
void* thread_func_int(void *params)
{
    int thread_id = *(int *)params;  // 将void*转换为int*
    
//	int data = *(int *)params;

	int i = 0;
    for(i = 0; i < 3; ++i){
        printf("线程%d打印第%d次\n", thread_id, i+1);
        sleep(1);
    }
	//这里注意段错误，因为data是局部变量，线程结束后，内存空间就要被释放
	//局部变量内存在栈里面，随着函数调用结束，栈空间释放，而我们返回了一个"将会不存在的内存空间"
	int data = 100;
	data = data + thread_id;
    return (void *)&data;
}

void* thread_func_struct(void *params)
{
    thread_param_t *param = (thread_param_t *)params;
    int i = 0;
    printf("线程%d (%s) 开始工作\n", param->thread_id, param->name);
    for(i = 0; i < param->count; ++i){
        printf("线程%d (%s) 打印第%d次\n", param->thread_id, param->name, i+1);
        sleep(1);
    }
    return NULL;
}

int main()
{
    pthread_t t1, t2, t3;
    int ret = -1;
    int *returnValue1;
    int *returnValue2;

	char data = 'a';
    // 方式1：传递整数
    int thread_id1 = 1;
    int thread_id2 = 2;
    ret = pthread_create(&t1, NULL, thread_func_int, &thread_id1);
    ret = pthread_create(&t2, NULL, thread_func_int, &thread_id2);
    
    pthread_join(t1, (void **)&returnValue1);
    pthread_join(t2, (void **)&returnValue2);

	printf("线程1结束：主线程打印返回值：%d\n",*returnValue1);//对一个非法地址访问，因为线程返回的地址在线程结束后被释放
	printf("线程2结束：主线程打印返回值：%d\n",*returnValue2);

// 方式2：传递结构体
/*    thread_param_t param1 = {1, "录制线程", 3};
    thread_param_t param2 = {2, "播放线程", 6};
    ret = pthread_create(&t1, NULL, thread_func_struct, &param1);
    ret = pthread_create(&t2, NULL, thread_func_struct, &param2);

	pthread_join(t1, NULL);
    pthread_join(t2, NULL);
*/
	return 0;
}
