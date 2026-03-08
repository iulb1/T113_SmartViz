#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

/**
 * @brief 用户信息结构体：存储用户核心数据
 * @note 二进制存储时，结构体成员需注意内存对齐（默认编译选项已处理，无需额外调整）
 */
typedef struct {
    int id;          // 用户ID（唯一关键字，用于索引查询）
    char name[32];   // 用户名（最大31个字符，留1字节存字符串结束符'\0'）
    int age;         // 年龄（整数类型，占4字节）
    char phone[16];  // 手机号（如11位手机号+结束符，16字节足够）
} User;

/**
 * @brief 索引结构体：建立用户ID与数据位置的映射
 * @note 用于快速查询，避免遍历整个数据文件，提升查询效率
 */
typedef struct {
    int user_id;       // 关联的用户ID（与User.id对应）
    off_t file_offset; // 用户数据在data文件中的偏移量（字节数）
} IndexItem;

/**
 * @brief 写入用户数据并更新索引
 * @param data_path  数据文件路径（存储User结构体二进制数据）
 * @param index_path 索引文件路径（存储IndexItem映射关系）
 * @param user       待写入的用户数据指针（需确保数据有效）
 * @return 0：成功，-1：失败
 * @note 1. 数据文件以追加模式打开，避免覆盖历史数据；
 *       2. 批量调用fsync刷盘，平衡性能与数据安全性；
 *       3. 需检查文件打开、写入等步骤的错误，避免资源泄漏。
 */
