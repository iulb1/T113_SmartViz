#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>


#define FIFO_PATH "./fifo_test"

// 写入线程
void *writer_thread(void *arg) {
	
	int write_count = 0;

	char buf[32];
	//打开管道。以只写方式打开
	int fd;
  // 以只写方式打开命名管道（会阻塞直到有读端打开）
    printf("[写线程] 打开命名管道（只写模式）...\n");
    fd = open(FIFO_PATH, O_WRONLY);
    if (fd == -1) {
        perror("[写线程] 打开命名管道失败");
        unlink(FIFO_PATH);  // 清理
        return NULL;
    }
	printf("写线程打开管道成功\n");
	//循环往管道中写入数据
	// 写入数据
    for (int i = 1; i <= 2; i++) {
        memset(buf, 0, sizeof(buf));
        snprintf(buf, sizeof(buf), "消息%d来自写线程", i);
        
        ssize_t n = write(fd, buf, strlen(buf));
        if (n > 0) {
            printf("[写线程] 第%d次写入，发送 %ld 字节: %s\n", 
                   ++write_count, n, buf);
        } else {
            perror("[写线程] 写入错误");
            break;
        }
        
        usleep(300000);
    }
    
    printf("[写线程] 写入完成，关闭写端\n");
    close(fd);
    

	//关闭管道

}

// 读取线程
void *reader_thread(void *arg) {
	
	int read_count = 0;
	char buf[32];

	//打开管道，以只读的方式打开
	int fd;
	 // 以只读方式打开命名管道（会阻塞直到有写端打开）
    printf("[读线程] 打开命名管道（只读模式）...\n");
    fd = open(FIFO_PATH, O_RDONLY);
    if (fd == -1) {
        perror("[读线程] 打开命名管道失败");
        return NULL;
    }
	printf("读线程打开管道成功\n");
	//循环读取管道中的数据
    while (read_count < 5) {
        memset(buf, 0, sizeof(buf));
        ssize_t n = read(fd, buf, sizeof(buf) - 1);
        
        if (n > 0) {
            printf("[读线程] 第%d次读取，收到 %ld 字节: %s\n", 
                   ++read_count, n, buf);
        } else if (n == 0) {
            // EOF: 写端已关闭
            printf("[读线程] 收到EOF，写端已关闭\n");
            break;
        } else {
            perror("[读线程] 读取错误");
            break;
        }
        
        usleep(200000);
    }
    
    close(fd);


}

int main()
{
	int ret;

	pthread_t reader, writer;
	//int mkfifo(const char *pathname, mode_t mode);
	
/*	ret = mkfifo(FIFO_PATH, 0666);
	if(ret < 0){
		printf("管道创建失败\n");
		return -1;
	}
	printf("创建管道成功\n");	
*/
	if (mkfifo(FIFO_PATH, 0666) == -1) {
        if (errno != EEXIST) {
            perror("[写线程] 创建命名管道失败");
            return NULL;
        } else {
            printf("[写线程] 命名管道已存在，直接使用\n");
        }
    }

	 // 先创建写线程（创建管道）
    pthread_create(&writer, NULL, writer_thread, NULL);
	 // 再创建读线程（打开管道）
    pthread_create(&reader, NULL, reader_thread, NULL);

	 // 等待线程完成
    pthread_join(writer, NULL);
    pthread_join(reader, NULL);

	unlink(FIFO_PATH);    
    printf("\n测试完成！命名管道实现了线程间的通信\n");

	return 0;

}
