#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QDebug"
#include "QColor"
#include "configdlg.h"
#include "Windows.h"
#include <QProcess>
#include "QDesktopServices"
#include "QUrl"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->pushButton->setVisible(0);


    locator_offset = getConfigValue(13);
    lastHeigth = getConfigValue(17);
    calibrationValue = getConfigValue(19).toDouble();

    ui->tableWidget->setColumnCount(3);
   // ui->tableWidget->setFixedWidth(200);




    ui->tableWidget->setRowCount(10);

//    ui->tableWidget->setItem(1,1, new QTableWidgetItem(QString::number(5.13)));
//    ui->tableWidget->item(1,1)->setBackgroundColor(QColor("red"));

    /*connet to the liveFrame thread*/
    frameGrabber = new CFrameGrabber;
    qRegisterMetaType< Mat >("Mat");

    connect(frameGrabber, SIGNAL(cameraErr()), this, SLOT(cameraWarningWdiget()));
    connect(frameGrabber, SIGNAL(updataFrame(Mat)), this, SLOT(displayLiveFrame(Mat)));

    connect(this, SIGNAL(grabCalibrationFrame()), frameGrabber, SLOT(grabCalibrateFrame()));
    connect(frameGrabber, SIGNAL(calibrateFrame(Mat)), this, SLOT(grabCalibrationFrame(Mat)));

    connect(frameGrabber, SIGNAL(sendDistDataToUI(QList<int>&)), this, SLOT(displayDistData(QList<int>&)));
    frameGrabber->start();

    /*open the serial port connected to PC*/
    serialPort = new QSerialPort;
    serialPort->setPortName(getConfigValue(4));
    serialPort->open(QIODevice::ReadWrite);

    if (!serialPort->isOpen())
    {
        QWidget *comPortWarning = new QWidget;
        QMessageBox::warning(comPortWarning,tr("错误"), tr("串口打开错误，请检查串口连接及串口号设置！"));

    }
    serialPort->setBaudRate(QSerialPort::Baud19200);
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setParity(QSerialPort::NoParity);
    serialPort->setStopBits(QSerialPort::OneStop);

    /*Notify the subthread ger ready to grab frames now*/
    connect(this, SIGNAL(grabFrameNow()), frameGrabber, SLOT(grabFrame()));
    connect(this, SIGNAL(stopGrabFrame()), frameGrabber, SLOT(stopGrabFrameSlot()));


}

MainWindow::~MainWindow()
{
    frameGrabber->terminate();
    frameGrabber->wait();

    delete frameGrabber;
    delete serialPort;
    delete ui;
}

void MainWindow::on_actionSettings_triggered()
{
    //qDebug()<<"The action was triggered now!";
    int res;
    configDlg gD(this);
    res = gD.exec();
    if(res == QDialog::Rejected)
        return;


}

QImage MainWindow::Mat2QImage(Mat& image)

{

    QImage img;


    if (image.channels()==3) {

        cvtColor(image, image, CV_BGR2RGB);

        img = QImage((const unsigned char *)(image.data), image.cols, image.rows,

                image.cols*image.channels(), QImage::Format_RGB888);

    } else if (image.channels()==1) {

        img = QImage((const unsigned char *)(image.data), image.cols, image.rows,

                image.cols*image.channels(), QImage::Format_ARGB32);

    } else {

        img = QImage((const unsigned char *)(image.data), image.cols, image.rows,

                image.cols*image.channels(), QImage::Format_RGB888);

    }


    return img;

}

void MainWindow::cameraWarningWdiget()
{
    QWidget *cameraWarning = new QWidget;
    QMessageBox::warning(cameraWarning,tr("错误"), tr("相机连接错误，请确认相机连接状况"));
}


void MainWindow::displayLiveFrame(Mat liveFrame)
{
    ui->liveFrame->setPixmap(QPixmap::fromImage(Mat2QImage(liveFrame)));
}


