#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
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
#include "inc/variables_list.h"


bool response_complete = false;
bool verbose = true;//false;
bool use_nng = true;//false;
uint16_t transaction_id = 0;
int32_t buffer_cnt = 0;
int32_t data_point_id = 0x1000;
int serial_port_fd = -1;
nng_socket nng_sock;
char dataBuff[256];
int connectionStatus = 0;
eb_data_id_t data_id;


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
uint32_t  mode_status = 0;

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
uint8_t channel = 0;
