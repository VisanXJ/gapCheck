#include "cprocessthread.h"
#include "QDebug"
#include "QFile"

CProcessThread::CProcessThread()
{
    isROI = true;
    roi_x = getConfigValue(8);
    roi_y = getConfigValue(9);
    roi_width = getConfigValue(10);
    roi_heigth = getConfigValue(11);
    yIndeForLocate = getConfigValue(15);

    lastHeigth = getConfigValue(17);
    distantArea =lastHeigth.toInt();

    ROIThreshold = getConfigValue(12);
    locator_offset = getConfigValue(13);
    isShowDebugFrame = getConfigValue(14);

    isWriteROI = getConfigValue(16);
    ROICount = 0;
}

CProcessThread::~CProcessThread()
{
    qDebug() << "Process thread is destructor";
}

bool CProcessThread::receiveDataOrNot(Mat liveFrame)
{
//    qDebug()<<"Receive Date at"<<QDateTime::currentDateTime();


    if(isROI)       //Obtain theta,  xIndex and yIndex for additional crop
    {
        isROI = false;
        frameCropped = liveFrame(Rect(roi_x.toInt(),roi_y.toInt(),roi_width.toInt(),roi_heigth.toInt()));
        cvtColor(frameCropped, frameCropped, CV_BGR2GRAY);

        imwrite("gapCheck Configuration\\source frames\\orginalFrame.bmp", liveFrame);
        imwrite("gapCheck Configuration\\source frames\\orginalCropped.bmp", frameCropped);
        if (isShowDebugFrame.toInt())
                imshow("ROI",frameCropped);


        threshold(frameCropped, frameBinarized, ROIThreshold.toInt(), 255, THRESH_BINARY);
        qDebug()<< "Threshold is" <<ROIThreshold.toInt();

        if (isShowDebugFrame.toInt())
            imshow("Binarized Frame", frameBinarized);

        imwrite("gapCheck Configuration\\source frames\\roi.bmp", frameBinarized);


        points = getPoints(frameBinarized, 255);

        cv::fitLine(points, line, CV_DIST_HUBER , 0, 0.01, 0.01);
        double cos_theta = line[0];
        double sin_theta = line[1];
        double x0 = line[2], y0 = line[3];
        double phi = atan2(sin_theta, cos_theta) + PI / 2.0;        //rotate angle:theta

        rotateTheta = phi / PI * 180;

        double rho = y0 * cos_theta - x0 * sin_theta;
        qDebug() << "phi = " << phi / PI * 180;
        qDebug() << "rho = " << rho;
        if (isShowDebugFrame.toInt())
            imshow("", frameBinarized);

        /*Rotate starts*/
        Point center = Point( frameBinarized.cols/2, frameBinarized.rows/2 );
        double angle = phi / PI * 180;

        angle = 11;

        qDebug()<<tr("Rotate angle is:") << angle;

        double scale = 1.0;

        Mat frameRotated;
        Mat rot_mat( 2, 3, CV_32FC1 );

        rot_mat = getRotationMatrix2D( center, angle, scale );
        warpAffine( frameBinarized, frameRotated, rot_mat, frameBinarized.size() );

        imwrite("gapCheck Configuration\\source frames\\frameRotated.jpg", frameRotated);
        if (isShowDebugFrame.toInt())
            imshow("Rotated Frame", frameRotated );
        /*Rotate ends*/


        /*Get X index point start*/
        int xIndex = 0;
        uchar* data = frameRotated.ptr<uchar>(roi_heigth.toInt() - 20);       //Serach X index at 20th row of image, at specify row of image
        for (int i = 30; i < frameRotated.cols; i++)        //The original value is 0
        { if(data[i] == 255)
            {
                xIndex = i;

                break;
            }
        }

        xIndex += 5;
        qDebug()<<"xIndex is"<<xIndex;
//        qDebug()<<"ok";
        /*Get X index ends*/


        /*Get Y index starts*/
        int yIndex = 10;    //Set a default value for yIndex
        for (int j = yIndeForLocate.toInt(); j < frameRotated.rows; j++)
        {
            uchar* data = frameRotated.ptr<uchar>(j);
            for (int i = 0; i < frameRotated.cols; i++)
            { if(data[i] == 255)
                {
                    yIndex = j;
                    break;
                }
            }
            if(yIndex != 10)
                break;

        }
        qDebug() << "yIndex is:" << yIndex;
        qDebug() << "Y done";

        xIndexForUse = xIndex;
        yIndexForUse = yIndex-1;

    }

    frameCropped = liveFrame(Rect(roi_x.toInt(),roi_y.toInt(),roi_width.toInt(),roi_heigth.toInt()));

//    imshow("Original ROI",frameCropped );

    cvtColor(frameCropped, frameCropped, CV_BGR2GRAY);
    threshold(frameCropped, frameBinarized, ROIThreshold.toInt(), 255, THRESH_BINARY);

    Point center = Point( frameBinarized.cols/2, frameBinarized.rows/2 );
    double angle = rotateTheta;
    double scale = 1.0;

    Mat frameRotated;
    Mat rot_mat( 2, 3, CV_32FC1 );

    rot_mat = getRotationMatrix2D( center, angle, scale );
    warpAffine( frameBinarized, frameRotated, rot_mat, frameBinarized.size());

//    qDebug() << "Frame Rotated Size is" << frameRotated.rows << frameRotated.cols;
//    qDebug() << "Crop index is:" <<xIndexForUse<<yIndexForUse;

    //当定位块处出现毛刺时， 会导致程序异常， yIndexForUse-distantArea < 0

    if(yIndexForUse < distantArea)
    {

        yIndexForUse = distantArea;
    }


    lastROI = frameRotated(Rect(xIndexForUse, yIndexForUse-distantArea, 3s0, distantArea));

    ROIList<<lastROI;
    if (isShowDebugFrame.toInt())
        imshow("Last ROI",lastROI);
    if(ROIList.length()==960)
    {
        QList<Mat>::Iterator i;
        for(i = ROIList.begin(); i != ROIList.end(); i++)
        {
            char file[400] = {'0'};
            sprintf(file, "gapCheck Configuration\\source frames\\Grabbed Frames\\img%d.jpg", ROICount);
            imwrite(file, *i);
            ROICount++;
        }

    }




    for (int j = lastROI.rows-10; j > 0; j--)
    {
        int flag = 0;
        uchar* data = lastROI.ptr<uchar>(j);
        for (int i = 0; i < lastROI.cols; i++)
        {
            if(data[i] == 255)
            {
                distantPixel.push_back(j);
                flag = 1;
                break;
            }
            if(j == 1)      /*倒数第40行，从下往上扫描，遇到255马上跳出循环*/
            {
                distantPixel.push_back(j);
                flag = 1;
                break;
            }
        }
        if(flag)
        {
            break;
        }
    }

//    for(int j = 0; j < lastROI.rows; j++)   /*改变策略，从上往下搜索，尽量排除掉毛刺的影响*/
//    {
//        int flag = 0;

//    }


    if(ROICount > 959)
    {
        QFile f("gapCheck Configuration\\calculatedData.txt");
        if(!f.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                qDebug() << "Open failed.";
            }
        QTextStream txtOutput(&f);
        QList<int>::iterator dataIndex;
        for(dataIndex = distantPixel.begin(); dataIndex != distantPixel.end(); dataIndex++)
        {
            txtOutput<<*dataIndex<<"\n";
        }
        qDebug()<<"all done";

        findFirstPeaks(distantPixel);

        ROICount = 0;
        distantPixel.clear();
        ROIList.clear();

    }


    return true;
}



