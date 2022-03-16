/*
 * Generated using zcbor version 0.3.99
 * https://github.com/NordicSemiconductor/zcbor
 * Generated with a --default-max-qty of 0
 */

#ifndef FILE_DOWNLOAD_RSP_ENCODE_H__
#define FILE_DOWNLOAD_RSP_ENCODE_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "zcbor_encode.h"
#include "file_download_rsp_types.h"

#if DEFAULT_MAX_QTY != 0
#error "The type file was generated with a different default_max_qty than this file"
#endif


uint_fast8_t cbor_encode_file_download_rsp(
		uint8_t *payload, size_t payload_len,
		const struct file_download_rsp *input,
		size_t *payload_len_out);


#endif /* FILE_DOWNLOAD_RSP_ENCODE_H__ */
