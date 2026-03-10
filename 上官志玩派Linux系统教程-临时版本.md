



## 一. 课程概述

### 1.1 课程目的

因为Linux系统应用层和底层都非常庞大，所以通过项目的方式来组织常用知识点。

别墅门禁可视化对讲系统主要针对音频，视频外设及Linux常用的多线程网络，开发套件移植等知识点进行系统学习。通过具象的项目实操培养通用项目能力，

也就是说通过本课程学习之后，大家可以做其他各种类型的项目。

### 1.2 项目演示

* 指纹识别功能
* 指纹注册功能
* 人脸识别功能
* 人脸识别注册功能
* 密码键盘功能
* 远程对话功能
  * 生产级别低成本音视频方案

* 上位机主动监视
* 上位机远程开关锁
* 上位机灯控

### 1.3 学习目标

![](G:\202507\pics\QQ截图20250724053213.jpg) 

### 1.4 基础要求

* C/C++编程调试能力
* 熟悉QT开发

### 1.5 开发环境

* 开发机Ubuntu18.04
* 运行机上官志玩派-全志T113-S3架构 Linux5.4.61内核



## 二. 上官志玩派

### 2.1 硬件参数说明

上官志玩派选用企业级全志T113-S3平台，为特定任务定制的**"迷你电脑"**

<img src="G:\202507\pics\zhiwanUI.png" style="zoom:80%;" /> 

全志T113-S3是一款基于ARM Cortex-A7双核架构的嵌入式处理器，主要特点如下：

**硬件规格**

- CPU: ARM Cortex-A7 双核，主频最高1.2GHz

- GPU: Mali-400 MP2
  -  辅助H.264/H.265视频解码
  -  加速Qt/LVGL图形界面显示，支持流畅的界面动画效果
  - 支持OpenCV等图像处理库

- 内存: 支持DDR3/DDR3L，本机128M运行内存

- 存储: 本机EMMC256M,可接SD卡

- 视频: 支持H.264/H.265解码，1080p@60fps

- 音频: 集成音频编解码器，支持多种音频格式

**主要应用场景**

智能门禁系统，视频监控设备，工业控制面板，智能家居设备，教育平板，车载终端

### 2.2 为什么要用SOC+Linux方案

> MCU(Microcontroller Unit) 微控制器单元，将CPU、内存、I/O接口集成在单一芯片上，体积小、功耗低、成本低
>
> * ARM架构：
>
>   - STM32：意法半导体，应用广泛
>
>   - NXP：恩智浦，汽车电子
>
>   - TI：德州仪器，工业控制
>
>   - Microchip：微芯，消费电子
>
>   其他架构：
>
>   - AVR：Atmel，Arduino常用
>
>   - PIC：Microchip，简单易用
>
>   - 8051：Intel，经典架构
>
>   - RISC-V：开源架构，新兴
>
> SOC（System on Chip）片上系统，将整个系统集成在单一芯片上，包含处理器、内存、I/O、专用硬件等，高度集成，功能完整
>
> * 智能手机SoC：
>   - 高通骁龙888，全志，瑞芯微
>   - 集成：CPU、GPU、5G基带、ISP、AI加速器
>   - 功能：通话、上网、拍照、游戏、AI处理
>   - 体积：小，集成度高
>
> MPU (Microprocessor Unit) 微处理器单元，只包含CPU核心，不包含内存和I/O，功能强大，可运行复杂操作系统
>
> * 工业计算机MPU：
>   - Intel Core i7
>   - 需要外接：内存、硬盘、网卡、显卡
>   - 功能：通用计算、数据处理
>   - 体积：大，扩展性强

#### 2.2.1 单片机方案产品mcuRTOS

![](G:\202507\pics\QQ截图20250803074445.png) 

#### 2.2.2 Linux方案产品SOCLinux

![](G:\202507\pics\QQ截图20250803074504.png) 

#### 2.2.3 单片机和Linux多方案

![](G:\202507\pics\QQ截图20250803074520.png) 



### 2.3 软硬件系统对比

| **对比     维度** | **普通电脑 + Windows系统**                                   | **嵌入式SOC + Linux系统**                                    | **嵌入式MCU + RTOS系统**                                     |
| ----------------- | ------------------------------------------------------------ | ------------------------------------------------------------ | ------------------------------------------------------------ |
| **硬件平台**      | x86/x64架构高性能CPU GB级内存 大容量HDD/SSD 独立显卡、声卡、网口等丰富外设 | ARM Cortex-A系列低功耗CPU MB-GB级内存 Flash存储 集成GPU、网口等有限外设 | ARM Cortex-M系列超低功耗CPU KB-MB级内存 Flash存储 仅基本外设（如GPIO、UART） |
| **操作系统定位**  | 通用桌面操作系统，支持多用户、多任务、图形界面               | 嵌入式专用操作系统，支持多任务、可选图形界面                 | 实时操作系统，专注实时控制，无图形界面                       |
| **系统复杂度**    | 非常复杂，数百万行代码，完整功能栈                           | 中等复杂，几十万行代码，模块化设计                           | 简单，几万行代码，精简功能                                   |
| **启动时间**      | 30秒至几分钟，完整系统初始化                                 | 几秒至几十秒，中等启动时间                                   | 几毫秒至几秒，快速启动                                       |
| **内存管理**      | 虚拟内存、内存分页、内存保护                                 | 虚拟内存、内存分页、内存保护（类似桌面系统）                 | 静态内存分配，简单内存管理，无内存保护                       |
| **文件系统**      | NTFS/FAT32，完整文件系统，支持大文件                         | ext4/ubifs，完整文件系统，支持大文件                         | 无或简单Flash文件系统，小文件支持                            |
| **网络支持**      | 完整TCP/IP栈，多种网络协议，WiFi/以太网                      | 完整TCP/IP栈，多种网络协议，WiFi/以太网                      | 有限网络支持，简单协议，以太网为主                           |
| **图形界面**      | 完整GUI，Windows界面，丰富图形库                             | 可选GUI，如Qt/GTK，嵌入式界面                                | 无图形界面，LED/LCD显示，简单UI                              |
| **多任务处理**    | 抢占式多任务，进程/线程，复杂调度                            | 抢占式多任务，进程/线程，复杂调度                            | 协作式/抢占式，任务切换，简单调度                            |
| **实时性**        | 非实时，响应时间不确定，适合桌面应用                         | 软实时，毫秒级响应，适合多媒体应用                           | 硬实时，微秒级响应，适合控制应用                             |
| **功耗**          | 高功耗，几十瓦至几百瓦，需要散热                             | 中等功耗，几瓦至几十瓦，部分需要散热                         | 低功耗，几毫瓦至几瓦，无需散热                               |
| **成本**          | 高成本，几百至几千元，完整PC硬件                             | 中等成本，几十至几百元，复杂SOC                              | 低成本，几元至几十元，简单MCU                                |
| **可靠性**        | 一般，需要定期维护，可能崩溃                                 | 高，稳定运行，偶尔重启                                       | 高，长期稳定运行，很少崩溃                                   |
| **安全性**        | 复杂安全机制，病毒防护，权限管理                             | 中等安全，Linux安全，网络防护                                | 简单安全，物理隔离，有限攻击面                               |
| **更新升级**      | 在线更新，自动升级，版本管理                                 | 在线/离线更新，包管理，版本管理                              | 固件烧录，手动升级，版本固定                                 |
| **生态系统**      | 庞大生态，大量软件，丰富资源                                 | 丰富生态，开源软件，资源丰富                                 | 有限生态，专用软件，资源有限                                 |
| **学习曲线**      | 中等，Windows熟悉，开发工具多                                | 中等，Linux系统，软件为主                                    | 简单，RTOS概念，硬件为主                                     |

