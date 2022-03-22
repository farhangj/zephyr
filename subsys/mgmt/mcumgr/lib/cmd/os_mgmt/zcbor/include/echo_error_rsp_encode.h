/*
 * Generated using zcbor version 0.3.99
 * https://github.com/NordicSemiconductor/zcbor
 * Generated with a --default-max-qty of 0
 */

#ifndef ECHO_ERROR_RSP_ENCODE_H__
#define ECHO_ERROR_RSP_ENCODE_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "zcbor_encode.h"
#include "echo_error_rsp_types.h"

#if DEFAULT_MAX_QTY != 0
#error "The type file was generated with a different default_max_qty than this file"
#endif


uint_fast8_t cbor_encode_echo_error_rsp(
		uint8_t *payload, size_t payload_len,
		const struct echo_error_rsp *input,
		size_t *payload_len_out);


#endif /* ECHO_ERROR_RSP_ENCODE_H__ */
