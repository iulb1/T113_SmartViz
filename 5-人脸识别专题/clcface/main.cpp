#include "widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_Use96Dpi);
    Widget w;

    QFont font;
    font.setPointSize(6); // 你想要的字号
    a.setFont(font);
    w.show();
    return a.exec();
}