### 2.4 嵌入式系统概念

嵌入式系统的学术概念-中国学术界共识定义

> 是一种**以应用为中心、以计算机技术为基础**的专用计算机系统，
>
> 其软硬件可裁剪，适应应用系统对功能、可靠性、成本、体积、功耗有严格要求的专用计算机系统

![](G:\202507\pics\QQ截图20250724045245.jpg) 

## 三. Linux环境搭建

### 3.1 Linux有哪些发行版

以下是一些流行的Linux发行版及其特点的简要概述：

| 发行版     | 特点                                                         |
| ---------- | ------------------------------------------------------------ |
| **Ubuntu** | 用户友好，适合初学者和桌面用户。广泛的软件支持和强大的社区。 |
| **Fedora** | 先进的技术和创新，频繁更新。Red Hat支持，适合那些想体验最新Linux特性的用户。 |
| Debian     | 稳定性和安全性非常高，适用于服务器和高可靠性需求的环境。软件包管理优秀。 |
| CentOS     | 免费的企业级发行版，与Red Hat Enterprise Linux（RHEL）高度兼容。适合企业服务器。 |
| Arch Linux | 滚动更新，高度可定制，适合经验丰富的用户。强调简洁和现代设计。 |
| openSUSE   | 专注于用户友好和稳定性。拥有独特的YaST配置工具。适合桌面和服务器。 |
| Manjaro    | 基于Arch Linux，但更加易于使用和设置。适合那些喜欢Arch理念但希望更友好的用户体验的人。 |
| Linux Mint | 基于Ubuntu，提供更传统的桌面体验。易于使用，适合初学者和希望即插即用体验的用户。 |
| Kali Linux | 专为渗透测试和网络安全领域设计。内置大量安全工具。           |
| Raspbian   | 为Raspberry Pi定制的发行版，轻量级，适用于教育和实验项目。   |

### 3.2 快速构建一个Linux操作系统

1. 电脑上虚拟一台电脑
2. Linux的一个镜像

![](G:\202507\pics\QQ截图20250803082806.png) 

### 3.3 如何传输文件

* samba服务器
* VmwareTool共享文件夹
* xshell
  * 用NAT模式共享网络
  * 用桥接模式（不轻易尝试，以免配置错误）
  * 查看电脑的iP地址  

<img src="G:\202507\pics\QQ截图20250803091645.png" style="zoom:67%;" />  

根据ip addr的命令获得Ubuntu的IP地址，我这里是192.168.1.51

这样通过Session->选择SSH

<img src="G:\202507\pics\QQ截图20250803091923.png" style="zoom:76%;" /> 

登录后的界面

<img src="G:\202507\pics\QQ截图20250803083405.png" style="zoom:60%;" />  



## 四.QT跨平台

### 4.1 Qt跨平台含义

- 同一套代码可以在不同操作系统上运行

- 无需修改或少量修改即可适配

- 支持Windows、Linux、macOS、Android、iOS等

**编译工具决定在哪个平台运行！**

### 4.2 移植记事本的案例

解决两个问题：具体看视频课程实操

* 怎么编译
* 怎么传输到开发板运行

### 4.3 命令笔记参考

~~~shell
ls             		 # 显示当前目录内容
pwd          		 # 显示当前工作目录
rm  file1            # 删除file1文件

mkdir dirname        # 创建单个目录
mkdir dir1 dir2 dir3    # 同时创建多个目录
mkdir -p /path/to/dir   # 创建嵌套目录（父目录不存在时自动创建）

cd documents              # 切换到相对路径
cd ./documents            # 同上
cd ../documents           # 上级目录下的documents
cd ../../documents        # 上两级目录下的documents
cd ..                     # 上级目录
cd -                      # 回到跳转目录之前的目录

mv file1 /path/to/dir/      # 移动文件到目录
mv file1 file2              # 重命名文件 把文件file1重命名成file2

unzip archive.zip              # 解压到当前目录
unzip archive.zip -d destpath  # 解压到指定目录destpath

ip addr                   # 显示所有接口（同ip addr show）

adb devices				 # 查看连接的设备


adb shell				 # 进入开发板shell环境

adb push file.txt /root           # 推送单个文件file.txt 到开发板的/root文件夹下

ps            # 查看系统所有进程
kill -9 1234  # 终止进程ID为1234的进程

history                    # 显示所有历史命令
history 10                 # 显示最近10条命令
history -c                 # 清除所有历史记录
~~~



### 五. 门禁项目UI设计

* 工程建立和UI布局规划运行
* 按键布局设计
* 时间布局设计
* UI美化

![](G:\202507\pics\QQ截图20250804044156.png) 

## 六. AI工具

### 6.1 下载地址

> https://cursor.com/en/downloads

### 6.2 基本使用

![](G:\202507\pics\QQ截图20250804024859.png) 

### 6.3 vi编辑器

~~~shell
#打开/创建文件
vi hello.c

#插入模式，在命令行模式底下，按下i进入到插入模式，才能正常编辑

插入模式返回到命令行模式，按下Esc
怎么保存，在命令行模式下，按下冒号，才能输入命令
	保存w
	退出vi工具q
	wq保存退出
#命令模式,是vi工具默认启动的时候的默认模式，无法进行输入
	##删除当前行
	dd 5dd
	##复制行
	命令行模式底下：yy    5yy
	##黏贴行
	p
	##退出
	q
	##保存退出
	wq
~~~

### 6.4 开机默认启动

~~~
vi /etc/init.d/rc.modules
QT程序需要有环境变量
export 列出当前环境变量

export AA=1  配置一个环境变量参数AA，值是1
~~~

## 七. Linux系统时间编程

### 7.1 示例代码

~~~c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

