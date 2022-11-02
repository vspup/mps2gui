#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include "inc/electabuzz_client.h"
#include "inc/cwpack.h"
#include "inc/cwpack_utils.h"

#ifndef EB_MAX_OUTSTANDING_TRANSACTIONS
#define EB_MAX_OUTSTANDING_TRANSACTIONS     64
#endif

typedef eb_result_t (*eb_response_handler)(const struct eb_packet_s* response_p, void* parameter_p); 


// struct to create a list of outstanding transactions (reads / writes)
struct eb_transaction_list_entry_s {
    uint16_t                        transaction_id;
    time_stamp_t                    start_time;
    eb_read_data_response_handler_t   read_response_handler;
    eb_write_data_response_handler_t  write_response_handler;
    void*                           parameter_p;
};


struct eb_response_handlers_registers_s {
    enum eb_packet_type_e   packet_type;
    eb_response_handler     handler;
    void* parameter_p;
};




static inline void eb_write_uint16(uint8_t* buffer, uint16_t value);

//static eb_result_t eb_encode_metadata(const struct eb_packet_s* packet_p);

static uint16_t eb_get_next_transaction_id();

static eb_result_t eb_register_transaction(const struct eb_transaction_list_entry_s* new_entry_p, size_t* index_p);

static eb_result_t eb_process_mt_processing_err(const struct eb_packet_s* response_p, void* parameter_p);

static eb_result_t eb_process_ping_response_packet(const struct eb_packet_s* response_p, void* parameter_p);

static eb_result_t eb_process_read_data_response_packet(const struct eb_packet_s* response_p, void* parameter_p);

static eb_result_t eb_process_write_data_response_packet(const struct eb_packet_s* response_p, void* parameter_p);

static eb_result_t eb_process_read_desc_response_packet(const struct eb_packet_s* response_p, void* parameter_p);

static ssize_t eb_ongoing_transaction_index(uint16_t transaction_id);

static void eb_remove_onging_transaction(size_t index);

static eb_result_t eb_populte_read_result(cw_unpack_context* ctx_p, struct eb_read_data_point_result_s* read_result_p);

// list of all messages that can be parsed and handeld by this implementation
static const struct eb_response_handlers_registers_s eb_response_handlers[] = {
    {.packet_type=EB_MT_PROCESSING_ERR, .handler=&eb_process_mt_processing_err},
    {.packet_type=EB_MT_PING_RSP, .handler=&eb_process_ping_response_packet},
    {.packet_type=EB_MT_READ_DATA_RSP, .handler=&eb_process_read_data_response_packet},
    {.packet_type=EB_MT_WRITE_DATA_RSP, .handler=&eb_process_write_data_response_packet},
    {.packet_type=EB_MT_READ_DESC_RSP, .handler=&eb_process_read_desc_response_packet},
};



// list of outstanding transactions
static struct eb_transaction_list_entry_s eb_ongoing_transactions[EB_MAX_OUTSTANDING_TRANSACTIONS];
static size_t eb_number_ongoing_transactions = 0;

// TODO: how do we assign transaction numbers? If each requestor assigns them, how can requests be routed? (Using id translation?)
// if this is handeled by lower layer protocols, the registry must never execute messages out of order. (which it currently does not.)
// How do we stay synced if a message is lost (e.g. due to a CRC mismatch?)
static uint16_t eb_next_transaction_id = 0;



void eb_client_poll(void)
{
    // check for outstanding transactions that timed out
    // TODO
}


