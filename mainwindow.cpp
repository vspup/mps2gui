#define SENT_ZERO   1
#include "mainwindow.h"
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
#include <termios.h>
#include <unistd.h>
#include <getopt.h>
#include <ctype.h>
#include <nng/nng.h>
#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/reqrep0/req.h>
#include "electabuzz_client.h"
#include "electabuzz_common.h"
#include "uart-escape.h"
#include <QFile>
#include <QTextStream>
//-------------------------------------------------------------
static bool prepare_nng(const char* url);
void eb_read_data_response_handler(const struct eb_read_data_point_result_s* read_result_p, void* parameter_p);
int ReadData (void);
void eb_write_data_response_handler(const struct eb_write_data_point_result_s* write_result_p, void* parameter_p);
//-------------------------------------------------------------

bool response_complete = false;
bool verbose = true;//false;
bool use_nng = true;//false;
uint16_t transaction_id = 0;
static int32_t buffer_cnt = 0;
int32_t data_point_id = 0x1000;
int serial_port_fd = -1;
nng_socket nng_sock;
    //eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
    //eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
//-r tcp://192.168.3.4:5555 -i 0x1000
char dataBuff[256];
int connectionStatus = 0;
eb_data_id_t data_id;

#define  GET_VOLTAGE         0x1202

#define  GET_CHANNEL         0x2002
#define  GET_SET_CURRENT     0x1200
#define  GET_ON_OFF_STATUS   0x2000
#define  SET_ON_OFF_STATUS   0x2001
#define  SET_VOLTAGE         0x1202





#define  GET_CURRENT         0x1000
#define  GET_VA              0x3101
#define  GET_VB              0x3102
#define  GET_TEMP_A          0x3104
#define  GET_TEMP_B          0x3105

#define  GET_VIN             0x0201
#define  GET_IIN             0x0203
#define  GET_VOUT            0x0202
#define  GET_IOUT            0x0204
#define  GET_BCM_TEMP        0x0205

//ramp up cmd
#define  GET_TERMINAL_VOLTAGE    0x1002
#define  GET_I_MAIN_REF          0x1103

#define  GET_LINE_VOLTAGE        0x1003
#define  GET_MAIN_VOLTAGE        0x1004

#define  SET_I_MAIN              0x1100
#define  SET_U_MAIN              0x1101

#define  GET_RAMP_UP_STATUS        0x2000
#define  SET_RAMP_DOWN_STATUS      0x2001

#define  GET_SET_FAN_PWM         0x0002
#define  GET_M_FAN_SPEED         0x0003

#define  GET_SET_MODE                 0x0102
#define  GET_SET_CURRENT_HEATERS      0x0101
#define  GET_SET_I_SETPOINT_HEATERS   0x0100

double    SetCurrentData[6]  = {0};
double    ReadCurrentData[8] = {0};
double    ReadVA[8] ={0};
double    ReadVB[8] ={0};

float     ReadVIN[8] ={0};
float     ReadIIN[8] ={0};
float     ReadVOUT[8] ={0};
float     ReadIOUT[8] ={0};

uint32_t  channelVal;
double    getVoltage = 0;
uint32_t  on_off_status = 0;
uint32_t  rampUp_status =0;

double    pwmFAN     = 0;
double    tempA[8]   = {0};
double    tempB[8]   = {0};
float     BCMtemp[8] = {0};

float terminalVoltage;
float mainCueerntRef;
float lineVoltage;
float mainVoltage;

double fan_speed = 0;

float currentPSH[2] ={0};
float setpointCurrPSH[2] = {0};
uint8_t mode  = 0;
uint8_t pshModeRampUP =0;
uint8_t pshModeSHIM =0;

#define MODE_SHIM        1
#define MODE_RAMPUP      2
#define MODE_RAMPDOWN    3

uint8_t channel = 0;
#define CHANNEL_AX       1
#define CHANNEL_T1       2
#define CHANNEL_T2       3

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

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

    //ui->pTextIPaddr->setPlainText("192.168.8.106");
    ui->Settings->show();
    ui->SHIM->setDisabled(true);
    ui->RampUP->setDisabled(true);
    ui->RampDOWN->setDisabled(true);
    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(slotTimerAlarm()));
    timer->start(500);
}

MainWindow::~MainWindow()
{
    delete ui;
}
uint8_t exeMode;
void MainWindow::slotTimerAlarm()
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
       else if(exeMode == MODE_RAMPUP)
       {

       }
    }
}

QString tempStr;
QByteArray tempVal;
void MainWindow::updateGeneralGUI(void)
{

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


    data_id = GET_CURRENT;
    eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
    ReadData();
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

   /* for(uint8_t n =1; n<8; n++)
    {
        tempStr.setNum(ReadCurrentData[n], 'f', 5);
        ui->pltextIm->insertPlainText(tempStr + '\n');
    }*/



    data_id = GET_VA;
    eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
    ReadData();
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

    data_id = GET_VB;
    eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
    ReadData();
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

    data_id = GET_TEMP_A;
    eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
    ReadData();
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


    data_id = GET_TEMP_B;
    eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
    ReadData();
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



    data_id = GET_VIN;
    eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
    ReadData();
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




    data_id = GET_VOUT;
    eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
    ReadData();
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


    data_id = GET_IIN;
    eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
    ReadData();
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




    data_id = GET_IOUT;
    eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
    ReadData();
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




    data_id = GET_BCM_TEMP;
    eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
    ReadData();

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

    data_id = GET_SET_FAN_PWM;
    eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
    ReadData();
    tempStr.setNum(pwmFAN, 'f', 1);
    ui->lbFAN_value->setText("FAN VALUE: " + tempStr);

}