int main() {
    printf("=== Linux系统时间获取示例 ===\n\n");
    
    // 方法1: 使用time()函数获取时间戳
    printf("1. 使用time()函数:\n");
    time_t timestamp = time(NULL);
    printf("   时间戳: %ld\n", timestamp);
    printf("   可读时间: %s", ctime(&timestamp));
    
    // 方法2: 使用gettimeofday()获取微秒精度时间
    printf("\n2. 使用gettimeofday()函数:\n");
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    printf("   秒: %ld\n", tv.tv_sec);
    printf("   微秒: %ld\n", tv.tv_usec);
    printf("   时区偏移(分钟): %d\n", tz.tz_minuteswest);
    
    // 方法3: 使用clock_gettime()获取高精度时间
    printf("\n3. 使用clock_gettime()函数:\n");
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    printf("   秒: %ld\n", ts.tv_sec);
    printf("   纳秒: %ld\n", ts.tv_nsec);
    
    // 方法4: 使用localtime()格式化时间
    printf("\n4. 使用localtime()格式化时间:\n");
    struct tm *local_time = localtime(&timestamp);
    printf("   年: %d\n", local_time->tm_year + 1900);
    printf("   月: %d\n", local_time->tm_mon + 1);
    printf("   日: %d\n", local_time->tm_mday);
    printf("   时: %d\n", local_time->tm_hour);
    printf("   分: %d\n", local_time->tm_min);
    printf("   秒: %d\n", local_time->tm_sec);
    printf("   星期: %d\n", local_time->tm_wday);
    
    // 方法5: 使用strftime()自定义格式
    printf("\n5. 使用strftime()自定义格式:\n");
    char time_str[100];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
    printf("   格式化时间: %s\n", time_str);
    
    strftime(time_str, sizeof(time_str), "%Y年%m月%d日 %H时%M分%S秒", local_time);
    printf("   中文格式: %s\n", time_str);
    
    // 方法6: 获取CPU时间
    printf("\n6. 获取CPU时间:\n");
    clock_t cpu_time = clock();
    printf("   CPU时间(时钟周期): %ld\n", cpu_time);
    printf("   CPU时间(秒): %.2f\n", (double)cpu_time / CLOCKS_PER_SEC);
    
    // 方法7: 获取单调时间(不受系统时间调整影响)
    printf("\n7. 获取单调时间:\n");
    struct timespec monotonic_ts;
    clock_gettime(CLOCK_MONOTONIC, &monotonic_ts);
    printf("   单调时间(秒): %ld\n", monotonic_ts.tv_sec);
    printf("   单调时间(纳秒): %ld\n", monotonic_ts.tv_nsec);
    
    // 方法8: 时间计算示例
    printf("\n8. 时间计算示例:\n");
    time_t now = time(NULL);
    time_t tomorrow = now + 24 * 60 * 60;  // 加一天
    printf("   现在: %s", ctime(&now));
    printf("   明天: %s", ctime(&tomorrow));
    
    // 计算时间差
    struct timeval start, end;
    gettimeofday(&start, NULL);
    usleep(100000);  // 睡眠100毫秒
    gettimeofday(&end, NULL);
    
    long seconds = end.tv_sec - start.tv_sec;
    long microseconds = end.tv_usec - start.tv_usec;
    if (microseconds < 0) {
        seconds--;
        microseconds += 1000000;
    }
    printf("   时间差: %ld.%06ld 秒\n", seconds, microseconds);
    
    return 0;
}

~~~

### 7.2 设置时间的命令

~~~shell
 1126  sudo timedatectl set-ntp true
 1127  date
 1128  timedatectl status
 1129  timedatectl
 1130  sudo timedatectl set-timezone Asia/Shanghai
 1131  date
 1132  history

# 查看当前时区
timedatectl

# 重新设置时区
sudo timedatectl set-timezone Asia/Shanghai

# 设置日期和时间 (格式: MMDDhhmmYYYY)
sudo date MMDDhhmmYYYY

# 例如：设置为2024年1月15日14:30:00
sudo date 011514302024

# 或者使用更直观的格式
sudo date -s "2024-01-15 14:30:00"

# 启用NTP同步
sudo timedatectl set-ntp true

# 查看NTP状态
timedatectl status

# 在开发板上设置系统时间
date -s "2025-08-04 07:56:30"
~~~

**遗留问题：是不是每次开机都要重新设置时间，这个时间准不准，客户使用重新启动，不懂设置时间怎么办**

**解决方案：网络同步时间**

* 开发板联网

  ~~~shell
  
  cd /lib/modules/5.4.61/
  # 1.安装网卡驱动
  insmod r8188eu.ko 
  # 2. 查找网卡
  ifconfig -a  
  # 3. 配置 wifi 参数
  vi /etc/wpa_supplicant.conf   #echo -e "\033[?25h" 把光标在终端中显示出来
  # ssid 为要连接的 wifi 名字
  # psk 为要连接的 wifi 密码
  
  # 4.联网
  ifconfig wlan0 up
  wpa_supplicant -D wext -c /etc/wpa_supplicant.conf -i wlan0 &
  
  # 5. 动态分配 ip
  udhcpc -i wlan0
  # 6. ping 外网测试
  ping baidu.com
  
  # 7.其他的一些命令
  #查询网卡状态
  wpa_cli -p/tmp/lock/wpa_supplicant -iwlan0 status
  #搜索附近网络功能 no/ok
  wpa_cli -p/tmp/lock/wpa_supplicant -i wlan0 scan
  #搜索附近网络,并列出结果
  wpa_cli -p/tmp/lock/wpa_supplicant -i wlan0 scan_resul
  #查看当前连接的是哪个网络
  wpa_cli -p/tmp/lock/wpa_supplicant -i wlan0 list_network
  ~~~

* 使用ntpclient做网络同步

  * ntpclient是一个轻量级的NTP客户端程序，用于从NTP服务器同步时间。
    它比标准的ntpd更简单，占用资源更少，适合嵌入式系统和小型设备使用。

  * 交叉编译

    交叉编译就是在一种架构的计算机上编译出能在另一种架构上运行的程序。

    比如在x86的PC上编译出能在ARM开发板上运行的程序。

  * 移植ntpclient

    ~~~shell
    # 1.下载安装包
    wget http://doolittle.icarus.com/ntpclient/ntpclient_2015_365.tar.gz
    # 2. 解压
    tar xvf ntpclient_2015_365.tar.gz
    # 3. 交叉编译 
    cd ntpclient-2015
    arm-openwrt-linux-gcc -Wall -Os -s -o ntpclient ntpclient.c phaselock.c
    # 4. 发送运行
    adb push ntpclient /root
        
    ~~~

  * 运行

    ~~~shell
    # 设置为中国时区（UTC+8）
    export TZ=CST-8
    
    ntpclient -s -h time.nist.gov 
    
    # 写到开机
    ~~~

### 7.3 项目显示时间

~~~cpp
#include "widget.h"
#include "ui_widget.h"
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <QTimer>


Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    //定义一个定时器，每个100ms刷新以下系统时间显示
    QTimer *getSysTimeTimer = new QTimer(this);
    connect(getSysTimeTimer,SIGNAL(timeout()),this,SLOT(time_reflash()));
    getSysTimeTimer->start(500);

}

void Widget::time_reflash()
{
    time_t timestamp = time(NULL);
    struct tm *local_time = localtime(&timestamp);

    char time_str_time[30];
    char time_str_date[30];
    strftime(time_str_time, sizeof(time_str_time), "%H:%M:%S", local_time);
    strftime(time_str_date, sizeof(time_str_time), "%Y-%m-%d", local_time);

    ui->labeldate->setText(QString(time_str_date));
    ui->labeltime->setText(QString(time_str_time));
}

