
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>

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

extern double    SetCurrentData[6];
extern double    ReadCurrentData[8];
extern double    ReadVA[8];
extern double    ReadVB[8];

extern float     ReadVIN[8];
extern float     ReadIIN[8];
extern float     ReadVOUT[8];
extern float     ReadIOUT[8];

extern uint32_t  channelVal;
extern double    getVoltage;
extern uint32_t  mode_status;

extern double    pwmFAN;
extern double    tempA[8];
extern double    tempB[8];
extern float     BCMtemp[8];

extern float terminalVoltage;
extern float mainCueerntRef;
extern float lineVoltage;
extern float mainVoltage;

extern double fan_speed;

extern float currentPSH[2];
extern float setpointCurrPSH[2];
extern uint8_t mode;
extern uint8_t oldMode;

extern uint8_t pshModeRampUP;
extern uint8_t pshModeSHIM;
extern uint8_t channel;

extern bool response_complete;
extern bool verbose;
extern bool use_nng;
extern uint16_t transaction_id;
extern int32_t buffer_cnt;
//extern int32_t data_point_id;
extern int serial_port_fd;
extern nng_socket nng_sock;
extern char dataBuff[256];
extern int connectionStatus;
extern eb_data_id_t data_id;
extern double dataArray[6];
extern int connectionStatusLost;
extern float dataFloatArray[2];
extern int connectionAttempts;