vector<Point> CProcessThread::getPoints(Mat &image, int value)
{
    int nRows = image.rows; // number of lines
    int nCols = image.cols;  //" * image.channels();

    std::vector<cv::Point> points;
    for (int j = nRows-1; j > nRows - 30; j--)
    {
        uchar* data = image.ptr<uchar>(j);
        for (int i = 0; i < nCols; i++)
        { if(data[i] == value)
            {
                points.push_back(cv::Point(i, j));
                break;
            }
        }


//    for(int i = 0; i < nc; i++)
//    {
//        uchar* data = image.ptr<uchar>(i);
//        for(int j = 0; j < nr; j++)
//        {
//            if(data[j] == value )
//            {
//                points.push_back(cv::Point(i, j));
//                break;
//            }
//        }
    }
    return points;
}


void CProcessThread::drawLine(Mat &image, double theta, double rho, cv::Scalar color)
{
    if (theta < PI/4. || theta > 3.*PI/4.)// ~vertical line
    {
        Point pt1(rho/cos(theta), 0);
        Point pt2((rho - image.rows * sin(theta))/cos(theta), image.rows);
        cv::line( image, pt1, pt2, cv::Scalar(255), 1); }
    else
    {
        Point pt1(0, rho/sin(theta));
        Point pt2(image.cols, (rho - image.cols * cos(theta))/sin(theta));
        cv::line(image, pt1, pt2, color, 1); }


}