Widget::~Widget()
{
    delete ui;
}
~~~

### 7.4 时间API补充说明

#### 7.4.1 time函数

~~~c
#include <time.h>
time_t time(time_t *tloc);

/*
功能: 获取当前时间戳（从1970年1月1日00:00:00 UTC开始的秒数）格林威治时间
参数:
tloc: 可选参数，如果为NULL，函数返回时间戳；如果不为NULL，时间戳也会存储到该地址
返回值: 成功返回时间戳，失败返回-1
使用场景: 获取基本的时间戳，适用于不需要高精度的场合

*/
~~~

#### 7.4.2 localtime函数

~~~c
#include <time.h>
struct tm *localtime(const time_t *timep);
/*
功能: 将时间戳转换为本地时间结构体
返回值: 指向tm结构体的指针
tm结构体成员:
struct tm {
    int tm_sec;    // 秒 [0,61]
    int tm_min;    // 分 [0,59]
    int tm_hour;   // 时 [0,23]
    int tm_mday;   // 日 [1,31]
    int tm_mon;    // 月 [0,11] (0表示1月)
    int tm_year;   // 年 (从1900年开始)
    int tm_wday;   // 星期 [0,6] (0表示星期日)
    int tm_yday;   // 一年中的第几天 [0,365]
    int tm_isdst;  // 夏令时标志
};
*/
~~~

#### 7.4.3 strftime() 函数

~~~c
#include <time.h>
size_t strftime(char *s, size_t max, const char *format, const struct tm *tm);

/*
功能: 将时间格式化为字符串
参数:
s: 输出缓冲区
max: 缓冲区大小
format: 格式字符串
tm: 时间结构体指针
常用格式符:
%Y: 四位年份
%m: 月份 (01-12)
%d: 日期 (01-31)
%H: 小时 (00-23)
%M: 分钟 (00-59)
%S: 秒 (00-59)
%w: 星期 (0-6, 0是星期日)
*/
~~~

### 7.5 纯Linux时间编程

#### 7.5.1 简易版本

~~~c
#include <stdio.h>
#include <time.h>

int main() {
    while(1) {
        time_t now = time(NULL);
        if(now == -1){
            printf("time error\n");
            return -1;
        }
        struct tm *local_time = localtime(&now);
        
        printf("\r当前时间: %02d:%02d:%02d", 
               local_time->tm_hour, 
               local_time->tm_min, 
               local_time->tm_sec);
        fflush(stdout);
        
        sleep(1);
    }
    return 0;
}
~~~

问题是这个除了显示时间没有其他作用，类似单片机程序！

#### 7.5.2 多线程版本

线程（Thread）是程序执行的最小单位，是进程中的一个实际运行单位。一个进程可以包含多个线程，这些线程共享进程的资源。

多线程的优势：

- 并发执行：多个任务可以同时进行

- 资源共享：线程间共享内存空间

- 响应性：避免阻塞主线程

- 效率：创建和销毁线程比进程更快

**线程创建 - pthread_create()**

~~~c
#include <pthread.h>

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*start_routine) (void *), void *arg);
/*

参数解释：
thread: 指向线程ID的指针
attr: 线程属性（NULL表示默认属性）
start_routine: 线程函数指针
arg: 传递给线程函数的参数

返回值：
成功：返回 0
失败：返回错误码（非0值）
常见错误码：
EAGAIN：系统资源不足
EINVAL：参数无效
EPERM：权限不足
*/

//示例
pthread_t thread_id;
void* thread_function(void* arg) {
    printf("线程函数执行\n");
    return NULL;
}

// 创建线程
pthread_create(&thread_id, NULL, thread_function, NULL);
~~~

**线程等待 - pthread_join()**

~~~c
int pthread_join(pthread_t thread, void **retval);
/*
pthread_t thread
类型：pthread_t（线程ID类型）
作用：指定要等待的线程
说明：必须是有效的线程ID，通常由pthread_create()返回

void **retval
类型：指向void*的指针
作用：用于接收线程函数的返回值
说明：如果不需要返回值，可以传NULL

返回值：
成功：返回 0
失败：返回错误码（非0值）
常见错误码：
EINVAL：线程ID无效或线程已分离
ESRCH：线程不存在
EDEADLK：死锁（线程等待自己）

*/
~~~

多线程版本时间显示

~~~c
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

// 全局变量
pthread_t timer_thread;
int running = 1;

// 定时器线程函数
void* timer_function(void* arg) {
    while(running) {
        time_t now = time(NULL);
        struct tm *local_time = localtime(&now);
        
        printf("\r当前时间: %02d:%02d:%02d", 
               local_time->tm_hour, 
               local_time->tm_min, 
               local_time->tm_sec);
        fflush(stdout);
        
        sleep(1);
    }
    return NULL;
}

int main() {
    printf("多线程时间显示程序启动...\n");
    printf("按 Ctrl+C 退出\n\n");
    
    // 创建定时器线程
    if (pthread_create(&timer_thread, NULL, timer_function, NULL) != 0) {
        perror("pthread_create");
        return 1;
    }
    
    // 主线程等待
    pthread_join(timer_thread, NULL);
    
    return 0;
}
~~~

## 八. Linux系统定时器编程

#### 8.1 alam简单定时器

vi 编辑器的配置    vi ~/.vimrc

~~~shell
set tabstop=4
set shiftwidth=4
set nu

#全局对齐方式：gg 回到代码文件头，=G全代码格式化
~~~

~~~c
#include <unistd.h>
unsigned int alarm(unsigned int seconds); //a

/*
seconds: 定时器秒数，0表示取消定时器
返回剩余秒数，如果之前没有定时器则返回0

特点
只能设置一个定时器
精度为秒级
使用SIGALRM信号

*/

#include <signal.h>
#include <unistd.h>

void alarm_handler(int sig) {
    printf("定时器触发\n");
}

signal(SIGALRM, alarm_handler);
alarm(5);  // 5秒后触发

~~~

示例：

~~~c
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <unistd.h>


void func(int sig)
{

    time_t now = time(NULL);
    if(now == -1){
        printf("time error\n");
        return ;
    }
    struct tm *local_time = localtime(&now);
    if(local_time == NULL){
        printf("localtime error\n");
        return ;
    }

    printf("\r当前时间: %02d:%02d:%02d",
            local_time->tm_hour,
            local_time->tm_min,
            local_time->tm_sec);

    fflush(stdout);

    alarm(1);
}

int main()
{
    int ret;
    long int i;
    long int j;

    if(SIG_ERR == signal(SIGALRM,func)){
        printf("signal bind error\n");
        return -1;
    }

    alarm(1);

    while(1){
        //  printf("main:print In while\n");
        for(i=0;i<10000;i++)
            for(j=0;j<80000;j++);
        //  ret = sleep(10);
        //  printf("ret = %d\n",ret);

    }
    return 0;
}

~~~

#### 8.2 setitimer() - 高精度定时器

~~~c

#include <sys/time.h>
int setitimer(int which, const struct itimerval *new_value, struct itimerval *old_value);

