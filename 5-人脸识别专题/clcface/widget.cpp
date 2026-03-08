#include "widget.h"                 // 包含Widget类的头文件
#include "ui_widget.h"               // 包含Qt Designer生成的UI界面头文件
#include <QImage>                     // Qt图像类，用于图像处理
#include <QPixmap>                    // Qt像素图类，用于图像显示
#include <opencv2/opencv.hpp>         // OpenCV核心库
#include <QTimer>                     // Qt定时器类
#include <QDebug>                     // Qt调试输出类
#include <QPainter>                   // Qt绘图类，用于在图像上绘制矩形框


// 将OpenCV的Mat格式转换为Qt的QImage格式
// 参数: mat - OpenCV的Mat图像对象
// 返回: 转换后的QImage对象
QImage Widget::cvMatToQImage(const cv::Mat& mat) {
    switch (mat.type()) {             // 根据Mat的数据类型进行不同处理
        case CV_8UC4: {               // 4通道8位无符号整数（BGRA格式）
            // 创建QImage，直接使用Mat的数据指针，避免数据拷贝
            // mat.data: 图像数据指针
            // mat.cols: 图像宽度（列数）
            // mat.rows: 图像高度（行数）
            // mat.step: 每行的字节数（步长）
            // QImage::Format_RGB32: 32位RGB格式
            QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB32);
            return image.rgbSwapped().copy();  // 交换RGB通道（OpenCV是BGR，Qt是RGB），并复制数据
        }
        case CV_8UC3: {               // 3通道8位无符号整数（BGR格式）
            // 创建RGB888格式的QImage
            QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
            return image.rgbSwapped().copy();  // 交换RGB通道并复制数据
        }
        case CV_8UC1: {               // 单通道8位无符号整数（灰度图）
            // 创建灰度图格式的QImage
            QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8);
            return image.copy();      // 灰度图不需要交换通道，直接复制
        }
        default:                       // 其他不支持的格式
            qDebug() << "不支持的图像格式";  // 输出错误信息
            return QImage();           // 返回空的QImage对象
    }
}

// Widget类的构造函数，初始化所有成员变量和资源
// 参数: parent - 父窗口指针，默认为空
Widget::Widget(QWidget *parent)
    : QWidget(parent)                 // 调用基类QWidget的构造函数，传入父窗口指针
    , ui(new Ui::Widget)              // 创建UI界面对象，用于访问界面控件
    , timer(new QTimer(this))         // 创建定时器对象，this作为父对象，自动管理内存
{
    ui->setupUi(this);               // 设置UI界面，将界面控件与当前Widget对象关联

    // 加载Haar Cascade人脸检测分类器
    // 参数: XML文件路径，包含训练好的人脸检测模型
    loadFaceCascade("/root/haarcascade_frontalface_default.xml");
    
    // 使用OpenCV打开摄像头
    cap.open(0);                      // 打开默认摄像头，0表示第一个可用摄像头，也可以使用"/dev/video0"等设备路径
    if (!cap.isOpened()) {            // 检查摄像头是否成功打开
        qDebug() << "打开摄像头失败";  // 输出错误信息
    } else {
        // 设置摄像头分辨率
        cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);   // 设置帧宽度为640像素
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);   // 设置帧高度为480像素
        qDebug() << "摄像头打开成功";  // 输出成功信息
    }
    
    // 从ONNX文件加载深度学习模型，用于人脸特征提取
    // 参数: ONNX模型文件路径
    faceNet = cv::dnn::readNetFromONNX("/mnt/zeng_sim.onnx");
    
    // 连接定时器的timeout信号到updateFrame槽函数
    // 当定时器超时时，自动调用updateFrame函数更新画面
    connect(timer, &QTimer::timeout, this, &Widget::updateFrame);
    
    // 启动定时器，设置间隔为30毫秒（约33帧/秒）
    timer->start(30);
}