int write_user(const char *data_path, const char *index_path, User *user) {
    // 1. 校验入参：避免空指针导致的内存访问错误
    if (data_path == NULL || index_path == NULL || user == NULL) {
        fprintf(stderr, "write_user error: invalid input parameter (NULL pointer)\n");
        return -1;
    }

    // 2. 打开数据文件和索引文件
    // 数据文件：O_WRONLY（只写）| O_CREAT（不存在则创建）| O_APPEND（追加模式，不覆盖历史数据）
    int data_fd = open(data_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    // 索引文件：O_RDWR（读写）| O_CREAT（不存在则创建）
    int index_fd = open(index_path, O_RDWR | O_CREAT, 0644);

    // 检查文件打开是否成功，失败则返回错误
    if (data_fd == -1 || index_fd == -1) {
        perror("write_user: open file failed");
        // 若其中一个文件打开成功，需关闭避免FD泄漏
        if (data_fd != -1) close(data_fd);
        if (index_fd != -1) close(index_fd);
        return -1;
    }

    // 3. 获取新用户数据在data文件中的偏移量（当前文件末尾位置）
    off_t data_offset = lseek(data_fd, 0, SEEK_END);
    if (data_offset == -1) {
        perror("write_user: lseek data file failed");
        close(data_fd);
        close(index_fd);
        return -1;
    }

    // 4. 写入用户数据到data文件（二进制存储，无解析开销）
    ssize_t write_ret = write(data_fd, user, sizeof(User));
    if (write_ret != sizeof(User)) {
        perror("write_user: write user data failed");
        close(data_fd);
        close(index_fd);
        return -1;
    }

    // 5. 写入索引到index文件（建立ID与偏移量的映射）
    IndexItem index = {
        .user_id = user->id,        // 关联用户ID
        .file_offset = data_offset  // 关联数据偏移量
    };
    // 定位到索引文件末尾，追加新索引
    if (lseek(index_fd, 0, SEEK_END) == -1) {
        perror("write_user: lseek index file failed");
        close(data_fd);
        close(index_fd);
        return -1;
    }
    // 写入索引数据
    if (write(index_fd, &index, sizeof(IndexItem)) != sizeof(IndexItem)) {
        perror("write_user: write index failed");
        close(data_fd);
        close(index_fd);
        return -1;
    }

    // 6. 批量同步到磁盘：每100条数据刷盘一次，减少fsync阻塞次数
    static int count = 0;  // 静态变量：记录写入次数，程序运行期间不重置
    if (++count % 100 == 0) {
        // fsync：强制将内核缓冲区数据刷入磁盘，确保断电不丢失
        if (fsync(data_fd) == -1) perror("write_user: fsync data failed");
        if (fsync(index_fd) == -1) perror("write_user: fsync index failed");
        printf("write_user: batch sync success (count: %d)\n", count);
    }

    // 7. 释放资源：关闭文件FD，避免泄漏
    close(data_fd);
    close(index_fd);

    return 0;
}

/**
 * @brief 根据用户ID查询用户信息（通过索引快速定位）
 * @param data_path  数据文件路径
 * @param index_path 索引文件路径
 * @param user_id    待查询的用户ID
 * @param user       输出参数：存储查询到的用户数据（需提前分配内存）
 * @return 0：成功，-1：失败（用户不存在或文件操作错误）
 */
int read_user(const char *data_path, const char *index_path, int user_id, User *user) {
    // 1. 校验入参：避免空指针或无效ID
    if (data_path == NULL || index_path == NULL || user == NULL || user_id <= 0) {
        fprintf(stderr, "read_user error: invalid input parameter (NULL/negative ID)\n");
        return -1;
    }

    // 2. 打开索引文件，遍历查找目标用户ID对应的偏移量
    int index_fd = open(index_path, O_RDONLY);  // 索引文件只读即可
    if (index_fd == -1) {
        perror("read_user: open index file failed");
        return -1;
    }

    IndexItem index;
    int found = 0;          // 标记是否找到目标用户
    off_t target_offset = 0;// 目标用户数据的偏移量（找到后赋值）

    // 循环读取索引条目：每次读一个IndexItem，直到文件末尾或找到目标
    while (read(index_fd, &index, sizeof(IndexItem)) == sizeof(IndexItem)) {
        if (index.user_id == user_id) {
            found = 1;
            target_offset = index.file_offset;
            break;  // 找到后退出循环，无需继续遍历
        }
    }

    // 关闭索引文件：无论是否找到，都需释放资源
    close(index_fd);

    // 3. 若未找到目标用户，返回错误
    if (!found) {
        fprintf(stderr, "read_user: user ID %d not found in index\n", user_id);
        return -1;
    }

    // 4. 打开数据文件，根据偏移量读取用户数据
    int data_fd = open(data_path, O_RDONLY);  // 数据文件只读即可
    if (data_fd == -1) {
        perror("read_user: open data file failed");
        return -1;
    }

    // 定位到目标用户数据的偏移量
    if (lseek(data_fd, target_offset, SEEK_SET) == -1) {
        perror("read_user: lseek to target offset failed");
        close(data_fd);
        return -1;
    }

    // 读取用户数据到输出参数
    ssize_t read_ret = read(data_fd, user, sizeof(User));
    if (read_ret != sizeof(User)) {
        perror("read_user: read user data failed");
        close(data_fd);
        return -1;
    }

    // 5. 释放资源：关闭数据文件
    close(data_fd);

    return 0;
}

// ------------------------------ 测试代码 ------------------------------
int main() {
    // 1. 定义测试用户数据（初始化字符串，避免垃圾数据）
    User user1 = {
        .id = 1,
        .name = "Zhang San",  // 字符串自动带'\0'，无需手动添加
        .age = 25,
        .phone = "13800138000"
    };

    // 2. 写入用户数据到文件（data文件：users.data，index文件：users.index）
    int write_ret = write_user("users.data", "users.index", &user1);
    if (write_ret != 0) {
        fprintf(stderr, "main: write user1 failed\n");
        return EXIT_FAILURE;
    }
    printf("main: write user1 success (ID: %d)\n", user1.id);

    // 3. 读取用户数据（根据ID=1查询）
    User read_user1;
    memset(&read_user1, 0, sizeof(User));  // 初始化输出结构体，避免垃圾数据
    int read_ret = read_user("users.data", "users.index", 1, &read_user1);
    if (read_ret != 0) {
        fprintf(stderr, "main: read user1 failed\n");
        return EXIT_FAILURE;
    }

    // 4. 打印查询结果
    printf("\nmain: read user success:\n");
    printf("ID: %d\n", read_user1.id);
    printf("Name: %s\n", read_user1.name);
    printf("Age: %d\n", read_user1.age);
    printf("Phone: %s\n", read_user1.phone);

    return EXIT_SUCCESS;
}
