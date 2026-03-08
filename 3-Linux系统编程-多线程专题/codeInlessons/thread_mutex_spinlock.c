/**
 * 示例：互斥锁在高频短时锁场景下的问题
 * 演示互斥锁导致的上下文切换开销问题
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#define NUM_THREADS 4
#define OPERATIONS_PER_THREAD 100000

// 共享计数器
volatile int g_counter = 0;

// 获取当前时间（微秒）
long long get_time_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000LL + tv.tv_usec;
}

// ========== 测试1：使用互斥锁（问题演示）==========
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

void* mutex_thread(void *arg) {
    int thread_id = *(int *)arg;
    
    for (int i = 0; i < OPERATIONS_PER_THREAD; i++) {
        // 加锁（互斥锁：如果锁被占用，线程会进入睡眠，发生上下文切换）
        pthread_mutex_lock(&g_mutex);
        
        // 临界区：非常短的操作（只有几微秒）
        g_counter++;
        // 模拟非常短的临界区操作（1微秒）
        // 注意：实际应用中，这里可能是简单的内存操作
        
        pthread_mutex_unlock(&g_mutex);
    }
    
    printf("互斥锁线程 %d 完成\n", thread_id);
    return NULL;
}

// ========== 测试2：使用自旋锁（解决方案）==========
pthread_spinlock_t g_spinlock;

void* spinlock_thread(void *arg) {
    int thread_id = *(int *)arg;
    
    for (int i = 0; i < OPERATIONS_PER_THREAD; i++) {
        // 加锁（自旋锁：如果锁被占用，线程会忙等待，不进入睡眠）
        pthread_spin_lock(&g_spinlock);
        
        // 临界区：非常短的操作（只有几微秒）
        g_counter++;
        // 模拟非常短的临界区操作（1微秒）
        
        pthread_spin_unlock(&g_spinlock);
    }
    
    printf("自旋锁线程 %d 完成\n", thread_id);
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    long long start_time, end_time;
    double elapsed_seconds;
    
    printf("========================================\n");
    printf("自旋锁 vs 互斥锁：高频短时锁场景对比\n");
    printf("========================================\n\n");
    
    printf("场景说明：\n");
    printf("- 多个线程频繁访问共享资源\n");
    printf("- 每次加锁后只执行非常短的操作（微秒级）\n");
    printf("- 锁的竞争非常激烈\n");
    printf("- 每个线程执行 %d 次操作\n\n", OPERATIONS_PER_THREAD);
    
    // ========== 测试1：互斥锁 ==========
    printf("========================================\n");
    printf("测试1：使用互斥锁（问题演示）\n");
    printf("========================================\n");
    printf("问题：当锁被占用时，线程会进入睡眠，发生上下文切换\n");
    printf("     如果锁持有时间很短，上下文切换的开销可能比实际工作还大\n");
    printf("     导致性能下降\n\n");
    
    g_counter = 0;
    start_time = get_time_us();
    
    // 创建多个线程
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i + 1;
        pthread_create(&threads[i], NULL, mutex_thread, &thread_ids[i]);
    }
    
    // 等待所有线程完成
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    end_time = get_time_us();
    elapsed_seconds = (end_time - start_time) / 1000000.0;
    
    printf("\n互斥锁结果：\n");
    printf("  counter = %d (期望: %d)\n", g_counter, NUM_THREADS * OPERATIONS_PER_THREAD);
    printf("  总耗时: %.3f 秒\n", elapsed_seconds);
    printf("  平均每次操作: %.3f 微秒\n\n", (end_time - start_time) / (double)(NUM_THREADS * OPERATIONS_PER_THREAD));
    
    sleep(1);
    
    // ========== 测试2：自旋锁 ==========
    printf("========================================\n");
    printf("测试2：使用自旋锁（解决方案）\n");
    printf("========================================\n");
    printf("优势：当锁被占用时，线程会忙等待（自旋），不进入睡眠\n");
    printf("     避免了上下文切换的开销\n");
    printf("     在锁持有时间很短、多核CPU的场景下，性能更好\n\n");
    
    pthread_spin_init(&g_spinlock, PTHREAD_PROCESS_PRIVATE);
    g_counter = 0;
    start_time = get_time_us();
    
    // 创建多个线程
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i + 1;
        pthread_create(&threads[i], NULL, spinlock_thread, &thread_ids[i]);
    }
    
    // 等待所有线程完成
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    end_time = get_time_us();
    elapsed_seconds = (end_time - start_time) / 1000000.0;
    
    printf("\n自旋锁结果：\n");
    printf("  counter = %d (期望: %d)\n", g_counter, NUM_THREADS * OPERATIONS_PER_THREAD);
    printf("  总耗时: %.3f 秒\n", elapsed_seconds);
    printf("  平均每次操作: %.3f 微秒\n\n", (end_time - start_time) / (double)(NUM_THREADS * OPERATIONS_PER_THREAD));
    
    pthread_spin_destroy(&g_spinlock);
    
    // ========== 总结 ==========
    printf("========================================\n");
    printf("总结：为什么需要自旋锁？\n");
    printf("========================================\n\n");
    
    printf("1. 互斥锁的问题（在高频短时锁场景下）：\n");
    printf("   - 当锁被占用时，线程会进入睡眠状态\n");
    printf("   - 发生上下文切换（保存/恢复寄存器、切换页表等）\n");
    printf("   - 上下文切换的开销通常为几微秒到几十微秒\n");
    printf("   - 如果锁持有时间很短（几微秒），上下文切换的开销可能比实际工作还大\n");
    printf("   - 导致性能下降\n\n");
    
    printf("2. 自旋锁的优势：\n");
    printf("   - 当锁被占用时，线程会忙等待（自旋），不进入睡眠\n");
    printf("   - 避免了上下文切换的开销\n");
    printf("   - 在多核CPU上，等待锁的线程可以在其他核心上自旋\n");
    printf("   - 锁释放后，自旋的线程可以立即获得锁，响应更快\n");
    printf("   - 在锁持有时间很短的场景下，性能更好\n\n");
    
    printf("3. 自旋锁的适用场景：\n");
    printf("   ✓ 锁持有时间很短（微秒级）\n");
    printf("   ✓ 多核CPU环境（单核CPU上自旋会浪费CPU）\n");
    printf("   ✓ 锁竞争激烈，但持有时间短\n");
    printf("   ✓ 不能进入睡眠的上下文（如中断处理程序）\n\n");
    
    printf("4. 互斥锁的适用场景：\n");
    printf("   ✓ 锁持有时间较长（毫秒级或更长）\n");
    printf("   ✓ 单核CPU环境\n");
    printf("   ✓ 锁竞争不激烈，或持有时间长\n");
    printf("   ✓ 需要让出CPU给其他线程的场景\n\n");
    
    printf("5. 性能对比说明：\n");
    printf("   - 理论上，在高频短时锁场景下，自旋锁应该比互斥锁快\n");
    printf("   - 但实际上，现代互斥锁（futex）已经做了很多优化\n");
    printf("   - 在竞争不激烈时，互斥锁可能不会真正进入睡眠（快速路径）\n");
    printf("   - 因此，在很多场景下，互斥锁的性能已经很好，甚至可能更快\n");
    printf("   - 自旋锁的优势主要体现在：多核CPU + 激烈竞争 + 极短时锁（纳秒级）\n");
    printf("   - 如果测试中互斥锁更快，这是正常的，说明当前场景更适合互斥锁\n");
    printf("   - 选择原则：\n");
    printf("     * 大多数场景：使用互斥锁（已经优化得很好）\n");
    printf("     * 特殊场景（多核+激烈竞争+纳秒级锁）：考虑自旋锁\n");
    printf("     * 不能睡眠的上下文（中断处理）：必须使用自旋锁\n\n");
    
    return 0;
}