// send a request to read one ore more data points identified by their ids
eb_result_t eb_send_read_request(const eb_data_id_t id_codes[], size_t num_id_codes, uint16_t* transaction_id_p,
                                 eb_read_data_response_handler_t response_handler, void* handler_param_p)
{
    if (id_codes == NULL) {
        return EB_ERR_WRONG_PARAMETER;
    }

    // get a buffer and prepare the meta data
    struct eb_packet_s tx_packet;
    uint16_t transaction_id = eb_get_next_transaction_id(); // go to next id, even if we later fail to assemble this message
#ifdef EB_DBG_PRINT
    printf("%s: next transaction_id: %"PRIu16"\n", __func__, transaction_id);
#endif
    eb_result_t ret = eb_prepare_tx_packet(&tx_packet, transaction_id);
    
    if (ret != EB_OK) {
        return ret;
    }
    tx_packet.header.type = EB_MT_READ_DATA_REQ;
    //printf("%s: preparing pack context, payload_p=%p, packet_buffer_p=%p\n", __func__, tx_packet.payload_p, tx_packet.packet_buffer_p);
    cw_pack_context ctx = {0};
    cw_pack_context_init(&ctx, tx_packet.payload_p, tx_packet.max_payload_length, 0);
#ifdef EB_DBG_PRINT
    printf("%s: return code of cw_pack_context_init: %d\n", __func__, (int)ctx.return_code);
    fflush(stdout);
#endif
    // simply pack all id codes we want to read in the payload
    for (size_t i=0; i<num_id_codes; i++) {
        //printf("%s: packing data point id %"PRIu16"\n", __func__, id_codes[i]);
        cw_pack_unsigned(&ctx, (uint64_t)id_codes[i]);
#ifdef EB_DBG_PRINT
        printf("%s: packing of id_code %"PRIu16" returned %d\n", __func__, id_codes[i], ctx.return_code);
#endif
        fflush(stdout);
        if (ctx.return_code != CWP_RC_OK) {
            // packing failed
            printf("%s: packing failed: %d\n", __func__, (int)ctx.return_code);
            eb_free_packet_buffer(tx_packet.packet_buffer_p);
            return EB_ERR_OVERFLOW;
        }
    }
    // update length of packed data
    tx_packet.payload_length = ctx.current - ctx.start;
    
    ret = eb_encode_metadata(&tx_packet);
    if (ret != EB_OK) {
        printf("%s: eb_encode_metadata returned %d\n", __func__, (int)ret);
        eb_free_packet_buffer(tx_packet.packet_buffer_p);
        return ret;
    }
        
    // remember that we expect a response
    struct eb_transaction_list_entry_s entry = {0};
    entry.start_time = eb_get_time_stamp();
    entry.transaction_id = transaction_id;
    entry.read_response_handler = response_handler;
    entry.write_response_handler = NULL;
    entry.parameter_p = handler_param_p;
    // return transaction id to calling function
    if (transaction_id_p != NULL) {
        *transaction_id_p = transaction_id;
    }
    
    ret = eb_register_transaction(&entry, NULL);
    if (ret != EB_OK) {
        printf("%s: eb_register_transaction returned %d\n", __func__, (int)ret);
        eb_free_packet_buffer(tx_packet.packet_buffer_p);
        return ret;
    }

    // initate the transmission
#ifdef EB_DBG_PRINT
    printf("%s: starting transmission\n", __func__);
#endif
    eb_client_transmit_buffer(tx_packet.packet_buffer_p, tx_packet.payload_length + EB_HEADER_LENGTH);
        
    return ret;
}


