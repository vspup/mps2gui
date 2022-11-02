#include "inc/uart-escape.h"

#define INVERT(V) ((uint8_t)(~(V)))
enum UART_ESCAPE_STATE uart_escape_state_in;
//enum UART_ESCAPE_STATE uart_escape_state_out;

void uart_escape_reset(void){
	uart_escape_state_in = UART_ESCAPE_STATE_NORMAL;
}

void uart_escape_handle_byte(uint8_t byte);
void uart_escape_handle_invalid(uint8_t byte);
void uart_escape_handle_start(void);
void uart_escape_handle_end(void);
void uart_escape_handle_write(uint8_t byte);
void uart_escape_handle_alarm();


void uart_escape_byte_in(uint8_t byte){
	if (uart_escape_state_in == UART_ESCAPE_STATE_SEQUENCE){
		uart_escape_state_in = UART_ESCAPE_STATE_NORMAL;
		switch(byte){
			case INVERT(UART_ESCAPE_CHAR):
			case INVERT(UART_ESCAPE_SOT):
			case INVERT(UART_ESCAPE_EOT):
			case INVERT(UART_ESCAPE_ALARM):
				uart_escape_handle_byte(INVERT(byte));
			break;
			case UART_ESCAPE_ALARM:
				uart_escape_handle_alarm();
				uart_escape_state_in = UART_ESCAPE_STATE_SEQUENCE;
			break;
			default:
				uart_escape_handle_invalid(byte);
			break;
		}
	}
	else{
		switch(byte){
			case UART_ESCAPE_CHAR:
				uart_escape_state_in = UART_ESCAPE_STATE_SEQUENCE;
			break;
			case UART_ESCAPE_SOT:
				uart_escape_handle_start();
			break;
			case UART_ESCAPE_EOT:
				uart_escape_handle_end();
			break;
			case UART_ESCAPE_ALARM:
				uart_escape_handle_alarm();
			break;
			default:
				uart_escape_handle_byte(byte);
			break;
		}
	}
}


enum DECODER_RETURN uart_escape_decode_byte(enum UART_ESCAPE_STATE* state_p, uint8_t input_byte,  uint8_t* buffer, size_t* index_p, const size_t buf_size)
{
	uint8_t byte = 0;
	if (*state_p == UART_ESCAPE_STATE_SEQUENCE){
		*state_p = UART_ESCAPE_STATE_NORMAL;
		switch(input_byte){
			case INVERT(UART_ESCAPE_CHAR):
			case INVERT(UART_ESCAPE_SOT):
			case INVERT(UART_ESCAPE_EOT):
			case INVERT(UART_ESCAPE_ALARM):
				byte = INVERT(input_byte);
				break;
			case UART_ESCAPE_ALARM: // Note: The alarm character can be inserted anywhere, including during an esc sequence
				*state_p = UART_ESCAPE_STATE_SEQUENCE; // the escape seq. continues with the next char
				return UE_DECODER_ALARM;
			default:
				return UE_DECODER_INVALID;
			break;
		}
	}
	else{
		switch(input_byte){
			case UART_ESCAPE_CHAR:
				*state_p = UART_ESCAPE_STATE_SEQUENCE;
				return UE_DECODER_DATA;
			break;
			case UART_ESCAPE_SOT:
				return UE_DECODER_START;
			break;
			case UART_ESCAPE_EOT:
				return UE_DECODER_END;
			break;
			case UART_ESCAPE_ALARM:
				return UE_DECODER_ALARM;
			default:
				byte = input_byte;
			break;
		}
	}
	if (*index_p >= buf_size) {
		return UE_DECODER_BUFFER_OVERFLOW;
	}
	buffer[*index_p] = byte;
	(*index_p)++;
	return UE_DECODER_DATA;
}


/* Not working
void uart_escape_reset_encoder(void){
	uart_escape_state_out = UART_ESCAPE_STATE_NORMAL;
}

uint8_t* uart_escape_encode_byte(uint8_t *buf, size_t* len){
	if(uart_escape_state_out == UART_ESCAPE_STATE_SEQUENCE){
		uart_escape_handle_write(INVERT(*buf));
		uart_escape_state_out = UART_ESCAPE_STATE_NORMAL;
		(*len)--;
		return buf++;
	}
	else{
		switch(*buf){
			case UART_ESCAPE_CHAR:
			case UART_ESCAPE_SOT:
			case UART_ESCAPE_EOT:
			case UART_ESCAPE_ALARM:
				uart_escape_state_out = UART_ESCAPE_STATE_SEQUENCE;
				uart_escape_handle_write(UART_ESCAPE_CHAR);
				return buf;
			break;
			default:
				uart_escape_handle_write(*buf);
				(*len)--;
				return buf++;
			break;
		}
		
	}
} */

ssize_t uart_escape_encode(const uint8_t *msg, size_t msglen, uint8_t* buf, size_t buflen){
	ssize_t encodelen=1;
	if(encodelen > buflen){
		return -1;
	}
	*(buf++) = UART_ESCAPE_SOT;
	for(int i=0; i<msglen; i++){
		switch(msg[i]){
			case UART_ESCAPE_CHAR:
			case UART_ESCAPE_SOT:
			case UART_ESCAPE_EOT:
			case UART_ESCAPE_ALARM:
				encodelen += 2;
				if(encodelen > buflen){
					return -1;
				}
				(*buf++) = UART_ESCAPE_CHAR;
				(*buf++) = INVERT(msg[i]);
			break;
			default:
				if(++encodelen > buflen){
					return -1;
				}
				(*buf++) = msg[i];
			break;
			
		}
	}
	encodelen += 1;
	if(encodelen > buflen){
		return -1;
	}
	*(buf++) = UART_ESCAPE_EOT;
	
	return encodelen;
}

#pragma weak uart_escape_handle_byte
void uart_escape_handle_byte(uint8_t byte)
{
}

#pragma weak uart_escape_handle_invalid
void uart_escape_handle_invalid(uint8_t byte)
{
}

#pragma weak uart_escape_handle_start
void uart_escape_handle_start(void)
{
}

#pragma weak uart_escape_handle_end
void uart_escape_handle_end(void)
{
}

#pragma weak uart_escape_handle_alarm
void uart_escape_handle_alarm(void)
{
}


#pragma weak uart_escape_handle_write
void uart_escape_handle_write(uint8_t byte)
{
}

