/*
 * Generated using zcbor version 0.3.99
 * https://github.com/NordicSemiconductor/zcbor
 * Generated with a --default-max-qty of 0
 */

#ifndef FILE_DOWNLOAD_RSP_TYPES_H__
#define FILE_DOWNLOAD_RSP_TYPES_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "zcbor_encode.h"

/** Which value for --default-max-qty this file was created with.
 *
 *  The define is used in the other generated file to do a build-time
 *  compatibility check.
 *
 *  See `zcbor --help` for more information about --default-max-qty
 */
#define DEFAULT_MAX_QTY 0

struct file_download_rsp_len {

	uint32_t len;

};



struct file_download_rsp {

	uint32_t offset;

	struct zcbor_string data;

	int32_t rc;

	struct file_download_rsp_len len;

	uint_fast32_t len_present;

};


#endif /* FILE_DOWNLOAD_RSP_TYPES_H__ */