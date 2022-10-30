#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include "cwpack.h"

// length of the packet header in bytes
// todo: this should probably not be a gobal define
#define EB_HEADER_LENGTH    8

#define EB_REQUESTED_PAYLOAD_LENGTH     1024



#define EB_PROTOCOL_VERSION  0

typedef uint16_t eb_data_id_t;


// packet type field. encoded as uint16_t in serialized format.
enum eb_packet_type_e {
    EB_MT_PROCESSING_ERR    = 0x0000,
    EB_MT_NOT_SUPPORTED     = 0x0001,
    // 0x1xxx: requests
    EB_MT_PING_REQ          = 0x1000,
    EB_MT_READ_DATA_REQ     = 0x1001,
    EB_MT_WRITE_DATA_REQ    = 0x1002,
    EB_MT_READ_DESC_REQ     = 0x1003,
    // 0x2xxx: responses
    EB_MT_PING_RSP          = 0x2000,
    EB_MT_READ_DATA_RSP     = 0x2001,
    EB_MT_WRITE_DATA_RSP    = 0x2002,
    EB_MT_READ_DESC_RSP     = 0x2003,
};


enum eb_result_codes_e {
    EB_OK                   = 0x0000,
    EB_ERR_NOT_FOUND        = 0x0001,
    //EB_ERR_READ_ONLY        = 0x0002,
    //EB_ERR_WRITE_ONLY       = 0x0003,
    EB_ERR_OTHER            = 0x0004,
    EB_ERR_NOT_UNIQUE       = 0x0005,
    EB_ERR_NOT_IMPLEMENTED  = 0x0006,
    EB_ERR_WRONG_PARAMETER  = 0x0007,
    EB_ERR_NO_MEMORY        = 0x0008,
    EB_ERR_INTERNAL_ERR     = 0x0009,
    EB_ERR_MSG_FORMAT       = 0x000A,
    EB_ERR_OVERFLOW         = 0x000B,
    EB_ERR_TYPE             = 0x000C,
};

typedef enum eb_result_codes_e eb_result_t;

// data types which can be used for a data point. (This is a subset of what message pack can transport)
enum eb_data_type_e {
    EB_TYPE_NIL     = 0x00,
    EB_TYPE_BOOL    = 0x01,
    EB_TYPE_UINT8   = 0x02,        
    EB_TYPE_INT8    = 0x03,      
    EB_TYPE_UINT16  = 0x04,        
    EB_TYPE_INT16   = 0x05,      
    EB_TYPE_UINT32  = 0x06,        
    EB_TYPE_INT32   = 0x07,      
    EB_TYPE_UINT64  = 0x08,        
    EB_TYPE_INT64   = 0x09,      
    EB_TYPE_FLOAT   = 0x0a,
    EB_TYPE_DOUBLE  = 0x0b,
    EB_TYPE_STR     = 0x0c,
    EB_TYPE_BIN     = 0x0d,
    EB_TYPE_UNKOWN  = 0xFF,
};





// decoded packet header data. Used for easy processing. This is not a serialized format.
struct eb_packet_header_s {
    uint16_t version;
    uint16_t transaction_id;
    enum eb_packet_type_e type;
};


struct eb_packet_s {
    struct eb_packet_header_s header;
    uint16_t payload_length;
    uint8_t* payload_p;
    // the following are not transmitted. Used for internal processing in the lib only
    uint16_t max_payload_length;    
    uint8_t* packet_buffer_p; // tx / rx buffer
    //uint32_t crc;   // move one layer down?
};





// decode serialized uint16_t 
// use big endian because mesage pack also uses this
static inline uint16_t eb_read_uint16(const uint8_t* buffer)
{
    return (((uint16_t)buffer[0]) << 8) | ((uint16_t)buffer[1]);
}


static inline void eb_write_uint16(uint8_t* buffer, uint16_t value)
{
    buffer[0] = (uint8_t)((value >> 8)&0xFF);
    buffer[1] = (uint8_t)((value)&0xFF);
}


eb_result_t eb_encode_metadata(const struct eb_packet_s* packet_p);


void eb_decode_packet(const uint8_t* buffer_p, struct eb_packet_s* packet_p);


eb_result_t eb_prepare_tx_packet(struct eb_packet_s* tx_packet_p, uint16_t transaction_id);


// to be implemented by external code:
// allocate memory to hold a serialized packet. must be (at least) length bytes long
// returns a pointer to the allocated memory or NULL if no memory is available.
uint8_t* eb_allocate_buffer(uint16_t length);


// to be implemented by external code:
// free a previously allocated packet buffer.
void eb_free_packet_buffer(uint8_t* buffer_p);


void eb_print_packet(const struct eb_packet_s* packet_p);


eb_result_t eb_sizeof_type(enum eb_data_type_e type, size_t* sizeof_data_p);

eb_result_t eb_pack_value(cw_pack_context* ctx_p, enum eb_data_type_e type, const void* value_p, size_t element_length, size_t* sizeof_data_p);

eb_result_t eb_unpack_value(cw_unpack_context* ctx_p, enum eb_data_type_e type, void* value_p);

#ifdef __cplusplus
}
#endif