void MainWindow::updateRampUpGUI(void)
{
    data_id = GET_TERMINAL_VOLTAGE;
    eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
    ReadData();
    tempStr.setNum(terminalVoltage, 'f', 4);
    ui->pltextVrampUP->setPlainText(tempStr);

    double current = 0;
    for(int i =0; i < 8; i++)
    {
       current += ReadCurrentData[i];
    }
    tempStr.setNum(current, 'f', 4);
    ui->pltextIrampUP->setPlainText(tempStr);

    data_id = GET_LINE_VOLTAGE;
    eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
    ReadData();

    tempStr.setNum(lineVoltage, 'f', 9);
    ui->pltextVlead->setPlainText(tempStr);

    data_id = GET_MAIN_VOLTAGE;
    eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
    ReadData();

    tempStr.setNum(mainVoltage, 'f', 9);
    ui->pltextVmagnet->setPlainText(tempStr);

    data_id = GET_RAMP_UP_STATUS;
    eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
    ReadData();
    if(rampUp_status)
    {
      ui->pushButton_OnOffRampUp->setText("OFF");
      ui->lbRampUp_status->setText("Status:ON");
    }
    else
    {
      ui->lbRampUp_status->setText("Status:OFF");
      ui->pushButton_OnOffRampUp->setText("ON");
    }

    if(pshModeRampUP)
    {
       data_id = GET_SET_CURRENT_HEATERS;
       eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
       ReadData();
       tempStr.setNum((currentPSH[0] * 1000), 'f', 4);
       ui->lbReadMain->setText(tempStr);
       tempStr.setNum((currentPSH[1] * 1000), 'f', 4);
       ui->lbReadCh->setText(tempStr);
    }

    data_id = GET_SET_I_SETPOINT_HEATERS;
    eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
    ReadData();
}

void MainWindow::updateGUI(void)
{
    char tempBuff[64] = {0};
    //tempStr = ui->pTextIPaddr->toPlainText();
    //QByteArray ba=tempStr.toLatin1();

    memset(dataBuff, 0x00, sizeof(dataBuff));
    data_id = GET_CURRENT;
    eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
    ReadData();
if(channel == CHANNEL_AX)
{
    tempStr.setNum(ReadCurrentData[1], 'f', 4);
    ui->pTextCurrCH1_get->setPlainText(tempStr);
    tempStr.setNum(ReadCurrentData[4], 'f', 4);
    ui->pTextCurrCH2_get->setPlainText(tempStr);
    tempStr.setNum(ReadCurrentData[3], 'f', 4);
    ui->pTextCurrCH3_get->setPlainText(tempStr);
    tempStr.setNum(ReadCurrentData[2], 'f', 4);
    ui->pTextCurrCH4_get->setPlainText(tempStr);
    tempStr.setNum(ReadCurrentData[0], 'f', 4);
    ui->pTextCurrCH5_get->setPlainText(tempStr);
    tempStr.setNum(ReadCurrentData[5], 'f', 4);
    ui->pTextCurrCH6_get->setPlainText(tempStr);
}
else
{
    tempStr.setNum(ReadCurrentData[0], 'f', 4);
    ui->pTextCurrCH1_get->setPlainText(tempStr);
    tempStr.setNum(ReadCurrentData[1], 'f', 4);
    ui->pTextCurrCH2_get->setPlainText(tempStr);
    tempStr.setNum(ReadCurrentData[3], 'f', 4);
    ui->pTextCurrCH3_get->setPlainText(tempStr);
    tempStr.setNum(ReadCurrentData[4], 'f', 4);
    ui->pTextCurrCH4_get->setPlainText(tempStr);
    tempStr.setNum(ReadCurrentData[2], 'f', 4);
    ui->pTextCurrCH5_get->setPlainText(tempStr);
    tempStr.setNum(ReadCurrentData[5], 'f', 4);
    ui->pTextCurrCH6_get->setPlainText(tempStr);
}


    data_id = GET_CHANNEL;
    memset(dataBuff, 0x00, sizeof(dataBuff));
    eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
    ReadData();

    if(pshModeSHIM)
    {
        switch (channelVal)
        {
            case 0: tempStr = "AX MODE";  break;
            case 1: tempStr = "T1 MODE";  break;
            case 2: tempStr = "T2 MODE";  break;
            default:tempStr = "ERROR MODE";break;
        }

        ui->pTextCH1_status->setPlainText(tempStr);
        ui->label_Mode_SHIM->setText(tempStr);
    }



    data_id = GET_SET_CURRENT;
    eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
    ReadData();

    data_id = GET_VOLTAGE;
    eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
    ReadData();

    tempStr.setNum(getVoltage);
    ui->pTextV_get->setPlainText(tempStr);

    data_id = GET_ON_OFF_STATUS;
    eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
    ReadData();

    if(on_off_status)
    {
        ui->pTextON_OFF->setPlainText("ON");
    }
    else
    {
       ui->pTextON_OFF->setPlainText("OFF");
    }

    if(pshModeSHIM)
    {
       data_id = GET_SET_CURRENT_HEATERS;
       eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
       ReadData();
       //tempStr.setNum(currentPSH[0], 'f', 4);
       //ui->lbReadMain->setText(tempStr);
       tempStr.setNum((currentPSH[1]*1000), 'f', 4);
       ui->plabel_Current_SHIM->setText(tempStr);
    }

    data_id = GET_SET_I_SETPOINT_HEATERS;
    eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
    ReadData();

}

