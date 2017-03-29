#include "configdlg.h"
#include "ui_configdlg.h"
#include "QSettings"

configDlg::configDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::configDlg)
{
    ui->setupUi(this);

    QSettings *configFile = new QSettings("gapCheck Configuration\\gabCheckConfig.ini", QSettings::IniFormat);

    ui->cameraID->setText(configFile->value("basicInfo/cameraID").toString());
    ui->CV_CAP_PROP_FRAME_WIDTH->setText(configFile->value("basicInfo/CV_CAP_PROP_FRAME_WIDTH").toString());
    ui->CV_CAP_PROP_FRAME_HEIGHT->setText(configFile->value("basicInfo/CV_CAP_PROP_FRAME_HEIGHT").toString());
    ui->comPort->setText(configFile->value("basicInfo/comPort").toString());
}

configDlg::~configDlg()
{
    delete ui;
}



void configDlg::on_buttonBox_accepted()
{
    accept();
}

void configDlg::on_buttonBox_rejected()
{
    reject();
}
