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
    QString path = "/mnt/haarcascade_frontalface_default.xml";
    // 从ONNX文件加载深度学习模型，用于人脸特征提取
    // 参数: ONNX模型文件路径
    faceNet = cv::dnn::readNetFromONNX("/mnt/zeng_sim.onnx");

    face_cascade.load(path.toStdString());


    // 连接定时器的timeout信号到updateFrame槽函数
    // 当定时器超时时，自动调用updateFrame函数更新画面
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Widget::updateFrame);

    // 启动定时器，设置间隔为30毫秒（约33帧/秒）
    timer->start(30);

    addUserWidget = new AddUserWidget(this);
    addUserWidget->hide();

    m_face_registered = false;
    // 加载已经注册过的特征量，用于人脸识别, 读取特征量文件中特征数据到特征量列表里
    loadRegisteredFeature();
    // 创建人脸识别线程
    if( pthread_create(&faceRegnizeThread, nullptr, faceRegnizeFunc, this) !=0 ){
        qDebug() << "创建识别线程失败";
    }

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
        facesOriginal = faces;
        for (auto &face : faces) {
            face.x = int(face.x * 2) + 30 ;        // 调整X坐标
            face.y = int(face.y * 2) -25;        // 调整Y坐标
            face.width = int(face.width * 2) + 30; // 调整宽度
            face.height = int(face.height * 2) + 55; // 调整高度
        }

        // 发起人脸对比
        //faceRegnizetion();

    }

    // 在图像上绘制检测框，标记检测到的人脸位置
    QPainter painter(&image);           // 创建绘图对象，在img图像上绘制
    if(m_face_registered == true){
        painter.setPen(QPen(Qt::green, 3)); // 设置画笔为红色，线宽3像素
    }else{
        painter.setPen(QPen(Qt::red, 3)); // 设置画笔为红色，线宽3像素
    }
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

void Widget::getFaceFeatureInVector()
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
    cv::Rect* maxFace = nullptr;
    int maxArea = 0;
    for(int i = 0; i < facesOriginal.size(); i++){
        int currentArea = facesOriginal[i].area();
        if(currentArea > maxArea){
            maxArea = currentArea;
            maxFace = &facesOriginal[i];
        }
    }
    if(maxFace == nullptr) return ;

    // 使用OpenCV获取当前视频帧
    cv::Mat frame;                    // 创建帧对象
    // 从摄像头读取最新帧
    if (!cap.read(frame) || frame.empty()) {
        qDebug() << "获取视频帧失败";  // 输出错误信息
        return;                        // 提前返回
    }

    cv::Mat smallFrame;            // 创建小尺寸帧对象
    // 将原图缩放到320x240，减少检测计算量
    cv::resize(frame, smallFrame, cv::Size(320, 240));


    // 裁剪人脸区域
    // 创建人脸矩形区域对象
    cv::Rect faceRect(maxFace->x, maxFace->y, maxFace->width, maxFace->height);
    // 从原图中提取人脸区域并克隆（避免引用原图数据）
    cv::Mat faceMat = smallFrame(faceRect).clone();


    // 提取人脸特征向量
    cv::Mat feat = getFaceFeature(faceMat);

    // 保存人脸特征文件
    appendFeatureToFile(feat);

    // 将特征向量添加到已注册特征列表（克隆数据，避免引用局部变量）
    registeredFeatures.append(feat.clone());
    collectCount++;                    // 增加采集计数
    qDebug() << "第" << collectCount << "次采集成功";  // 输出成功信息
    if(collectCount >= 3){
        addUserWidget->hide();
        collectCount = 0;
    }

}

// 比对按钮点击事件处理函数，将当前人脸与已注册的人脸进行比对
// 该函数会计算相似度并判断是否为同一人
void Widget::faceRegnizetion()
{
    // 检查是否检测到人脸
    if ( registeredFeatures.size() < 3) {
        qDebug() << "未进行训练，无法比对";  // 输出提示信息
        return;                        // 提前返回
    }
    if (faces.empty()) return ;

    // 从检测到的多个人脸中选择面积最大的人脸
    auto maxFace = std::max_element(facesOriginal.begin(), facesOriginal.end(),
                                    [](const cv::Rect& a, const cv::Rect& b) { return a.area() < b.area(); });
    // 检查是否找到最大人脸
    if (maxFace == facesOriginal.end()) return;

    // 使用OpenCV获取当前视频帧
    cv::Mat frame;                    // 创建帧对象
    // 从摄像头读取最新帧
    if (!cap.read(frame) || frame.empty()) {
        qDebug() << "获取视频帧失败";  // 输出错误信息
        return;                        // 提前返回
    }

    cv::Mat smallFrame;            // 创建小尺寸帧对象
    // 将原图缩放到320x240，减少检测计算量
    cv::resize(frame, smallFrame, cv::Size(320, 240));


    // 裁剪人脸区域
    // 创建人脸矩形区域对象
    cv::Rect faceRect(maxFace->x, maxFace->y, maxFace->width, maxFace->height);
    // 从原图中提取人脸区域并克隆
    cv::Mat faceMat = smallFrame(faceRect).clone();
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
        m_face_registered = true;
    } else {
        qDebug() << "比对结果：不同人";  // 相似度小于阈值，判定为不同人
        m_face_registered = false;
    }
}

void *Widget::faceRegnizeFunc(void *parent)
{
    Widget *widget = static_cast<Widget*>(parent); //获得主窗口类
    while(true){

        widget->faceRegnizetion();
        usleep(100000);
    }
}

void Widget::appendFeatureToFile(cv::Mat &feature)
{
    FILE* fp = fopen(filePath, "ab");
    if( fp == nullptr){
        qDebug() << "无法打开文件进行追加" << filePath;
        return;
    }
    int rows = feature.rows;
    int cols = feature.cols;
    int type = feature.type();
    fwrite( &rows, sizeof(int), 1, fp);
    fwrite( &cols, sizeof(int), 1, fp);
    fwrite( &type, sizeof(int), 1, fp);

    int dataSize = feature.rows * feature.cols * feature.elemSize();
    fwrite(feature.data, 1, dataSize, fp);

    fclose(fp);
    qDebug() << "已把特征量保存到文件";

}

void Widget::loadRegisteredFeature()
{
    FILE *fp = fopen(filePath, "rb");
    if( fp == nullptr){
        return;
    }
    registeredFeatures.clear();
    while(true){
        int rows = 0, cols = 0, type = 0;
        //读取特征信息
        if (fread(&rows,sizeof(int),1,fp) != 1){
            //文件读取完毕
            break;
        }
        if ( fread(&cols, sizeof(int),1,fp) != 1 || fread(&type, sizeof(int),1, fp) != 1){
            //读取失败
            break;
        }
        cv::Mat feature(rows, cols, type);
        int dataSize = rows * cols * feature.elemSize();
        if( fread(feature.data, 1, dataSize, fp) != dataSize){
            //读取特征失败
            qDebug() << "读取特征数据失败";
            break;
        }

        registeredFeatures.append(feature);
    }
    fclose(fp);
    qDebug() << "已加载" << registeredFeatures.size() << "个人脸特征";
}

void Widget::on_btnadd_clicked()
{
    if(!addUserWidget) return;
    if(ui->widgetbtn){
        addUserWidget->setGeometry(ui->widgetbtn->geometry());
        addUserWidget->show();
        addUserWidget->raise();
        addUserWidget->activateWindow();
    }

}
