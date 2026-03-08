/**
 * 示例：为什么读取也需要加锁？
 * 演示"读取不加锁，只给写入加互斥锁"会导致的数据一致性问题
 * 
 * 问题：既然读锁是共享的，多个读线程可以同时读取，
 *       那是不是读取时不需要加锁？只给写入加互斥锁不就行了吗？
 * 
 * 答案：不行！读取不加锁会导致数据一致性问题。
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#define NUM_READERS 5
#define READ_OPERATIONS 10

// ========== 错误方式：读取不加锁，只给写入加互斥锁 ==========
typedef struct {
    int value1;      // 假设这是一个复杂结构的一部分
    int value2;      // 需要同时读取多个字段才能保证一致性
    int checksum;    // 校验和，用于验证数据一致性
} complex_data_t;

complex_data_t g_data_wrong = {100, 200, 300};  // value1 + value2 = checksum
pthread_mutex_t g_write_mutex_wrong = PTHREAD_MUTEX_INITIALIZER;

// 错误：读取不加锁
void read_data_wrong(complex_data_t *data) {
    // 没有加锁！可能读到正在被修改的数据
    int v1 = data->value1;
    usleep(50000);  // 模拟读取操作耗时（在这期间数据可能被修改）
    int v2 = data->value2;
    int cs = data->checksum;
    
    // 验证数据一致性
    if (v1 + v2 != cs) {
        printf("❌ 数据不一致！value1=%d, value2=%d, checksum=%d (应该是 %d)\n", 
               v1, v2, cs, v1 + v2);
    } else {
        printf("✓ 数据一致：value1=%d, value2=%d, checksum=%d\n", v1, v2, cs);
    }
}

// 写入时加互斥锁
void write_data_wrong(complex_data_t *data, int new_v1, int new_v2) {
    pthread_mutex_lock(&g_write_mutex_wrong);
    
    // 模拟非原子写入（分步写入，中间可能被打断）
    data->value1 = new_v1;
    usleep(100000);  // 模拟写入耗时（在这期间读线程可能读取到部分更新的数据）
    data->value2 = new_v2;
    usleep(100000);
    data->checksum = new_v1 + new_v2;  // 最后更新校验和
    
    printf("写入完成：value1=%d, value2=%d, checksum=%d\n", 
           data->value1, data->value2, data->checksum);
    
    pthread_mutex_unlock(&g_write_mutex_wrong);
}

// ========== 正确方式：使用读写锁 ==========
complex_data_t g_data_correct = {100, 200, 300};
pthread_rwlock_t g_rwlock = PTHREAD_RWLOCK_INITIALIZER;

// 正确：读取时加读锁（阻止写线程）
void read_data_correct(complex_data_t *data) {
    pthread_rwlock_rdlock(&g_rwlock);  // 加读锁：阻止写线程修改数据
    
    int v1 = data->value1;
    usleep(50000);  // 模拟读取操作耗时
    int v2 = data->value2;
    int cs = data->checksum;
    
    // 验证数据一致性
    if (v1 + v2 != cs) {
        printf("❌ 数据不一致！value1=%d, value2=%d, checksum=%d\n", v1, v2, cs);
    } else {
        printf("✓ 数据一致：value1=%d, value2=%d, checksum=%d\n", v1, v2, cs);
    }
    
    pthread_rwlock_unlock(&g_rwlock);
}

// 写入时加写锁（阻止所有读线程和写线程）
void write_data_correct(complex_data_t *data, int new_v1, int new_v2) {
    pthread_rwlock_wrlock(&g_rwlock);  // 加写锁：独占访问
    
    data->value1 = new_v1;
    usleep(100000);
    data->value2 = new_v2;
    usleep(100000);
    data->checksum = new_v1 + new_v2;
    
    printf("写入完成：value1=%d, value2=%d, checksum=%d\n", 
           data->value1, data->value2, data->checksum);
    
    pthread_rwlock_unlock(&g_rwlock);
}

// ========== 读取线程 ==========
void *reader_thread_wrong(void *arg) {
    int reader_id = *(int *)arg;
    
    for (int i = 0; i < READ_OPERATIONS; i++) {
        printf("[读线程 %d] ", reader_id);
        read_data_wrong(&g_data_wrong);
        usleep(100000);  // 两次读取之间休息
    }
    
    return NULL;
}

void *reader_thread_correct(void *arg) {
    int reader_id = *(int *)arg;
    
    for (int i = 0; i < READ_OPERATIONS; i++) {
        printf("[读线程 %d] ", reader_id);
        read_data_correct(&g_data_correct);
        usleep(100000);
    }
    
    return NULL;
}

// ========== 写入线程 ==========
void *writer_thread_wrong(void *arg) {
    for (int i = 1; i <= 3; i++) {
        usleep(200000);  // 等待一段时间，让读线程先运行
        printf("\n[写入线程] 开始写入新值...\n");
        write_data_wrong(&g_data_wrong, 100 + i * 10, 200 + i * 10);
    }
    return NULL;
}

void *writer_thread_correct(void *arg) {
    for (int i = 1; i <= 3; i++) {
        usleep(200000);
        printf("\n[写入线程] 开始写入新值...\n");
        write_data_correct(&g_data_correct, 100 + i * 10, 200 + i * 10);
    }
    return NULL;
}

int main() {
    pthread_t readers[NUM_READERS], writer;
    int reader_ids[NUM_READERS];
    
    printf("========================================\n");
    printf("演示：为什么读取也需要加锁？\n");
    printf("========================================\n\n");
    
    printf("场景说明：\n");
    printf("- 共享数据包含 value1, value2, checksum 三个字段\n");
    printf("- 数据一致性要求：value1 + value2 == checksum\n");
    printf("- 写入操作需要分步完成（非原子操作）\n");
    printf("- 如果读取不加锁，可能在写入过程中读取，导致读到不一致的数据\n\n");
    
    // ========== 测试1：错误方式 ==========
    printf("========================================\n");
    printf("测试1：错误方式 - 读取不加锁，只给写入加互斥锁\n");
    printf("========================================\n");
    printf("问题：读线程可能在写线程修改数据的过程中读取数据\n");
    printf("     导致读到部分更新的数据（数据不一致）\n\n");
    
    // 创建读取线程
    for (int i = 0; i < NUM_READERS; i++) {
        reader_ids[i] = i + 1;
        pthread_create(&readers[i], NULL, reader_thread_wrong, &reader_ids[i]);
    }
    
    // 创建写入线程
    pthread_create(&writer, NULL, writer_thread_wrong, NULL);
    
    // 等待所有线程完成
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }
    pthread_join(writer, NULL);
    
    printf("\n观察：应该会看到 '❌ 数据不一致' 的错误信息\n");
    printf("原因：读线程在写线程更新 value1/value2/checksum 的过程中读取了数据\n");
    printf("     导致读到的 value1 和 value2 可能来自不同的更新操作\n\n");
    
    sleep(2);
    
    // ========== 测试2：正确方式 ==========
    printf("========================================\n");
    printf("测试2：正确方式 - 使用读写锁\n");
    printf("========================================\n");
    printf("解决：读线程加读锁，写线程加写锁\n");
    printf("     读锁会阻止写线程，确保读取时数据不会被修改\n");
    printf("     写锁会阻止所有读线程和写线程，确保写入的原子性\n\n");
    
    // 创建读取线程
    for (int i = 0; i < NUM_READERS; i++) {
        reader_ids[i] = i + 1;
        pthread_create(&readers[i], NULL, reader_thread_correct, &reader_ids[i]);
    }
    
    // 创建写入线程
    pthread_create(&writer, NULL, writer_thread_correct, NULL);
    
    // 等待所有线程完成
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }
    pthread_join(writer, NULL);
    
    printf("\n观察：所有读取都显示 '✓ 数据一致'\n");
    printf("原因：读锁阻止了写线程，确保读取时数据不会被修改\n\n");
    
    // ========== 总结 ==========
    printf("========================================\n");
    printf("总结：读写锁的真正意义\n");
    printf("========================================\n\n");
    
    printf("1. 读锁的作用：\n");
    printf("   - 允许多个读线程同时读取（共享锁）\n");
    printf("   - 阻止写线程修改数据（关键！）\n");
    printf("   - 确保读取时数据不会被修改，保证数据一致性\n\n");
    
    printf("2. 写锁的作用：\n");
    printf("   - 独占访问，阻止所有读线程和写线程\n");
    printf("   - 确保写入操作的原子性\n\n");
    
    printf("3. 为什么不能'读取不加锁，只给写入加互斥锁'？\n");
    printf("   - 如果读取不加锁，写线程加互斥锁写入时，读线程仍然可以读取\n");
    printf("   - 读线程可能在写线程修改数据的过程中读取数据\n");
    printf("   - 导致读到部分更新的数据，造成数据不一致\n");
    printf("   - 例如：读到新的 value1 但旧的 value2，或者读到旧的 value1 但新的 value2\n\n");
    
    printf("4. 读写锁的优势：\n");
    printf("   - 多个读线程可以并发执行（性能优势）\n");
    printf("   - 读锁阻止写线程，保证数据一致性（正确性）\n");
    printf("   - 写锁独占访问，保证写入原子性（正确性）\n\n");
    
    printf("5. 适用场景：\n");
    printf("   - 读多写少的场景\n");
    printf("   - 需要保证数据一致性的场景\n");
    printf("   - 读取操作耗时较长的场景（多个读线程可以并发）\n\n");
    
    // 清理
    pthread_mutex_destroy(&g_write_mutex_wrong);
    pthread_rwlock_destroy(&g_rwlock);
    
    return 0;
}


