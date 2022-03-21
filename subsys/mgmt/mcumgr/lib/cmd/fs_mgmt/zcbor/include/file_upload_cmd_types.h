/*
 * Generated using zcbor version 0.3.99
 * https://github.com/NordicSemiconductor/zcbor
 * Generated with a --default-max-qty of 0
 */

#ifndef FILE_UPLOAD_CMD_TYPES_H__
#define FILE_UPLOAD_CMD_TYPES_H__

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

struct file_upload_cmd_len {

	uint32_t len;

};



struct file_upload_cmd {

	uint32_t offset;

	struct zcbor_string data;

	struct file_upload_cmd_len len;

	uint_fast32_t len_present;

	struct zcbor_string name;

};


#endif /* FILE_UPLOAD_CMD_TYPES_H__ */
