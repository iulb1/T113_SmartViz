#include "widget.h"

#include <QApplication>
#include <QScreen>
#include <QFont>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // 设置固定的DPI缩放因子为1.0，禁用自动DPI缩放
    // 这样字体大小在不同DPI设备上保持一致
    qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "0");
    qputenv("QT_SCALE_FACTOR", "1");
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
    
    // 对于Qt 5.6+，使用以下方式禁用高DPI缩放
    #if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
        QApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
    #endif
    
    Widget w;
    w.show();
    return a.exec();
}
