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

    QPalette pe;

    void keyPressEvent(QKeyEvent *);    //Key board callback function
    bool luanchBtnStage;

    ~MainWindow();

private slots:
    void on_actionSettings_triggered();




    void on_originalPointBtn_clicked();

    void on_lightBtn_clicked();

    void on_launchBtn_clicked();

private:
    Ui::MainWindow *ui;
    CFrameGrabber *frameGrabber;

public slots:
    void cameraWarningWdiget();
    void displayLiveFrame(Mat liveFrame);
    void displayDistData(QList<int>& listData);


signals:
    void grabFrameNow();
    void stopGrabFrame();
};

#endif // MAINWINDOW_H
