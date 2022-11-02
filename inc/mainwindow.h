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
    void ClearTable (void);
    //void nngGetRequest( int cmd);
    ~MainWindow();
signals:
    void transmit_to_nng(int cmd);

private slots:
    void receive_from_gui(bool value);

    void on_pushButtonAX_clicked();

    void on_pushButton_T1_clicked();

    void on_pushButton_T2_clicked();

    void on_pushButton_SetV_clicked();

    void on_pushButton_SetV_0_clicked();

    void on_pushButton_Refresh_clicked();

    void on_pushButton_ON_clicked();

    void on_pushButton_SetCurr_clicked();

    void on_pushButton_SetCurr0_clicked();

    void slotTimerAlarm();

    void nngGetRequest(int);

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

    void on_pushButton_SetFAN_0_clicked();

    void on_btConnect_clicked();

    void on_btSetSHIM_Tab_clicked();

    void on_btSetMain_Tab_clicked();

    void on_btSetFAN_MainTab_clicked();

    void on_btSetFAN0_MainTab_clicked();

    void on_btSetFAN_ShimTab_clicked();

    void on_btSetFAN0_ShimTab_clicked();

    void on_btPSH_ShimSetCurrent0_clicked();

    void on_btPSH_ShimSetCurrent_clicked();

private:
    Ui::MainWindow *ui;
    QTimer *timer;
};




#endif // MAINWINDOW_H
