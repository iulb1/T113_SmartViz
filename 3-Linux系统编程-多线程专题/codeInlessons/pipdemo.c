	
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>


int pipefd[2];

//void ( *signal(int signum, void (*handler)(int)) ) (int);
void sigpipe_handler(int signum)
{
	printf("捕捉到SIGPIPE，此情况是管道读取端关闭\n");
	exit(0);
}

void *worker_thread(void *arg) {

	char buf[32];
	int i = 0;
	int readnum  = -1;
	//读取管道中的数据
	//ssize_t read(int fd, void *buf, size_t count);
	while(i<10){
		
		memset(buf, 0, sizeof(buf));
		readnum = read(pipefd[0], buf, sizeof(buf));
		if(readnum > 0){	
	
			printf("%d次从管道去读%d个数据:%s\n",++i,readnum, buf);
		}
		else if(readnum == 0){

			printf("写端关闭，读端读取完缓冲区中的数据后会返回EOF（0字节），表示没有更多数据\n");
			close(pipefd[0]);
			pthread_exit(0);
		} 
	}
}

void *control_thread(void *arg) {
	//往管道中写入数据
	char buf[32];
	memset(buf, 0 ,sizeof(buf));

	//ssize_t write(int fd, const void *buf, size_t count);
	int i = 0;
	while(i < 3){
		sprintf(buf, "hello%d",++i);
		write(pipefd[1], buf, strlen(buf));
		sleep(1);
	}
	sleep(3);
	printf("关闭写段，将会看到读取端的EOF异常\n");
	write(pipefd[1], "end and quit", strlen("end and quit"));
	
//	close(pipefd[1]);

	pipe(pipefd);
	sleep(1);
	printf("直接关闭读端\n");
	close(pipefd[0]);
	sleep(1);
	int nwrite = write(pipefd[1],"test",strlen("test"));
	printf("nwrite = %d\n",nwrite);
}


int main() {
    pthread_t worker, control;
   
	/*
	#include <signal.h>

       typedef void (*sighandler_t)(int);

       sighandler_t signal(int signum, sighandler_t handler);

	*/ 
	signal(SIGPIPE, sigpipe_handler);
    // 创建管道
   	int ret = pipe(pipefd);
 
    printf("=== 管道命令传递测试 ===\n\n");
    
    // 创建工作线程
    pthread_create(&worker, NULL, worker_thread, NULL);
    
    // 创建控制线程
    pthread_create(&control, NULL, control_thread, NULL);
    
    // 等待线程完成
    pthread_join(control, NULL);
    pthread_join(worker, NULL);
    
    // 关闭管道
   	close(pipefd[0]); 
   	close(pipefd[1]); 
    printf("\n测试完成！管道实现了线程间的命令传递\n");
    return 0;


}