// send write requests to one or more data points in one packet. 
// Beware: This is not atomic. It can happen that some wirte attempts are executed and others fail.
eb_result_t eb_send_multi_write_request(const struct eb_write_data_point_info_s writes[], size_t num_writes, uint16_t* transaction_id_p, 
                                        eb_write_data_response_handler_t response_handler, void* handler_param_p)
{
    if (writes == NULL) {
        return EB_ERR_WRONG_PARAMETER;
    }

    // get a buffer and prepare the meta data
    struct eb_packet_s tx_packet;
    uint16_t transaction_id = eb_get_next_transaction_id(); // go to next id, even if we later fail to assemble this message
    eb_result_t ret = eb_prepare_tx_packet(&tx_packet, transaction_id);
    
    if (ret != EB_OK) {
        return ret;
    }
    tx_packet.header.type = EB_MT_WRITE_DATA_REQ;

    cw_pack_context ctx;
    cw_pack_context_init(&ctx, tx_packet.payload_p, tx_packet.max_payload_length, 0);

    // simply pack all id codes we want to read in the payload
    for (size_t i=0; i<num_writes; i++) {
        // the message format is simple: it consists of the data piont id followed by the data to be written. This continues until the end
        cw_pack_unsigned(&ctx, writes[i].data_point_id);

        size_t num_elements = writes[i].array_length;
        if (num_elements > 0) {
            printf("%s: packing an array with %zu elements\n", __func__, num_elements);
            // write an array of elements to this data point
            cw_pack_array_size(&ctx, num_elements);
        } else {
            // we need to pack just one element. For simplicity we use the loop below
            num_elements = 1;
        }

        for (size_t element_index=0; element_index<num_elements; element_index++) {
            // abbrevation of the current element:
            struct eb_data_element_s* element_p = &(writes[i].elements_p[element_index]);
            // pack the data
            ret = eb_pack_value(&ctx, writes[i].type, element_p->value_p, element_p->length, NULL);
            if (ret != EB_OK) {
                eb_free_packet_buffer(tx_packet.packet_buffer_p);
                return ret;    
            }
        }

        if (ctx.return_code != CWP_RC_OK) {
            // packing failed
            eb_free_packet_buffer(tx_packet.packet_buffer_p);
            return EB_ERR_OVERFLOW;
        }
    }
    // update length of packed data
    tx_packet.payload_length = ctx.current - ctx.start;

    ret = eb_encode_metadata(&tx_packet);
    if (ret != EB_OK) {
        printf("%s: eb_encode_metadata returned %d\n", __func__, (int)ret);
        eb_free_packet_buffer(tx_packet.packet_buffer_p);
        return ret;
    }
        
    // remember that we expect a response
    struct eb_transaction_list_entry_s entry = {0};
    entry.start_time = eb_get_time_stamp();
    entry.transaction_id = transaction_id;
    entry.write_response_handler = response_handler;
    entry.parameter_p = handler_param_p;
    entry.read_response_handler = NULL;
    // return transaction id to calling function
    if (transaction_id_p != NULL) {
        *transaction_id_p = transaction_id;
    }
    ret = eb_register_transaction(&entry, NULL);
    if (ret != EB_OK) {
        printf("%s: eb_register_transaction returned %d\n", __func__, (int)ret);
        eb_free_packet_buffer(tx_packet.packet_buffer_p);
        return ret;
    }

    // initate the transmission
    eb_client_transmit_buffer(tx_packet.packet_buffer_p, tx_packet.payload_length + EB_HEADER_LENGTH);
        
    return ret;
}


// main entry point for packets: low level code passes a packet for processing. Buffer can be freed / destoryed when this function returns
void eb_client_process_incoming(uint8_t* rx_buffer_p, size_t rx_length)
{
    struct eb_packet_s rx_packet;
        
    if (rx_length < EB_HEADER_LENGTH) {
        // packet is too short
        return;
    }

    eb_decode_packet(rx_buffer_p, &rx_packet);
#ifdef EB_DBG_PRINT    
    eb_print_packet(&rx_packet);
#endif

    // validate payload length 
    size_t payload_length = rx_length - EB_HEADER_LENGTH;

    if (payload_length != rx_packet.payload_length) {
        // mismatch between length of received message and indicated payload length
        return;
    }

    // search the handler for the given message type
    bool found_handler = false;
    //eb_result_t result = EB_ERR_NOT_FOUND;
    if (rx_packet.header.version == EB_PROTOCOL_VERSION) {
        
        for (size_t i=0; i<(sizeof(eb_response_handlers)/sizeof(eb_response_handlers[0])); i++) {
            if (eb_response_handlers[i].packet_type == rx_packet.header.type) {
                eb_response_handlers[i].handler(&rx_packet, eb_response_handlers[i].parameter_p);
                found_handler = true;
                break;
            }
        }
    }

    if (!found_handler) {
        // TODO: well what should we do? Log it?
        printf("%s: no handler found for packet type x%04"PRIx16"\n", __func__, rx_packet.header.type);
        return;
    }    
}



static uint16_t eb_get_next_transaction_id()
{
    uint16_t next_id = eb_next_transaction_id;
    eb_next_transaction_id++;   
    return next_id;
}


