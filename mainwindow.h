#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    void MainSetText(QString insrt);
    void updateGUI(void);
    void updateGeneralGUI(void);
    void updateRampUpGUI(void);
    ~MainWindow();

private slots:
    void on_pushButton_Conn_clicked();

    void on_pushButtonAX_clicked();

    void on_pushButton_T1_clicked();

    void on_pushButton_T2_clicked();

    void on_pushButton_SetV_clicked();

    void on_pushButton_SetV_0_clicked();

    void on_pushButton_Refresh_clicked();

    void on_pushButton_ON_clicked();

    void on_pushButton_OFF_clicked();

    void on_pushButton_SetCurr_clicked();

    void on_pushButton_SetCurr0_clicked();

    void on_pushButton_clicked();

    void on_pushButton_SetV_2_clicked();

    void slotTimerAlarm();

    void on_pushButton_17_clicked();


    void on_pushButton_18_clicked();

    void on_pushButton_19_clicked();


    void on_pushButton_set_I_zero_clicked();

    void on_pushButton_V_set0_clicked();

    void on_pushButton_V_set_clicked();

    void on_pushButton_setI_clicked();

    void on_pushButton_PlusV_clicked();

    void on_pushButton_pllusI_clicked();

    void on_pushButton_minusV_clicked();

    void on_pushButton_minusI_clicked();

    void on_pushButton_OnOffRampUp_clicked();

    void on_pushButton_setax0_clicked();

    void on_pushButton_setT1_0_clicked();

    void on_pushButton_setT1_clicked();

    void on_pushButton_setT2_0_clicked();

    void on_pushButton_setT2_clicked();

    void on_pushButton_setmain0_clicked();

    void on_pushButton_setmain_clicked();

    void on_pButt_setax0_SHIM_clicked();

    void on_pButt_setT1_SHIM_clicked();

    void on_pButt_setT1_SHIM_2_clicked();

    void on_pButt_setT2_SHIM_clicked();

    void on_pButton_setT2_SHIM_clicked();

private:
    Ui::MainWindow *ui;
    QTimer *timer;
};




#endif // MAINWINDOW_H