/*
参数解读
which: 定时器类型
    ITIMER_REAL: 真实时间
    ITIMER_VIRTUAL: 进程虚拟时间
    ITIMER_PROF: 进程和系统时间
    
new_value: 新的定时器设置

old_value: 保存旧的定时器设置（可为NULL）

返回值
成功返回0，失败返回-1

*/

/*
struct itimerval {
    struct timeval it_interval;  // 重复间隔
    struct timeval it_value;     // 初始延迟
};

struct timeval {
    time_t      tv_sec;   // 秒
    suseconds_t tv_usec;  // 微秒
};

*/

struct itimerval timer;
timer.it_value.tv_sec = 1;      // 1秒后开始
timer.it_value.tv_usec = 0;
timer.it_interval.tv_sec = 2;   // 每2秒重复
timer.it_interval.tv_usec = 0;

setitimer(ITIMER_REAL, &timer, NULL);
~~~

~~~c
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

void func(int sig)
{

    time_t now = time(NULL);
    if(now == -1){
        printf("time error\n");
        return ;
    }
    struct tm *local_time = localtime(&now);
    if(local_time == NULL){
        printf("localtime error\n");
        return ;
    }

    printf("\r当前时间: %02d:%02d:%02d",
            local_time->tm_hour,
            local_time->tm_min,
            local_time->tm_sec);

    fflush(stdout);

    //  alarm(1);
}

int main()
{
    int ret;
    long int i;
    long int j;
    struct itimerval timer;

    if(SIG_ERR == signal(SIGALRM,func)){
        printf("signal bind error\n");
        return -1;
    }

    //  alarm(1);
    timer.it_value.tv_sec = 3;      // 3秒后开始
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 1;   // 每1秒重复
    timer.it_interval.tv_usec = 0;

    if(setitimer(ITIMER_REAL, &timer, NULL) == -1){
        printf("setitimer error\n");
        return -1;
    }
    while(1){
        //  printf("main:print In while\n");
        for(i=0;i<10000;i++)
            for(j=0;j<80000;j++);
        //  ret = sleep(10);
        //  printf("ret = %d\n",ret);

    }
    return 0;
}

~~~

#### 8.3 POSIX定时器

特点

* 可选回调函数触发方式
* 可选信号类型
* 纳秒级别

编译加 -lrt

~~~c
#include <time.h>
#include <signal.h>

//创建posix定时器
int timer_create(clockid_t clk_id, struct sigevent *sevp, timer_t *timerid);
//启动定时器
int timer_settime(timer_t timerid, int flags, const struct itimerspec *new_value, struct itimerspec *old_value);
/*POSIX定时器 - 删除定时器,成功返回0，失败返回-1*/
int timer_delete(timer_t timerid);

/*
//创建posix定时器
int timer_create(clockid_t clk_id, struct sigevent *sevp, timer_t *timerid);

clk_id        // 时钟类型
    CLOCK_REALTIME      // 系统实时时间
    CLOCK_MONOTONIC     // 单调时间（不受系统时间调整影响）
    CLOCK_PROCESS_CPUTIME_ID  // 进程CPU时间
    CLOCK_THREAD_CPUTIME_ID   // 线程CPU时间

struct sigevent {
    int          sigev_notify;           // 通知类型
    int          sigev_signo;            // 信号编号
    union sigval sigev_value;            // 传递的数据
    void         (*sigev_notify_function)(union sigval);  // 回调函数
    pthread_attr_t *sigev_notify_attributes;              // 线程属性
};
        int sigev_notify;

        // 可选值：
        SIGEV_NONE     // 不通知
        SIGEV_SIGNAL   // 发送信号
        SIGEV_THREAD   // 创建线程执行回调函数
        
        int sigev_signo;

        // 当sigev_notify = SIGEV_SIGNAL时使用
        // 常用的信号：
        SIGALRM        // 闹钟信号
        SIGUSR1        // 用户自定义信号1
        SIGUSR2        // 用户自定义信号2

        union sigval {
            int   sival_int;    // 整数值
            void *sival_ptr;    // 指针值
        };


//启动定时器
int timer_settime(timer_t timerid, int flags, const struct itimerspec *new_value, struct itimerspec *old_value)
参数解读
timerid: 定时器ID

flags: 标志位
	0: 相对时间
	TIMER_ABSTIME: 绝对时间
	
new_value: 新的定时器设置
old_value: 保存旧的设置（可为NULL）


struct itimerspec {
    struct timespec it_interval;  // 重复间隔
    struct timespec it_value;     // 初始延迟
};
                                    struct timespec {
                                        time_t   tv_sec;   // 秒
                                        long     tv_nsec;  // 纳秒
                                    };
*/


timer_t timerid;

struct sigevent sev;
sev.sigev_notify = SIGEV_THREAD;
sev.sigev_notify_function = timer_handler;

timer_create(CLOCK_REALTIME, &sev, &timerid);

struct itimerspec its;
its.it_value.tv_sec = 1;
its.it_interval.tv_sec = 1;
timer_settime(timerid, 0, &its, NULL);



~~~

**示例一：用信号的方式触发**

~~~c
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>

void func(int sig)
{

	time_t now = time(NULL);
	if(now == -1){
		printf("time error\n");
		return ;
	}
	struct tm *local_time = localtime(&now);
	if(local_time == NULL){
		printf("localtime error\n");
		return ;
	}

	printf("\r当前时间: %02d:%02d:%02d",
			local_time->tm_hour,
			local_time->tm_min,
			local_time->tm_sec);

	fflush(stdout);

}

int main()
{
	int ret;
	long int i;
	long int j;
	timer_t timerid;



	//if(SIG_ERR == signal(SIGALRM,func)){
	if(SIG_ERR == signal(SIGUSR1,func)){
		printf("signal bind error\n");
		return -1;
	}

	struct sigevent sevp;
	sevp.sigev_notify = SIGEV_SIGNAL;
	//sevp.sigev_signo = SIGALRM;
	sevp.sigev_signo = SIGUSR1;
	ret = timer_create(CLOCK_REALTIME, &sevp, &timerid);
	if(ret == -1)
	{
		perror("timer_create error");
		return -1;
	}
	struct itimerspec timecontrl;
	//延迟启动时间
	timecontrl.it_value.tv_sec = 3;
	timecontrl.it_value.tv_nsec = 0;
	//定时时间
	timecontrl.it_interval.tv_sec = 1;
	timecontrl.it_interval.tv_nsec = 0;//纳秒如果不设置的话，启动定时器会报错，参数错误！！！！

	ret =  timer_settime(timerid, 0, &timecontrl, NULL);
	if(ret == -1)
	{
		printf("timer_settime error %s\n", strerror(errno));
		return -1;
	}	


	while(1){
		//	printf("main:print In while\n");
		for(i=0;i<10000;i++)
			for(j=0;j<80000;j++);
		//	ret = sleep(10);
		//	printf("ret = %d\n",ret);

	}
	return 0;
}

~~~

**示例二：不用信号，用线程触发回调函数**