// add a new ongoing transaction to the list
static eb_result_t eb_register_transaction(const struct eb_transaction_list_entry_s* new_entry_p, size_t* index_p)
{
    if (eb_number_ongoing_transactions >= EB_MAX_OUTSTANDING_TRANSACTIONS) {
        return EB_ERR_NO_MEMORY;
    }
    eb_ongoing_transactions[eb_number_ongoing_transactions] = *new_entry_p;
    if (index_p != NULL) {
        *index_p = eb_number_ongoing_transactions;
    }    
    eb_number_ongoing_transactions++;
#ifdef EB_DBG_PRINT
    printf("%s: eb_number_ongoing_transactions=%zu\n", __func__, eb_number_ongoing_transactions);
#endif
        return EB_OK;
}



static eb_result_t eb_process_mt_processing_err(const struct eb_packet_s* response_p, void* parameter_p)
{
    if (response_p->payload_length > 0) {
        cw_unpack_context ctx;
        cw_unpack_context_init(&ctx, response_p->payload_p, response_p->payload_length, 0);
        enum eb_result_codes_e srv_result = cw_unpack_next_unsigned16(&ctx);
        if (ctx.return_code == CWP_RC_OK) {
            printf("%s: received result code from server: x%04"PRIx16"\n", __func__, srv_result);
        }
        
    }
    
    return EB_OK;
}


static eb_result_t eb_process_ping_response_packet(const struct eb_packet_s* response_p, void* parameter_p)
{
    return EB_ERR_NOT_IMPLEMENTED;
}


