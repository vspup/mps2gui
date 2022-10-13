#ifdef __cplusplus
extern "C" {
#endif

#include "electabuzz_client.h"

eb_result_t eb_write_bool_request(eb_data_id_t data_point_id, bool value, uint16_t* transaction_id_p, 
                                        eb_write_data_response_handler_t response_handler, void* handler_param_p);

eb_result_t eb_write_uint32_request(eb_data_id_t data_point_id, uint32_t value, uint16_t* transaction_id_p, 
                                        eb_write_data_response_handler_t response_handler, void* handler_param_p);

eb_result_t eb_write_int32_request(eb_data_id_t data_point_id, int32_t value, uint16_t* transaction_id_p, 
                                        eb_write_data_response_handler_t response_handler, void* handler_param_p);

eb_result_t eb_write_float_request(eb_data_id_t data_point_id, float value, uint16_t* transaction_id_p, 
                                        eb_write_data_response_handler_t response_handler, void* handler_param_p);

eb_result_t eb_write_double_request(eb_data_id_t data_point_id, double value, uint16_t* transaction_id_p, 
                                        eb_write_data_response_handler_t response_handler, void* handler_param_p);

#ifdef __cplusplus
}
#endif
