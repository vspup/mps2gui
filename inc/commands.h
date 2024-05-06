#define  GET_VOLTAGE                  0x1202
#define  GET_CHANNEL                  0x2002
#define  GET_SET_CURRENT              0x1200
#define  GET_ON_OFF_STATUS            0x2000
#define  SET_ON_OFF_STATUS            0x2001
#define  SET_VOLTAGE                  0x1202

#define  GET_CURRENT                  0x1000
#define  GET_VA                       0x3101
#define  GET_VB                       0x3102
#define  GET_TEMP_A                   0x3104
#define  GET_TEMP_B                   0x3105

#define  GET_VIN                      0x0201
#define  GET_IIN                      0x0203
#define  GET_VOUT                     0x0202
#define  GET_IOUT                     0x0204
#define  GET_BCM_TEMP                 0x0205


#define  GET_TERMINAL_VOLTAGE         0x1002
#define  GET_I_MAIN_REF               0x1103

#define  GET_LINE_VOLTAGE             0x1003
#define  GET_MAIN_VOLTAGE             0x1004

#define  SET_I_MAIN                   0x1100
#define  SET_U_MAIN                   0x1101

#define  GET_RAMP_UP_STATUS           0x2000
#define  SET_RAMP_DOWN_STATUS         0x2001

#define  GET_SET_FAN_PWM              0x0002
#define  GET_M_FAN_SPEED              0x0003

#define  GET_SET_MODE                 0x0102
#define  GET_SET_CURRENT_HEATERS      0x0101
#define  GET_SET_I_SETPOINT_HEATERS   0x0100

#define SET_PWR_FUSE_ON               0x3107 // bool  array (8)  //RW


#define CMD_UPDATE_BY_TIMER           1
#define CMD_SET_FAN                   2
#define CMD_SET_FAN0                  3
#define CMD_SET_AX                    4
#define CMD_SET_T1                    5
#define CMD_SET_T2                    6
#define CMD_SET_VOLTAGE_SHIM          7
#define CMD_SET_VOLTAGE0_SHIM         8
#define CMD_SET_CURRENT_SHIM          9
#define CMD_SET_CURRENT0_SHIM         10
#define CMD_SET_SHIM_PSH_CURRENT      11
#define CMD_SET_SHIM_PSH_CURRENT0     12
#define CMD_SET_ON_OFF                13
#define CMD_SET_VOLTAGE_MAIN          14
#define CMD_SET_VOLTAGE0_MAIN         15
#define CMD_SET_CURRENT_MAIN          16
#define CMD_SET_CURRENT0_MAIN         17
#define CMD_SET_PSH_MAIN_I_0          18
#define CMD_SET_PSH_MAIN_I            19
#define CMD_SET_PSH_AX_I_0            20
#define CMD_SET_PSH_AX_I              21
#define CMD_SET_PWR_FUSE_ON           22

#define MODE_SELECTING   0
#define SHIM_MODE        1
#define MAIN_MODE        2
//#define MODE_RAMPDOWN    3


#define CHANNEL_AX       1
#define CHANNEL_T1       2
#define CHANNEL_T2       3
