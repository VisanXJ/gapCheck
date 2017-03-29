#ifndef CONFIGDLG_H
#define CONFIGDLG_H

#include <QDialog>

namespace Ui {
class configDlg;
}

class configDlg : public QDialog
{
    Q_OBJECT

public:
    explicit configDlg(QWidget *parent = 0);
    ~configDlg();

private slots:


    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::configDlg *ui;
};

#endif // CONFIGDLG_H