// Widget类的析构函数，释放所有资源
Widget::~Widget()
{
    if (cap.isOpened()) {             // 检查摄像头是否已打开
        cap.release();                 // 释放摄像头资源，关闭视频流
    }
    delete ui;                         // 删除UI界面对象，释放内存
}

// 加载Haar Cascade人脸检测分类器
// 参数: path - XML分类器文件的路径（QString类型）
// 返回: 加载是否成功（true表示成功，false表示失败）
bool Widget::loadFaceCascade(const QString &path) {
    // 将QString转换为std::string，然后加载分类器文件
    return face_cascade.load(path.toStdString());
}
// 定时器触发的更新函数，获取视频帧、检测人脸并显示
// 该函数每30毫秒被调用一次，实现实时视频显示
void Widget::updateFrame()
{
    static int detect_counter = 0;    // 静态计数器，记录已处理的帧数（函数调用次数）
    const int detect_interval = 10;   // 检测间隔，每10帧检测一次人脸（性能优化）
    
    // 使用OpenCV获取视频帧
    cv::Mat frame;                    // 创建Mat对象存储视频帧
    // 从摄像头读取一帧图像，read()返回false或图像为空表示读取失败
    if (!cap.read(frame) || frame.empty()) {
        qDebug() << "未获取到视频帧";  // 输出错误信息
        return;                        // 提前返回，不执行后续操作
    }
    
    // 将OpenCV的Mat格式转换为Qt的QImage格式，用于显示
    QImage img = cvMatToQImage(frame);
    if (img.isNull()) {               // 检查转换是否成功
        qDebug() << "图像转换失败";    // 输出错误信息
        return;                        // 提前返回
    }
    
    // 判断是否需要执行人脸检测（每10帧检测一次）
    // ++detect_counter: 先递增计数器
    // % detect_interval: 取模运算，当计数器是10的倍数时返回0
    bool doDetect = (++detect_counter % detect_interval == 0);
    
    // 如果需要检测且分类器已加载，则执行人脸检测
    if (doDetect && !face_cascade.empty()) {
        // 缩小检测分辨率以提高速度（性能优化策略）
        cv::Mat smallFrame;            // 创建小尺寸帧对象
        // 将原图缩放到320x240，减少检测计算量
        cv::resize(frame, smallFrame, cv::Size(320, 240));
        
        cv::Mat mat_gray;              // 创建灰度图对象
        // 将BGR彩色图转换为灰度图（Haar Cascade需要灰度图）
        cv::cvtColor(smallFrame, mat_gray, cv::COLOR_BGR2GRAY);
        
        faces.clear();                 // 清空之前检测到的人脸列表
        
        // 使用Haar Cascade进行多尺度人脸检测
        // 参数说明:
        // mat_gray: 输入的灰度图像
        // faces: 输出的人脸矩形区域列表
        // 1.2: 缩放因子，每次检测窗口放大1.2倍
        // 5: 最小邻居数，候选矩形需要至少5个邻居才被认为是人脸（减少误检）
        // 0: 标志位，0表示使用默认设置
        // cv::Size(60, 60): 最小人脸尺寸，小于60x60的目标被忽略
        face_cascade.detectMultiScale(mat_gray, faces, 1.2, 5, 0, cv::Size(60, 60));
        
        // 将检测结果从缩小图映射回原图尺寸
        // 计算X方向的缩放比例
        double scaleX = double(frame.cols) / smallFrame.cols;
        // 计算Y方向的缩放比例
        double scaleY = double(frame.rows) / smallFrame.rows;
        
        // 遍历所有检测到的人脸，将坐标和尺寸按比例放大
        for (auto &face : faces) {
            face.x = int(face.x * scaleX);        // 调整X坐标
            face.y = int(face.y * scaleY);        // 调整Y坐标
            face.width = int(face.width * scaleX); // 调整宽度
            face.height = int(face.height * scaleY); // 调整高度
        }
        qDebug() << "检测到人脸数量:" << faces.size();  // 输出检测到的人脸数量
    } else if (face_cascade.empty()) {  // 如果分类器未加载
        qDebug() << "人脸分类器未加载成功";  // 输出警告信息
    }
    
    // 在图像上绘制检测框，标记检测到的人脸位置
    QPainter painter(&img);           // 创建绘图对象，在img图像上绘制
    painter.setPen(QPen(Qt::red, 3)); // 设置画笔为红色，线宽3像素
    // 遍历所有检测到的人脸，绘制红色矩形框
    for (const auto &face : faces) {
        // 绘制矩形框，参数: x坐标, y坐标, 宽度, 高度
        painter.drawRect(face.x, face.y, face.width, face.height);
    }
    painter.end();                    // 结束绘制操作
    
    // 将处理后的图像显示到界面上
    QSize targetSize(780, 520);       // 定义目标显示尺寸
    // 缩放图像到目标尺寸（保持宽高比），转换为QPixmap，并设置到界面标签
    ui->camlab->setPixmap(QPixmap::fromImage(img.scaled(targetSize, Qt::KeepAspectRatio)));
}
// 从人脸图像中提取特征向量（使用深度学习模型）
// 参数: faceImg - 输入的人脸图像（OpenCV Mat格式）
// 返回: 归一化后的特征向量（Mat格式）
cv::Mat Widget::getFaceFeature(const cv::Mat& faceImg) {
    cv::Mat input;                    // 创建输入图像对象
    // 将人脸图像调整到模型要求的输入尺寸（112x112）
    cv::resize(faceImg, input, cv::Size(112, 112));
    
    // 将像素值从[0,255]归一化到[0,1]范围
    // CV_32F: 32位浮点数类型
    // 1.0/255: 归一化系数
    input.convertTo(input, CV_32F, 1.0/255);
    
    // 将图像转换为DNN模型需要的blob格式（批量、通道、高、宽）
    // 参数说明:
    // input: 输入图像
    // 1.0: 缩放因子（已归一化，所以为1.0）
    // cv::Size(112,112): 目标尺寸
    // cv::Scalar(0,0,0): 均值（已归一化，所以为0）
    // true: swapRB=true，交换红蓝通道（OpenCV是BGR，模型可能需要RGB）
    // false: crop=false，不裁剪图像
    cv::Mat blob = cv::dnn::blobFromImage(input, 1.0, cv::Size(112,112), cv::Scalar(0,0,0), true, false);
    
    // 将blob设置为DNN网络的输入
    faceNet.setInput(blob);
    
    // 执行前向传播，获取网络输出（特征向量）
    cv::Mat feat = faceNet.forward();
    
    // L2归一化特征向量，使特征向量的模长为1
    // 归一化后，两个向量的点积等于它们的余弦相似度
    cv::normalize(feat, feat);
    
    // 返回特征向量的副本（避免返回局部变量的引用）
    return feat.clone();
}

