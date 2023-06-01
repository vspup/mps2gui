#include <QDebug>

#include "inc/selectwindow.h"
#include "ui_selectwindow.h"
#include "inc/menu.h"

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#if (__unix__)
#endif
#include <unistd.h>
#include <getopt.h>
#include <ctype.h>
#if defined(_WIN32) || defined(WIN32)
#include <windows.h>
#include "lib/nng/include/nng/nng.h"
#include "lib/nng/include/nng/protocol/reqrep0/req.h"
#else
#include "lib/nng/include/nng/nng.h"
#include "lib/nng/include/nng/protocol/reqrep0/req.h"
#endif
#include "inc/electabuzz_client.h"
#include "inc/electabuzz_common.h"
#include "inc/uart-escape.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include "inc/commands.h"
#include "inc/variables_list.h"
#include "inc/response_parser.h"
#include <QDateTime>
#include <QTime>
#include <QThread>
#include <QDir>
QFile filelog;



SelectWindow::SelectWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SelectWindow)
{
    ui->setupUi(this);

    mps_menu_state_t start_menu = MPS_MENU_CONNECT_PSU;

    ui->stackedWidget->setCurrentIndex(start_menu);
}

SelectWindow::~SelectWindow()
{
    delete ui;
}

void SelectWindow::on_btConnect_clicked()
{
    qInfo() << "on_btConnect_clicked";

    if(connectionStatus)
    {
        writeLog("USER: CLICED \"DISCONNECT\"");

        nng_close(nng_sock);
        connectionStatus = 0;
        ui->btConnect->setText("CONNECT");
        ui->gbConnection->setTitle("CONNECTION: DISCONNECTED");
        mode = 0;

        ui->stackedWidget->setCurrentIndex(0);
        filelog.close();
        return;
    }

    if(!QDir("Logs").exists())
    {
        QDir().mkdir("Logs");
    }

    QDate date = QDate::currentDate();
    QTime time = QTime::currentTime();
    QString log_name  = "Logs/" + time.toString() + " " + date.toString() +"_Log.txt";

    log_name.replace(":", "_");
    log_name.replace(" ", "_");

    filelog.setFileName(log_name);
    if(!filelog.open(QIODevice::ReadWrite))
    {
        qCritical() << "Could not open file";
    }
    writeLog("USER: CLICED \"CONNECT\"");

    char tempBuff[64] = {0};
    QString tempStr =ui->comboBox->currentText();

    QByteArray ba=tempStr.toLatin1();
    sprintf((char*)&tempBuff[0], "tcp://%s:5555", ba.data());
    ui->gbConnection->setTitle("CONNECTION: CONNECTING...");
    ui->gbConnection->repaint();
    if(prepare_nng(tempBuff))
    {
        /*double data = 0.15;
        struct eb_write_data_point_info_s dp_write = {0};
        struct eb_data_element_s* data_elements_p = (eb_data_element_s*)malloc(sizeof(struct eb_data_element_s)*1);

        data_elements_p[0].value_p = &data;

        data_point_id = GET_SET_FAN_PWM;
        dp_write.data_point_id = data_point_id;
        dp_write.array_length = 0;
        dp_write.type = EB_TYPE_DOUBLE;
        dp_write.elements_p = data_elements_p;
        eb_send_multi_write_request(&dp_write, 1, &transaction_id, &eb_write_data_response_handler, NULL);*/
        ui->btConnect->setText("DISCONNECT");
        ui->gbConnection->setTitle("CONNECTION: CONNECTED");
        //ui->btSetMain_Tab->setEnabled(true);
        //ui->btSetSHIM_Tab->setEnabled(true);
        //ui->gbStatus->setEnabled(true);

        connectionStatus = 1;
        writeLog("CONNECTED");
    }
    else
    {
        ui->gbConnection->setTitle("CONNECTION: ERROR");
        connectionStatus = 0;
        writeLog("CONNECTION: ERROR");
        filelog.close();
    }
}

void SelectWindow::writeLog(QString logstr)
{
    QTextStream stream(&filelog);
    QDateTime dt = QDateTime::currentDateTime();
    stream << (dt.toString() + "\r\n");
    stream << (logstr + "\r\n");
    filelog.flush();
}