void MainWindow::on_pushButton_Conn_clicked()
{
   if(connectionStatus)
   {
     nng_close(nng_sock);
     connectionStatus = 0;
     ui->pushButton_Conn->setText("CONNECT");
     return;
   }

   char tempBuff[64] = {0};
   QString tempStr =ui->comboBox->currentText();
   //QString tempStr = ui->pTextIPaddr->toPlainText();
   QByteArray ba=tempStr.toLatin1();
   sprintf((char*)&tempBuff[0], "tcp://%s:5555", ba.data()); //"tcp://" + ui->pTextIPaddr->toPlainText() + ":5555";

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
       ui->pushButton_Conn->setText("DISCONNECT");
       connectionStatus = 1;
   }
   else
   {
       connectionStatus = 0;
   }
}


void MainWindow::on_pushButtonAX_clicked()
{
    uint32_t data =0;
    struct eb_write_data_point_info_s dp_write = {0};
    struct eb_data_element_s* data_elements_p = (eb_data_element_s*)malloc(sizeof(struct eb_data_element_s)*1);

     data_elements_p[0].value_p = &data;
     data_point_id = GET_CHANNEL;
     dp_write.data_point_id = data_point_id;
     dp_write.array_length = 0;
     dp_write.type = EB_TYPE_UINT32;
     dp_write.elements_p = data_elements_p;
     eb_send_multi_write_request(&dp_write, 1, &transaction_id, &eb_write_data_response_handler, NULL);
     channel = CHANNEL_AX;
     ui->lnCh1->setText("AX1");
     ui->lnCh2->setText("AX2");
     ui->lnCh3->setText("AX3");
     ui->lnCh4->setText("AX4");
     ui->lnCh5->setText("AX5");
     ui->lnCh6->setText("AX6");

     //ui->pushButton_T1->setDisabled(true);
     //ui->pushButton_T2->setDisabled(true);
     pshModeSHIM = 3;
     ui->pushButton_ON->setEnabled(true);
     ui->pushButton_OFF->setEnabled(true);
     ui->pButt_setT2_SHIM->setEnabled(true);
     ui->pButton_setT2_SHIM->setEnabled(true);
}


void MainWindow::on_pushButton_T1_clicked()
{
    uint32_t data = 1;
    struct eb_write_data_point_info_s dp_write = {0};
    struct eb_data_element_s* data_elements_p = (eb_data_element_s*)malloc(sizeof(struct eb_data_element_s)*1);

     data_elements_p[0].value_p = &data;
     data_point_id = GET_CHANNEL;
     dp_write.data_point_id = data_point_id;
     dp_write.array_length = 0;
     dp_write.type = EB_TYPE_UINT32;
     dp_write.elements_p = data_elements_p;
     eb_send_multi_write_request(&dp_write, 1, &transaction_id, &eb_write_data_response_handler, NULL);

     channel = CHANNEL_T1;
     ui->lnCh1->setText("T1 1");
     ui->lnCh2->setText("T1 2");
     ui->lnCh3->setText("T1 3");
     ui->lnCh4->setText("T1 4");
     ui->lnCh5->setText("T1 5");
     ui->lnCh6->setText("T1 6");

     pshModeSHIM = 1;
     ui->pushButton_ON->setEnabled(true);
     ui->pushButton_OFF->setEnabled(true);
     ui->pButt_setT2_SHIM->setEnabled(true);
     ui->pButton_setT2_SHIM->setEnabled(true);

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
     ui->pushButton_OFF->setEnabled(true);
     ui->pButt_setT2_SHIM->setEnabled(true);
     ui->pButton_setT2_SHIM->setEnabled(true);
}


void MainWindow::on_pushButton_SetV_clicked()
{
    double data;
    struct eb_write_data_point_info_s dp_write = {0};
    struct eb_data_element_s* data_elements_p = (eb_data_element_s*)malloc(sizeof(struct eb_data_element_s)*1);

     data_elements_p[0].value_p = &data;
     QString tempData = ui->pTextV_set->toPlainText();
     data =  tempData.toDouble();
     data_point_id = SET_VOLTAGE;
     dp_write.data_point_id = data_point_id;
     dp_write.array_length = 0;
     dp_write.type = EB_TYPE_DOUBLE;
     dp_write.elements_p = data_elements_p;
     eb_send_multi_write_request(&dp_write, 1, &transaction_id, &eb_write_data_response_handler, NULL);

}


