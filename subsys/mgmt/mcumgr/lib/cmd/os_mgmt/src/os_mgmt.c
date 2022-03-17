/*
 * Copyright (c) 2018-2021 mcumgr authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <logging/log.h>
LOG_MODULE_REGISTER(os_mgmt, CONFIG_MGMT_OS_LOG_LEVEL);

#include <sys/util.h>
#include <assert.h>
#include <string.h>

#include "tinycbor/cbor.h"
#include "cborattr/cborattr.h"
#include "mgmt/mgmt.h"
#include "os_mgmt/os_mgmt.h"
#include "os_mgmt/os_mgmt_impl.h"
#include "os_mgmt/os_mgmt_config.h"

/**
 * Command handler: os echo
 */
#if OS_MGMT_ECHO
#if 0
static int
os_mgmt_echo(struct mgmt_ctxt *ctxt)
{
	char echo_buf[128];
	CborError err;

	const struct cbor_attr_t attrs[2] = {
		[0] = {
			.attribute = "d",
			.type = CborAttrTextStringType,
			.addr.string = echo_buf,
			.nodefault = 1,
			.len = sizeof(echo_buf),
		},
		[1] = {
			.attribute = NULL
		}
	};

	echo_buf[0] = '\0';

	err = cbor_read_object(&ctxt->it, attrs);
	if (err != 0) {
		return MGMT_ERR_EINVAL;
	}

	err |= cbor_encode_text_stringz(&ctxt->encoder, "r");
	err |= cbor_encode_text_string(&ctxt->encoder, echo_buf, strlen(echo_buf));

	if (err != 0) {
		return MGMT_ERR_ENOMEM;
	}

	return 0;
}
#else
#include "net/buf.h"
#include "mgmt/mcumgr/buf.h"
#include "echo_cmd_decode.h"
#include "echo_rsp_encode.h"

static int os_mgmt_echo(struct mgmt_ctxt *ctxt)
{
	char rsp[256] = { 0 };

	uint_fast8_t decode_err;
	uint_fast8_t encode_err;
	size_t encode_len = 0;
	int write_status;
	struct cbor_nb_reader *cnr = (struct cbor_nb_reader *)ctxt->parser.d;
	size_t cbor_size = cnr->nb->len;
	struct cbor_encoder_writer *w = ctxt->encoder.writer;
	struct echo_cmd cmd;
	size_t decode_len = sizeof(cmd);
	struct echo_rsp echo_rsp;

	/* Use net buf because other layers have been stripped (Base64/SMP header) */
	decode_err = cbor_decode_echo_cmd(cnr->nb->data, cbor_size, &cmd,
					 &decode_len);
	LOG_DBG("decode err: %d len: %u size: %u", decode_err, decode_len,
		cbor_size);
	LOG_HEXDUMP_DBG(cnr->nb->data, cbor_size, "cbor in");
	LOG_HEXDUMP_DBG(cmd.d.value, cmd.d.len, "d");

	echo_rsp.r.value = cmd.d.value;
	echo_rsp.r.len = cmd.d.len;

	encode_err = cbor_encode_echo_rsp(rsp, sizeof(rsp), &echo_rsp, &encode_len);

    LOG_DBG("Encode length %u", encode_len);
	LOG_HEXDUMP_DBG(rsp, encode_len, "cbor out");

	if (encode_err) {
		LOG_ERR("Unable to encode response");
		return MGMT_ERR_EPERUSER;
	}

	write_status = w->write(w, rsp, encode_len);
	if (write_status != 0) {
		LOG_ERR("Unable to write response %d", write_status);
		return MGMT_ERR_EPERUSER + 1;
	}

	return MGMT_ERR_EOK;
}
#endif /* ECHO TEST */
#endif

#if OS_MGMT_TASKSTAT
/**
 * Encodes a single taskstat entry.
 */
