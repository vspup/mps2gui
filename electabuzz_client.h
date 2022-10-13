#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include "cwpack.h"
#include "electabuzz_common.h"

// TO BE CONFIGURED BY IMPLEMENTATION
typedef uint32_t time_stamp_t;      // data type for time stamps. 


// END OF IMPLEMENTATION CONFIG


struct eb_read_data_point_result_s;

struct eb_write_data_point_result_s;


// function pointer for a handler called by EB when the result of a read request (or an error) was received
typedef void (*eb_read_data_response_handler_t)(const struct eb_read_data_point_result_s* read_result_p, void* parameter_p);

// function pointer for a handler called by EB when the result of a write request (or an error) was received
typedef void (*eb_write_data_response_handler_t)(const struct eb_write_data_point_result_s* write_result_p, void* parameter_p);



// data, meta data and result code of read operation on other side. Will be forwarded to upper layer logic of the requestors side for further processing
struct eb_read_data_point_result_s {
    uint16_t            transaction_id;
    eb_data_id_t        data_point_id;
    eb_result_t         result_code;
    const void*         value_p;
    size_t              value_len;      // length in bytes. Only used for str and bin types
    enum eb_data_type_e data_type;
    size_t              num_elements;
    size_t              element_index;  // incremented for each element of an array    
};

// result code of write operation on other side. Will be forwarded to upper layer logic of the requestor's (client's) side for further processing
struct eb_write_data_point_result_s {
    eb_data_id_t        data_point_id;
    eb_result_t         result_code;
    uint16_t            transaction_id;
};

// one data element. This can be one entry in an array of elements which form one data point
struct eb_data_element_s {
    const void*     value_p;    // pointer to the data
    size_t          length;     // length in bytes, only used for bin & str elements
};

// all information required to encode data point writes
struct eb_write_data_point_info_s {
    eb_data_id_t                data_point_id;
    enum eb_data_type_e         type;   // data type to be written
    size_t                      array_length;   // how many elemets (>0 means this data point is an array, 0 means it's a regular value. Note that an array with 1 element is also valid)
    struct eb_data_element_s*   elements_p;   // array with num_elements entries, all must have the same type
    // TODO: support writing only certain elements of an array?
};



// TO BE IMPLEMENTED BY HIGHER LEVEL LOGIC

// returns current time 
time_stamp_t eb_get_time_stamp();

// calculates time delta between current time and start and compares this with the given timeout in milli senconds. Returns true if time delta exceeds the timeout
bool eb_is_timeout(time_stamp_t start, uint32_t timeout_in_ms);



// TO BE IMPELEMENTED BY LOW LEVEL LOGIC

// to be implemented by external code:
// transmit a previously allocated buffer. Freeing the buffer when transmission is completed must be done by external code
void eb_client_transmit_buffer(uint8_t* buffer_p, size_t length);



// API FOR HIGHER LEVEL LOGIC

eb_result_t eb_send_read_request(const eb_data_id_t id_codes[], size_t num_id_codes, uint16_t* transaction_id_p,
                                 eb_read_data_response_handler_t response_handler, void* handler_param_p);

eb_result_t eb_send_multi_write_request(const struct eb_write_data_point_info_s writes[], size_t num_writes, uint16_t* transaction_id_p, 
                                        eb_write_data_response_handler_t response_handler, void* handler_param_p);

void eb_client_process_incoming(uint8_t* rx_buffer_p, size_t rx_length);

#ifdef __cplusplus
}
#endif