QString CProcessThread::getConfigValue(int propId)
{
    QSettings *configFile = new QSettings("gapCheck Configuration\\gabCheckConfig.ini", QSettings::IniFormat);

    switch (propId)
    {
        case 8:
        {
            QString roi_x  = configFile->value("ROIInfo/roi_x").toString();
            return roi_x ;
            break;
        }
        case 9:
        {
            QString roi_y = configFile->value("ROIInfo/roi_y").toString();
            return roi_y;
            break;
        }
        case 10:
        {
            QString roi_width = configFile->value("ROIInfo/roi_width").toString();
            return roi_width;
            break;
        }
        case 11:
        {
            QString roi_heigth = configFile->value("ROIInfo/roi_heigth").toString();
            return roi_heigth;
            break;
        }
        case 12:
        {
            QString ROIThreshold = configFile->value("ROIInfo/threshold").toString();
            return ROIThreshold;
            break;
        }
        case 13:
        {
            QString locator_offset = configFile->value("ROIInfo/locator_offset").toString();
            return locator_offset;
            break;
        }
        case 14:
        {
            QString isShowDebugFrame = configFile->value("ROIInfo/isShowDebugFrame").toString();
            return isShowDebugFrame;
            break;
        }

        case 15:
        {
            QString yIndeForLocate = configFile->value("ROIInfo/yIndeForLocate").toString();
            return yIndeForLocate;
            break;
        }
        case 16:
        {
            QString isWriteROI = configFile->value("ROIInfo/isWriteROI").toString();
            return isWriteROI;
            break;
        }
        case 17:
        {
            QString lastHeigth = configFile->value("ROIInfo/lastHeigth").toString();
            return lastHeigth;
            break;
        }
        case 19:
        {
            QString calibrateValue = configFile->value("ROIInfo/calibrateValue").toString();
            return calibrateValue;
            break;
        }
        case 20:
        {
            QString max_distant = configFile->value("ROIInfo/max_distant").toString();
            return max_distant;
            break;
        }
        case 21:
        {
            QString min_distant = configFile->value("ROIInfo/min_distant").toString();
            return min_distant;
            break;
        }
        default:
            return 0;
            break;
    }
}



void CProcessThread::setROISlot()
{
    isROI = true;
}



