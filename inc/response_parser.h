#include <QString>



//extern bool prepare_nng(const char* url);
bool prepare_nng(const char* url);
void eb_read_data_response_handler(const struct eb_read_data_point_result_s* read_result_p, void* parameter_p);
int  ReadData (void);
void eb_write_data_response_handler(const struct eb_write_data_point_result_s* write_result_p, void* parameter_p);
extern QString logTransaction;
