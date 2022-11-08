#include "inc/shim_window.h"
#include "inc/mainwindow.h"
#include "./ui_mainwindow.h"
#include "inc/variables_list.h"
#include "inc/commands.h"
#include <QMessageBox>

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
     ui->btShimOnOff->setEnabled(true);
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
     ui->btShimOnOff->setEnabled(true);
     ui->btPSH_ShimSetCurrent->setEnabled(true);
     ui->btPSH_ShimSetCurrent0->setEnabled(true);
     emit transmit_to_nng(CMD_SET_T1);

}

void MainWindow::on_pushButton_T2_clicked()
{
     channel = CHANNEL_T2;
     ui->lnCh1->setText("T2 1");
     ui->lnCh2->setText("T2 2");
     ui->lnCh3->setText("T2 3");
     ui->lnCh4->setText("T2 4");
     ui->lnCh5->setText("T2 5");
     ui->lnCh6->setText("T2 6");
     pshModeSHIM = 2;
     ui->btShimOnOff->setEnabled(true);
     ui->btPSH_ShimSetCurrent->setEnabled(true);
     ui->btPSH_ShimSetCurrent0->setEnabled(true);

     emit transmit_to_nng(CMD_SET_T2);
}


void MainWindow::on_pushButton_SetV_0_clicked()
{
     ui->pTextV_set->setPlainText("0.1");
     emit transmit_to_nng(CMD_SET_VOLTAGE0_SHIM);
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


     /*data_point_id = GET_SET_CURRENT;
     dp_write.data_point_id = data_point_id;
     dp_write.array_length = 6;
     dp_write.type = EB_TYPE_DOUBLE;
     dp_write.elements_p = data_elements_p;
     eb_send_multi_write_request(&dp_write, 1, &transaction_id, &eb_write_data_response_handler, NULL);
     ReadData();*/

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

void MainWindow::on_btSetFAN0_MainTab_clicked()
{
    emit transmit_to_nng(CMD_SET_FAN0);
}

void MainWindow::on_btSetFAN_MainTab_clicked()
{
    emit transmit_to_nng(CMD_SET_FAN);
}

void MainWindow::on_btShimOnOff_clicked()
{
   emit transmit_to_nng(CMD_SET_ON_OFF);
}

