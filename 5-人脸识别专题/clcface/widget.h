#ifndef WIDGET_H                    // 头文件保护宏，防止重复包含
#define WIDGET_H                    // 定义头文件保护宏

#include <QWidget>                  // Qt窗口基类，用于创建GUI界面
#include <QTimer>                   // Qt定时器类，用于定时触发事件
#include <opencv2/opencv.hpp>       // OpenCV核心库，包含基本图像处理功能
#include <QImage>                   // Qt图像类，用于图像显示和处理
#include <opencv2/objdetect.hpp>    // OpenCV目标检测模块，包含Haar Cascade分类器
#include <opencv2/face.hpp>          // OpenCV人脸识别模块（当前未使用，保留用于扩展）
#include <opencv2/dnn.hpp>          // OpenCV深度学习模块，用于加载ONNX模型

QT_BEGIN_NAMESPACE                  // Qt命名空间开始
namespace Ui { class Widget; }      // 前向声明UI命名空间中的Widget类（由Qt Designer生成）
QT_END_NAMESPACE                    // Qt命名空间结束

class Widget : public QWidget        // Widget类定义，继承自QWidget，实现主窗口功能
{
    Q_OBJECT                         // Qt元对象系统宏，启用信号槽机制

public:
    Widget(QWidget *parent = nullptr);  // 构造函数，parent为父窗口指针，默认为空
    ~Widget();                       // 析构函数，释放资源

private slots:                      // 私有槽函数区域，用于响应信号
    void updateFrame();             // 定时器触发的槽函数，更新视频帧并检测人脸

    void on_btnbidui_clicked();     // 比对按钮点击槽函数，执行人脸比对操作

    void on_btncaiji_clicked();     // 采集按钮点击槽函数，采集人脸特征

private:                            // 私有成员区域
    Ui::Widget *ui;                 // UI界面指针，指向Qt Designer生成的界面对象
    QTimer *timer;                  // 定时器指针，用于定时触发updateFrame函数
    cv::VideoCapture cap;           // OpenCV视频捕获对象，用于从摄像头获取视频流
    bool loadFaceCascade(const QString &path);  // 加载Haar Cascade人脸检测分类器
    QImage cvMatToQImage(const cv::Mat& mat);  // 将OpenCV的Mat格式转换为Qt的QImage格式
    cv::CascadeClassifier face_cascade;        // Haar Cascade分类器对象，用于人脸检测

    std::vector<cv::Rect> faces;    // 存储检测到的人脸矩形区域列表

    int collectCount = 0;             // 已采集的人脸特征数量计数器
    const int collectTarget = 3;      // 目标采集次数，需要采集3张人脸才能进行比对

    cv::dnn::Net faceNet;            // OpenCV DNN网络对象，用于加载和运行ONNX模型
    QVector<cv::Mat> registeredFeatures;  // 存储已注册的人脸特征向量列表
    cv::Mat getFaceFeature(const cv::Mat& faceImg);  // 从人脸图像中提取特征向量
};
#endif // WIDGET_H                  // 头文件保护宏结束
