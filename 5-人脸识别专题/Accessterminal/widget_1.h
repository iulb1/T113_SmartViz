#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDebug>
#include <QPainter>
#include <opencv2/opencv.hpp>
#include "adduserwidget.h"
#include <pthread.h>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    void getFaceFeatureInVector();

private:
    Ui::Widget *ui;
    cv::VideoCapture cap;           // OpenCV视频捕获对象，用于从摄像头获取视频流
    QTimer *timer;                  // 定时器指针，用于定时触发updateFrame函数
    bool m_face_registered;
    cv::CascadeClassifier face_cascade;        // Haar Cascade分类器对象，用于人脸检测
    std::vector<cv::Rect> faces;  // 存储检测结果,画框
    std::vector<cv::Rect> facesOriginal;  // 存储检测结果，用于特征提取

    AddUserWidget *addUserWidget;

    cv::dnn::Net faceNet;            // OpenCV DNN网络对象，用于加载和运行ONNX模型
    QVector<cv::Mat> registeredFeatures;  // 存储已注册的人脸特征向量列表
    int collectCount = 0;             // 已采集的人脸特征数量计数器
    const int collectTarget = 3;      // 目标采集次数，需要采集3张人脸才能进行比对

    cv::Mat getFaceFeature(const cv::Mat& faceImg);  // 从人脸图像中提取特征向量
    void faceRegnizetion();

    pthread_t faceRegnizeThread;
    static void* faceRegnizeFunc(void *); //线程回调函数
    void appendFeatureToFile(cv::Mat& feature);
    void loadRegisteredFeature();
    const char* filePath = "/mnt/registered_faces.dat";

private slots:
    void time_reflash();
    void updateFrame();             // 定时器触发的槽函数，更新视频帧并检测人脸
    void on_btnadd_clicked();
};
#endif // WIDGET_H