void MainWindow::on_pushButton_SetV_0_clicked()
{
    double data;
    struct eb_write_data_point_info_s dp_write = {0};
    struct eb_data_element_s* data_elements_p = (eb_data_element_s*)malloc(sizeof(struct eb_data_element_s)*1);

     ui->pTextV_set->setPlainText("0.1");
     data_elements_p[0].value_p = &data;
     data =  0.1;
     data_point_id = SET_VOLTAGE;
     dp_write.data_point_id = data_point_id;
     dp_write.array_length = 0;
     dp_write.type = EB_TYPE_DOUBLE;
     dp_write.elements_p = data_elements_p;
     eb_send_multi_write_request(&dp_write, 1, &transaction_id, &eb_write_data_response_handler, NULL);
     ReadData();

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


void MainWindow::on_pushButton_OFF_clicked()
{
     uint32_t data;
     struct eb_write_data_point_info_s dp_write = {0};
     struct eb_data_element_s* data_elements_p = (eb_data_element_s*)malloc(sizeof(struct eb_data_element_s)*2);

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


void MainWindow::on_pushButton_SetCurr_clicked()
{

    double data[6] = {0};//{0.17, 0.18, 0.19, 0.20, 0.21, 0.22};
    struct eb_write_data_point_info_s dp_write = {0};
    struct eb_data_element_s* data_elements_p = (eb_data_element_s*)malloc(sizeof(struct eb_data_element_s)*6);

     QString tempData = ui->pTextCurrCH1_set->toPlainText();
     data[0] =  tempData.toDouble();

     if(channel == CHANNEL_AX)
     {
         tempData = ui->pTextCurrCH1_set->toPlainText();
         data[1] =  tempData.toDouble();
         tempData = ui->pTextCurrCH2_set->toPlainText();
         data[4] =  tempData.toDouble();
         tempData = ui->pTextCurrCH3_set->toPlainText();
         data[3] =  tempData.toDouble();
         tempData = ui->pTextCurrCH4_set->toPlainText();
         data[2] =  tempData.toDouble();
         tempData = ui->pTextCurrCH5_set->toPlainText();
         data[0] =  tempData.toDouble();
         tempData = ui->pTextCurrCH6_set->toPlainText();
         data[5] =  tempData.toDouble();
     }
     else
     {
         tempData = ui->pTextCurrCH1_set->toPlainText();
         data[0] =  tempData.toDouble();
         tempData = ui->pTextCurrCH2_set->toPlainText();
         data[1] =  tempData.toDouble();
         tempData = ui->pTextCurrCH3_set->toPlainText();
         data[3] =  tempData.toDouble();
         tempData = ui->pTextCurrCH4_set->toPlainText();
         data[4] =  tempData.toDouble();
         tempData = ui->pTextCurrCH5_set->toPlainText();
         data[2] =  tempData.toDouble();
         tempData = ui->pTextCurrCH6_set->toPlainText();
         data[5] =  tempData.toDouble();
     }


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
     //QString tempData = ui->pTextV_set->toPlainText();
     //tempData.toDouble();


     data_point_id = GET_SET_CURRENT;
     dp_write.data_point_id = data_point_id;
     dp_write.array_length = 6;
     dp_write.type = EB_TYPE_DOUBLE;
     dp_write.elements_p = data_elements_p;
     eb_send_multi_write_request(&dp_write, 1, &transaction_id, &eb_write_data_response_handler, NULL);
     ReadData();
     //updateGUI();
}



void MainWindow::on_pushButton_clicked()
{

}


void MainWindow::on_pushButton_SetV_2_clicked()
{
    double data;
    struct eb_write_data_point_info_s dp_write = {0};
    struct eb_data_element_s* data_elements_p = (eb_data_element_s*)malloc(sizeof(struct eb_data_element_s)*1);

     data_elements_p[0].value_p = &data;
     QString tempData = ui->plainTextSetFAN->toPlainText();
     data =  tempData.toDouble();
     data_point_id = GET_SET_FAN_PWM;//SET_VOLTAGE;
     dp_write.data_point_id = data_point_id;
     dp_write.array_length = 0;
     dp_write.type = EB_TYPE_DOUBLE;
     dp_write.elements_p = data_elements_p;
     eb_send_multi_write_request(&dp_write, 1, &transaction_id, &eb_write_data_response_handler, NULL);
     ReadData();
     //updateGUI();
}

int ReadData (void)
{
    int rv = -1;
    uint8_t* buffer_p;
    size_t  buffer_size = 0;
    response_complete = false;

    do
    {
       if ((rv = nng_recv(nng_sock, &buffer_p, &buffer_size, NNG_FLAG_ALLOC)) != 0)
       {
          printf("%s: nng_recv failed: %s", __func__, nng_strerror(rv));
              return -1;
            }
           eb_client_process_incoming(buffer_p, buffer_size);
            nng_free(buffer_p, buffer_size);

            // TODO: Timeout
       } while (!response_complete);
    return 1;
}



// implementation of sever-side low-level callbacks
uint8_t* eb_allocate_buffer(uint16_t length)
{
    uint8_t* p = static_cast<uint8_t *>(malloc(length));
    if (p != NULL) {
        buffer_cnt++;
    }
    if (verbose) {
        printf("%s: allocated buffer at %p, count is now %" PRIi32"\n", __func__, p, buffer_cnt);
    }
    return p;
}


void eb_free_packet_buffer(uint8_t* buffer_p)
{
    if (buffer_p != NULL) {
        buffer_cnt--;
        if (verbose) {
            printf("freeing buffer at %p, count is now %" PRIi32"\n", buffer_p, buffer_cnt);
        }
        free(buffer_p);
    }
}

void eb_client_transmit_buffer(uint8_t* buffer_p, size_t length)
{
    if (verbose) {
        printf("sending buffer with %zu bytes to server:\n", length);
        for(size_t i=0; i<length; i++) {
            printf("0x%02x ", (unsigned int)buffer_p[i]);
            if (i == 7) {
                printf("| ");
            }
        }
        printf("\n");
    }

    if (use_nng) {
        int rv = -1;
        if ((rv = nng_send(nng_sock, buffer_p, length, 0)) != 0) {
            printf("%s: nng_send failed: %s", __func__, nng_strerror(rv));
            return;
        }

    } else {
        // use serial port to transfer the data
        const size_t tx_buffer_len = 2*length+2;
        uint8_t * tx_buffer_p = static_cast<uint8_t *>(malloc(tx_buffer_len));
        if (tx_buffer_p == NULL) {
            // No memory to do uart escaping
            return;
        }

        // encode all bytes in the buffer. This directly puts them into the new buffer
        ssize_t encoded_len = uart_escape_encode(buffer_p, length, tx_buffer_p, tx_buffer_len);

        // messaged encoded (and hence copied to a new buffer), old buffer can be used again
        eb_free_packet_buffer(buffer_p);

        if (verbose) {
            printf("%s: uart encoded message: ", __func__);
            for (ssize_t i=0; i<encoded_len; i++) {
                    printf("0x%02x ", (unsigned int)tx_buffer_p[i]);
            }
            printf("\n");
        }

        if (encoded_len > 0) {
            int wlen = write(serial_port_fd, tx_buffer_p, encoded_len);
            if (wlen != encoded_len) {
                printf("%s: Error from write: %d, %d\n", __func__, wlen, errno);
            }
            //tcdrain(serial_port_fd);    /* delay for output */
        }
        free(tx_buffer_p);
    }
}


static const char* get_result_str(eb_result_t result)
{
    switch(result) {
        case EB_OK: return "EB_OK";
        case EB_ERR_NOT_FOUND: return "EB_ERR_NOT_FOUND";
        case EB_ERR_OTHER: return "EB_ERR_OTHER";
        case EB_ERR_NOT_UNIQUE: return "EB_ERR_NOT_UNIQUE";
        case EB_ERR_NOT_IMPLEMENTED: return "EB_ERR_NOT_IMPLEMENTED";
        case EB_ERR_WRONG_PARAMETER: return "EB_ERR_WRONG_PARAMETER";
        case EB_ERR_NO_MEMORY: return "EB_ERR_NO_MEMORY";
        case EB_ERR_INTERNAL_ERR: return "EB_ERR_INTERNAL_ERR";
        case EB_ERR_MSG_FORMAT: return "EB_ERR_MSG_FORMAT";
        case EB_ERR_OVERFLOW: return "EB_ERR_OVERFLOW";
        case EB_ERR_TYPE: return "EB_ERR_TYPE";
        default: return "unkown err code";
    }
}


static const char* get_type_str(enum eb_data_type_e data_type)
{
    switch(data_type) {
        case EB_TYPE_NIL: return "NIL";
        case EB_TYPE_BOOL: return "BOOL";
        case EB_TYPE_UINT8: return "UINT8";
        case EB_TYPE_INT8: return "INT8";
        case EB_TYPE_UINT16: return "UINT16";
        case EB_TYPE_INT16: return "INT16";
        case EB_TYPE_UINT32: return "UINT32";
        case EB_TYPE_INT32: return "INT32";
        case EB_TYPE_UINT64: return "UINT64";
        case EB_TYPE_INT64: return "INT64";
        case EB_TYPE_FLOAT: return "FLOAT";
        case EB_TYPE_DOUBLE: return "DOUBLE";
        case EB_TYPE_STR: return "STR";
        case EB_TYPE_BIN: return "BIN";
        case EB_TYPE_UNKOWN: return "UNKOWN";
        default: return "unkown type code";
    }
}



void eb_read_data_response_handler(const struct eb_read_data_point_result_s* read_result_p, void* parameter_p)
{
    if (verbose) {
        printf("Client: Read Response Handler:\n");
        printf("  transaction id: 0x%04x\n", (unsigned int)read_result_p->transaction_id);
        printf("  data point id: 0x%04x\n", (unsigned int)read_result_p->data_point_id);
        printf("  result code: 0x%04x (%s)\n", (unsigned int)read_result_p->result_code, get_result_str(read_result_p->result_code));
        printf("  value length: %u\n", (unsigned int)read_result_p->value_len);
        printf("  data type: 0x%02x (%s)\n", (unsigned int)read_result_p->data_type, get_type_str(read_result_p->data_type));
        printf("  number of elements: %u\n", (unsigned int)read_result_p->num_elements);
        printf("  element index: %u\n", (unsigned int)read_result_p->element_index);
        printf("  value: ");
    }


    /*int len = strlen(dataBuff);
    sprintf(&dataBuff[len], "element index: %u\n", (unsigned int)read_result_p->element_index);
    len = strlen(dataBuff);
    sprintf(&dataBuff[len], "value: %f\n", *((double*)(read_result_p->value_p)));*/


    if(read_result_p->data_point_id == GET_SET_CURRENT )
    {
        SetCurrentData[read_result_p->element_index]  = *((double*)read_result_p->value_p);
    }
    else if(read_result_p->data_point_id == GET_CHANNEL)
    {
        channelVal = *((uint32_t*)read_result_p->value_p);
    }
    else if(read_result_p->data_point_id == GET_VOLTAGE)
    {
        getVoltage = *((double*)read_result_p->value_p);
    }
    else if(read_result_p->data_point_id == GET_ON_OFF_STATUS)
    {
        on_off_status = *((uint32_t*)read_result_p->value_p);
        rampUp_status = *((uint32_t*)read_result_p->value_p);
    }
    else if(read_result_p->data_point_id == GET_SET_FAN_PWM)
    {
        pwmFAN = *((double*)read_result_p->value_p);
    }
    else if(read_result_p->data_point_id == GET_CURRENT)
    {
        ReadCurrentData[read_result_p->element_index] = *((double*)read_result_p->value_p);
    }
    else if(read_result_p->data_point_id == GET_VA)
    {
        ReadVA[read_result_p->element_index] = *((double*)read_result_p->value_p);
    }
    else if(read_result_p->data_point_id == GET_VB)
    {
        ReadVB[read_result_p->element_index] = *((double*)read_result_p->value_p);
    }
    else if(read_result_p->data_point_id == GET_TEMP_A)
    {
        tempA[read_result_p->element_index] = *((double*)read_result_p->value_p);
    }
    else if(read_result_p->data_point_id == GET_TEMP_B)
    {
        tempB[read_result_p->element_index] = *((double*)read_result_p->value_p);
    }
    else if(read_result_p->data_point_id == GET_VIN)
    {
        ReadVIN[read_result_p->element_index] = *((float*)read_result_p->value_p);
    }
    else if(read_result_p->data_point_id == GET_IIN)
    {
        ReadIIN[read_result_p->element_index] = *((float*)read_result_p->value_p);
    }
    else if(read_result_p->data_point_id == GET_VOUT)
    {
        ReadVOUT[read_result_p->element_index] = *((float*)read_result_p->value_p);
    }
    else if(read_result_p->data_point_id == GET_IOUT)
    {
        ReadIOUT[read_result_p->element_index] = *((float*)read_result_p->value_p);
    }
    else if(read_result_p->data_point_id == GET_BCM_TEMP)
    {
        BCMtemp[read_result_p->element_index] = *((float*)(read_result_p->value_p));
    }
    else if(read_result_p->data_point_id == GET_TERMINAL_VOLTAGE)
    {
        terminalVoltage = *((double*)read_result_p->value_p);
    }
    else if(read_result_p->data_point_id == GET_I_MAIN_REF)
    {
        mainCueerntRef = *((double*)read_result_p->value_p);
    }
    else if(read_result_p->data_point_id == GET_LINE_VOLTAGE)
    {
        lineVoltage = *((double*)read_result_p->value_p);
    }
    else if(read_result_p->data_point_id == GET_MAIN_VOLTAGE)
    {
        mainVoltage = *((double*)read_result_p->value_p);
    }
    else if(read_result_p->data_point_id == GET_M_FAN_SPEED)
    {
        fan_speed = *((double*)read_result_p->value_p);
    }
    else if(read_result_p->data_point_id ==  GET_SET_CURRENT_HEATERS)
    {
        currentPSH[read_result_p->element_index] = *((float*)(read_result_p->value_p));
    }
    else if(read_result_p->data_point_id == GET_SET_I_SETPOINT_HEATERS)
    {
        setpointCurrPSH[read_result_p->element_index] = *((float*)(read_result_p->value_p));
    }
  /*  else if(read_result_p->data_point_id == GET_RAMP_UP_STATUS)
    {
        rampUp_status = *((uint32_t*)read_result_p->value_p);
    }*/


    if (read_result_p->result_code != EB_OK) {
        printf("Error: x%04x (%s)\n", (unsigned int)read_result_p->result_code, get_result_str(read_result_p->result_code));
    } else {
        switch (read_result_p->data_type) {
            case EB_TYPE_NIL: printf("NIL\n"); break;
            case EB_TYPE_BOOL: if (*((bool*)(read_result_p->value_p))) printf("true\n"); else printf("false\n"); break;
            case EB_TYPE_UINT8: printf("%" PRIu8"\n", *((uint8_t*)(read_result_p->value_p))); break;
            case EB_TYPE_INT8: printf("%" PRIi8"\n", *((int8_t*)(read_result_p->value_p))); break;
            case EB_TYPE_UINT16: printf("%" PRIu16"\n", *((uint16_t*)(read_result_p->value_p))); break;
            case EB_TYPE_INT16: printf("%" PRIi16"\n", *((int16_t*)(read_result_p->value_p))); break;
            case EB_TYPE_UINT32: printf("%" PRIu32"\n", *((uint32_t*)(read_result_p->value_p))); break;
            case EB_TYPE_INT32: printf("%" PRIi32"\n", *((int32_t*)(read_result_p->value_p))); break;
            case EB_TYPE_UINT64: printf("%" PRIu64"\n", *((uint64_t*)(read_result_p->value_p))); break;
            case EB_TYPE_INT64: printf("%" PRIi64"\n", *((int64_t*)(read_result_p->value_p))); break;
            case EB_TYPE_FLOAT: printf("%f\n", *((float*)(read_result_p->value_p))); break;
            case EB_TYPE_DOUBLE: printf("%f\n", *((double*)(read_result_p->value_p))); break;
            case EB_TYPE_STR: {
                // cave: strings are not null-terminated
                char* v_p = (char*)read_result_p->value_p;
                for (size_t i=0; i<read_result_p->value_len; i++) {
                    printf("%c", v_p[i]);
                }
                printf("\n");
                break;
            }
            case EB_TYPE_BIN: {
                uint8_t* v_p = (uint8_t*)read_result_p->value_p;
                for (size_t i=0; i<read_result_p->value_len; i++) {
                    printf("%02" PRIx8" ", v_p[i]);
                }
                printf("\n");
                break;
            }
            case EB_TYPE_UNKOWN:
            default: {
                printf("can't print type UNKOWN\n");
                break;
            }
        }
    }

    if (read_result_p->transaction_id == transaction_id) {
        if (read_result_p->num_elements == 0) {
            // this is a single value, not an array, so the transfer is complete
            response_complete = true;    // signal main loop that we are done
        }
        if (read_result_p->element_index == (read_result_p->num_elements-1)) {
            // this was the last element, so we received all values
            response_complete = true;    // signal main loop that we are done
        }
    }
}



time_stamp_t eb_get_time_stamp()
{
    struct timespec tms = {0};
    timespec_get(&tms, TIME_UTC);
    time_stamp_t time_stamp = tms.tv_sec * 1000 + (tms.tv_nsec/1000000);
    return time_stamp;
}

static bool prepare_nng(const char* url)
{
    nng_dialer dialer;
    int        rv;
    size_t     sz;
    char *     buf = NULL;


    if ((rv = nng_req0_open(&nng_sock)) != 0) {
        printf("nng_req0_open: %s\n", nng_strerror(rv));
        return false;
    }

    if (verbose) {
        printf("socket created\n");
    }

    if ((rv = nng_dialer_create(&dialer, nng_sock, url)) != 0) {
        printf("nng_dialer_create: %s\n", nng_strerror(rv));
        return false;
    }

    if (verbose) {
        printf("dialer created\n");
    }

    if (strncmp(url, "zt://", 5) == 0) {
        printf("zero tier transport is not supported\n");
        return false;
    } else {
        nng_socket_set_ms(nng_sock, NNG_OPT_REQ_RESENDTIME, 500);
    }

    nng_dialer_start(dialer, NNG_FLAG_NONBLOCK);

    if (verbose) {
        printf("dialer started\n");
    }

    return true;
}



void eb_write_data_response_handler(const struct eb_write_data_point_result_s* write_result_p, void* parameter_p)
{
    if (verbose) {
        printf("Client: Write Response Handler:\n");
        printf("  transaction id: 0x%04x\n", (unsigned int)write_result_p->transaction_id);
        printf("  data point id: 0x%04x\n", (unsigned int)write_result_p->data_point_id);
        printf("  result code: 0x%04x (%s)\n", (unsigned int)write_result_p->result_code, get_result_str(write_result_p->result_code));
    } else {
        // happy programms don't talk
        if (write_result_p->result_code != EB_OK) {
            printf("Error: 0x%04x (%s)\n", (unsigned int)write_result_p->result_code, get_result_str(write_result_p->result_code));
        }
    }

    if (write_result_p->transaction_id == transaction_id) {
        response_complete = true;    // signal main loop that we are done
    }

}










void MainWindow::on_pushButton_17_clicked()
{
    mode = MODE_SHIM;
    ui->RampUP->setDisabled(true);
    ui->RampDOWN ->  setDisabled(true);
    ui->SHIM->setEnabled(true);
    ui->labelMODE->setText("MODE: SHIM");
    ui->pButt_setT2_SHIM ->  setDisabled(true);
    ui->pButton_setT2_SHIM ->  setDisabled(true);
    ui->pushButton_ON -> setDisabled(true);
    ui->pushButton_OFF->setDisabled(true);
    ui->pButt_setax0_SHIM ->setDisabled(true);
    ui->pButt_setT1_SHIM ->setDisabled(true);
    ui->pButt_setT1_SHIM_2->setDisabled(true);
}


void MainWindow::on_pushButton_18_clicked()
{
    mode = MODE_RAMPUP;
    ui->SHIM->       setDisabled(true);
    ui->RampDOWN ->  setDisabled(true);
    ui->RampUP   ->  setEnabled(true);
    ui->labelMODE->setText("MODE: RAMP UP");
    /*ui->pushButton_V_set0 ->  setDisabled(true);
    ui->pushButton_set_I_zero ->  setDisabled(true);
    ui->pushButton_V_set    ->  setDisabled(true);
    ui->pushButton_setI     ->  setDisabled(true);
    ui->pushButton_PlusV    ->  setDisabled(true);
    ui->pushButton_pllusI   ->  setDisabled(true);
    ui->pushButton_minusV   ->  setDisabled(true);
    ui->pushButton_minusI   ->  setDisabled(true);*/
    ui->pushButton_setmain0 ->  setDisabled(true);
    ui->pushButton_setmain  ->  setDisabled(true);
    ui->pushButton_setT2_0  ->  setDisabled(true);
    ui->pushButton_setT2    ->  setDisabled(true);
}


void MainWindow::on_pushButton_19_clicked()
{
    mode = MODE_RAMPDOWN;
    ui->SHIM->       setDisabled(true);
    ui->RampDOWN ->  setDisabled(true);
    ui->RampUP   ->  setEnabled(true);
    ui->labelMODE->setText("MODE: RAMP DOWN");
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
    struct eb_write_data_point_info_s dp_write = {0};
    struct eb_data_element_s* data_elements_p = (eb_data_element_s*)malloc(sizeof(struct eb_data_element_s)*1);

     data_elements_p[0].value_p = &data;
     QString tempData = ui->pltextSetV->toPlainText();
     data =  tempData.toDouble();
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
    struct eb_write_data_point_info_s dp_write = {0};
    struct eb_data_element_s* data_elements_p = (eb_data_element_s*)malloc(sizeof(struct eb_data_element_s)*1);

     data_elements_p[0].value_p = &data;
     QString tempData = ui->pltextSetI->toPlainText();
     data =  tempData.toDouble();
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

   if(rampUp_status)
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
    ui->lbPSH_ModeRampUP->setText("AX");
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

    ui->lbPSH_ModeRampUP->setText("T1");
    /*ui->pushButton_V_set0 ->      setEnabled(true);
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


    ui->lbPSH_ModeRampUP->setText("T2");
    /*ui->pushButton_V_set0 ->      setEnabled(true);
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
     data[1] =  (tempData.toFloat()/1000);
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
    struct eb_write_data_point_info_s dp_write = {0};
    struct eb_data_element_s* data_elements_p = (eb_data_element_s*)malloc(sizeof(struct eb_data_element_s)*2);

     QString tempData = ui->plTextMainPSH->toPlainText();
     data[0] =  (tempData.toFloat() / 1000);
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


void MainWindow::on_pButt_setax0_SHIM_clicked()
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

    ui->label_Mode_SHIM->setText("AX");
    pshModeSHIM = 3;
    ui->pButt_setT2_SHIM ->  setEnabled(true);
    ui->pButton_setT2_SHIM ->  setEnabled(true);

    tempStr.setNum((setpointCurrPSH[1] * 1000), 'f', 4);
    ui->pTextEditCurrent_SHIM->setPlainText(tempStr);
}


void MainWindow::on_pButt_setT1_SHIM_clicked()
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

    ui->label_Mode_SHIM->setText("T1");
    pshModeSHIM = 1;
    ui->pButt_setT2_SHIM ->  setEnabled(true);
    ui->pButton_setT2_SHIM ->  setEnabled(true);

    tempStr.setNum((setpointCurrPSH[1] * 1000), 'f', 4);
    ui->pTextEditCurrent_SHIM->setPlainText(tempStr);
}


void MainWindow::on_pButt_setT1_SHIM_2_clicked()
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

    ui->label_Mode_SHIM->setText("T2");
    pshModeSHIM = 1;
    ui->pButt_setT2_SHIM ->  setEnabled(true);
    ui->pButton_setT2_SHIM ->  setEnabled(true);

    tempStr.setNum((setpointCurrPSH[1] * 1000), 'f', 4);
    ui->pTextEditCurrent_SHIM->setPlainText(tempStr);
}


void MainWindow::on_pButt_setT2_SHIM_clicked()
{
    float data[2] = {0};
    struct eb_write_data_point_info_s dp_write = {0};
    struct eb_data_element_s* data_elements_p = (eb_data_element_s*)malloc(sizeof(struct eb_data_element_s)*2);

     ui->pTextEditCurrent_SHIM->setPlainText("0");
     data[1] = 0;
     data[0] = 0;

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


void MainWindow::on_pButton_setT2_SHIM_clicked()
{
    float data[2] = {0};
    struct eb_write_data_point_info_s dp_write = {0};
    struct eb_data_element_s* data_elements_p = (eb_data_element_s*)malloc(sizeof(struct eb_data_element_s)*2);

     QString tempData = ui->pTextEditCurrent_SHIM->toPlainText();
     data[1] =  (tempData.toFloat()/1000);
     data[0] = 0;


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