~~~c
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>

void func(union sigval val)
{
	printf("func run!%s\n",(char *)val.sival_ptr);
	time_t now = time(NULL);
	if(now == -1){
		printf("time error\n");
		return ;
	}
	struct tm *local_time = localtime(&now);
	if(local_time == NULL){
		printf("localtime error\n");
		return ;
	}

	printf("\r当前时间: %02d:%02d:%02d",
			local_time->tm_hour,
			local_time->tm_min,
			local_time->tm_sec);

	fflush(stdout);

}

int main()
{
	int ret;
	long int i;
	long int j;
	timer_t timerid;


	struct sigevent sevp;
	sevp.sigev_notify = SIGEV_THREAD;
	sevp.sigev_notify_function = func;
	sevp.sigev_notify_attributes = NULL;	//线程属性不设置，就会导致线程创建失败，段错误!
	sevp.sigev_value.sival_ptr = (void *)"chenlichen shuai";
	//sevp.sigev_signo = SIGALRM;
	//sevp.sigev_signo = SIGUSR1;
	ret = timer_create(CLOCK_REALTIME, &sevp, &timerid);
	if(ret == -1)
	{
		perror("timer_create error");
		return -1;
	}
	struct itimerspec timecontrl;
	//延迟启动时间
	timecontrl.it_value.tv_sec = 3;
	timecontrl.it_value.tv_nsec = 0;
	//定时时间
	timecontrl.it_interval.tv_sec = 1;
	timecontrl.it_interval.tv_nsec = 0;

	ret =  timer_settime(timerid, 0, &timecontrl, NULL);
	if(ret == -1)
	{
		printf("timer_settime error %s\n", strerror(errno));
		return -1;
	}	


	while(1){
		//	printf("main:print In while\n");
		for(i=0;i<10000;i++)
			for(j=0;j<80000;j++);
		//	ret = sleep(10);
		//	printf("ret = %d\n",ret);

	}
	return 0;
}

~~~



#### 8.5 Linux定时器替换QTimer

参考视频21节，不建议。但是学习了一个静态串口句柄的玩法！

#### 8.5 对比

| API         | 精度   | 特点              |
| :---------- | :----- | :---------------- |
| alarm()     | 秒级   | 简单，只能一个    |
| setitimer() | 微秒级 | 支持重复，3种类型 |
| POSIX定时器 | 纳秒级 | 高精度，支持多个  |

POSIX标准的重要性：

* 可移植性：代码可以在不同UNIX系统间移植

* 标准化：提供统一的接口规范

* 兼容性：确保应用程序的兼容性

* 发展：推动操作系统接口的标准化

POSIX标准是现代UNIX/Linux系统编程的基础，掌握POSIX标准对于系统编程非常重要！

| 标准/接口         | 用途              | 特点                 |
| :---------------- | :---------------- | :------------------- |
| **POSIX**         | **系统编程基础**  | **可移植性，标准化** |
| **Linux系统调用** | **Linux特有功能** | **直接内核接口**     |
| Berkeley Socket   | 网络编程          | 跨平台网络接口       |
| X11/Wayland       | 图形界面          | 显示服务器协议       |
| ALSA/PulseAudio   | 音频处理          | 音频系统接口         |
| **System V IPC**  | **进程间通信**    | **传统IPC机制**      |
| D-Bus             | 现代IPC           | 桌面环境通信         |
| FUSE              | 用户空间文件系统  | 自定义文件系统       |
| OCI               | 容器标准          | 容器运行时           |
| eBPF              | 内核编程          | 高性能内核程序       |

这些标准和接口共同构成了现代Linux系统的完整生态，每种都有其特定的应用场景和优势！

## 九. Linux库专题

### 9.1 库的基本概念

**库就是一堆“已经写好的功能”被打包成文件**

Linux库或者厂家库比如瑞芯微音视频，AI库，全志音频编解码，视频解码库，图像的UI库如QT，LVGL

* 静态库：libxxx.a（简单理解：一本复印好的书，编译时整本塞进你的程序）。
* 共享库（动态库）：libxxx.so（像图书馆里的书，程序运行时去“借”来看，用完放回，所有程序可共用）

这些文件里装的是机器码（*.o 的集合），头文件（*.h）则是说明书，告诉你怎么调用这些机器码。

**为什么要用库**

* 省时间：别人已经写好了开平方、画窗口、编解码等常用功能，直接用，不重复造轮子。
* 省空间：动态库在内存/磁盘里只有一份，所有程序共用，系统不会给每个程序都复制一份。
* 易升级：更新 openssl、libc 等核心库，只需替换一个 *.so，所有依赖它的程序立即受益，无需重新编译。
* 易分工：大公司里 A 组做算法库，B 组做界面，双方只约定 *.h 接口即可并行开发。
* 保密：对外提供编译过的字节流和头文件，隐藏实现细节

库的概念不专门属于Linux，操作系统都有这样的概念，因为上操作系统的产品都非常庞大，需要系统内的库或者厂家库

**Windows 与 Linux 库概念对照表**

| 维度             | Linux                           | Windows                               | 备注/一句话解释                  |
| ---------------- | ------------------------------- | ------------------------------------- | -------------------------------- |
| **概念**         | 把常用代码预先编译成可复用文件  | 与 Linux 完全一致                     | 都是“现成功能包”                 |
| **静态库文件**   | libxxx.a                        | xxx.lib（或 legacy .a）               | 编译时整块复制进可执行文件       |
| **动态库文件**   | libxxx.so                       | xxx.dll（代码）+ xxx.lib（导入库）    | Windows 的 .lib 可能只是个“目录” |
| **头文件**       | xxx.h                           | xxx.h                                 | 声明接口，无差别                 |
| **编译选项**     | gcc main.c -lxxx                | cl main.c xxx.lib                     | Linux 用 -l，Windows 直接写 .lib |
| ****运行时查找** | LD_LIBRARY_PATH、/lib、/usr/lib | 当前目录 → PATH → System32            | 找不到就报错                     |
| **安装目录**     | /usr/local/lib 等               | %ProgramFiles%\Vendor\Lib 或 System32 | 系统约定不同                     |

结论：概念相同，只是文件后缀、命名约定和工具链差异。

**使用库，以数学库为例**

~~~shell
mkdir ~/learn_lib && cd ~/learn_lib
~~~

~~~c
// main.c
#include <stdio.h>
#include <math.h>      // 系统头文件
int main() {
    double x = sqrt(49.0);   // sqrt 在 libm.so 里
    printf("sqrt(49)=%.1f\n", x);
    return 0;
}
~~~

编译链接：

~~~c
gcc main.c -o main -lm      # -lm 就是链接 libm.so
./main
~~~

你已经会“使用系统库”了，核心就两步：

- \#include 头文件 → 让编译器知道函数原型
- -lxxx 链接库 → 让链接器找到实现

### 9.2 制作并调用库

#### 9.2.1编译过程

一步到位的简写

* 预处理（Pre-processing） 把 `#include`、宏展开，生成纯 C 文件 `.i`

  * ```bash
    gcc -E main.c -o main.i
    ```

