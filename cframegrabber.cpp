#include "cframegrabber.h"
#include "QList"
#include "QDebug"
#include "cprocessthread.h"
#include "QDateTime"

CFrameGrabber::CFrameGrabber()
{
    isGrabFrame = false;
    isGrabCalibrateFrame = false;
    frameCount = 0;
    isWriteFrame  =  getConfigValue(6).toInt();


    prcThd = new CProcessThread;
    connect(this, SIGNAL(sendDataToProcessThread(Mat)), prcThd, SLOT(receiveDataOrNot(Mat)));
    connect(this, SIGNAL(setROI()), prcThd, SLOT(setROISlot()));
    connect(prcThd, SIGNAL(sendDistDataToGrabThread(QList<int>&)), this, SLOT(distData(QList<int>&)));
    prcThd->start();
}


QString CFrameGrabber::getConfigValue(int propId)
{
    QSettings *configFile = new QSettings("gapCheck Configuration\\gabCheckConfig.ini", QSettings::IniFormat);

    switch (propId)
    {
        case 1:
        {
            QString cameraID = configFile->value("basicInfo/cameraID").toString();
            return cameraID;
            break;
        }
        case 2:
        {
            QString FRAME_WIDTH = configFile->value("basicInfo/CV_CAP_PROP_FRAME_WIDTH").toString();
            return FRAME_WIDTH;
            break;
        }
        case 3:
        {
            QString FRAME_HEIGHT = configFile->value("basicInfo/CV_CAP_PROP_FRAME_HEIGHT").toString();
            return FRAME_HEIGHT;
            break;
        }
        case 6:
        {
            QString isWriteFrame = configFile->value("basicInfo/isWriteFrame").toString();
            return isWriteFrame;
            break;
        }
        default:
            return 0;
            break;
    }
}

void CFrameGrabber::run()
{
    VideoCapture cap(getConfigValue(1).toInt());

//    qDebug()<<"Camera ID is:"<<getConfigValue(1).toInt();
    cap.set(CV_CAP_PROP_FRAME_WIDTH , this->getConfigValue(2).toInt());     //2592
    cap.set(CV_CAP_PROP_FRAME_HEIGHT , this->getConfigValue(3).toInt());    //1944
//    cap.set(CV_CAP_PROP_EXPOSURE, this->getConfigValue(4).toDouble());

    if(!cap.isOpened())
    {
        //qDebug()<<"camera opening err!";
        emit cameraErr();
    }
//    else
//        qDebug()<<"opening Camera successful";

    int showInterval = 1;
    int frameCount = 0;     //the index of that frames written to disk

    QList<Mat>frameList;


    while(1)
    {
        Mat frameLogged;
        cap>>frameLogged;

        if((frameLogged.data) && (showInterval%5 == 0))
        {
            emit updataFrame(frameLogged); //send updata GUI signal
        }

        if(isGrabCalibrateFrame)    //send calibration frame to Main Thread
        {
            emit calibrateFrame(frameLogged);
            isGrabCalibrateFrame = false;
        }






        if((frameLogged.data) && (isGrabFrame))
        {
//            char file[400] = {'0'};
//            sprintf(file, "D:\\Program Files\\gapCheck Configuration\\source frames\\Grabbed Frames\\img%d.bmp", frameCount);
//            imwrite(file, frameLogged);
//            frameCount ++;

            frameList<<frameLogged;

//            qDebug()<<"Send Data at"<<QDateTime::currentDateTime();
            if((frameList.length()>=240) && (frameList.length()<MAXSIZE_OF_LIST) )
                emit sendDataToProcessThread(frameLogged);
//            qDebug()<<"The length of the containor is "<<frameList.length();
        }
        if((frameList.length() == MAXSIZE_OF_LIST)&&(isGrabFrame))
        {
            //isGrabFrame  = false;
            qDebug()<<"The length of containor is :"<<frameList.length();
            qDebug()<<"frameCount is:"<<frameCount;



            /*Write frame to disk*/
            if(isWriteFrame )
            {
                QList<Mat>::Iterator i;
                for(i = frameList.begin()+240; i!=frameList.end(); i++)
                {


                    char file[400] = {'0'};
                    sprintf(file, "D:\\Program Files\\gapCheck Configuration\\source frames\\Grabbed Frames\\img%d.jpg", frameCount);
                    imwrite(file, *i);
                    frameCount++;
                }
            }


        }
        if(frameList.length() == MAXSIZE_OF_LIST)
        {
            isGrabFrame  = false;
            frameList.clear();
            frameCount = 0;
        }



        showInterval++;
        if(showInterval == 5000)
            showInterval = 0;


    }
}


void CFrameGrabber::grabFrame()
{
    isGrabFrame = true;
    emit setROI();
}


void CFrameGrabber::stopGrabFrameSlot()
{
    isGrabFrame = false;
    frameCount = 0;
}

void CFrameGrabber::distData(QList<int>& listData)
{
    qDebug()<<"grab thread receive data";
    emit sendDistDataToUI(listData);
}

void CFrameGrabber::grabCalibrateFrame()
{
    isGrabCalibrateFrame = true;
}


CFrameGrabber::~CFrameGrabber()
{
    delete prcThd;
}
