#ifndef CFRAMEGRABBER_H
#define CFRAMEGRABBER_H

#include "QThread"
#include "opencv2/opencv.hpp"
#include "QString"
#include "QSettings"
#include "QDebug"
#include "QList"
#include "time.h"
#include "cprocessthread.h"

using namespace cv;

class CFrameGrabber : public QThread
{
    Q_OBJECT
public:
    CFrameGrabber();
    ~CFrameGrabber();
    QString getConfigValue(int propId);

    QImage Mat2QImage(Mat& image);

    bool isGrabFrame;
    bool isGrabCalibrateFrame;

    int frameCount;

    int isWriteFrame;
    const int MAXSIZE_OF_LIST = 1200;

    CProcessThread *prcThd;


protected:
    void run();

signals:
    void cameraErr();
    void updataFrame(Mat liveFrame);

    void calibrateFrame(Mat calibrateFrame);

    bool sendDataToProcessThread(Mat liveFrame);

    void setROI();
    void sendDistDataToUI(QList<int>& listData);

public slots:
    void grabFrame();
    void grabCalibrateFrame();
    void stopGrabFrameSlot();
    void distData(QList<int> &listData);


};

#endif // CFRAMEGRABBER_H
