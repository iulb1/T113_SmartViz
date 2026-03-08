QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    widget.cpp

HEADERS += \
    widget.h

FORMS += \
    widget.ui

INCLUDEPATH += -I /home/zhiwan/cprogrammer/camera/opencv/_install/include/opencv4

INCLUDEPATH += -I $$PWD


LIBS += -L/home/zhiwan/t113/Tina-Linux/out/t113-zhiwan_v1/staging_dir/target/usr/lib/
LIBS += -lz -ljpeg -lpng12

LIBS += -L/home/zhiwan/cprogrammer/camera/opencv/_install/lib/
# OpenCV库链接顺序：从基础库到高级库（链接器从右到左解析依赖）
# 人脸检测使用opencv_objdetect（Haar Cascade分类器）
# DNN模块用于人脸特征提取
LIBS += -lopencv_dnn -lopencv_objdetect -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_imgproc -lopencv_core
# 添加rpath-link帮助链接器找到依赖库
QMAKE_LFLAGS += -Wl,-rpath-link,/home/zhiwan/cprogrammer/camera/opencv/_install/lib/

LIBS += -L/home/zhiwan/t113/Tina-Linux/out/t113-zhiwan_v1/staging_dir/target/rootfs/usr/lib/arm-qt/lib/
LIBS += -lQt5Core -lQt5Gui -lQt5Widgets -lQt5Test
# pthread库（Linux原生线程）
LIBS += -lpthread

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    pics.qrc