// 采集按钮点击事件处理函数，采集当前检测到的人脸特征
// 该函数会提取人脸特征向量并存储，用于后续的人脸识别
void Widget::on_btncaiji_clicked()
{
    // 检查是否检测到人脸
    if (faces.empty()) {
        qDebug() << "未检测到人脸，无法采集";  // 输出提示信息
        return;                        // 提前返回，不执行采集操作
    }
    
    // 检查是否已达到目标采集数量
    if (collectCount >= collectTarget) {
        qDebug() << "已采集" << collectTarget << "张人脸，请勿重复采集";  // 输出提示信息
        return;                        // 提前返回，防止重复采集
    }
    
    // 从检测到的多个人脸中选择面积最大的人脸（通常是最清晰、最正面的）
    // std::max_element: 查找最大元素
    // faces.begin(), faces.end(): 查找范围
    // lambda表达式: 比较两个人脸的面积，返回面积较小的（用于找最大）
    auto maxFace = std::max_element(faces.begin(), faces.end(),
        [](const cv::Rect& a, const cv::Rect& b) { return a.area() < b.area(); });
    
    // 检查是否找到最大人脸（理论上不会为空，但安全起见进行检查）
    if (maxFace == faces.end()) return;
    
    // 使用OpenCV获取当前视频帧
    cv::Mat frame;                    // 创建帧对象
    // 从摄像头读取最新帧
    if (!cap.read(frame) || frame.empty()) {
        qDebug() << "获取视频帧失败";  // 输出错误信息
        return;                        // 提前返回
    }
    
    // 裁剪人脸区域
    // 创建人脸矩形区域对象
    cv::Rect faceRect(maxFace->x, maxFace->y, maxFace->width, maxFace->height);
    // 从原图中提取人脸区域并克隆（避免引用原图数据）
    cv::Mat faceMat = frame(faceRect).clone();
    // 调整人脸图像尺寸到112x112（模型输入要求）
    cv::resize(faceMat, faceMat, cv::Size(112, 112));
    
    // 提取人脸特征向量
    cv::Mat feat = getFaceFeature(faceMat);
    
    // 将特征向量添加到已注册特征列表（克隆数据，避免引用局部变量）
    registeredFeatures.append(feat.clone());
    collectCount++;                    // 增加采集计数
    qDebug() << "第" << collectCount << "次采集成功";  // 输出成功信息
}