static int
os_mgmt_taskstat_encode_one(struct CborEncoder *encoder, const struct os_mgmt_task_info *task_info)
{
	CborEncoder task_map;
	CborError err;

	err = 0;
	err |= cbor_encode_text_stringz(encoder, task_info->oti_name);
	err |= cbor_encoder_create_map(encoder, &task_map, CborIndefiniteLength);
	err |= cbor_encode_text_stringz(&task_map, "prio");
	err |= cbor_encode_uint(&task_map, task_info->oti_prio);
	err |= cbor_encode_text_stringz(&task_map, "tid");
	err |= cbor_encode_uint(&task_map, task_info->oti_taskid);
	err |= cbor_encode_text_stringz(&task_map, "state");
	err |= cbor_encode_uint(&task_map, task_info->oti_state);
	err |= cbor_encode_text_stringz(&task_map, "stkuse");
	err |= cbor_encode_uint(&task_map, task_info->oti_stkusage);
	err |= cbor_encode_text_stringz(&task_map, "stksiz");
	err |= cbor_encode_uint(&task_map, task_info->oti_stksize);
#ifndef CONFIG_OS_MGMT_TASKSTAT_ONLY_SUPPORTED_STATS
	err |= cbor_encode_text_stringz(&task_map, "cswcnt");
	err |= cbor_encode_uint(&task_map, task_info->oti_cswcnt);
	err |= cbor_encode_text_stringz(&task_map, "runtime");
	err |= cbor_encode_uint(&task_map, task_info->oti_runtime);
	err |= cbor_encode_text_stringz(&task_map, "last_checkin");
	err |= cbor_encode_uint(&task_map, task_info->oti_last_checkin);
	err |= cbor_encode_text_stringz(&task_map, "next_checkin");
	err |= cbor_encode_uint(&task_map, task_info->oti_next_checkin);
#endif
	err |= cbor_encoder_close_container(encoder, &task_map);

	if (err != 0) {
		return MGMT_ERR_ENOMEM;
	}

	return 0;
}

/**
 * Command handler: os taskstat
 */
static int
os_mgmt_taskstat_read(struct mgmt_ctxt *ctxt)
{
	struct os_mgmt_task_info task_info;
	struct CborEncoder tasks_map;
	CborError err;
	int task_idx;
	int rc;

	err = 0;
	err |= cbor_encode_text_stringz(&ctxt->encoder, "tasks");
	err |= cbor_encoder_create_map(&ctxt->encoder, &tasks_map, CborIndefiniteLength);
	if (err != 0) {
		return MGMT_ERR_ENOMEM;
	}

	/* Iterate the list of tasks, encoding each. */
	for (task_idx = 0; ; task_idx++) {
		rc = os_mgmt_impl_task_info(task_idx, &task_info);
		if (rc == MGMT_ERR_ENOENT) {
			/* No more tasks to encode. */
			break;
		} else if (rc != 0) {
			return rc;
		}

		rc = os_mgmt_taskstat_encode_one(&tasks_map, &task_info);
		if (rc != 0) {
			cbor_encoder_close_container(&ctxt->encoder, &tasks_map);
			return rc;
		}
	}

	err = cbor_encoder_close_container(&ctxt->encoder, &tasks_map);
	if (err != 0) {
		return MGMT_ERR_ENOMEM;
	}

	return 0;
}
#endif

/**
 * Command handler: os reset
 */
static int
os_mgmt_reset(struct mgmt_ctxt *ctxt)
{
	return os_mgmt_impl_reset(OS_MGMT_RESET_MS);
}

static const struct mgmt_handler os_mgmt_group_handlers[] = {
#if OS_MGMT_ECHO
	[OS_MGMT_ID_ECHO] = {
		os_mgmt_echo, os_mgmt_echo, true
	},
#endif
#if OS_MGMT_TASKSTAT
	[OS_MGMT_ID_TASKSTAT] = {
		os_mgmt_taskstat_read, NULL
	},
#endif
	[OS_MGMT_ID_RESET] = {
		NULL, os_mgmt_reset
	},
};

#define OS_MGMT_GROUP_SZ ARRAY_SIZE(os_mgmt_group_handlers)

static struct mgmt_group os_mgmt_group = {
	.mg_handlers = os_mgmt_group_handlers,
	.mg_handlers_count = OS_MGMT_GROUP_SZ,
	.mg_group_id = MGMT_GROUP_ID_OS,
};


void
os_mgmt_register_group(void)
{
	mgmt_register_group(&os_mgmt_group);
}

void
os_mgmt_module_init(void)
{
	os_mgmt_register_group();
}