* 编译（Compilation）,把 `.i` 翻译成汇编 `.s`

  * ```bash
    gcc -S main.i -o main.s
    ```

* 汇编（Assembly）把 `.s` 翻译成机器码，生成可重定位目标文件 `.o`

  * ```bash
    gcc -c main.s -o main.o
    ```

* 链接（Linking）

  * ```bash
    gcc main.o -o main       
    ```

#### 9.2.2 自制库

**先搞清楚几个概念**

* 源文件：你写的 .c / .cpp
* 目标文件：gcc 编译后得到的 .o
* 静态库：把一堆 .o 打包成 libxxx.a，链接时整块复制到可执行文件里。
* 共享库（动态库）：把一堆 .o 打包成 libxxx.so，运行时由系统动态加载，体积更小、升级方便。

**目标：写一个自己的数学函数 mysqrt()，做成库给 main 调用。**

```bash
mkdir mymath && cd mymath
```

mysqrt.h

~~~c
#ifndef MYSQRT_H
#define MYSQRT_H
double mysqrt(double x);
#endif
~~~

mysqrt.c

~~~c
#include "mysqrt.h"
#include <math.h>
double mysqrt(double x) { return sqrt(x); }
~~~

编译成目标文件

~~~shell
gcc -c mysqrt.c -o mysqrt.o   # -c 只编译不链接
~~~

打包成静态库

~~~c
ar rcs libmysqrt.a mysqrt.o   # rcs=replace/create/symbol
~~~

打包成共享库

```bash
gcc -shared -fPIC mysqrt.o -o libmysqrt.so
```

用我们刚做的库，退回上层目录：

~~~shell
cd ~/learn_lib
~~~

写新的 main_use_lib.c

~~~c
#include <stdio.h>
#include "mymath/mysqrt.h"   // 注意路径

int main() {
    double v = mysqrt(81.0);
    printf("mysqrt(81)=%.1f\n", v);
    return 0;
}
~~~

方式 A：链接静态库

~~~shell
gcc main_use_lib.c -L./mymath -lmysqrt -I./mymath -o main_static
# -L 告诉链接器去 mymath 目录找库
# -I 告诉预处理器去 mymath 目录找头文件
./main_static        # 输出 mysqrt(81)=9.0
~~~

方式 B：链接共享库

~~~shell
gcc main_use_lib.c -L./mymath -lmysqrt -I./mymath -o main_shared
export LD_LIBRARY_PATH=./mymath:$LD_LIBRARY_PATH   # 运行时告诉系统去哪找 .so
./main_shared
~~~

让系统“永久”找到你的共享库（可选）

把 libmysqrt.so 拷到系统目录
sudo cp mymath/libmysqrt.so /usr/local/lib/

刷新缓存
sudo ldconfig

### 9.3 交叉编译库和程序

~~~
 arm-openwrt-linux-gcc 替代 gcc
 arm-openwrt-linux-ar 替代 ar
~~~

## 十. T113音频应用

### 10.1 音频开发体验

跑起来再说！

#### 10.1.1 命令操作

**查看声卡命令**

~~~shell
#查看声卡
root@TinaLinux:/# cat /proc/asound/cards
 0 [audiocodec     ]: audiocodec - audiocodec
                      audiocodec
 1 [sndspdif       ]: sndspdif - sndspdif
                      sndspdif


#查看音频设备
root@TinaLinux:/# aplay -l
**** List of PLAYBACK Hardware Devices ****
card 0: audiocodec [audiocodec], device 0: SUNXI-CODEC 2030000.codec-0 []
  Subdevices: 1/1
  Subdevice #0: subdevice #0
card 1: sndspdif [sndspdif], device 0: 2036000.spdif-snd-soc-dummy-dai snd-soc-dummy-dai-0 []
  Subdevices: 1/1
  Subdevice #0: subdevice #0

#查看audiocodec对应的设备
root@TinaLinux:/# ls -l /dev/snd/pcmC0D*  #真正的声卡设备名，hw:audiocodec只是别名
crw-------    1 root     root      116,  24 Jan  1  1970 /dev/snd/pcmC0D0c  #录制
crw-------    1 root     root      116,  16 Jan  1  1970 /dev/snd/pcmC0D0p  #播放



~~~

`/proc` 是 **内核实时信息的“文本窗口”**，而 `/dev` 是 **设备节点（文件）的“入口目录”**。

| 维度         | `/proc`                                                      | `/dev`                                                       |
| ------------ | ------------------------------------------------------------ | ------------------------------------------------------------ |
| **作用**     | 把内核运行时的状态、参数暴露给用户（只读/可写文本）          | 提供对硬件/内核子系统的 **文件操作接口**（读写、ioctl）      |
| **来源**     | 内核 **procfs**（虚拟文件系统，挂载在 `/proc`）              | 内核 **devtmpfs/udev**（虚拟文件系统，挂载在 `/dev`）        |
| **内容**     | 纯文本、数字、目录树：进程 PID、CPU、内存、网络、声卡列表等  | 字符/块设备节点、符号链接：`/dev/sda`, `/dev/snd/pcmC0D0c`, `/dev/null` … |
| **I/O 行为** | 读文件 = 取内核信息；写文件 = 改内核参数（如 `/proc/sys/net/ipv4/ip_forward`） | 读写 = 与设备交互（如 `cat /dev/urandom` 取随机数）          |

类比：

- `/proc` 像 **仪表盘**，告诉你车现在速度、油量、引擎温度。
- `/dev` 像 **方向盘、油门、刹车**，你通过它 **控制**车。



**开启录制音频通路**

`amixer` 是 **ALSA（Advanced Linux Sound Architecture）** 提供的一个**命令行混音器工具**，用来读取或设置 **声卡控制参数**（音量、开关、路由等），无需写程序即可直接操控硬件。

<img src="G:\202507\pics\audio.png" style="zoom: 50%;" />  

~~~shell
#查看音频控制项
amixer -D hw:audiocodec controls
#   选路： 把 ADC3 的输入 MUX 切到 MIC3（0 代表 MIC3，1/2/3 可能是 LINE/FMIN 等）。 这一步决定了“声音从哪进来”。
amixer -D hw:audiocodec cset name='MIC3 Input Select' 0 
#   打开前置放大器：使能 片上 20 dB Boost，先把麦克风毫伏级信号放大到百毫伏级，减轻后端噪声。
amixer -D hw:audiocodec cset name='ADC3 Input MIC3 Boost Switch' 1 
#   设模拟增益:把 MIC3 前置 PGA 设为 30
amixer -D hw:audiocodec cset name='MIC3 gain volume' 30

#执行完后，ADC3 的录音通道已处于“最大灵敏度”状态，后续用 arecord / snd_pcm_open 打开 hw:0,3（即 hw:audiocodec,3）即可开始录音。
~~~

下面把 audiocodec 这块声卡的所有 control 项按“功能块”帮你梳理成一张速查表，并给出常用读写示例，方便后续脚本或应用直接调用。

