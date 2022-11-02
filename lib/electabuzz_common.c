
#include <stdio.h>
#include <inttypes.h>
#include <stdint.h>
#include "inc/electabuzz_common.h"
#include "inc/cwpack.h"




eb_result_t eb_encode_metadata(const struct eb_packet_s* packet_p)
{
    uint8_t* buffer_p = packet_p->packet_buffer_p;
    eb_write_uint16(buffer_p, packet_p->header.version);
    eb_write_uint16(buffer_p+2, packet_p->header.transaction_id);
    eb_write_uint16(buffer_p+4, packet_p->header.type);
    eb_write_uint16(buffer_p+6, packet_p->payload_length);
    if ((buffer_p+8 != packet_p->payload_p)) {
        return EB_ERR_NOT_IMPLEMENTED;
    }
    return EB_OK;
}


void eb_decode_packet(const uint8_t* buffer_p, struct eb_packet_s* packet_p)
{
    packet_p->header.version = eb_read_uint16(buffer_p);
    packet_p->header.transaction_id = eb_read_uint16(buffer_p+2);
    packet_p->header.type = (enum eb_packet_type_e)eb_read_uint16(buffer_p+4);
    packet_p->payload_length = eb_read_uint16(buffer_p+6);
    packet_p->payload_p = (void*)(buffer_p+8);
}




eb_result_t eb_prepare_tx_packet(struct eb_packet_s* tx_packet_p, uint16_t transaction_id)
{
    uint8_t* tx_buffer_p = eb_allocate_buffer(EB_REQUESTED_PAYLOAD_LENGTH + EB_HEADER_LENGTH);

    if (tx_buffer_p == NULL) {
        // lower level system is out of memory
        return EB_ERR_NO_MEMORY;
    }

    tx_packet_p->header.version = EB_PROTOCOL_VERSION;
    tx_packet_p->header.transaction_id = transaction_id;
    tx_packet_p->payload_length = 0;
    tx_packet_p->max_payload_length = EB_REQUESTED_PAYLOAD_LENGTH;
    tx_packet_p->payload_p = tx_buffer_p + EB_HEADER_LENGTH;
    tx_packet_p->packet_buffer_p = tx_buffer_p;
    return EB_OK;
}


void eb_print_packet(const struct eb_packet_s* packet_p)
{
    printf("packet\n");
    printf("  header:\n");
    printf("    version: %"PRIu16"\n", packet_p->header.version);
    printf("    transaction id: %"PRIu16"\n", packet_p->header.transaction_id);
    printf("    type: x%04"PRIx16"\n", packet_p->header.type);
    printf("  payload length: %"PRIu16"\n", packet_p->payload_length);
    printf("  payload_p: %p\n", packet_p->payload_p);
    printf("  max payload length: %"PRIu16"\n", packet_p->max_payload_length);
    printf("  packet_buffer_p: %p\n", packet_p->packet_buffer_p);
}


eb_result_t eb_sizeof_type(enum eb_data_type_e type, size_t* sizeof_data_p)
{
    if (sizeof_data_p == NULL) {
        return EB_ERR_WRONG_PARAMETER;
    }

    switch (type) {
        case EB_TYPE_NIL:
            *sizeof_data_p = 0;
            break;
        case EB_TYPE_BOOL:
            *sizeof_data_p = sizeof(bool);
            break;
        case EB_TYPE_UINT8:
            *sizeof_data_p = sizeof(uint8_t);
            break;
        case EB_TYPE_INT8:
            *sizeof_data_p = sizeof(int8_t);
            break;
        case EB_TYPE_UINT16:
            *sizeof_data_p = sizeof(uint16_t);
            break;
        case EB_TYPE_INT16:
            *sizeof_data_p = sizeof(int16_t);
            break;
        case EB_TYPE_UINT32:
            *sizeof_data_p = sizeof(uint32_t);
            break;
        case EB_TYPE_INT32:
            *sizeof_data_p = sizeof(int32_t);
            break;
        case EB_TYPE_UINT64:
            *sizeof_data_p = sizeof(uint64_t);
            break;
        case EB_TYPE_INT64:
            *sizeof_data_p = sizeof(int64_t);
            break;
        case EB_TYPE_FLOAT:
            *sizeof_data_p = sizeof(float);
            break;    
        case EB_TYPE_DOUBLE:
            *sizeof_data_p = sizeof(double);
            break;    
        case EB_TYPE_STR:
            return EB_ERR_NOT_IMPLEMENTED;
        case EB_TYPE_BIN:
            return EB_ERR_NOT_IMPLEMENTED;
        default:
            return EB_ERR_TYPE;
    }
    return EB_OK;
}


