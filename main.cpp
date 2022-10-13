#include "mainwindow.h"

#include <QApplication>

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
////-------------------------------------------------------------
//static bool prepare_nng(const char* url);
//void eb_read_data_response_handler(const struct eb_read_data_point_result_s* read_result_p, void* parameter_p);
////-------------------------------------------------------------
//bool response_complete = false;
//bool verbose = true;//false;
//bool use_nng = true;//false;
//uint16_t transaction_id = 0;
//static int32_t buffer_cnt = 0;
//int32_t data_point_id = 0x1000;
//int serial_port_fd = -1;
//nng_socket nng_sock;
//    //eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
//    //eb_send_read_request(&data_id, 1, &transaction_id, &eb_read_data_response_handler, NULL);
////-r tcp://192.168.3.4:5555 -i 0x1000
//char dataBuff[256];

#include <QFile>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}

