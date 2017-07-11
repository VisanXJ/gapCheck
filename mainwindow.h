#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QMenu>
#include <QToolBar>
#include "cframegrabber.h"
#include <QMetaType>
#include "QMessageBox"
#include <QSerialPort>
#include <QKeyEvent>
#include <math.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    QImage Mat2QImage(Mat& image);

    QString getConfigValue(int propId);
    QSerialPort *serialPort;

    //const int distantArea = 120;
    QString lastHeigth;
    QString locator_offset;

    double max_distant = getConfigValue(20).toDouble();
    double min_distant = getConfigValue(21).toDouble();

    QPalette pe;

    void keyPressEvent(QKeyEvent *);    //Key board callback function
    bool luanchBtnStage;

    double calibrationValue;
    void drawDetectLines(Mat& image,const vector<Vec4i>& lines,Scalar & color);
    double calculateCalibValue(Mat frameEdged);

    ~MainWindow();

private slots:
    void on_actionSettings_triggered();




    void on_originalPointBtn_clicked();

    void on_lightBtn_clicked();

    void on_launchBtn_clicked();

    void on_lightBtn_2_clicked();

    void on_actionOpen_triggered();



    void on_actionCalibrate_triggered();

    void on_pushButton_clicked(Mat calibFrame);



//    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    CFrameGrabber *frameGrabber;

public slots:
    void cameraWarningWdiget();
    void displayLiveFrame(Mat liveFrame);
    void grabCalibrationFrame(Mat frame4Calibrate);

    void displayDistData(QList<int>& listData);


signals:
    void grabFrameNow();

    void grabCalibrationFrame();

    void stopGrabFrame();
    void calibrateFrame();

    void signal_waveCheck();
};

#endif // MAINWINDOW_H
