#ifndef CPROCESSTHREAD_H
#define CPROCESSTHREAD_H
#include "QThread"
#include "QDateTime"
#include "opencv2/opencv.hpp"
#include "QSettings"
#include "QList"

using namespace cv;

class CProcessThread : public QThread
{
    Q_OBJECT
public:
    CProcessThread();
    ~CProcessThread();
    QString getConfigValue(int propId);

    const double PI = 3.1415926;

    bool isROI;      //默认处理线程接收到的第一幅帧画面为ROI， 接收后立马置 false
    QString ROIThreshold;
    QString locator_offset;
    QString isShowDebugFrame;
    QString isWriteROI;

    QString roi_x;  //ROI单纯的指定位块部分
    QString roi_y;
    QString roi_width;
    QString roi_heigth;
    QString yIndeForLocate;

    //const int distantArea = 120;
    int distantArea;
    QString lastHeigth;

    double rotateTheta;
    int xIndexForUse;
    int yIndexForUse;
    QList<cv::Mat>ROIList;

    int ROICount;



    Mat frameCropped;
    Mat frameBinarized;
    Mat lastROI;

    QList<int> distantPixel;

    std::vector<cv::Point> points;
    cv::Vec4f line;

    std::vector<cv::Point> getPoints(cv::Mat &image, int value);
    void drawLine(cv::Mat &image, double theta, double rho, cv::Scalar color);



    //计算最终的结果函数
    void findFirstPeaks(QList<int>& listData);

public slots:
    bool receiveDataOrNot(Mat liveFrame);
    void setROISlot();

signals:
    void sendDistDataToGrabThread(QList<int>& listData);
    void sendErrMsg();
};

#endif // CPROCESSTHREAD_H