QString MainWindow::getConfigValue(int propId)
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
        case 4:
        {
            QString comPort = configFile->value("basicInfo/comPort").toString();
            return comPort;
            break;
        }
        case 5:
        {
            QString cylinderDelay  = configFile->value("basicInfo/cylinderDelay").toString();
            return cylinderDelay;
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


void MainWindow::on_originalPointBtn_clicked()
{
    serialPort->write(QByteArray::number(9));  //go back to original position
    char buffer[50];
    while(serialPort->read(buffer, 50))
    {
        emit stopGrabFrame();
        qDebug()<<"Receive DATA now!"<<buffer;
    }

}

void MainWindow::on_lightBtn_clicked()
{
    //emit stopGrabFrame();
    QProcess::startDetached("explorer gapCheck Configuration");
}

void MainWindow::on_launchBtn_clicked()
{
    ui->launchBtn->setEnabled(false);
    luanchBtnStage = false;
    ui->ng_items->setText("Loading");

    emit grabFrameNow();

    ui->tableWidget->clear();

    pe.setColor(QPalette::WindowText, Qt::black);
    ui->label->setPalette(pe);
    ui->label->setText("LD");


    //Sleep(getConfigValue(5).toInt());
    //serialPort->write(QByteArray::number(7));   //闭合气缸

    serialPort->write(QByteArray::number(0));   //推动拨杆

//    char buffer[50];
//    while(!serialPort->read(buffer, 50))
//    {
//        //serialPort->read(buffer, 50) == 6;

//        //qDebug()<<"Receive DATA now!"<<buffer;
//    }
    //emit stopGrabFrame();
}


void MainWindow::displayDistData(QList<int>& listData)
{
    qDebug()<<"UI thread receive data now!";
    qDebug()<< "Dist Data is" << (getConfigValue(17).toInt() - listData[0])/calibrationValue - locator_offset.toDouble();

    int ng_itme = 0;

    int displayCount = 0;
    bool ngOrOK = true;
    double distData;
    for(int rows = 0; rows < 10; rows++)
    {

        for(int cols = 0; cols < 3; cols++)
        {
            distData = (getConfigValue(17).toInt() -listData[displayCount])/calibrationValue - locator_offset.toDouble();
            ui->tableWidget->setItem(rows, cols, new QTableWidgetItem(QString::number(distData)));
            if((distData > max_distant)||(distData < min_distant))
            {
                if(distData >max_distant)
                    ng_itme = 1;        // '1' means more than 0.5mm
                else if(distData < min_distant)
                    ng_itme = 2;        //'2' means less than 0.3mm
                else
                    ng_itme = 0;

                ui->tableWidget->item(rows,cols)->setBackgroundColor(QColor("red"));
                ngOrOK = false;

            }
            displayCount++;
        }
    }
    if(!ngOrOK)
    {
        ui->label->setText("NG");

        pe.setColor(QPalette::WindowText, Qt::red);
        ui->label->setPalette(pe);

        if(ng_itme == 1)    //more than 0.5mm
        {
            ui->ng_items->setText("0.5 NG");
        }

        else if(ng_itme ==2)
        {
            ui->ng_items->setText("0.3 NG");
        }
    }
    else
    {
        ui->label->setText("OK");
        pe.setColor(QPalette::WindowText, Qt::green);
        ui->label->setPalette(pe);

        ui->ng_items->setText("OK");
    }

    ui->launchBtn->setEnabled(true);
    luanchBtnStage = true;


}


void MainWindow::keyPressEvent(QKeyEvent *e)
{
    if((e->key() == Qt::Key_Enter) && luanchBtnStage)
    {
        on_launchBtn_clicked();
        qDebug() << "Detect keyboard is pressed!";
     }

}

void MainWindow::on_lightBtn_2_clicked()
{

}

void MainWindow::on_actionOpen_triggered()
{
    on_lightBtn_clicked();
}






void MainWindow::on_actionCalibrate_triggered()
{
    emit grabCalibrationFrame();
}

void MainWindow::grabCalibrationFrame(Mat frame4Calibrate)
{
    imwrite("gapCheck Configuration\\source frames\\frame4Calibrate.bmp", frame4Calibrate);
    on_pushButton_clicked(frame4Calibrate);
}

void MainWindow::on_pushButton_clicked(Mat calibFrame)
{
//    Mat calibFrame = imread("gapCheck Configuration\\source frames\\3.jpg", 1);
//    imshow("Original Frame", calibFrame);
    Mat imageEdged;
    Canny(calibFrame, imageEdged, 20, 40);
    imwrite("gapCheck Configuration\\source frames\\imageEdged.bmp", imageEdged);
//    imshow("", imageEdged);

////    HoughLines(imageEdged, );
//    vector<Vec4i> lines;
//    HoughLinesP(imageEdged, lines, 1, CV_PI/180, 80, 200, 10);
//    drawDetectLines(calibFrame,lines,Scalar(0, 255, 0));

//    imwrite("gapCheck Configuration\\source frames\\imageLined.jpg", calibFrame);
//    imshow("Line Detect", calibFrame);
    double calibValue =  calculateCalibValue(imageEdged);
//    ui->label_2->setText(tr("标定值："));
    ui->ng_items->setText(QString::number(1/calibValue));


}

void MainWindow::drawDetectLines(Mat& image,const vector<Vec4i>& lines,Scalar & color)
{
     // 将检测到的直线在图上画出来
     vector<Vec4i>::const_iterator it=lines.begin();
     while(it!=lines.end())
     {
         Point pt1((*it)[0],(*it)[1]);
         Point pt2((*it)[2],(*it)[3]);
         line(image,pt1,pt2,color,1); //  线条宽度设置为2
         ++it;
     }
 }
double MainWindow::calculateCalibValue(Mat frameEdged)
{
    //The specified col of those index is 150, 250, 350


    //Looking for upper X index for calibration
    QList<int> rowIndex4Calib_upper;
    int colIndex_upper = 150;
    for(int numIndex = 0; numIndex <3; numIndex++)
    {
        for (int j = 0; j < frameEdged.rows; j++)
        {
            uchar* data = frameEdged.ptr<uchar>(j);
            {
                if(data[colIndex_upper] == 255)
                {
                    rowIndex4Calib_upper.push_back(j);
                    colIndex_upper = colIndex_upper + 100;
                    break;
                }
            }
        }
    }

    //Looking for lower X index for calibration
    QList<int> rowIndex4Calib_lower;
    int colIndex_lower = 150;
    for(int numIndex = 0; numIndex <3; numIndex++)
    {
        for (int j = frameEdged.rows - 1; j > 0; j--)
        {
            uchar* data = frameEdged.ptr<uchar>(j);
            {
                if(data[colIndex_lower] == 255)
                {
                    rowIndex4Calib_lower.push_back(j);
                    colIndex_lower = colIndex_lower + 100;
                    break;
                }
            }
        }
    }


    QList<double> theta;

    double theta_upper;
    //theta 1
    if(rowIndex4Calib_upper[0] == rowIndex4Calib_upper[1])
    {
        theta_upper = 0;
        theta.push_back(theta_upper);
    }
    else
    {
        theta_upper = atan((rowIndex4Calib_upper[1] - rowIndex4Calib_upper[0])/100.0) *180.0/CV_PI;
        theta.push_back(theta_upper);
    }
    //theta 2
    if(rowIndex4Calib_upper[0] == rowIndex4Calib_upper[2])
    {
        theta_upper = 0;
        theta.push_back(theta_upper);
    }
    else
    {
        theta_upper = atan((rowIndex4Calib_upper[2] - rowIndex4Calib_upper[0])/200.0) *180.0/CV_PI;
        theta.push_back(theta_upper);
    }

    //theta 3
    if(rowIndex4Calib_upper[1] == rowIndex4Calib_upper[2])
    {
        theta_upper = 0;
        theta.push_back(theta_upper);
    }
    else
    {
        theta_upper = atan((rowIndex4Calib_upper[2] - rowIndex4Calib_upper[1])/100.0) *180.0/CV_PI;
        theta.push_back(theta_upper);
    }


    //theta 4
    if(rowIndex4Calib_lower[0] == rowIndex4Calib_lower[1])
    {
        theta_upper = 0;
        theta.push_back(theta_upper);
    }
    else
    {
        theta_upper = atan((rowIndex4Calib_lower[1] - rowIndex4Calib_lower[0])/100.0) *180.0/CV_PI;
        theta.push_back(theta_upper);
    }
    //theta 6
    if(rowIndex4Calib_lower[0] == rowIndex4Calib_lower[2])
    {
        theta_upper = 0;
        theta.push_back(theta_upper);
    }
    else
    {
        qDebug() << rowIndex4Calib_lower[1];

        theta_upper = atan((rowIndex4Calib_lower[2] - rowIndex4Calib_lower[0])/200.0) *180.0/CV_PI;
        theta.push_back(theta_upper);
    }

    //theta 6
    if(rowIndex4Calib_lower[1] == rowIndex4Calib_lower[2])
    {
        theta_upper = 0;
        theta.push_back(theta_upper);
    }
    else
    {


        theta_upper = atan((rowIndex4Calib_lower[2] - rowIndex4Calib_lower[1])/100.0) *180.0/CV_PI;
        theta.push_back(theta_upper);
    }

    double theta_sum = 0;
    for(auto i = theta.begin(); i != theta.end(); i++)
    {
        theta_sum += *i;
    }

    double theta_Ave = theta_sum/(theta.length());


    QList<int> vertical_LIST;
    for(int i = 0; i < 3; i++)
    {
        int vertical_Dist = rowIndex4Calib_lower[i] - rowIndex4Calib_upper[i];
        vertical_LIST.push_back(vertical_Dist);
    }

    double dist_sum = 0;
    for(auto i = vertical_LIST.begin(); i != vertical_LIST.end(); i++)
    {
        dist_sum += *i;
    }

    double dist_Ave = dist_sum/(vertical_LIST.length());

    double calibValue = dist_Ave * cos(theta_Ave/180*CV_PI);
    qDebug() << "The Calibration Value is " << calibValue;
    return calibValue;

}

