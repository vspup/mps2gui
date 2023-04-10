
#include "inc/response_parser.h"
//#include "inc/mainwindow.h"
#include "inc/variables_list.h"
#include "inc/commands.h"
#include "inc/electabuzz_client.h"
#include "inc/electabuzz_common.h"
#include "inc/uart-escape.h"
#include  <string.h>


bool prepare_nng(const char* url);
static const char* get_type_str(enum eb_data_type_e data_type);
static const char* get_result_str(eb_result_t result);

bool prepare_nng(const char* url)
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
        nng_socket_set_ms(nng_sock, NNG_OPT_REQ_RESENDTIME, 500); //nng_socket_set_ms(nng_sock, NNG_OPT_REQ_RESENDTIME, 500);//*YS*
    }

    if((rv =nng_dialer_start(dialer,NNG_FLAG_ALLOC)) != 0)//nng_dialer_start(dialer, NNG_FLAG_NONBLOCK);
    {
        printf("nng_dialer_start_ERR: %s\n", nng_strerror(rv));
        return false;
    }

    if (verbose) {
        printf("dialer started\n");
    }

    return true;
}

void encode_register (uint16_t register_id, char *ptData)
{
    switch( register_id)
    {
      case 0x0000: sprintf (ptData, " FW Version \n"); break;
      case 0x0002: sprintf (ptData, " FAN Duty Cycle\n"); break;
      case 0x0003: sprintf (ptData, " Measured FAN Speed\n"); break;

      case 0x0100: sprintf (ptData, " Switch Heater Current Setpoint\n"); break;
      case 0x0101: sprintf (ptData, " Measured Switch Heater Current\n");break;
      case 0x0102: sprintf (ptData, " Shim Switch Heater Channel\n");break;

      case 0x0201: sprintf (ptData, " BCM VIN  Meas\n"); break;
      case 0x0202: sprintf (ptData, " BCM VOUT Meas\n"); break;
      case 0x0203: sprintf (ptData, " BCM IIN  Meas\n"); break;
      case 0x0204: sprintf (ptData, " BCM IOUT Meas\n"); break;
      case 0x0205: sprintf (ptData, " BCM Temp\n"); break;


      case 0x1002: sprintf (ptData, " Filtered Terminal Voltage\n"); break;
      case 0x1003: sprintf (ptData, " Filtered End of Line Voltage\n");break;
      case 0x1004: sprintf (ptData, " Filtered Main Voltage\n");break;

      case 0x1100: sprintf (ptData, " I Main Set\n");break;
      case 0x1101: sprintf (ptData, " U Main Set\n");break;
      case 0x1103: sprintf (ptData, " I Main Ref\n");break;
      case 0x1000: sprintf (ptData, " Filtered module currents\n"); break;
      case 0x1200: sprintf (ptData, " I Shim Set\n"); break;
      case 0x1202: sprintf (ptData, " U Shim Max\n"); break;

      case 0x2000: sprintf (ptData, " Current Operating Mode \n"); break;
      case 0x2001: sprintf (ptData, " Request Operation Mode\n"); break;
      case 0x2002: sprintf (ptData, " Request Shim Ch\n"); break;

      case 0x3101: sprintf (ptData, " Voltage at Output A\n"); break;
      case 0x3102: sprintf (ptData, " Voltage at Output B\n"); break;
      case 0x3104: sprintf (ptData, " Temperature A\n"); break;
      case 0x3105: sprintf (ptData, " Temperature B\n"); break;
    }
}
QString logTransaction;
char tempBuff[1024];
void eb_read_data_response_handler(const struct eb_read_data_point_result_s* read_result_p, void* parameter_p)
{
    QString tempStr;

    memset((char*)&tempBuff[0], 0x00, sizeof(tempBuff));
    //memset()
    if (verbose) {

          if ( (read_result_p->num_elements == 0)  || (read_result_p->element_index == 0) )
          {
            printf("Client: Read Response Handler:\n");
            printf("  transaction id: 0x%04x\n", (unsigned int)read_result_p->transaction_id);
            sprintf(tempBuff, "  tr id: %d\n", (unsigned int)read_result_p->transaction_id);
            logTransaction += tempBuff;
            printf("  data point id: 0x%04x\n", (unsigned int)read_result_p->data_point_id);
            sprintf(tempBuff, "  dp id: 0x%04x", (unsigned int)read_result_p->data_point_id);
            logTransaction += tempBuff;
            memset((char*)&tempBuff[0], 0x00, sizeof(tempBuff));
            encode_register( (uint16_t)read_result_p->data_point_id, (char*)&tempBuff[0]);
            logTransaction += tempBuff;
            printf("  result code: 0x%04x (%s)\n", (unsigned int)read_result_p->result_code, get_result_str(read_result_p->result_code));
            //sprintf(tempBuff, "  result code: 0x%04x (%s)\n", (unsigned int)read_result_p->result_code, get_result_str(read_result_p->result_code));
            //logTransaction += tempBuff;
            printf("  value length: %u\n", (unsigned int)read_result_p->value_len);


            printf("  data type: 0x%02x (%s)\n", (unsigned int)read_result_p->data_type, get_type_str(read_result_p->data_type));
            printf("  number of elements: %u\n", (unsigned int)read_result_p->num_elements);
            //sprintf(tempBuff, "  number of elements: %u\n", (unsigned int)read_result_p->num_elements);
            //logTransaction += tempBuff;
            printf("  element index: %u\n", (unsigned int)read_result_p->element_index);
            //sprintf(tempBuff, "  element index: %u\n", (unsigned int)read_result_p->element_index);
            //logTransaction += tempBuff;
          }
          printf("  value: ");
          logTransaction += ("  value: ");

    }


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
        mode_status = *((uint32_t*)read_result_p->value_p);
        //rampUp_status = *((uint32_t*)read_result_p->value_p);
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
    else if(read_result_p->data_point_id == GET_SET_MODE)
    {
       shim_heater_ch = *((uint64_t*)(read_result_p->value_p));
    }
  /*  else if(read_result_p->data_point_id == GET_RAMP_UP_STATUS)
    {
        rampUp_status = *((uint32_t*)read_result_p->value_p);
    }*/


    if (read_result_p->result_code != EB_OK) {
        printf("Error: x%04x (%s)\n", (unsigned int)read_result_p->result_code, get_result_str(read_result_p->result_code));
    } else {
        switch (read_result_p->data_type) {
            case EB_TYPE_NIL:
                printf("NIL\n");
                logTransaction += "NIL\n";
            break;
            case EB_TYPE_BOOL: if (*((bool*)(read_result_p->value_p)))
            {
                printf("true\n");
                logTransaction += "true\n";
            }
            else
            {
                printf("false\n");
                logTransaction += "false\n";
            }
            break;
            case EB_TYPE_UINT8:
                 printf("%" PRIu8"\n", *((uint8_t*)(read_result_p->value_p)));
                 tempStr.setNum( (*((uint8_t*)read_result_p->value_p)));
                 logTransaction += tempStr;
            break;
            case EB_TYPE_INT8:
                 printf("%" PRIi8"\n", *((int8_t*)(read_result_p->value_p)));
                 tempStr.setNum( (*((int8_t*)read_result_p->value_p)));
                 logTransaction += tempStr;
            break;
            case EB_TYPE_UINT16:
                 printf("%" PRIu16"\n", *((uint16_t*)(read_result_p->value_p)));
                 tempStr.setNum( (*((uint16_t*)read_result_p->value_p)));
                 logTransaction += tempStr;
            break;
            case EB_TYPE_INT16:
                 printf("%" PRIi16"\n", *((int16_t*)(read_result_p->value_p)));
                 tempStr.setNum( (*((int16_t*)read_result_p->value_p)));
                 logTransaction += tempStr;
            break;
            case EB_TYPE_UINT32:
                 printf("%" PRIu32"\n", *((uint32_t*)(read_result_p->value_p)));
                 tempStr.setNum( (*((uint32_t*)read_result_p->value_p)));
                 logTransaction += tempStr;
            break;
            case EB_TYPE_INT32:
                 printf("%" PRIi32"\n", *((int32_t*)(read_result_p->value_p)));
                 tempStr.setNum( (*((int32_t*)read_result_p->value_p)));
                 logTransaction += tempStr;
            break;
            case EB_TYPE_UINT64:
                 printf("%" PRIu64"\n", *((uint64_t*)(read_result_p->value_p)));
                 tempStr.setNum( (*((uint64_t*)read_result_p->value_p)));
                 logTransaction += tempStr;
            break;
            case EB_TYPE_INT64:
                 printf("%" PRIi64"\n", *((int64_t*)(read_result_p->value_p)));
                 tempStr.setNum( (*((int64_t*)read_result_p->value_p)));
                 logTransaction += tempStr;
            break;
            case EB_TYPE_FLOAT:
                 printf("%f\n", *((float*)(read_result_p->value_p)));
                 tempStr.setNum( (*((float*)read_result_p->value_p)));
                 logTransaction += tempStr;
            break;
            case EB_TYPE_DOUBLE:
                 printf("%f\n", *((double*)(read_result_p->value_p)));
                 tempStr.setNum( (*((double*)read_result_p->value_p)));
                 logTransaction += tempStr;
            break;
            case EB_TYPE_STR: {
                // cave: strings are not null-terminated
                char* v_p = (char*)read_result_p->value_p;
                for (size_t i=0; i<read_result_p->value_len; i++) {
                    printf("%c", v_p[i]);
                    tempStr =v_p[i] ;
                    logTransaction += tempStr;
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
void eb_write_data_response_handler(const struct eb_write_data_point_result_s* write_result_p, void* parameter_p)
{
    if (verbose) {
        printf("Client: Write Response Handler:\n");
        printf("  transaction id: 0x%04x\n", (unsigned int)write_result_p->transaction_id);
        printf("  data point id: 0x%04x\n", (unsigned int)write_result_p->data_point_id);
        printf("  result code: 0x%04x (%s)\n", (unsigned int)write_result_p->result_code, get_result_str(write_result_p->result_code));

        sprintf(tempBuff, "  tr id: %d\n", (unsigned int)write_result_p->transaction_id);
        logTransaction += tempBuff;
        sprintf(tempBuff, "  dp id: 0x%04x", (unsigned int)write_result_p->data_point_id);
        logTransaction += tempBuff;
        encode_register( (uint16_t)write_result_p->data_point_id, (char*)&tempBuff[0]);
        logTransaction += tempBuff;
        //logTransaction += ("  value: ");
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
           printf("%s: free allocated buffer at %p, size %" PRIi32"\n", __func__, buffer_p, buffer_cnt);
            // TODO: Timeout
       } while (!response_complete);
    return 1;
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







time_stamp_t eb_get_time_stamp()
{
    time_stamp_t time_stamp = 0;
#if (__unix__)
    struct timespec tms = {0};
    timespec_get(&tms, TIME_UTC);
    time_stamp = tms.tv_sec * 1000 + (tms.tv_nsec/1000000);

#else
//    SYSTEMTIME t;
//    GetSystemTime(&t); // GetLocalTime(&t)
//    time_stamp = t.wMilliseconds;
    time_t ltime;
    time(&ltime);
    //printf("Local time as unix timestamp: %li\n", ltime);
    struct tm* timeinfo = gmtime(&ltime); /* Convert to UTC */
    ltime = mktime(timeinfo); /* Store as unix timestamp */
    //printf("UTC time as unix timestamp: %li\n", ltime);
    time_stamp = ltime;
#endif
    return time_stamp;
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
            /*int wlen = write(serial_port_fd, tx_buffer_p, encoded_len);
            if (wlen != encoded_len) {
                printf("%s: Error from write: %d, %d\n", __func__, wlen, errno);
            }
            //tcdrain(serial_port_fd);    // delay for output*/
        }
        free(tx_buffer_p);
    }
}

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
