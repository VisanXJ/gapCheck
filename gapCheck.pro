#-------------------------------------------------
#
# Project created by QtCreator 2017-02-21T12:47:51
#
#-------------------------------------------------

QT       += core gui
QT += core
QT += gui
QT += widgets
QT += serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = gapCheck
TEMPLATE = app


# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        mainwindow.cpp \
    cframegrabber.cpp \
    configdlg.cpp \
    cprocessthread.cpp

HEADERS  += mainwindow.h \
    cframegrabber.h \
    configdlg.h \
    cprocessthread.h

FORMS    += mainwindow.ui \
    configdlg.ui

RC_FILE = Gear_Gap.rc

INCLUDEPATH+=d:/opencv/build/include/opencv \
                    d:/opencv/build/include/opencv2 \
                   d:/opencv/build/include
LIBS+=D:\opencv\build\x64\vc12\lib\opencv_ml2413.lib
LIBS+=D:\opencv\build\x64\vc12\lib\opencv_calib3d2413.lib
LIBS+=D:\opencv\build\x64\vc12\lib\opencv_contrib2413.lib
LIBS+=D:\opencv\build\x64\vc12\lib\opencv_core2413.lib
LIBS+=D:\opencv\build\x64\vc12\lib\opencv_features2d2413.lib
LIBS+=D:\opencv\build\x64\vc12\lib\opencv_flann2413.lib
LIBS+=D:\opencv\build\x64\vc12\lib\opencv_gpu2413.lib
LIBS+=D:\opencv\build\x64\vc12\lib\opencv_highgui2413.lib
LIBS+=D:\opencv\build\x64\vc12\lib\opencv_imgproc2413.lib
LIBS+=D:\opencv\build\x64\vc12\lib\opencv_legacy2413.lib
LIBS+=D:\opencv\build\x64\vc12\lib\opencv_objdetect2413.lib
LIBS+=D:\opencv\build\x64\vc12\lib\opencv_ts2413.lib
LIBS+=D:\opencv\build\x64\vc12\lib\opencv_video2413.lib
LIBS+=D:\opencv\build\x64\vc12\lib\opencv_nonfree2413.lib
LIBS+=D:\opencv\build\x64\vc12\lib\opencv_ocl2413.lib
LIBS+=D:\opencv\build\x64\vc12\lib\opencv_photo2413.lib
LIBS+=D:\opencv\build\x64\vc12\lib\opencv_stitching2413.lib
LIBS+=D:\opencv\build\x64\vc12\lib\opencv_superres2413.lib
LIBS+=D:\opencv\build\x64\vc12\lib\opencv_videostab2413.lib


