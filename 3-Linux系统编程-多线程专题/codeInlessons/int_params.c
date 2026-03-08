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
    return NULL;
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
    

	char data = 'a';
    // 方式1：传递整数
    int thread_id1 = 1;
    int thread_id2 = 2;
    ret = pthread_create(&t1, NULL, thread_func_int, &thread_id1);
    ret = pthread_create(&t2, NULL, thread_func_int, &thread_id2);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

// 方式2：传递结构体
    thread_param_t param1 = {1, "录制线程", 3};
    thread_param_t param2 = {2, "播放线程", 6};
    ret = pthread_create(&t1, NULL, thread_func_struct, &param1);
    ret = pthread_create(&t2, NULL, thread_func_struct, &param2);

	pthread_join(t1, NULL);
    pthread_join(t2, NULL);

	return 0;
}
