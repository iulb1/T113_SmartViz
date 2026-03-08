#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDebug>
#include <QPainter>
#include <opencv2/opencv.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    Ui::Widget *ui;
    cv::VideoCapture cap;           // OpenCV视频捕获对象，用于从摄像头获取视频流
    QTimer *timer;                  // 定时器指针，用于定时触发updateFrame函数
    cv::CascadeClassifier face_cascade;        // Haar Cascade分类器对象，用于人脸检测
    std::vector<cv::Rect> faces;  // 存储检测结果

private slots:
    void time_reflash();
    void updateFrame();             // 定时器触发的槽函数，更新视频帧并检测人脸
};
#endif // WIDGET_H