static eb_result_t eb_process_read_data_response_packet(const struct eb_packet_s* response_p, void* parameter_p)
{
    if (response_p == NULL) {
        return EB_ERR_WRONG_PARAMETER;
    }

    // find the corresponding entry in the list of pending (ongoing) transactions
    ssize_t transaction_index = eb_ongoing_transaction_index(response_p->header.transaction_id);
    if (transaction_index < 0) {
        // TODO: log this?
        printf("%s: no transaction found\n", __func__);
        return EB_ERR_NOT_FOUND;
    }
#ifdef EB_DBG_PRINT
    printf("%s: transaction index: %zd\n", __func__, transaction_index);
#endif
    struct eb_transaction_list_entry_s transaction = eb_ongoing_transactions[transaction_index];
    eb_remove_onging_transaction(transaction_index);    

    if (transaction.read_response_handler == NULL) {
        printf("%s: read response handler is NULL\n", __func__);
        return EB_ERR_NOT_IMPLEMENTED;
    }

    cw_unpack_context ctx;
    cw_unpack_context_init(&ctx, response_p->payload_p, response_p->payload_length, 0);

    eb_result_t return_code = EB_OK;
    bool done = false;
    while (!done) {
        // first message pack element is the data point's id
        eb_data_id_t data_point_id = (eb_data_id_t)cw_unpack_next_unsigned16(&ctx);
        if (ctx.return_code == CWP_RC_END_OF_INPUT) { // check if we reached the end of the payload at a position where we expect it
            done = true;
            break;
        }
        if (ctx.return_code != CWP_RC_OK) {
            return_code = EB_ERR_MSG_FORMAT;
            done = true;
            break;
        }
#ifdef EB_DBG_PRINT        
        printf("%s: data point id: %"PRIu16"\n", __func__, data_point_id);
#endif
        // next comes the response code telling us if the read was successful or not
        eb_data_id_t server_result_code = (eb_data_id_t)cw_unpack_next_unsigned16(&ctx);
        if (ctx.return_code != CWP_RC_OK) {
            return_code = EB_ERR_MSG_FORMAT;
            done = true;
            break;
        }

        // prepare data for the callback
        struct eb_read_data_point_result_s read_result = {0};
        read_result.transaction_id = response_p->header.transaction_id;
        read_result.result_code = server_result_code;

        if (server_result_code != EB_OK) {
            // execute callback to inform higher level logic that this read failed
            transaction.read_response_handler(&read_result, transaction.parameter_p);
            continue;
        }

        // read was successful on the server side, so decode the data
        cw_unpack_next(&ctx);   // decode data item, or the array item telling us how many data items we have
        if ((ctx.return_code != CWP_RC_OK) && (ctx.return_code != CWP_RC_END_OF_INPUT)) {
            return_code = EB_ERR_MSG_FORMAT;
            done = true;
            continue;
        }
        read_result.data_point_id = data_point_id;
        read_result.data_type = EB_TYPE_UNKOWN; // default for safety reasons
        read_result.value_p = NULL;
        read_result.num_elements = 0;    // default assumption: it's not an array
        read_result.element_index = 0;
        
        if (ctx.item.type == CWP_ITEM_ARRAY) {    
            size_t num_elements = ctx.item.as.array.size;
            read_result.num_elements = num_elements;
#ifdef EB_DBG_PRINT            
            printf("%s: found an array with %zu elements\n", __func__, num_elements);
#endif
            cwpack_item_types array_data_type = CWP_NOT_AN_ITEM;
            for (size_t i=0; i<num_elements; i++) {
                cw_unpack_next(&ctx);   // decode the data item
                if ((ctx.return_code != CWP_RC_OK) && (ctx.return_code != CWP_RC_END_OF_INPUT))  {
                    return_code = EB_ERR_MSG_FORMAT;
                    done = true;
                    break;
                }
                // check that all elements of the array have the same data type
                if (i == 0) {
                    array_data_type = ctx.item.type;
                    // Note: CWP has no notion of a signed integer, number packed as positive or negative.
                    // so we cannot check between signed and unsigned here
                    if (array_data_type == CWP_ITEM_NEGATIVE_INTEGER) {
                        array_data_type = CWP_ITEM_POSITIVE_INTEGER;
                    }
                } else {
                    if (array_data_type == CWP_ITEM_POSITIVE_INTEGER) {
                        if ((ctx.item.type != CWP_ITEM_NEGATIVE_INTEGER) && (ctx.item.type != CWP_ITEM_POSITIVE_INTEGER)) {
                            return_code = EB_ERR_MSG_FORMAT;
                            done = true;
                            break;
                        }
                    } else {
                        if (ctx.item.type != array_data_type) {
                            return_code = EB_ERR_MSG_FORMAT;
                            done = true;
                            break;
                        }
                    }
                }
                eb_populte_read_result(&ctx, &read_result);
                read_result.element_index = i;
                transaction.read_response_handler(&read_result, transaction.parameter_p);
            }        
        } else {
            eb_populte_read_result(&ctx, &read_result);
            transaction.read_response_handler(&read_result, transaction.parameter_p);
        }
        
    }

    return return_code;
}


static eb_result_t eb_process_write_data_response_packet(const struct eb_packet_s* response_p, void* parameter_p)
{
    if (response_p == NULL) {
        return EB_ERR_WRONG_PARAMETER;
    }

    // find the corresponding entry in the list of pending (ongoing) transactions
    ssize_t transaction_index = eb_ongoing_transaction_index(response_p->header.transaction_id);
    if (transaction_index < 0) {
        // TODO: log this?
        printf("%s: no transaction found\n", __func__);
        return EB_ERR_NOT_FOUND;
    }
    
    struct eb_transaction_list_entry_s transaction = eb_ongoing_transactions[transaction_index];
    eb_remove_onging_transaction(transaction_index);    

    if (transaction.write_response_handler == NULL) {
        printf("%s: write_response_handler is NULL\n", __func__);
        return EB_ERR_NOT_IMPLEMENTED;
    }

    cw_unpack_context ctx;
    cw_unpack_context_init(&ctx, response_p->payload_p, response_p->payload_length, 0);

    eb_result_t return_code = EB_OK;
    bool done = false;
    while (!done) {
        // first message pack element is the data point's id
        eb_data_id_t data_point_id = (eb_data_id_t)cw_unpack_next_unsigned16(&ctx);
        if (ctx.return_code == CWP_RC_END_OF_INPUT) { // check if we reached the end of the payload at a position where we expect it
            done = true;
            break;
        }
        if (ctx.return_code != CWP_RC_OK) {
            return_code = EB_ERR_MSG_FORMAT;
            done = true;
            break;
        }
#ifdef EB_DBG_PRINT       
        printf("%s: found data point id %"PRIu16"\n", __func__, data_point_id);
#endif
        // next comes the response code telling us if the read was successful or not
        eb_data_id_t server_result_code = (eb_data_id_t)cw_unpack_next_unsigned16(&ctx);
        if (ctx.return_code != CWP_RC_OK) {
            return_code = EB_ERR_MSG_FORMAT;
            done = true;
            break;
        }
        
        // prepare data for the callback
        struct eb_write_data_point_result_s write_result = {0};
        write_result.transaction_id = response_p->header.transaction_id;
        write_result.result_code = server_result_code;
        write_result.data_point_id = data_point_id;
        // execute callback to inform higher level logic about this write
        transaction.write_response_handler(&write_result, transaction.parameter_p);
    }
    
    return return_code;
}


