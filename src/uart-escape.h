#pragma once
/**
* A library implementing start and stop characters for arbitrary binary
* protocols
* 
* It encodes by going through a binary string and replacing all occurences
* of UART_ESCAPE_CHAR, UART_ESCAPE_SOT, and UART_ESCAPE_EOT with a two characters
* UART_ESCAPE_CHAR plus the bit-flipped version of the character originally in the
* binary string.
* 
* If you modify the characters, make sure that the bit-flipped version of them is not
* a reserved character by itself.
**/

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>

/* DO NOT MODIFY UNLESS YOU KNOW WHAT YOUR DOING
* 
* The bit-inverted values MUST NOT be a special character again.
*/
/** The escape character used **/
#define UART_ESCAPE_CHAR 0x42
/** The start of transmission character used **/
#define UART_ESCAPE_SOT 0x43
/** end of transmission character **/
#define UART_ESCAPE_EOT 0x44
/** alarm character, can be inserted at any position to notify the receiver of an emergency situation **/
#define UART_ESCAPE_ALARM 0x45

enum UART_ESCAPE_STATE{
	UART_ESCAPE_STATE_NORMAL = 0,
	UART_ESCAPE_STATE_SEQUENCE
};

enum DECODER_RETURN {
	UE_DECODER_DATA,
	UE_DECODER_START,
	UE_DECODER_END,
	UE_DECODER_ALARM,
	UE_DECODER_INVALID,
	UE_DECODER_BUFFER_OVERFLOW,
};


/**
* The current state of the decoder.
**/
extern enum UART_ESCAPE_STATE uart_escape_state_in;
/**
* The current state of the encoder.
**/
//extern enum UART_ESCAPE_STATE uart_escape_state_out;


/*
* PUBLIC API
*/

/**
* Reset the decoder
**/
void uart_escape_reset(void);

/**
* Reset the encoder
**/
//void uart_escape_reset_encoder(void);

/**
* Feeds a new byte to the decoder
**/
void uart_escape_byte_in(uint8_t byte);

/**
* Feeds a new byte to the decoder and stores decoded message in the buffer & increaes index
**/
enum DECODER_RETURN uart_escape_decode_byte(enum UART_ESCAPE_STATE* state_p, uint8_t input_byte, uint8_t* buffer, size_t* index_p, const size_t buf_size);

/**
* Encode a new byte from a buffer.
* 
* This api is for non-blocking byte-by-byte transfer in a
* Interrupt service routine, when TX-fifo is free.
* @bugs: Untested and unused.
**/

// Not working
//uint8_t* uart_escape_encode_byte(uint8_t *buf, size_t* len);

/**
* Escape a message to a buffer.
* 
* buflen needs to be at most 2*msglen+2
* SOT and EOT are automatically appended/prepended to buf
* 
* @param[in] msg The buffer to read a message to escape
* @param[in] msglen The length of the message contained in msg
* @param[out] buf The buffer to write the escaped string to
* @param[out] buflen The size of this buffer
* @return -1 if the escaped string exceeds buflen, the number of bytes written to buflen otherwise
**/
ssize_t uart_escape_encode(const uint8_t *msg, size_t msglen, uint8_t* buf, size_t buflen);


/*
* THE API FROM THE USER
* 
* All programs linking to this library have to implement these functions
*/

/**
* \brief Handle a byte of new data
* 
* This is called by the decoder process in uart_escape_byte_in, when
* a new byte has arrived (with all the escape sequences unescaped)
**/
void uart_escape_handle_byte(uint8_t byte);

/**
* \brief Handle an invalid escape sequence
* 
* This is called by the decoder process in uart_escape_byte_in, when
* an invalid escape sequences has been encountered.
* byte is the byte following UART_ESCAPE_CHAR
**/
void uart_escape_handle_invalid(uint8_t byte);

/**
* \brief Handle the occurence of a start byte
* 
* This is called by the decoder process in uart_escape_byte_in, when
* a UART_ESCAPE_SOT is encountered.
**/
void uart_escape_handle_start(void);

/**
* \brief Handle the occurence of an end byte
* 
* This is called by the decoder process in uart_escape_byte_in, when
* a UART_ESCAPE_EOT is encountered.
**/
void uart_escape_handle_end(void);


/**
* \brief Send out a new byte
* 
* This is called by the encoder process in uart_escape_encode_byte when
* it wants to transmit a byte
**/
void uart_escape_handle_write(uint8_t byte);

#ifdef __cplusplus
}
#endif
