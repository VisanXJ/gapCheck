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

    int displayCount = 0;
    bool ngOrOK = true;
    double distData;
    for(int rows = 0; rows < 10; rows++)
    {

        for(int cols = 0; cols < 3; cols++)
        {
            distData = (getConfigValue(17).toInt() -listData[displayCount])/calibrationValue - locator_offset.toDouble();
            ui->tableWidget->setItem(rows, cols, new QTableWidgetItem(QString::number(distData)));
            if((distData >0.5)||(distData < 0.3))
            {

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
    }
    else
    {
        ui->label->setText("OK");
        pe.setColor(QPalette::WindowText, Qt::green);
        ui->label->setPalette(pe);
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
    imwrite("gapCheck Configuration\\source frames\\frame4Calibrate.jpg", frame4Calibrate);
}
