/*
 * Generated using zcbor version 0.3.99
 * https://github.com/NordicSemiconductor/zcbor
 * Generated with a --default-max-qty of 0
 */

#ifndef FILE_UPLOAD_RSP_TYPES_H__
#define FILE_UPLOAD_RSP_TYPES_H__

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

struct file_upload_rsp {
	int32_t _file_upload_rsp_rc;
	uint32_t _file_upload_rsp_off;
};


#endif /* FILE_UPLOAD_RSP_TYPES_H__ */