// pack one data element (e.g. an int or a string) into a CWPack context
eb_result_t eb_pack_value(cw_pack_context* ctx_p, enum eb_data_type_e type, const void* value_p, size_t element_length, size_t* sizeof_data_p)
{
    switch (type) {
        case EB_TYPE_NIL:
            cw_pack_nil(ctx_p);
            if (sizeof_data_p != NULL) {
                *sizeof_data_p = 0;
            }
            break;
        case EB_TYPE_BOOL:
            cw_pack_boolean(ctx_p, *((const bool*)value_p));
            if (sizeof_data_p != NULL) {
                *sizeof_data_p = sizeof(bool);
            }
            break;
        case EB_TYPE_UINT8:
            cw_pack_unsigned(ctx_p, *((const uint8_t*)value_p));
            if (sizeof_data_p != NULL) {
                *sizeof_data_p = sizeof(uint8_t);
            }
            break;
        case EB_TYPE_INT8:
            cw_pack_signed(ctx_p, *((const int8_t*)value_p));
            if (sizeof_data_p != NULL) {
                *sizeof_data_p = sizeof(int8_t);
            }
            break;
        case EB_TYPE_UINT16:
            cw_pack_unsigned(ctx_p, *((const uint16_t*)value_p));
            if (sizeof_data_p != NULL) {
                *sizeof_data_p = sizeof(uint16_t);
            }
            break;
        case EB_TYPE_INT16:
            cw_pack_signed(ctx_p, *((const int16_t*)value_p));
            if (sizeof_data_p != NULL) {
                *sizeof_data_p = sizeof(int16_t);
            }
            break;
        case EB_TYPE_UINT32:
            cw_pack_unsigned(ctx_p, *((const uint32_t*)value_p));
            if (sizeof_data_p != NULL) {
                *sizeof_data_p = sizeof(uint32_t);
            }
            break;
        case EB_TYPE_INT32:
            cw_pack_signed(ctx_p, *((const int32_t*)value_p));
            if (sizeof_data_p != NULL) {
                *sizeof_data_p = sizeof(int32_t);
            }
            break;
        case EB_TYPE_UINT64:
            cw_pack_unsigned(ctx_p, *((const uint64_t*)value_p));
            if (sizeof_data_p != NULL) {
                *sizeof_data_p = sizeof(uint64_t);
            }
            break;
        case EB_TYPE_INT64:
            cw_pack_signed(ctx_p, *((const int64_t*)value_p));
            if (sizeof_data_p != NULL) {
                *sizeof_data_p = sizeof(int64_t);
            }
            break;
        case EB_TYPE_FLOAT:
            cw_pack_float(ctx_p, *((const float*)value_p));
            if (sizeof_data_p != NULL) {
                *sizeof_data_p = sizeof(float);
            }
            break;    
        case EB_TYPE_DOUBLE:
            cw_pack_double(ctx_p, *((const double*)value_p));
            if (sizeof_data_p != NULL) {
                *sizeof_data_p = sizeof(double);
            }
            break;    
        case EB_TYPE_STR:
            cw_pack_str(ctx_p, (const char*)value_p, element_length);
            if (sizeof_data_p != NULL) {
                *sizeof_data_p = sizeof(char) * element_length;
            }
            break;
        case EB_TYPE_BIN:
            cw_pack_bin(ctx_p, value_p, element_length);
            if (sizeof_data_p != NULL) {
                *sizeof_data_p = element_length;
            }
            break;    
        default:
            return EB_ERR_TYPE;
    }
    if (ctx_p->return_code != CWP_RC_OK) {
        return EB_ERR_OVERFLOW;
    }
    return EB_OK;
}


