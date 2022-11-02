#include <stdbool.h>
#include <string.h>
#include "inc/electabuzz_client.h"



eb_result_t eb_write_bool_request(eb_data_id_t data_point_id, bool value, uint16_t* transaction_id_p, 
                                        eb_write_data_response_handler_t response_handler, void* handler_param_p)
{
    struct eb_write_data_point_info_s write_info = {0};

    struct eb_data_element_s data = {0};
    data.value_p = (void*)&value;
    data.length = 0;    // only used for BIN & STR types

    write_info.data_point_id = data_point_id;
    write_info.type = EB_TYPE_BOOL;
    write_info.array_length = 0;    // single element write
    write_info.elements_p = &data;

    return eb_send_multi_write_request(&write_info, 1, transaction_id_p, response_handler, handler_param_p);    
}


eb_result_t eb_write_uint32_request(eb_data_id_t data_point_id, uint32_t value, uint16_t* transaction_id_p, 
                                        eb_write_data_response_handler_t response_handler, void* handler_param_p)
{
    struct eb_write_data_point_info_s write_info = {0};

    struct eb_data_element_s data = {0};
    data.value_p = (void*)&value;
    data.length = 0;    // only used for BIN & STR types

    write_info.data_point_id = data_point_id;
    write_info.type = EB_TYPE_UINT32;
    write_info.array_length = 0;    // single element write
    write_info.elements_p = &data;

    return eb_send_multi_write_request(&write_info, 1, transaction_id_p, response_handler, handler_param_p);    
}


eb_result_t eb_write_int32_request(eb_data_id_t data_point_id, int32_t value, uint16_t* transaction_id_p, 
                                        eb_write_data_response_handler_t response_handler, void* handler_param_p)
{
    struct eb_write_data_point_info_s write_info = {0};

    struct eb_data_element_s data = {0};
    data.value_p = (void*)&value;
    data.length = 0;    // only used for BIN & STR types

    write_info.data_point_id = data_point_id;
    write_info.type = EB_TYPE_INT32;
    write_info.array_length = 0;    // single element write
    write_info.elements_p = &data;

    return eb_send_multi_write_request(&write_info, 1, transaction_id_p, response_handler, handler_param_p);    
}


eb_result_t eb_write_float_request(eb_data_id_t data_point_id, float value, uint16_t* transaction_id_p, 
                                        eb_write_data_response_handler_t response_handler, void* handler_param_p)
{
    struct eb_write_data_point_info_s write_info = {0};

    struct eb_data_element_s data = {0};
    data.value_p = (void*)&value;
    data.length = 0;    // only used for BIN & STR types

    write_info.data_point_id = data_point_id;
    write_info.type = EB_TYPE_FLOAT;
    write_info.array_length = 0;    // single element write
    write_info.elements_p = &data;

    return eb_send_multi_write_request(&write_info, 1, transaction_id_p, response_handler, handler_param_p);    
}


eb_result_t eb_write_double_request(eb_data_id_t data_point_id, double value, uint16_t* transaction_id_p, 
                                        eb_write_data_response_handler_t response_handler, void* handler_param_p)
{
    struct eb_write_data_point_info_s write_info = {0};

    struct eb_data_element_s data = {0};
    data.value_p = (void*)&value;
    data.length = 0;    // only used for BIN & STR types

    write_info.data_point_id = data_point_id;
    write_info.type = EB_TYPE_DOUBLE;
    write_info.array_length = 0;    // single element write
    write_info.elements_p = &data;

    return eb_send_multi_write_request(&write_info, 1, transaction_id_p, response_handler, handler_param_p);    
}


