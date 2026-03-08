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

    // 使用OpenCV打开摄像头
    cap.open(0);                      // 打开默认摄像头，0表示第一个可用摄像头，也可以使用"/dev/video0"等设备路径
    if (!cap.isOpened()) {            // 检查摄像头 是否成功打开
        qDebug() << "打开摄像头失败";  // 输出错误信息
    } else {
        // 设置摄像头分辨率
        cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);   // 设置帧宽度为640像素
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);   // 设置帧高度为480像素
        cap.set(cv::CAP_PROP_FPS, 30);            // 设置帧率（可选）
        cap.set(cv::CAP_PROP_BUFFERSIZE, 1);     //优化图像获取速度
        qDebug() << "摄像头打开成功";  // 输出成功信息
    }
    QString path = "/root/haarcascade_frontalface_default.xml";

    face_cascade.load(path.toStdString());


    // 连接定时器的timeout信号到updateFrame槽函数
    // 当定时器超时时，自动调用updateFrame函数更新画面
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Widget::updateFrame);

    // 启动定时器，设置间隔为30毫秒（约33帧/秒）
    timer->start(30);


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

void Widget::updateFrame()
{

    static int detect_counter = 0;    // 静态计数器，记录已处理的帧数（函数调用次数）
    const int detect_interval = 10;   // 检测间隔，每10帧检测一次人脸（性能优化）


    cv::Mat frame;                    // 创建Mat对象存储视频帧
    // 从摄像头读取一帧图像，read()返回false或图像为空表示读取失败
    if (!cap.read(frame) || frame.empty()) {
        qDebug() << "未获取到视频帧";  // 输出错误信息
        return;                        // 提前返回，不执行后续操作
    }
    // 将OpenCV的Mat格式转换为Qt的QImage格式，用于显示
    QImage image(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
    image = image.rgbSwapped().copy();

    //img.scaled(targetSize, Qt::KeepAspectRatio))
    //ui->labelcam->size();
    image = image.scaled(QSize(471,531),Qt::KeepAspectRatioByExpanding);

    bool doDetect = (++detect_counter % detect_interval == 0);
    if(doDetect && !face_cascade.empty()){
        cv::Mat smallFrame;            // 创建小尺寸帧对象
        // 将原图缩放到320x240，减少检测计算量
        cv::resize(frame, smallFrame, cv::Size(320, 240));

        cv::Mat mat_gray;              // 创建灰度图对象
        // 将BGR彩色图转换为灰度图（Haar Cascade需要灰度图）
        cv::cvtColor(smallFrame, mat_gray, cv::COLOR_BGR2GRAY);
        faces.clear();                 // 清空之前检测到的人脸列表
        // 执行多尺度人脸检测
        face_cascade.detectMultiScale(
                    mat_gray,              // 输入灰度图
                    faces,                 // 输出检测结果
                    1.2,                   // scaleFactor: 每次缩放比例
                    5,                     // minNeighbors: 最小邻居数
                    0,                     // flags: 检测标志
                    cv::Size(60, 60)       // minSize: 最小人脸尺寸
                    );
        // 遍历所有检测到的人脸，将坐标和尺寸按比例放大
        for (auto &face : faces) {
            face.x = int(face.x * 2) + 30 ;        // 调整X坐标
            face.y = int(face.y * 2) -25;        // 调整Y坐标
            face.width = int(face.width * 2) + 30; // 调整宽度
            face.height = int(face.height * 2) + 55; // 调整高度
        }
    }

    // 在图像上绘制检测框，标记检测到的人脸位置
    QPainter painter(&image);           // 创建绘图对象，在img图像上绘制
    painter.setPen(QPen(Qt::red, 3)); // 设置画笔为红色，线宽3像素
    // 遍历所有检测到的人脸，绘制红色矩形框
    for (const auto &face : faces) {
        // 绘制矩形框，参数: x坐标, y坐标, 宽度, 高度
        painter.drawRect(face.x, face.y, face.width, face.height);
    }
    painter.end();

    ui->labelcam->setPixmap(QPixmap::fromImage(image));

}


Widget::~Widget()
{
    delete ui;
}