// unpack given type from cwpack and place result in *value_p. 
// STR and BIN types are not handeled
eb_result_t eb_unpack_value(cw_unpack_context* ctx_p, enum eb_data_type_e type, void* value_p)
{
    cw_unpack_next(ctx_p);
    if (ctx_p->return_code != CWP_RC_OK) {
        return EB_ERR_MSG_FORMAT;
    }

    switch(type) {
        case EB_TYPE_NIL: {
            if (ctx_p->item.type == CWP_ITEM_NIL) {
                return EB_OK;
            }
            return EB_ERR_TYPE;
        }

        case EB_TYPE_BOOL: {
            if (ctx_p->item.type != CWP_ITEM_BOOLEAN) {
                return EB_ERR_TYPE;
            }
            *((bool*)value_p) = ctx_p->item.as.boolean;
            return EB_OK;
        }

        case EB_TYPE_UINT8: {
            if (ctx_p->item.type != CWP_ITEM_POSITIVE_INTEGER) {
                return EB_ERR_TYPE;
            }
            uint64_t v = ctx_p->item.as.u64;
            if (v > UINT8_MAX) {
                return EB_ERR_TYPE;
            }
            *((uint8_t*)value_p) = v;
            return EB_OK;
        }

        case EB_TYPE_INT8: {
            if ((ctx_p->item.type != CWP_ITEM_POSITIVE_INTEGER) && (ctx_p->item.type != CWP_ITEM_NEGATIVE_INTEGER)) {
                return EB_ERR_TYPE;
            }
            int64_t v = ctx_p->item.as.i64;
            if ((v < INT8_MIN) || (v > INT8_MAX)) {
                return EB_ERR_TYPE;
            }
            *((int8_t*)value_p) = v;
            return EB_OK;
        }

        case EB_TYPE_UINT16: {
            if (ctx_p->item.type != CWP_ITEM_POSITIVE_INTEGER) {
                return EB_ERR_TYPE;
            }
            uint64_t v = ctx_p->item.as.u64;
            if (v > UINT16_MAX) {
                return EB_ERR_TYPE;
            }
            *((uint16_t*)value_p) = v;
            return EB_OK;
        }

        case EB_TYPE_INT16: {
            if ((ctx_p->item.type != CWP_ITEM_POSITIVE_INTEGER) && (ctx_p->item.type != CWP_ITEM_NEGATIVE_INTEGER)) {
                return EB_ERR_TYPE;
            }
            int64_t v = ctx_p->item.as.i64;
            if ((v < INT16_MIN) || (v > INT16_MAX)) {
                return EB_ERR_TYPE;
            }
            *((int16_t*)value_p) = v;
            return EB_OK;
        }

        case EB_TYPE_UINT32: {
            if (ctx_p->item.type != CWP_ITEM_POSITIVE_INTEGER) {
                return EB_ERR_TYPE;
            }
            uint64_t v = ctx_p->item.as.u64;
            if (v > UINT32_MAX) {
                return EB_ERR_TYPE;
            }
            *((uint32_t*)value_p) = v;
            return EB_OK;
        }

        case EB_TYPE_INT32: {
            if ((ctx_p->item.type != CWP_ITEM_POSITIVE_INTEGER) && (ctx_p->item.type != CWP_ITEM_NEGATIVE_INTEGER)) {
                return EB_ERR_TYPE;
            }
            int64_t v = ctx_p->item.as.i64;
            if ((v < INT32_MIN) || (v > INT32_MAX)) {
                return EB_ERR_TYPE;
            }
            *((int32_t*)value_p) = v;
            return EB_OK;
        }

        case EB_TYPE_UINT64: {
            if (ctx_p->item.type != CWP_ITEM_POSITIVE_INTEGER) {
                return EB_ERR_TYPE;
            }
            uint64_t v = ctx_p->item.as.u64;
            *((uint64_t*)value_p) = v;
            return EB_OK;
        }

        case EB_TYPE_INT64: {
            if ((ctx_p->item.type != CWP_ITEM_POSITIVE_INTEGER) && (ctx_p->item.type != CWP_ITEM_NEGATIVE_INTEGER)) {
                return EB_ERR_TYPE;
            }
            int64_t v = ctx_p->item.as.i64;
            *((int64_t*)value_p) = v;
            return EB_OK;
        }
        
        case EB_TYPE_FLOAT: {
            if (ctx_p->item.type != CWP_ITEM_FLOAT) {
                return EB_ERR_TYPE;
            }
            *((float*)value_p) = ctx_p->item.as.real;
            return EB_OK;    
        }

        case EB_TYPE_DOUBLE: {
            if (ctx_p->item.type != CWP_ITEM_DOUBLE) {
                return EB_ERR_TYPE;
            }
            *((double*)value_p) = ctx_p->item.as.long_real;
            return EB_OK;    
        }

        case EB_TYPE_STR:
            return EB_ERR_NOT_IMPLEMENTED;

        case EB_TYPE_BIN:
            return EB_ERR_NOT_IMPLEMENTED;

        default:
            return EB_ERR_TYPE;
    }
}