// 比对按钮点击事件处理函数，将当前人脸与已注册的人脸进行比对
// 该函数会计算相似度并判断是否为同一人
void Widget::on_btnbidui_clicked()
{
    // 检查是否已完成足够数量的采集（需要采集3张才能比对）
    if (collectCount < collectTarget) {
        qDebug() << "请先采集" << collectTarget << "张人脸";  // 输出提示信息
        return;                        // 提前返回
    }
    
    // 检查是否检测到人脸
    if (faces.empty()) {
        qDebug() << "未检测到人脸，无法比对";  // 输出提示信息
        return;                        // 提前返回
    }
    
    // 从检测到的多个人脸中选择面积最大的人脸
    auto maxFace = std::max_element(faces.begin(), faces.end(),
        [](const cv::Rect& a, const cv::Rect& b) { return a.area() < b.area(); });
    // 检查是否找到最大人脸
    if (maxFace == faces.end()) return;
    
    // 使用OpenCV获取当前视频帧
    cv::Mat frame;                    // 创建帧对象
    // 从摄像头读取最新帧
    if (!cap.read(frame) || frame.empty()) {
        qDebug() << "获取视频帧失败";  // 输出错误信息
        return;                        // 提前返回
    }
    
    // 裁剪人脸区域
    // 创建人脸矩形区域对象
    cv::Rect faceRect(maxFace->x, maxFace->y, maxFace->width, maxFace->height);
    // 从原图中提取人脸区域并克隆
    cv::Mat faceMat = frame(faceRect).clone();
    // 调整人脸图像尺寸到112x112（模型输入要求）
    cv::resize(faceMat, faceMat, cv::Size(112, 112));
    
    // 提取当前人脸的特征向量
    cv::Mat feat = getFaceFeature(faceMat);
    
    // 与所有已注册的特征向量进行比对，找出最大相似度
    double maxSim = -1;               // 初始化最大相似度为-1
    // 遍历所有已注册的特征向量
    for (const auto& regFeat : registeredFeatures) {
        // 计算余弦相似度（由于特征向量已L2归一化，点积等于余弦相似度）
        // dot(): 计算两个向量的点积
        double sim = feat.dot(regFeat);
        // 更新最大相似度
        if (sim > maxSim) maxSim = sim;
    }
    
    // 输出最大相似度值
    qDebug() << "最大相似度:" << maxSim;
    
    // 根据相似度阈值判断是否为同一人
    // 阈值0.5是经验值，可根据实际应用调整
    if (maxSim > 0.5) {
        qDebug() << "比对结果：同一人";  // 相似度大于阈值，判定为同一人
    } else {
        qDebug() << "比对结果：不同人";  // 相似度小于阈值，判定为不同人
    }
}
