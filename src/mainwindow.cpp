#define SENT_ZERO   1
#include "inc/mainwindow.h"
#include "./ui_mainwindow.h"

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
QFile filelog;


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

#if defined(_WIN32) || defined(WIN32)
    // test windows timstamp
    uint32_t ts = eb_get_time_stamp();
    char str[50];
    sprintf_s(str, sizeof(str), "windows timestamp: %d \n", ts);
    OutputDebugStringA(str);
#else
    // test linux timestamp
    uint32_t ts = eb_get_time_stamp();
    printf("linux timestamp:%u\n", ts);
#endif

    QFile file("ip_config.txt");
    if(!file.open(QIODevice::ReadWrite))
    {
        qCritical() << "Could not open file";
        qCritical() << file.errorString();
    }

    QTextStream stream(&file);
    while(!stream.atEnd())
    {
        QString line = stream.readLine();
        ui->comboBox->addItem(line);
    }

    QDate date = QDate::currentDate();
    QTime time = QTime::currentTime();
    QString log_name  = time.toString() + " " + date.toString() +"_Log.txt";

    log_name.replace(":", "_");
    log_name.replace(" ", "_");


    filelog.setFileName(log_name);
    if(!filelog.open(QIODevice::ReadWrite))
    {
       qCritical() << "Could not open file";
       qCritical() << file.errorString();
       //MessageBox::warning(this, "Error open log file",log_name);
    }

    ui->btSetMain_Tab->setDisabled(true);
    ui->btSetSHIM_Tab->setDisabled(true);
    ui->gbStatus->setDisabled(true);

    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(slotTimerAlarm()));
    timer->start(100);
    connect(this,SIGNAL(transmit_to_nng(int)),this,SLOT(nngGetRequest(int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

uint8_t exeMode;
double dataArray[6] = {0};

#define CMD_UPDATE_BY_TIMER           1
#define CMD_SET_FAN_MAIN_TAB          2
#define CMD_SET_FAN0_MAIN_TAB         3
#define CMD_SET_FAN_SHIM_TAB          4
#define CMD_SET_FAN0_SHIM_TAB         5
#define CMD_SET_AX                    6
#define CMD_SET_T1                    7
#define CMD_SET_T2                    8
#define CMD_SET_VOLTAGE_SHIM          9
#define CMD_SET_VOLTAGE0_SHIM         10
#define CMD_SET_CURRENT_SHIM          11
#define CMD_SET_CURRENT0_SHIM         12
#define CMD_SET_SHIM_PSH_CURRENT      13
#define CMD_SET_SHIM_PSH_CURRENT0     14

void MainWindow::nngGetRequest( int cmd)
{

    //QString temp;
    //temp.setNum(cmd);
    //QMessageBox::warning(this, "got signal", "debug msg" + temp);
   if(cmd == CMD_UPDATE_BY_TIMER)
   {
    if(connectionStatus)
    {

       updateGeneralGUI();
       exeMode =  mode;
       if(exeMode == MODE_SHIM)
       {
          updateGUI();
       }
       else if(exeMode == MODE_RAMPUP)
       {
          updateRampUpGUI();
       }
    }
     return;
   }

   QString tempData;


   double data;
   uint32_t dataU32 =0;
   struct eb_write_data_point_info_s dp_write = {0};
   struct eb_data_element_s* data_elements_p = (eb_data_element_s*)malloc(sizeof(struct eb_data_element_s)*6);

   data_elements_p[0].value_p = &data;
   dp_write.array_length = 0;

   switch(cmd)
   {

      case CMD_SET_FAN_MAIN_TAB:
           tempData = ui->plainTextSetFAN->toPlainText();
           data =  tempData.toDouble()/100;
           dp_write.data_point_id = GET_SET_FAN_PWM;
           dp_write.type = EB_TYPE_DOUBLE;
      break;

      case CMD_SET_FAN0_MAIN_TAB:
           data =  0;
           dp_write.data_point_id = GET_SET_FAN_PWM;
           dp_write.type = EB_TYPE_DOUBLE;
      break;

      case CMD_SET_FAN_SHIM_TAB:
           tempData = ui->plainTextSetFAN_2->toPlainText();
           data =  tempData.toDouble()/100;
           dp_write.data_point_id = GET_SET_FAN_PWM;
           dp_write.type = EB_TYPE_DOUBLE;
      break;

      case CMD_SET_FAN0_SHIM_TAB:
           data =  0;
           dp_write.data_point_id = GET_SET_FAN_PWM;
           dp_write.type = EB_TYPE_DOUBLE;
      break;

      case CMD_SET_AX:
           dataU32 = 0;// ax mode
           data_elements_p[0].value_p = &dataU32;
           dp_write.data_point_id = GET_CHANNEL;
           dp_write.type = EB_TYPE_UINT32;
      break;

      case CMD_SET_T1:
           dataU32 = 1;// T1 mode
           data_elements_p[0].value_p = &dataU32;
           dp_write.data_point_id = GET_CHANNEL;
           dp_write.type = EB_TYPE_UINT32;
      break;

      case CMD_SET_T2:
           dataU32 = 2;// T2 mode
           data_elements_p[0].value_p = &dataU32;
           dp_write.data_point_id = GET_CHANNEL;
           dp_write.type = EB_TYPE_UINT32;
      break;

      case CMD_SET_VOLTAGE_SHIM:
           tempData = ui->pTextV_set->toPlainText();
           data =  tempData.toDouble();
           dp_write.data_point_id = SET_VOLTAGE;
           dp_write.type = EB_TYPE_DOUBLE;
      break;

      case CMD_SET_VOLTAGE0_SHIM:
           tempData = ui->plainTextSetFAN_2->toPlainText();
           data =  0.1;
           dp_write.data_point_id = SET_VOLTAGE;
           dp_write.type = EB_TYPE_DOUBLE;
      break;

      case CMD_SET_CURRENT_SHIM:
           data_elements_p[0].value_p = &dataArray[0];
           data_elements_p[1].value_p = &dataArray[1];
           data_elements_p[2].value_p = &dataArray[2];
           data_elements_p[3].value_p = &dataArray[3];
           data_elements_p[4].value_p = &dataArray[4];
           data_elements_p[5].value_p = &dataArray[5];
           dp_write.data_point_id = GET_SET_CURRENT;
           dp_write.array_length = 6;
           dp_write.type = EB_TYPE_DOUBLE;

      break;

      case CMD_SET_CURRENT0_SHIM:
           for(int i=0; i<6; i++)
           {
              dataArray[i] = 0;
              data_elements_p[i].value_p = &dataArray[i];
           }
           dp_write.data_point_id = GET_SET_CURRENT;
           dp_write.array_length = 6;
           dp_write.type = EB_TYPE_DOUBLE;
      break;

      case CMD_SET_SHIM_PSH_CURRENT:
           data_elements_p[0].value_p = &dataArray[0];
           data_elements_p[1].value_p = &dataArray[1];
           dp_write.data_point_id = GET_SET_I_SETPOINT_HEATERS;
           dp_write.array_length = 2;
           dp_write.type = EB_TYPE_FLOAT;
      break;

      case CMD_SET_SHIM_PSH_CURRENT0:
           dataArray[0] = 0;
           dataArray[1] = 0;
           data_elements_p[0].value_p = &dataArray[0];
           data_elements_p[1].value_p = &dataArray[1];
           dp_write.data_point_id = GET_SET_I_SETPOINT_HEATERS;
           dp_write.array_length = 2;
           dp_write.type = EB_TYPE_FLOAT;
      break;

   }



       dp_write.elements_p = data_elements_p;
       eb_send_multi_write_request(&dp_write, 1, &transaction_id, &eb_write_data_response_handler, NULL);

}

void MainWindow::slotTimerAlarm()
{
  emit transmit_to_nng(CMD_UPDATE_BY_TIMER);
}

QString tempStr;
QByteArray tempVal;

int cmdList[] =
{
   GET_ON_OFF_STATUS,
   GET_CURRENT,
   GET_VA,
   GET_VB,
   GET_TEMP_A,
   GET_TEMP_B,
   GET_VIN,
   GET_VOUT,
   GET_IIN,
   GET_IOUT,
   GET_BCM_TEMP,
   GET_SET_FAN_PWM
};

void MainWindow::updateGeneralGUI(void)
{
  static int cmdCounter =0;
//ReadCurrentData
//ReadVA
//ReadVB
//tempA
//tempB
//ReadVIN
//ReadIIN
//ReadVOUT
//ReadIOUT
//BCMtemp

    data_id = cmdList [cmdCounter];
    eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
    ReadData();

    cmdCounter++;

    if(cmdCounter >= 12)
    {
       cmdCounter = 0;
    }

    //GET_ON_OFF_STATUS;
    if(mode == MODE_SELECTING)
    {
        switch(mode_status)
        {
           case 0: ui->gbStatus->setTitle("STATUS: DISABLED");
                   ui->btSetSHIM_Tab->setEnabled(true);
                   ui->btSetMain_Tab->setEnabled(true);
           break;
           case 1: ui->gbStatus->setTitle("STATUS: Main Coil");
                   ui->btSetSHIM_Tab->setDisabled(true);
                   ui->btSetMain_Tab->setEnabled(true);
           break;
           case 2: ui->gbStatus->setTitle("STATUS: Shim Coils");
                   ui->btSetSHIM_Tab->setEnabled(true);
                   ui->btSetMain_Tab->setDisabled(true);
           break;
           case 100: ui->gbStatus->setTitle("STATUS: ERROR");
                   ui->btSetSHIM_Tab->setDisabled(true);
                   ui->btSetMain_Tab->setDisabled(true);
           break;
        }
    }

    tempStr.setNum(ReadCurrentData[0], 'f', 5);
    ui->txtIm0->setPlainText(tempStr);
    tempStr.setNum(ReadCurrentData[1], 'f', 5);
    ui->txtIm1->setPlainText(tempStr);
    tempStr.setNum(ReadCurrentData[2], 'f', 5);
    ui->txtIm2->setPlainText(tempStr);
    tempStr.setNum(ReadCurrentData[3], 'f', 5);
    ui->txtIm3->setPlainText(tempStr);
    tempStr.setNum(ReadCurrentData[4], 'f', 5);
    ui->txtIm4->setPlainText(tempStr);
    tempStr.setNum(ReadCurrentData[5], 'f', 5);
    ui->txtIm5->setPlainText(tempStr);
    tempStr.setNum(ReadCurrentData[6], 'f', 5);
    ui->txtIm6->setPlainText(tempStr);
    tempStr.setNum(ReadCurrentData[7], 'f', 5);
    ui->txtIm7->setPlainText(tempStr);

    //ReadVA
    tempStr.setNum(ReadVA[0], 'f', 4);
    ui->txtUa0->setPlainText(tempStr);
    tempStr.setNum(ReadVA[1], 'f', 4);
    ui->txtUa1->setPlainText(tempStr);
    tempStr.setNum(ReadVA[2], 'f', 4);
    ui->txtUa2->setPlainText(tempStr);
    tempStr.setNum(ReadVA[3], 'f', 4);
    ui->txtUa3->setPlainText(tempStr);
    tempStr.setNum(ReadVA[4], 'f', 4);
    ui->txtUa4->setPlainText(tempStr);
    tempStr.setNum(ReadVA[5], 'f', 4);
    ui->txtUa5->setPlainText(tempStr);
    tempStr.setNum(ReadVA[6], 'f', 4);
    ui->txtUa6->setPlainText(tempStr);
    tempStr.setNum(ReadVA[7], 'f', 4);
    ui->txtUa7->setPlainText(tempStr);

    //ReadVB
    tempStr.setNum(ReadVB[0], 'f', 4);
    ui->txtUb0->setPlainText(tempStr);
    tempStr.setNum(ReadVB[1], 'f', 4);
    ui->txtUb1->setPlainText(tempStr);
    tempStr.setNum(ReadVB[2], 'f', 4);
    ui->txtUb2->setPlainText(tempStr);
    tempStr.setNum(ReadVB[3], 'f', 4);
    ui->txtUb3->setPlainText(tempStr);
    tempStr.setNum(ReadVB[4], 'f', 4);
    ui->txtUb4->setPlainText(tempStr);
    tempStr.setNum(ReadVB[5], 'f', 4);
    ui->txtUb5->setPlainText(tempStr);
    tempStr.setNum(ReadVB[6], 'f', 4);
    ui->txtUb6->setPlainText(tempStr);
    tempStr.setNum(ReadVB[7], 'f', 4);
    ui->txtUb7->setPlainText(tempStr);

    tempStr.setNum(ReadVA[0] - ReadVB[0], 'f', 4);
    ui->txtUab0->setPlainText(tempStr);
    tempStr.setNum(ReadVA[1] - ReadVB[1], 'f', 4);
    ui->txtUab1->setPlainText(tempStr);
    tempStr.setNum(ReadVA[2] - ReadVB[2], 'f', 4);
    ui->txtUab2->setPlainText(tempStr);
    tempStr.setNum(ReadVA[3] - ReadVB[3], 'f', 4);
    ui->txtUab3->setPlainText(tempStr);
    tempStr.setNum(ReadVA[4] - ReadVB[4], 'f', 4);
    ui->txtUab4->setPlainText(tempStr);
    tempStr.setNum(ReadVA[5] - ReadVB[5], 'f', 4);
    ui->txtUab5->setPlainText(tempStr);
    tempStr.setNum(ReadVA[6] - ReadVB[6], 'f', 4);
    ui->txtUab6->setPlainText(tempStr);
    tempStr.setNum(ReadVA[7] - ReadVB[7], 'f', 4);
    ui->txtUab7->setPlainText(tempStr);

    //tempA
    tempStr.setNum(tempA[0], 'f', 1);
    ui->txtTa0->setPlainText(tempStr);
    tempStr.setNum(tempA[1], 'f', 1);
    ui->txtTa1->setPlainText(tempStr);
    tempStr.setNum(tempA[2], 'f', 1);
    ui->txtTa2->setPlainText(tempStr);
    tempStr.setNum(tempA[3], 'f', 1);
    ui->txtTa3->setPlainText(tempStr);
    tempStr.setNum(tempA[4], 'f', 1);
    ui->txtTa4->setPlainText(tempStr);
    tempStr.setNum(tempA[5], 'f', 1);
    ui->txtTa5->setPlainText(tempStr);
    tempStr.setNum(tempA[6], 'f', 1);
    ui->txtTa6->setPlainText(tempStr);
    tempStr.setNum(tempA[7], 'f', 1);
    ui->txtTa7->setPlainText(tempStr);

    //tempB
    tempStr.setNum(tempB[0], 'f', 1);
    ui->txtTb0->setPlainText(tempStr);
    tempStr.setNum(tempB[1], 'f', 1);
    ui->txtTb1->setPlainText(tempStr);
    tempStr.setNum(tempB[2], 'f', 1);
    ui->txtTb2->setPlainText(tempStr);
    tempStr.setNum(tempB[3], 'f', 1);
    ui->txtTb3->setPlainText(tempStr);
    tempStr.setNum(tempB[4], 'f', 1);
    ui->txtTb4->setPlainText(tempStr);
    tempStr.setNum(tempB[5], 'f', 1);
    ui->txtTb5->setPlainText(tempStr);
    tempStr.setNum(tempB[6], 'f', 1);
    ui->txtTb6->setPlainText(tempStr);
    tempStr.setNum(tempB[7], 'f', 1);
    ui->txtTb7->setPlainText(tempStr);

    //ReadVIN
    tempStr.setNum(ReadVIN[0],'f', 1);
    ui->txtVin0->setPlainText(tempStr);
    tempStr.setNum(ReadVIN[1],'f', 1);
    ui->txtVin1->setPlainText(tempStr);
    tempStr.setNum(ReadVIN[2],'f', 1);
    ui->txtVin2->setPlainText(tempStr);
    tempStr.setNum(ReadVIN[3],'f', 1);
    ui->txtVin3->setPlainText(tempStr);
    tempStr.setNum(ReadVIN[4],'f', 1);
    ui->txtVin4->setPlainText(tempStr);
    tempStr.setNum(ReadVIN[5],'f', 1);
    ui->txtVin5->setPlainText(tempStr);
    tempStr.setNum(ReadVIN[6],'f', 1);
    ui->txtVin6->setPlainText(tempStr);
    tempStr.setNum(ReadVIN[7],'f', 1);
    ui->txtVin7->setPlainText(tempStr);

    //ReadVOUT
    tempStr.setNum(ReadVOUT[0], 'f', 4);
    ui->txtVout0->setPlainText(tempStr);
    tempStr.setNum(ReadVOUT[1], 'f', 4);
    ui->txtVout1->setPlainText(tempStr);
    tempStr.setNum(ReadVOUT[2], 'f', 4);
    ui->txtVout2->setPlainText(tempStr);
    tempStr.setNum(ReadVOUT[3], 'f', 4);
    ui->txtVout3->setPlainText(tempStr);
    tempStr.setNum(ReadVOUT[4], 'f', 4);
    ui->txtVout4->setPlainText(tempStr);
    tempStr.setNum(ReadVOUT[5], 'f', 4);
    ui->txtVout5->setPlainText(tempStr);
    tempStr.setNum(ReadVOUT[6], 'f', 4);
    ui->txtVout6->setPlainText(tempStr);
    tempStr.setNum(ReadVOUT[7], 'f', 4);
    ui->txtVout7->setPlainText(tempStr);

    //ReadIIN
    tempStr.setNum(ReadIIN[0], 'f', 4);
    ui->txtIin0->setPlainText(tempStr);
    tempStr.setNum(ReadIIN[1], 'f', 4);
    ui->txtIin1->setPlainText(tempStr);
    tempStr.setNum(ReadIIN[2], 'f', 4);
    ui->txtIin2->setPlainText(tempStr);
    tempStr.setNum(ReadIIN[3], 'f', 4);
    ui->txtIin3->setPlainText(tempStr);
    tempStr.setNum(ReadIIN[4], 'f', 4);
    ui->txtIin4->setPlainText(tempStr);
    tempStr.setNum(ReadIIN[5], 'f', 4);
    ui->txtIin5->setPlainText(tempStr);
    tempStr.setNum(ReadIIN[6], 'f', 4);
    ui->txtIin6->setPlainText(tempStr);
    tempStr.setNum(ReadIIN[7], 'f', 4);
    ui->txtIin7->setPlainText(tempStr);

    //
    tempStr.setNum(ReadIOUT[0], 'f', 4);
    ui->txtIout0->setPlainText(tempStr);
    tempStr.setNum(ReadIOUT[1], 'f', 4);
    ui->txtIout1->setPlainText(tempStr);
    tempStr.setNum(ReadIOUT[2], 'f', 4);
    ui->txtIout2->setPlainText(tempStr);
    tempStr.setNum(ReadIOUT[3], 'f', 4);
    ui->txtIout3->setPlainText(tempStr);
    tempStr.setNum(ReadIOUT[4], 'f', 4);
    ui->txtIout4->setPlainText(tempStr);
    tempStr.setNum(ReadIOUT[5], 'f', 4);
    ui->txtIout5->setPlainText(tempStr);
    tempStr.setNum(ReadIOUT[6], 'f', 4);
    ui->txtIout6->setPlainText(tempStr);
    tempStr.setNum(ReadIOUT[7], 'f', 4);
    ui->txtIout7->setPlainText(tempStr);




    //GET_BCM_TEMP;
    tempStr.setNum(BCMtemp[0], 'f', 1);
    ui->txtTemp0->setPlainText(tempStr);
    tempStr.setNum(BCMtemp[1], 'f', 1);
    ui->txtTemp1->setPlainText(tempStr);
    tempStr.setNum(BCMtemp[2], 'f', 1);
    ui->txtTemp2->setPlainText(tempStr);
    tempStr.setNum(BCMtemp[3], 'f', 1);
    ui->txtTemp3->setPlainText(tempStr);
    tempStr.setNum(BCMtemp[4], 'f', 1);
    ui->txtTemp4->setPlainText(tempStr);
    tempStr.setNum(BCMtemp[5], 'f', 1);
    ui->txtTemp5->setPlainText(tempStr);
    tempStr.setNum(BCMtemp[6], 'f', 1);
    ui->txtTemp6->setPlainText(tempStr);
    tempStr.setNum(BCMtemp[7], 'f', 1);
    ui->txtTemp7->setPlainText(tempStr);


  /*  data_id = GET_M_FAN_SPEED;
    eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
    ReadData();
    tempStr.setNum(fan_speed, 'f', 1);
    ui->pwmFANlb->setText("FAN SPEED: " + tempStr);*/

    //GET_SET_FAN_PWM;
    tempStr.setNum((pwmFAN * 100), 'f', 0);
    ui->lbFAN_value->setText(tempStr);
    ui->lbFAN_value_2->setText(tempStr);
}

void MainWindow::updateRampUpGUI(void)
{
    data_id = GET_TERMINAL_VOLTAGE;
    eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
    ReadData();
    tempStr.setNum(terminalVoltage, 'f', 4);
    ui->lbVrampUP->setText(tempStr);
    double current = 0;
    for(int i =0; i < 8; i++)
    {
       current += ReadCurrentData[i];
    }
    tempStr.setNum(current, 'f', 4);

    ui->lbIrampUP->setText(tempStr);
    data_id = GET_LINE_VOLTAGE;
    eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
    ReadData();

    tempStr.setNum(lineVoltage, 'f', 9);

    ui->lbVlead->setText(tempStr);
    data_id = GET_MAIN_VOLTAGE;
    eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
    ReadData();

    tempStr.setNum(mainVoltage, 'f', 9);

    ui->lbVmagnet->setText(tempStr);
    data_id = GET_RAMP_UP_STATUS;
    eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
    ReadData();

    switch(mode_status)
    {
       case 0: ui->gbRampUp_status->setTitle("Status: Disabled"); break;
       case 1: ui->gbRampUp_status->setTitle("Status: Main Coil"); break;
       case 2: ui->gbRampUp_status->setTitle("Status: Shim Coils"); break;
       case 3: ui->gbRampUp_status->setTitle("Status: PWM_Test Shim"); break;
       case 100: ui->gbRampUp_status->setTitle("Status: ERROR"); break;
    }

    if(mode_status)
    {
      ui->pushButton_OnOffRampUp->setText("OFF");
    }
    else
    {
      ui->pushButton_OnOffRampUp->setText("ON");
    }

    if(pshModeRampUP)
    {
       data_id = GET_SET_CURRENT_HEATERS;
       eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
       ReadData();
       tempStr.setNum((currentPSH[0] * 1000), 'f', 0);
       ui->lbReadMain->setText(tempStr);
       tempStr.setNum((currentPSH[1] * 1000), 'f', 0);
       ui->lbReadCh->setText(tempStr);
    }

    data_id = GET_SET_I_SETPOINT_HEATERS;
    eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
    ReadData();
}

void MainWindow::updateGUI(void)
{
    char tempBuff[64] = {0};

    memset(dataBuff, 0x00, sizeof(dataBuff));
    data_id = GET_CURRENT;
    eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
    ReadData();

    if(channel == CHANNEL_AX)
    {
        tempStr.setNum(ReadCurrentData[1], 'f', 4);
        ui->lbCurrCH1_get->setText(tempStr);
        tempStr.setNum(ReadCurrentData[4], 'f', 4);
        ui->lbCurrCH2_get->setText(tempStr);
        tempStr.setNum(ReadCurrentData[3], 'f', 4);
        ui->lbCurrCH3_get->setText(tempStr);
        tempStr.setNum(ReadCurrentData[2], 'f', 4);
        ui->lbCurrCH4_get->setText(tempStr);
        tempStr.setNum(ReadCurrentData[0], 'f', 4);
        ui->lbCurrCH5_get->setText(tempStr);
        tempStr.setNum(ReadCurrentData[5], 'f', 4);
        ui->lbCurrCH6_get->setText(tempStr);
    }
    else
    {
        tempStr.setNum(ReadCurrentData[0], 'f', 4);
        ui->lbCurrCH1_get->setText(tempStr);
        tempStr.setNum(ReadCurrentData[1], 'f', 4);
        ui->lbCurrCH2_get->setText(tempStr);
        tempStr.setNum(ReadCurrentData[3], 'f', 4);
        ui->lbCurrCH3_get->setText(tempStr);
        tempStr.setNum(ReadCurrentData[4], 'f', 4);
        ui->lbCurrCH4_get->setText(tempStr);
        tempStr.setNum(ReadCurrentData[2], 'f', 4);
        ui->lbCurrCH5_get->setText(tempStr);
        tempStr.setNum(ReadCurrentData[5], 'f', 4);
        ui->lbCurrCH6_get->setText(tempStr);
    }


    data_id = GET_CHANNEL;
    memset(dataBuff, 0x00, sizeof(dataBuff));
    eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
    ReadData();

    if(pshModeSHIM)
    {
        switch (channelVal)
        {
            case 0: tempStr = "AX";  break;
            case 1: tempStr = "T1";  break;
            case 2: tempStr = "T2";  break;
            default:tempStr = "ERROR MODE";break;
        }

        ui->gbChannel_SHIM->setTitle("CHANNEL: " + tempStr);
    }



    data_id = GET_SET_CURRENT;
    eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
    ReadData();

    data_id = GET_VOLTAGE;
    eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
    ReadData();

    tempStr.setNum(getVoltage);
    ui->lbV_get->setText(tempStr + " V");

    /*data_id = GET_ON_OFF_STATUS;
    eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
    ReadData();*/

    switch(mode_status)
    {
       /*case 0: ui->lbStatusSHIM->setText("Status: Disabled"); break;  ???
       case 1: ui->lbStatusSHIM->setText("Status: Main Coil"); break;
       case 2: ui->lbStatusSHIM->setText("Status: Shim Coils"); break;
       case 3: ui->lbStatusSHIM->setText("Status: PWM_Test Shim"); break;
       case 100: ui->lbStatusSHIM->setText("Status: ERROR"); break;*/
    }

    if(mode_status)
    {
        //ui->lbStatusSHIM->setText("ON");
        ui->gbStatusSHIM->setTitle("Status: ON");
        ui->pushButton_ON->setText("OFF");
        switch(pshModeSHIM)
        {
           case 1: ui->pushButtonAX->setDisabled(true);   ui->pushButton_T2->setDisabled(true); break;
           case 2: ui->pushButtonAX->setDisabled(true);   ui->pushButton_T1->setDisabled(true); break;
           case 3: ui->pushButton_T2->setDisabled(true);   ui->pushButton_T1->setDisabled(true); break;
        }
    }
    else
    {
       ui->gbStatusSHIM->setTitle("Status: OFF");
       ui->pushButton_ON->setText("ON");
       ui->pushButtonAX->setEnabled(true);
       ui->pushButton_T1->setEnabled(true);
       ui->pushButton_T2 ->setEnabled(true);
    }

    if(pshModeSHIM)
    {
       data_id = GET_SET_CURRENT_HEATERS;
       eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
       ReadData();
       tempStr.setNum((currentPSH[1]*1000), 'f', 0);
       ui->plabel_Current_SHIM->setText(tempStr);
    }

    data_id = GET_SET_I_SETPOINT_HEATERS;
    eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
    ReadData();

}

void MainWindow::receive_from_gui(bool value)
{
    //state = value;

    //emit transmit_to_gui(state);
}

void MainWindow::writeLog(QString logstr)
{
    QTextStream stream(&filelog);
    stream << (logstr + "\r\n");
    filelog.flush();
}

void MainWindow::on_btConnect_clicked()
{
   //emit transmit_to_nng(1);
   //return;
   if(connectionStatus)
   {
     writeLog("USER: CLICED \"DISCONNECT\"");

     nng_close(nng_sock);
     connectionStatus = 0;
     ui->btConnect->setText("CONNECT");
     ui->gbConnection->setTitle("CONNECTION: DISCONNECTED");
     ui->gbStatus->setTitle("STATUS: ");
     ui->gbStatus->setDisabled(true);
     mode = 0;

     ui->btSetSHIM_Tab->setDisabled(true);
     ui->btSetMain_Tab->setDisabled(true);

     ui->stackedWidget->setCurrentIndex(0);
     ClearTable();
     return;
   }

   writeLog("USER: CLICED \"CONNECT\"");


   char tempBuff[64] = {0};
   QString tempStr =ui->comboBox->currentText();

   QByteArray ba=tempStr.toLatin1();
   sprintf((char*)&tempBuff[0], "tcp://%s:5555", ba.data());
   ui->gbStatus->setTitle("CONNECTION: CONNECTING...");
   ui->gbStatus->repaint();
   if(prepare_nng(tempBuff))
   {     
       double data = 0.15;
       struct eb_write_data_point_info_s dp_write = {0};
       struct eb_data_element_s* data_elements_p = (eb_data_element_s*)malloc(sizeof(struct eb_data_element_s)*1);

       data_elements_p[0].value_p = &data;

       data_point_id = GET_SET_FAN_PWM;
       dp_write.data_point_id = data_point_id;
       dp_write.array_length = 0;
       dp_write.type = EB_TYPE_DOUBLE;
       dp_write.elements_p = data_elements_p;
       eb_send_multi_write_request(&dp_write, 1, &transaction_id, &eb_write_data_response_handler, NULL);
       ui->btConnect->setText("DISCONNECT");
       ui->gbStatus->setTitle("CONNECTION: CONNECTED");
       ui->btSetMain_Tab->setEnabled(true);
       ui->btSetSHIM_Tab->setEnabled(true);
       //ui->pushButton_19->setEnabled(true);
       ui->gbStatus->setEnabled(true);
       connectionStatus = 1;
       writeLog("CONNECTED");
   }
   else
   {
       ui->gbStatus->setTitle("CONNECTION: ERROR");
       connectionStatus = 0;
       writeLog("CONNECTION: ERROR");
   }
}


void MainWindow::on_pushButtonAX_clicked()
{
     channel = CHANNEL_AX;
     ui->lnCh1->setText("AX1");
     ui->lnCh2->setText("AX2");
     ui->lnCh3->setText("AX3");
     ui->lnCh4->setText("AX4");
     ui->lnCh5->setText("AX5");
     ui->lnCh6->setText("AX6");
     pshModeSHIM = 3;
     ui->pushButton_ON->setEnabled(true);
     ui->btPSH_ShimSetCurrent0->setEnabled(true);
     ui->btPSH_ShimSetCurrent->setEnabled(true);
     emit transmit_to_nng(CMD_SET_AX);
}


void MainWindow::on_pushButton_T1_clicked()
{


     channel = CHANNEL_T1;
     ui->lnCh1->setText("T1 1");
     ui->lnCh2->setText("T1 2");
     ui->lnCh3->setText("T1 3");
     ui->lnCh4->setText("T1 4");
     ui->lnCh5->setText("T1 5");
     ui->lnCh6->setText("T1 6");

     pshModeSHIM = 1;
     ui->pushButton_ON->setEnabled(true);
     ui->btPSH_ShimSetCurrent->setEnabled(true);
     ui->btPSH_ShimSetCurrent0->setEnabled(true);
     emit transmit_to_nng(CMD_SET_T1);

}


void MainWindow::on_pushButton_T2_clicked()
{
    uint32_t data =2;
    struct eb_write_data_point_info_s dp_write = {0};
    struct eb_data_element_s* data_elements_p = (eb_data_element_s*)malloc(sizeof(struct eb_data_element_s)*1);

     data_elements_p[0].value_p = &data;
     data_point_id = GET_CHANNEL;
     dp_write.data_point_id = data_point_id;
     dp_write.array_length = 0;
     dp_write.type = EB_TYPE_UINT32;
     dp_write.elements_p = data_elements_p;
     eb_send_multi_write_request(&dp_write, 1, &transaction_id, &eb_write_data_response_handler, NULL);

     channel = CHANNEL_T2;
     ui->lnCh1->setText("T2 1");
     ui->lnCh2->setText("T2 2");
     ui->lnCh3->setText("T2 3");
     ui->lnCh4->setText("T2 4");
     ui->lnCh5->setText("T2 5");
     ui->lnCh6->setText("T2 6");
     pshModeSHIM = 2;
     ui->pushButton_ON->setEnabled(true);
     ui->btPSH_ShimSetCurrent->setEnabled(true);
     ui->btPSH_ShimSetCurrent0->setEnabled(true);

     emit transmit_to_nng(CMD_SET_T2);
}


void MainWindow::on_pushButton_SetV_clicked()
{
    double data;
    QString tempData = ui->pTextV_set->toPlainText();
    data =  tempData.toDouble();

    if( (data < 0) || (data > 13))
    {
        QMessageBox::warning(this, "Error set voltage", "Value must be in reange from 0 to 13V");
        return;
    }

    emit transmit_to_nng(CMD_SET_VOLTAGE_SHIM);
}


void MainWindow::on_pushButton_SetV_0_clicked()
{
     ui->pTextV_set->setPlainText("0.1");
     emit transmit_to_nng(CMD_SET_VOLTAGE0_SHIM);
}


void MainWindow::on_pushButton_Refresh_clicked()
{
  updateGUI();
}


void MainWindow::on_pushButton_ON_clicked()
{
    uint32_t data;
    struct eb_write_data_point_info_s dp_write = {0};
    struct eb_data_element_s* data_elements_p = (eb_data_element_s*)malloc(sizeof(struct eb_data_element_s)*1);

    if(mode_status == 0)
    {
     data_elements_p[0].value_p = &data;
     data =  2;
     data_point_id = SET_ON_OFF_STATUS;
     dp_write.data_point_id = data_point_id;
     dp_write.array_length = 0;
     dp_write.type = EB_TYPE_UINT32;
     dp_write.elements_p = data_elements_p;
     eb_send_multi_write_request(&dp_write, 1, &transaction_id, &eb_write_data_response_handler, NULL);
     ReadData();
    }
    else
    {
     //uint32_t data;
     //struct eb_write_data_point_info_s dp_write = {0};
     //struct eb_data_element_s* data_elements_p = (eb_data_element_s*)malloc(sizeof(struct eb_data_element_s)*2);

#ifdef SENT_ZERO
      float data1[2] = {0};

        data1[1] = 0;
        data1[0] = 0;


        data_elements_p[0].value_p = &data1[0];
        data_elements_p[1].value_p = &data1[1];

        data_point_id = GET_SET_I_SETPOINT_HEATERS;
        dp_write.data_point_id = data_point_id;
        dp_write.array_length = 2;
        dp_write.type = EB_TYPE_FLOAT;
        dp_write.elements_p = data_elements_p;
        eb_send_multi_write_request(&dp_write, 1, &transaction_id, &eb_write_data_response_handler, NULL);
        ui->pTextEditCurrent_SHIM->setPlainText("0");
#endif
     //struct eb_write_data_point_info_s dp_write = {0};
     //struct eb_data_element_s* data_elements_p = (eb_data_element_s*)malloc(sizeof(struct eb_data_element_s)*1);

     data_elements_p[0].value_p = &data;
     data =  0;
     data_point_id = SET_ON_OFF_STATUS;
     dp_write.data_point_id = data_point_id;
     dp_write.array_length = 0;
     dp_write.type = EB_TYPE_UINT32;
     dp_write.elements_p = data_elements_p;
     eb_send_multi_write_request(&dp_write, 1, &transaction_id, &eb_write_data_response_handler, NULL);
     ReadData();
    }
}



void MainWindow::on_pushButton_SetCurr_clicked()
{

     QString tempData;

     if(channel == CHANNEL_AX)
     {
         tempData = ui->pTextCurrCH1_set->toPlainText();
         dataArray[1] =  tempData.toDouble();
         tempData = ui->pTextCurrCH2_set->toPlainText();
         dataArray[4] =  tempData.toDouble();
         tempData = ui->pTextCurrCH3_set->toPlainText();
         dataArray[3] =  tempData.toDouble();
         tempData = ui->pTextCurrCH4_set->toPlainText();
         dataArray[2] =  tempData.toDouble();
         tempData = ui->pTextCurrCH5_set->toPlainText();
         dataArray[0] =  tempData.toDouble();
         tempData = ui->pTextCurrCH6_set->toPlainText();
         dataArray[5] =  tempData.toDouble();
     }
     else
     {
         tempData = ui->pTextCurrCH1_set->toPlainText();
         dataArray[0] =  tempData.toDouble();
         tempData = ui->pTextCurrCH2_set->toPlainText();
         dataArray[1] =  tempData.toDouble();
         tempData = ui->pTextCurrCH3_set->toPlainText();
         dataArray[3] =  tempData.toDouble();
         tempData = ui->pTextCurrCH4_set->toPlainText();
         dataArray[4] =  tempData.toDouble();
         tempData = ui->pTextCurrCH5_set->toPlainText();
         dataArray[2] =  tempData.toDouble();
         tempData = ui->pTextCurrCH6_set->toPlainText();
         dataArray[5] =  tempData.toDouble();
     }

     for(int i = 0; i < 6; i++)
     {
         if((dataArray[i]<0) || (dataArray[i]>30))
         {
             QMessageBox::warning(this, "Error set current", "Value must be in reange from 0 to 30A");
             return;
         }

     }

     emit transmit_to_nng(CMD_SET_CURRENT_SHIM);
}


void MainWindow::on_pushButton_SetCurr0_clicked()
{

    double data[6] = {0};//{0.17, 0.18, 0.19, 0.20, 0.21, 0.22};
    struct eb_write_data_point_info_s dp_write = {0};
    struct eb_data_element_s* data_elements_p = (eb_data_element_s*)malloc(sizeof(struct eb_data_element_s)*6);


     ui->pTextCurrCH1_set->setPlainText("0");

     ui->pTextCurrCH2_set->setPlainText("0");

     ui->pTextCurrCH3_set->setPlainText("0");

     ui->pTextCurrCH4_set->setPlainText("0");

     ui->pTextCurrCH5_set->setPlainText("0");

     ui->pTextCurrCH6_set->setPlainText("0");


     data_elements_p[0].value_p = &data[0];
     data_elements_p[1].value_p = &data[1];
     data_elements_p[2].value_p = &data[2];
     data_elements_p[3].value_p = &data[3];
     data_elements_p[4].value_p = &data[4];
     data_elements_p[5].value_p = &data[5];


     data_point_id = GET_SET_CURRENT;
     dp_write.data_point_id = data_point_id;
     dp_write.array_length = 6;
     dp_write.type = EB_TYPE_DOUBLE;
     dp_write.elements_p = data_elements_p;
     eb_send_multi_write_request(&dp_write, 1, &transaction_id, &eb_write_data_response_handler, NULL);
     ReadData();
     //updateGUI();
}








void MainWindow::on_pushButton_set_I_zero_clicked()
{
    double data;
    struct eb_write_data_point_info_s dp_write = {0};
    struct eb_data_element_s* data_elements_p = (eb_data_element_s*)malloc(sizeof(struct eb_data_element_s)*1);

     ui->pltextSetI->setPlainText("0");
     data_elements_p[0].value_p = &data;
     data =  0;
     data_point_id = SET_I_MAIN;
     dp_write.data_point_id = data_point_id;
     dp_write.array_length = 0;
     dp_write.type = EB_TYPE_DOUBLE;
     dp_write.elements_p = data_elements_p;
     eb_send_multi_write_request(&dp_write, 1, &transaction_id, &eb_write_data_response_handler, NULL);
}


void MainWindow::on_pushButton_V_set0_clicked()
{
    double data;
    struct eb_write_data_point_info_s dp_write = {0};
    struct eb_data_element_s* data_elements_p = (eb_data_element_s*)malloc(sizeof(struct eb_data_element_s)*1);

     data_elements_p[0].value_p = &data;
     ui->pltextSetV->setPlainText("0");
     data = 0;
     data_point_id = SET_U_MAIN;
     dp_write.data_point_id = data_point_id;
     dp_write.array_length = 0;
     dp_write.type = EB_TYPE_DOUBLE;
     dp_write.elements_p = data_elements_p;
     eb_send_multi_write_request(&dp_write, 1, &transaction_id, &eb_write_data_response_handler, NULL);
}


void MainWindow::on_pushButton_V_set_clicked()
{
    double data;



     QString tempData = ui->pltextSetV->toPlainText();
     data =  tempData.toDouble();

     if((data < 0) || (data > 13))
     {
        QMessageBox::warning(this, "Error set voltage", "Value must be in reange from 0 to 13V");
        return;
     }


     struct eb_write_data_point_info_s dp_write = {0};
     struct eb_data_element_s* data_elements_p = (eb_data_element_s*)malloc(sizeof(struct eb_data_element_s)*1);
     data_elements_p[0].value_p = &data;


     data_point_id = SET_U_MAIN;
     dp_write.data_point_id = data_point_id;
     dp_write.array_length = 0;
     dp_write.type = EB_TYPE_DOUBLE;
     dp_write.elements_p = data_elements_p;
     eb_send_multi_write_request(&dp_write, 1, &transaction_id, &eb_write_data_response_handler, NULL);
}


void MainWindow::on_pushButton_setI_clicked()
{
    double data;

     QString tempData = ui->pltextSetI->toPlainText();
     data =  tempData.toDouble();

     if((data < 0) || (data > 800))
     {
        QMessageBox::warning(this, "Error set current", "Value must be in reange from 0 to 800A");
        return;
     }

     struct eb_write_data_point_info_s dp_write = {0};
     struct eb_data_element_s* data_elements_p = (eb_data_element_s*)malloc(sizeof(struct eb_data_element_s)*1);

     data_elements_p[0].value_p = &data;

     data_point_id = SET_I_MAIN;
     dp_write.data_point_id = data_point_id;
     dp_write.array_length = 0;
     dp_write.type = EB_TYPE_DOUBLE;
     dp_write.elements_p = data_elements_p;
     eb_send_multi_write_request(&dp_write, 1, &transaction_id, &eb_write_data_response_handler, NULL);
}


void MainWindow::on_pushButton_PlusV_clicked()
{

}


void MainWindow::on_pushButton_pllusI_clicked()
{

}


void MainWindow::on_pushButton_minusV_clicked()
{

}


void MainWindow::on_pushButton_minusI_clicked()
{

}


void MainWindow::on_pushButton_OnOffRampUp_clicked()
{
    uint32_t data = 0;
    struct eb_write_data_point_info_s dp_write = {0};
    struct eb_data_element_s* data_elements_p = (eb_data_element_s*)malloc(sizeof(struct eb_data_element_s)*2);

   if(mode_status)
   {
        data = 0;

        #ifdef SENT_ZERO
        float data1[2] = {0};

        data1[1] = 0;
        data1[0] = 0;


        data_elements_p[0].value_p = &data1[0];
        data_elements_p[1].value_p = &data1[1];

        data_point_id = GET_SET_I_SETPOINT_HEATERS;
        dp_write.data_point_id = data_point_id;
        dp_write.array_length = 2;
        dp_write.type = EB_TYPE_FLOAT;
        dp_write.elements_p = data_elements_p;
        eb_send_multi_write_request(&dp_write, 1, &transaction_id, &eb_write_data_response_handler, NULL);
        ui->pTextEditCurrent_SHIM->setPlainText("0");
        #endif
   }
   else
   {
      data = 1;
   }




   data_elements_p[0].value_p = &data;
   data_point_id = SET_ON_OFF_STATUS;
   dp_write.data_point_id = data_point_id;
   dp_write.array_length = 0;
   dp_write.type = EB_TYPE_UINT32;
   dp_write.elements_p = data_elements_p;
   eb_send_multi_write_request(&dp_write, 1, &transaction_id, &eb_write_data_response_handler, NULL);

}


void MainWindow::on_pushButton_setax0_clicked()
{
    uint32_t data =3;
    struct eb_write_data_point_info_s dp_write = {0};
    struct eb_data_element_s* data_elements_p = (eb_data_element_s*)malloc(sizeof(struct eb_data_element_s)*1);

    data_elements_p[0].value_p = &data;
    data_point_id = GET_SET_MODE;
    dp_write.data_point_id = data_point_id;
    dp_write.array_length = 0;
    dp_write.type = EB_TYPE_UINT32;
    dp_write.elements_p = data_elements_p;
    eb_send_multi_write_request(&dp_write, 1, &transaction_id, &eb_write_data_response_handler, NULL);
    pshModeRampUP = 3;

   /* ui->pushButton_V_set0 ->      setEnabled(true);
    ui->pushButton_set_I_zero ->  setEnabled(true);
    ui->pushButton_V_set    ->    setEnabled(true);
    ui->pushButton_setI     ->    setEnabled(true);
    ui->pushButton_PlusV    ->    setEnabled(true);
    ui->pushButton_pllusI   ->    setEnabled(true);
    ui->pushButton_minusV   ->  setEnabled(true);
    ui->pushButton_minusI   ->  setEnabled(true);*/
    ui->pushButton_setmain0 ->  setEnabled(true);
    ui->pushButton_setmain  ->  setEnabled(true);
    ui->pushButton_setT2_0  ->  setEnabled(true);
    ui->pushButton_setT2    ->  setEnabled(true);

    tempStr.setNum((setpointCurrPSH[0] * 1000), 'f', 4);
    ui->plTextMainPSH->setPlainText(tempStr);

    tempStr.setNum((setpointCurrPSH[1] * 1000), 'f', 4);
    ui->plTextChPSH->setPlainText(tempStr);
}


void MainWindow::on_pushButton_setT1_0_clicked()
{
    uint32_t data =1;
    struct eb_write_data_point_info_s dp_write = {0};
    struct eb_data_element_s* data_elements_p = (eb_data_element_s*)malloc(sizeof(struct eb_data_element_s)*1);

    data_elements_p[0].value_p = &data;
    data_point_id = GET_SET_MODE;
    dp_write.data_point_id = data_point_id;
    dp_write.array_length = 0;
    dp_write.type = EB_TYPE_UINT32;
    dp_write.elements_p = data_elements_p;
    eb_send_multi_write_request(&dp_write, 1, &transaction_id, &eb_write_data_response_handler, NULL);
    pshModeRampUP = 1;

    ui->pushButton_setmain0 ->  setEnabled(true);
    ui->pushButton_setmain  ->  setEnabled(true);
    ui->pushButton_setT2_0  ->  setEnabled(true);
    ui->pushButton_setT2    ->  setEnabled(true);

    tempStr.setNum((setpointCurrPSH[0] * 1000), 'f', 4);
    ui->plTextMainPSH->setPlainText(tempStr);

    tempStr.setNum((setpointCurrPSH[1]*1000), 'f', 4);
    ui->plTextChPSH->setPlainText(tempStr);
}


void MainWindow::on_pushButton_setT1_clicked()
{
    uint32_t data =2;
    struct eb_write_data_point_info_s dp_write = {0};
    struct eb_data_element_s* data_elements_p = (eb_data_element_s*)malloc(sizeof(struct eb_data_element_s)*1);

    data_elements_p[0].value_p = &data;
    data_point_id = GET_SET_MODE;
    dp_write.data_point_id = data_point_id;
    dp_write.array_length = 0;
    dp_write.type = EB_TYPE_UINT32;
    dp_write.elements_p = data_elements_p;
    eb_send_multi_write_request(&dp_write, 1, &transaction_id, &eb_write_data_response_handler, NULL);
    pshModeRampUP = 2;

    ui->pushButton_setmain0 ->  setEnabled(true);
    ui->pushButton_setmain  ->  setEnabled(true);
    ui->pushButton_setT2_0  ->  setEnabled(true);
    ui->pushButton_setT2    ->  setEnabled(true);

    tempStr.setNum((setpointCurrPSH[0] * 1000), 'f', 4);
    ui->plTextMainPSH->setPlainText(tempStr);

    tempStr.setNum((setpointCurrPSH[1] * 1000), 'f', 4);
    ui->plTextChPSH->setPlainText(tempStr);
}


void MainWindow::on_pushButton_setT2_0_clicked()
{
    float data[2] = {0};
    struct eb_write_data_point_info_s dp_write = {0};
    struct eb_data_element_s* data_elements_p = (eb_data_element_s*)malloc(sizeof(struct eb_data_element_s)*2);

     ui->plTextChPSH->setPlainText("0");
     data[1] =  0;
     data[0] = setpointCurrPSH[0];


     data_elements_p[0].value_p = &data[0];
     data_elements_p[1].value_p = &data[1];




     data_point_id = GET_SET_I_SETPOINT_HEATERS;
     dp_write.data_point_id = data_point_id;
     dp_write.array_length = 2;
     dp_write.type = EB_TYPE_FLOAT;
     dp_write.elements_p = data_elements_p;
     eb_send_multi_write_request(&dp_write, 1, &transaction_id, &eb_write_data_response_handler, NULL);
     ReadData();
}


void MainWindow::on_pushButton_setT2_clicked()
{
    float data[2] = {0};
    struct eb_write_data_point_info_s dp_write = {0};
    struct eb_data_element_s* data_elements_p = (eb_data_element_s*)malloc(sizeof(struct eb_data_element_s)*2);

     QString tempData = ui->plTextChPSH->toPlainText();
     data[1] =  tempData.toFloat();

     if((data[1] < 0) || (data[1] > 1000))
     {
       QMessageBox::warning(this, "Error set current", "Value must be in reange from 0 to 1000 mA");
       return;
     }

     data[1] =  data[1] /1000;
     data[0] = setpointCurrPSH[0];

     data_elements_p[0].value_p = &data[0];
     data_elements_p[1].value_p = &data[1];

     data_point_id = GET_SET_I_SETPOINT_HEATERS;
     dp_write.data_point_id = data_point_id;
     dp_write.array_length = 2;
     dp_write.type = EB_TYPE_FLOAT;
     dp_write.elements_p = data_elements_p;
     eb_send_multi_write_request(&dp_write, 1, &transaction_id, &eb_write_data_response_handler, NULL);
     ReadData();
}


void MainWindow::on_pushButton_setmain0_clicked()
{
    float data[2] = {0};
    struct eb_write_data_point_info_s dp_write = {0};
    struct eb_data_element_s* data_elements_p = (eb_data_element_s*)malloc(sizeof(struct eb_data_element_s)*2);

    ui->plTextMainPSH->setPlainText("0");
     data[0] = 0;
     data[1] = setpointCurrPSH[1];

     data_elements_p[0].value_p = &data[0];
     data_elements_p[1].value_p = &data[1];

     data_point_id = GET_SET_I_SETPOINT_HEATERS;
     dp_write.data_point_id = data_point_id;
     dp_write.array_length = 2;
     dp_write.type = EB_TYPE_FLOAT;
     dp_write.elements_p = data_elements_p;
     eb_send_multi_write_request(&dp_write, 1, &transaction_id, &eb_write_data_response_handler, NULL);
     ReadData();
}


void MainWindow::on_pushButton_setmain_clicked()
{

    float data[2] = {0};


     QString tempData = ui->plTextMainPSH->toPlainText();
     data[0] = tempData.toFloat();

     if((data[0] < 0) || (data[0] > 1000))
     {
       QMessageBox::warning(this, "Error set current", "Value must be in reange from 0 to 1000 mA");
       return;
     }

     struct eb_write_data_point_info_s dp_write = {0};
     struct eb_data_element_s* data_elements_p = (eb_data_element_s*)malloc(sizeof(struct eb_data_element_s)*2);

     data[0] = data[0]/ 1000; //(tempData.toFloat() / 1000);
     data[1] = setpointCurrPSH[1];

     data_elements_p[0].value_p = &data[0];
     data_elements_p[1].value_p = &data[1];

     data_point_id = GET_SET_I_SETPOINT_HEATERS;
     dp_write.data_point_id = data_point_id;
     dp_write.array_length = 2;
     dp_write.type = EB_TYPE_FLOAT;
     dp_write.elements_p = data_elements_p;
     eb_send_multi_write_request(&dp_write, 1, &transaction_id, &eb_write_data_response_handler, NULL);
     ReadData();
}





void MainWindow::on_pushButton_SetFAN_0_clicked()
{
    double data;
    struct eb_write_data_point_info_s dp_write = {0};
    struct eb_data_element_s* data_elements_p = (eb_data_element_s*)malloc(sizeof(struct eb_data_element_s)*1);

     data_elements_p[0].value_p = &data;
     QString tempData = 0;//ui->plainTextSetFAN->toPlainText();
     data =  tempData.toDouble();
     data_point_id = GET_SET_FAN_PWM;
     dp_write.data_point_id = data_point_id;
     dp_write.array_length = 0;
     dp_write.type = EB_TYPE_DOUBLE;
     dp_write.elements_p = data_elements_p;
     eb_send_multi_write_request(&dp_write, 1, &transaction_id, &eb_write_data_response_handler, NULL);
     ReadData();
}




void MainWindow::ClearTable (void)
{

    tempStr = "";
    ui->txtIm0->setPlainText(tempStr);
    ui->txtIm1->setPlainText(tempStr);
    ui->txtIm2->setPlainText(tempStr);
    ui->txtIm3->setPlainText(tempStr);
    ui->txtIm4->setPlainText(tempStr);
    ui->txtIm5->setPlainText(tempStr);
    ui->txtIm6->setPlainText(tempStr);
    ui->txtIm7->setPlainText(tempStr);

    ui->txtUa0->setPlainText(tempStr);
    ui->txtUa1->setPlainText(tempStr);
    ui->txtUa2->setPlainText(tempStr);
    ui->txtUa3->setPlainText(tempStr);
    ui->txtUa4->setPlainText(tempStr);
    ui->txtUa5->setPlainText(tempStr);
    ui->txtUa6->setPlainText(tempStr);
    ui->txtUa7->setPlainText(tempStr);

    ui->txtUb0->setPlainText(tempStr);
    ui->txtUb1->setPlainText(tempStr);
    ui->txtUb2->setPlainText(tempStr);
    ui->txtUb3->setPlainText(tempStr);
    ui->txtUb4->setPlainText(tempStr);
    ui->txtUb5->setPlainText(tempStr);
    ui->txtUb6->setPlainText(tempStr);
    ui->txtUb7->setPlainText(tempStr);

    ui->txtUab0->setPlainText(tempStr);
    ui->txtUab1->setPlainText(tempStr);
    ui->txtUab2->setPlainText(tempStr);
    ui->txtUab3->setPlainText(tempStr);
    ui->txtUab4->setPlainText(tempStr);
    ui->txtUab5->setPlainText(tempStr);
    ui->txtUab6->setPlainText(tempStr);
    ui->txtUab7->setPlainText(tempStr);

    ui->txtTa0->setPlainText(tempStr);
    ui->txtTa1->setPlainText(tempStr);
    ui->txtTa2->setPlainText(tempStr);
    ui->txtTa3->setPlainText(tempStr);
    ui->txtTa4->setPlainText(tempStr);
    ui->txtTa5->setPlainText(tempStr);
    ui->txtTa6->setPlainText(tempStr);
    ui->txtTa7->setPlainText(tempStr);

    ui->txtTb0->setPlainText(tempStr);
    ui->txtTb1->setPlainText(tempStr);
    ui->txtTb2->setPlainText(tempStr);
    ui->txtTb3->setPlainText(tempStr);
    ui->txtTb4->setPlainText(tempStr);
    ui->txtTb5->setPlainText(tempStr);
    ui->txtTb6->setPlainText(tempStr);
    ui->txtTb7->setPlainText(tempStr);

    ui->txtVin0->setPlainText(tempStr);
    ui->txtVin1->setPlainText(tempStr);
    ui->txtVin2->setPlainText(tempStr);
    ui->txtVin3->setPlainText(tempStr);
    ui->txtVin4->setPlainText(tempStr);
    ui->txtVin5->setPlainText(tempStr);
    ui->txtVin6->setPlainText(tempStr);
    ui->txtVin7->setPlainText(tempStr);

    ui->txtVout0->setPlainText(tempStr);
    ui->txtVout1->setPlainText(tempStr);
    ui->txtVout2->setPlainText(tempStr);
    ui->txtVout3->setPlainText(tempStr);
    ui->txtVout4->setPlainText(tempStr);
    ui->txtVout5->setPlainText(tempStr);
    ui->txtVout6->setPlainText(tempStr);
    ui->txtVout7->setPlainText(tempStr);

    ui->txtIin0->setPlainText(tempStr);
    ui->txtIin1->setPlainText(tempStr);
    ui->txtIin2->setPlainText(tempStr);
    ui->txtIin3->setPlainText(tempStr);
    ui->txtIin4->setPlainText(tempStr);
    ui->txtIin5->setPlainText(tempStr);
    ui->txtIin6->setPlainText(tempStr);
    ui->txtIin7->setPlainText(tempStr);

    ui->txtIout0->setPlainText(tempStr);
    ui->txtIout1->setPlainText(tempStr);
    ui->txtIout2->setPlainText(tempStr);
    ui->txtIout3->setPlainText(tempStr);
    ui->txtIout4->setPlainText(tempStr);
    ui->txtIout5->setPlainText(tempStr);
    ui->txtIout6->setPlainText(tempStr);
    ui->txtIout7->setPlainText(tempStr);

    ui->txtTemp0->setPlainText(tempStr);
    ui->txtTemp1->setPlainText(tempStr);
    ui->txtTemp2->setPlainText(tempStr);
    ui->txtTemp3->setPlainText(tempStr);
    ui->txtTemp4->setPlainText(tempStr);
    ui->txtTemp5->setPlainText(tempStr);
    ui->txtTemp6->setPlainText(tempStr);
    ui->txtTemp7->setPlainText(tempStr);
}




void MainWindow::on_btSetSHIM_Tab_clicked()
{
    mode = MODE_SHIM;

    ui->btPSH_ShimSetCurrent0 ->  setDisabled(true);
    ui->btPSH_ShimSetCurrent  ->  setDisabled(true);
    ui->pushButton_ON -> setDisabled(true);
    ui->stackedWidget->setCurrentIndex(1);
}



void MainWindow::on_btSetMain_Tab_clicked()
{
    mode = MODE_RAMPUP;

    ui->stackedWidget->setCurrentIndex(2);

    uint32_t data =3;
    struct eb_write_data_point_info_s dp_write = {0};
    struct eb_data_element_s* data_elements_p = (eb_data_element_s*)malloc(sizeof(struct eb_data_element_s)*1);

    data_elements_p[0].value_p = &data;
    data_point_id = GET_SET_MODE;
    dp_write.data_point_id = data_point_id;
    dp_write.array_length = 0;
    dp_write.type = EB_TYPE_UINT32;
    dp_write.elements_p = data_elements_p;
    eb_send_multi_write_request(&dp_write, 1, &transaction_id, &eb_write_data_response_handler, NULL);
    pshModeRampUP = 3;
    tempStr.setNum((setpointCurrPSH[0] * 1000), 'f', 0);
    ui->plTextMainPSH->setPlainText(tempStr);

    tempStr.setNum((setpointCurrPSH[1] * 1000), 'f', 0);
    ui->plTextChPSH->setPlainText(tempStr);
}


void MainWindow::on_btSetFAN_MainTab_clicked()
{
    emit transmit_to_nng(CMD_SET_FAN_MAIN_TAB);
}


void MainWindow::on_btSetFAN0_MainTab_clicked()
{
    emit transmit_to_nng(CMD_SET_FAN0_MAIN_TAB);
}


void MainWindow::on_btSetFAN_ShimTab_clicked()
{
    emit transmit_to_nng(CMD_SET_FAN_SHIM_TAB);
}


void MainWindow::on_btSetFAN0_ShimTab_clicked()
{
    emit transmit_to_nng(CMD_SET_FAN0_SHIM_TAB);
}


void MainWindow::on_btPSH_ShimSetCurrent0_clicked()
{
    emit transmit_to_nng(CMD_SET_SHIM_PSH_CURRENT0);
}


void MainWindow::on_btPSH_ShimSetCurrent_clicked()
{
    QString tempData = ui->pTextEditCurrent_SHIM->toPlainText();
    dataArray[1] =  tempData.toFloat();


    if((dataArray[1] < 0) || (dataArray[1] > 1000))
    {
      QMessageBox::warning(this, "Error set current", "Value must be in reange from 0 to 1000 mA");
      return;
    }

    dataArray[1] =   dataArray[1]/1000;
    dataArray[0] = 0;

    emit transmit_to_nng(CMD_SET_SHIM_PSH_CURRENT);
}