------

**控制项分区速查**

| 功能块            | numid   | 名称（name）                 | 类型 | 典型用途 & 说明             |
| ----------------- | ------- | ---------------------------- | ---- | --------------------------- |
| **耳机输出**      | 17      | Headphone volume             | INT  | 0-63，耳机 DAC 数字音量     |
|                   | 30      | Headphone Switch             | BOOL | 0=关闭，1=开启耳机放大器    |
| **HP/扬声器切换** | 31      | HpSpeaker Switch             | BOOL | 0=耳机，1=外放；二选一      |
| **LINEOUT**       | 32      | LINEOUT Switch               | BOOL | 线路输出总开关              |
|                   | 16      | LINEOUT volume               | INT  | 0-63，线路输出数字音量      |
|                   | 18      | LINEOUTL Output Select       | ENUM | 左声道路由选择（见下文）    |
|                   | 19      | LINEOUTR Output Select       | ENUM | 右声道路由选择              |
| **ADC1**          | 6       | ADC1 volume                  | INT  | 0-31，ADC1 数字增益         |
|                   | 23      | ADC1 Input MIC1 Boost Switch | BOOL | 打开 MIC1 20 dB 增益        |
|                   | 24      | ADC1 Input FMINL Switch      | BOOL | 把 FMINL 接到 ADC1          |
|                   | 25      | ADC1 Input LINEINL Switch    | BOOL | 把 LINEINL 接到 ADC1        |
| **ADC2**          | 7       | ADC2 volume                  | INT  | 同上，右声道                |
|                   | 26      | ADC2 Input MIC2 Boost Switch | BOOL | MIC2 20 dB                  |
|                   | 27      | ADC2 Input FMINR Switch      | BOOL | FMINR → ADC2                |
|                   | 28      | ADC2 Input LINEINR Switch    | BOOL | LINEINR → ADC2              |
| **ADC3**          | 8       | ADC3 volume                  | INT  | 第三路 ADC                  |
|                   | 29      | ADC3 Input MIC3 Boost Switch | BOOL | MIC3 20 dB                  |
| **输入增益**      | 9/10/11 | MIC1/2/3 gain volume         | INT  | 模拟增益 0-7                |
|                   | 14/15   | LINEINL/R gain volume        | INT  | LINEIN 模拟增益 0-7         |
|                   | 12/13   | FMINL/R gain volume          | INT  | FM 输入模拟增益 0-7         |
| **数字/全局**     | 5       | DAC volume                   | INT  | 全局 DAC 数字音量           |
|                   | 4       | digital volume               | INT  | 额外数字衰减                |
|                   | 1       | codec hub mode               | INT  | 0/1，芯片工作模式（手册位） |
| **通道交换**      | 2       | ADC1 ADC2 swap               | BOOL | 0=正常，1=左右声道互换      |
|                   | 3       | ADC3 ADC4 swap               | BOOL | 同上                        |

**举例：打开耳机并设置音量 50**

```bash
amixer -D hw:audiocodec cset numid=30 1
amixer -D hw:audiocodec cset numid=17 50
```



**录音命令**

~~~
arecord -D hw:audiocodec -f S16_LE -t wav -r 16000 -d 5 test.wav
~~~

**开启播放音频通路**

~~~
amixer -D hw:audiocodec cset name='Headphone Switch' 1 && \
amixer -D hw:audiocodec cset name='Headphone volume' 25
~~~

**播放音频**

~~~
aplay -D hw:audiocodec test.wav
~~~

#### 10.1.2 代码程序操作

**参考代码**

~~~c
#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

#define PCM_DEVICE "hw:audiocodec" // 或 "hw:0,3"
#define SAMPLE_RATE 16000
#define CHANNELS 1
#define SAMPLE_FORMAT SND_PCM_FORMAT_S16_LE
#define RECORD_SECONDS 5
#define BUFFER_FRAMES 1024

int main() {
    snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *params;
    int rc;
    FILE *output = fopen("record.pcm", "wb");
    if (!output) {
        perror("fopen");
        return 1;
    }

    rc = snd_pcm_open(&pcm_handle, PCM_DEVICE, SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0) {
        fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(rc));
        return 1;
    }
	
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(pcm_handle, params);
    snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(pcm_handle, params, SAMPLE_FORMAT);
    snd_pcm_hw_params_set_channels(pcm_handle, params, CHANNELS);
    snd_pcm_hw_params_set_rate(pcm_handle, params, SAMPLE_RATE, 0);
    snd_pcm_hw_params(pcm_handle, params);

    int frames = BUFFER_FRAMES;
    int16_t *buffer = (int16_t *)malloc(frames * CHANNELS * sizeof(int16_t));
    int loops = SAMPLE_RATE * RECORD_SECONDS / frames;

    for (int i = 0; i < loops; ++i) {
        rc = snd_pcm_readi(pcm_handle, buffer, frames);
        if (rc == -EPIPE) {
            snd_pcm_prepare(pcm_handle);
            continue;
        } else if (rc < 0) {
            fprintf(stderr, "error from read: %s\n", snd_strerror(rc));
        } else if (rc != frames) {
            fprintf(stderr, "short read, read %d frames\n", rc);
        }
        fwrite(buffer, sizeof(int16_t), rc * CHANNELS, output);
    }

    free(buffer);
    fclose(output);
    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);
    printf("录音完成，已保存为 record.pcm\n");
    return 0;
}
~~~

**全志T113音频库位置**

~~~shell
/home/zhiwan/t113/Tina-Linux/out/t113-zhiwan_v1/staging_dir/target/usr/lib
音频库文件：
libasound.so
libasound.so.2
libasound.so.2.0.0
~~~

**编译指令**

~~~
arm-openwrt-linux-gcc testsound.c -L /home/zhiwan/t113/Tina-Linux/out/t113-zhiwan_v1/staging_dir/target/usr/lib/ -lasound -I /home/zhiwan/t113/Tina-Linux/out/t113-zhiwan_v1/staging_dir/target/usr/include/

播放音频：aplay -D hw:audiocodec -f S16_LE -r 16000 -c 1 record.pcm
~~~

### 10.2 Linux系统编程文件专题

见文件专题附件文件

* Linux文件系统，一切皆文件的概念
* Linux文件相关属性
* Linux文件编程两套API系统
* Linux高并发日志系统解决方案
* Linux大文件断点续传方案
* Linux高效文件存储检索方案

### 10.3 音频基本概念

* 声音格式
* 音频参数
* 音频文件大小计算
* 音频质量和文件大小关系
* 音频处理相关概念
* 实际应用示例

### 10.4 Linux音频ALSA开发专题

* 音频框架概述
* ALSA框架详解
* 应用层常用命令接口
* ALSA应用层API接口
* 实战开发
  * 音频录制
  * 音频播放
* ARM-Linux录音机
  * QT工程集成ALSA开发
  * QT线程引入
  * 多线程实现录制和播放
  * 信号与槽添加暂停功能
  * 线程互斥锁控制多线程录制
  * UI收尾-进度条-时间显示-波线图