void CProcessThread::findFirstPeaks(QList<int>& listData)
{
    int jumpValue = 20;     //
    std::vector<int> sign;
    //QList<int> sign;
    for(int i = 3; i < listData.length(); i++)      //第3项减去第1项
    {
        int diff = listData[i] - listData[i-3];
        sign.push_back(diff);
    }



    //挑选出 sign >= jumpValue 的index 然后截取一小段；再以index+15为起点，寻找 sign >= jumpValue 的index;

    QList<int> baseIndexContainer;
    //std::vector<int>::iterator baseIndex = sign.begin();      //存储 sign >= jumpValue 的下标
    int baseIndex = 0;
    std::vector<int>::iterator resultValueIndex;

    /*Searching for the FIRST index of THE FIRST MAX VALUE     start*/
    QList<int> indMax;
    for(int j = 0; j < sign.size(); j++)
    {
        if(sign[j] >= jumpValue)
        {
            baseIndex = baseIndex + j;
            baseIndexContainer.push_back(baseIndex);
            break;
        }

    }
    /*Searching for the FIRST index of THE FIRST MAX VALUE     end*/

    /*Obtain baseIndexContainer                              start*/
    std::vector<int>::iterator signIterator = sign.begin();
    for(int gearCount = 0; gearCount < 35; gearCount++)
    {
//        auto resultIndexRangLower = *(sign.begin()) + baseIndex;

//        resultValueIndex = std::max_element((sign.begin()+baseIndex), (sign.begin()+baseIndex+15));
//        qDebug() << "The Max Value is:" << listData[*resultValueIndex + 3] << "at:" << *resultValueIndex + 3;    //输出完后，需要重新查找下一个 sign >= 10的Index



        baseIndex = baseIndex + 12;
        int forwardCount = 0;
        for(auto newIndex = (signIterator + baseIndex); newIndex != sign.end() ; newIndex++)
        {
            forwardCount++;
            if(*newIndex >= jumpValue)
            {
                baseIndex = baseIndex + forwardCount;
                baseIndexContainer.push_back(baseIndex);
                break;
            }
        }
    }
    /*Obtain baseIndexContainer                              end*/



    //qDebug() << "The Index of First Max Value is:" << *resultValueIndex;



    /*start calculate distant                               start*/
    QList<int> pixelDistant;
    for(int ii = 0; ii < baseIndexContainer.length(); ii++)
    {
        //qDebug()<<"FOR expression is called"<<"and baseIndexContainor[ii] = "<<baseIndexContainer[ii];
        int distantMax = 0;
        for(int jj = baseIndexContainer[ii]; jj < baseIndexContainer[ii] + 15; jj++)
        {

            if (distantMax < listData[jj])
            {
                distantMax = listData[jj];
            }

            //qDebug()<<"pixelDistant = "<<listData[jj];
        }
        pixelDistant.push_back(distantMax);
        qDebug() << distantMax;
    }
    /*start calculate distant                                 end*/



    /*将测量结果写入TXT文件*/
    QFile f("gapCheck Configuration\\lastDistant.txt");
    if(!f.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Open failed." << endl;
    }



    QTextStream txtOutput(&f);
    QList<int>::iterator dataIndex;
    for(dataIndex = pixelDistant.begin(); dataIndex != pixelDistant.end(); dataIndex++)
    {
        txtOutput<<(distantArea-(*dataIndex))/getConfigValue(19).toDouble() - locator_offset.toDouble()<<"\n";
//        qDebug() << (distantArea-(*dataIndex))/getConfigValue(19).toDouble() - locator_offset.toDouble()<<"\n";
    }

    qDebug()<<"all done";

    emit sendDistDataToGrabThread(pixelDistant);

    qDebug() << "Length is" << indMax.length() << tr("极大值为");

}

void CProcessThread::slot_waveCheck()
{
    QFile dataFile("D:\\calculatedData.txt");
    if(!dataFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Open failed." << endl;
    }

    QList<int> listData;
    while(!dataFile.atEnd())
    {
        QByteArray lineData = dataFile.readLine();
        QString str(lineData);
        int pixelData = str.toInt();
        listData.push_back(pixelData);
    }

    this->findFirstPeaks(listData);
}