static eb_result_t eb_process_read_desc_response_packet(const struct eb_packet_s* response_p, void* parameter_p)
{
    return EB_ERR_NOT_IMPLEMENTED;
}


// try to find the index in the array of onging transactions which matches the given transaction_id
static ssize_t eb_ongoing_transaction_index(uint16_t transaction_id)
{
    for (size_t i=0; i<eb_number_ongoing_transactions; i++) {
        if (eb_ongoing_transactions[i].transaction_id == transaction_id) {
            return i;
        }
    }
    // could not find this id
    return -1;
}


static void eb_remove_onging_transaction(size_t index)
{
    // safety check
    if (eb_number_ongoing_transactions <= 0) {  
        return;
    }
    // move all transactions which are after this one position to the front
    eb_number_ongoing_transactions--;
    for (size_t i=index; i<eb_number_ongoing_transactions; i++) {
        eb_ongoing_transactions[i] = eb_ongoing_transactions[i+1];
    }
}


// CWP item must be unpacked before calling this function
static eb_result_t eb_populte_read_result(cw_unpack_context* ctx_p, struct eb_read_data_point_result_s* read_result_p)
{
    switch (ctx_p->item.type) {
        case CWP_ITEM_NIL: {
            read_result_p->data_type = EB_TYPE_NIL;            
            return EB_OK;
        }

        case CWP_ITEM_BOOLEAN: {
            read_result_p->data_type = EB_TYPE_BOOL;
            read_result_p->value_p = (void*)&ctx_p->item.as.boolean;
            return EB_OK;
        }

        case CWP_ITEM_POSITIVE_INTEGER: {
            read_result_p->data_type = EB_TYPE_UINT64;
            read_result_p->value_p = (void*)&ctx_p->item.as.u64;
            return EB_OK;
        }

        case CWP_ITEM_NEGATIVE_INTEGER: {
            read_result_p->data_type = EB_TYPE_INT64;
            read_result_p->value_p = (void*)&ctx_p->item.as.i64;
            return EB_OK;
        }

        case CWP_ITEM_FLOAT: {
            read_result_p->data_type = EB_TYPE_FLOAT;
            read_result_p->value_p = (void*)&ctx_p->item.as.real;
            return EB_OK;
        }

        case CWP_ITEM_DOUBLE: {
            read_result_p->data_type = EB_TYPE_DOUBLE;
            read_result_p->value_p = (void*)&ctx_p->item.as.long_real;
            return EB_OK;
        }

        case CWP_ITEM_STR: {
            read_result_p->data_type = EB_TYPE_STR;
            read_result_p->value_len = ctx_p->item.as.str.length;
            read_result_p->value_p = (void*)&ctx_p->item.as.str.start;
            return EB_OK;
        }

        case CWP_ITEM_BIN: {
            read_result_p->data_type = EB_TYPE_BIN;
            read_result_p->value_len = ctx_p->item.as.bin.length;
            read_result_p->value_p = (void*)&ctx_p->item.as.bin.start;
            return EB_OK;
        }

        default: {
            return EB_ERR_TYPE;
        } 
    }
    return EB_ERR_TYPE;
}

