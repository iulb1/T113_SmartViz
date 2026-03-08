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

