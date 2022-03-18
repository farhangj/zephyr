/**
 * @file fs_mgmt_client.c
 * @brief
 *
 * Copyright (c) 2022 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <logging/log.h>
LOG_MODULE_REGISTER(fs_mgmt_client, CONFIG_MCUMGR_CLIENT_FS_LOG_LEVEL);

/**************************************************************************************************/
/* Includes                                                                                       */
/**************************************************************************************************/
#include <zephyr.h>
#include <init.h>
#include <limits.h>
#include <string.h>
#include <tinycbor/cbor.h>
#include <tinycbor/cbor_buf_writer.h>
#include <cborattr/cborattr.h>
#include <net/buf.h>

#include <mgmt/mgmt.h>
#include <mgmt/endian.h>
#include <mgmt/mcumgr/smp.h>
#include <mgmt/mcumgr/buf.h>

#include "file_download_cmd_encode.h"
#include "file_download_rsp_decode.h"
#include "file_upload_cmd_encode.h"
#include "file_upload_rsp_decode.h"
#include "error_rsp_decode.h"
#include "zcbor_mgmt.h"

#include "fs_mgmt/fs_mgmt_config.h"
#include "fs_mgmt/fs_mgmt.h"

/**************************************************************************************************/
/* Local Function Prototypes                                                                      */
/**************************************************************************************************/
static int download_rsp_handler(struct mgmt_ctxt *ctxt);
static int upload_rsp_handler(struct mgmt_ctxt *ctxt);

static void fs_mgmt_event_callback(uint8_t event, const struct mgmt_hdr *hdr, void *arg);

/**************************************************************************************************/
/* Local Constant, Macro and Type Definitions                                                     */
/**************************************************************************************************/
#define MCUMGR_OVERHEAD (CBOR_AND_OTHER_HDR + 32)

#define CONFIG_FS_CMD_TIMEOUT_SECONDS 1 /* should this be based on transport? */

#define BUILD_NETWORK_HEADER(op, len, id) SET_NETWORK_HEADER(op, len, MGMT_GROUP_ID_FS, id)

/* clang-format off */
static const struct mgmt_handler FS_MGMT_CLIENT_HANDLERS[] = {
	[FS_MGMT_ID_FILE] = {
		.mh_read = download_rsp_handler,
		.mh_write = upload_rsp_handler,
		.use_custom_cbor_encoder = true
	}
};
/* clang-format on */

static struct mgmt_on_evt_cb_entry fs_cb_entry;

/**************************************************************************************************/
/* Local Data Definitions                                                                         */
/**************************************************************************************************/
static struct mgmt_group fs_mgmt_client_group = {
	.mg_handlers = FS_MGMT_CLIENT_HANDLERS,
	.mg_handlers_count = (sizeof FS_MGMT_CLIENT_HANDLERS / sizeof FS_MGMT_CLIENT_HANDLERS[0]),
	.mg_group_id = MGMT_GROUP_ID_FS,
};

static K_MUTEX_DEFINE(fs_client);

static struct {
	struct k_sem busy;
	struct mgmt_hdr cmd_hdr;
	uint8_t cmd[CONFIG_MCUMGR_BUF_SIZE];
	int sequence;
	volatile int status;
	volatile size_t offset;
	size_t size;
	size_t chunk_size;
	const uint8_t *data;
	const char *name;
} fs_ctx;

/**************************************************************************************************/
/* Sys Init                                                                                       */
/**************************************************************************************************/
static int fs_mgmt_client_init(const struct device *device)
{
	ARG_UNUSED(device);

	mgmt_register_client_group(&fs_mgmt_client_group);

	fs_cb_entry.cb = fs_mgmt_event_callback;
	mgmt_register_evt_cb(&fs_cb_entry);

	k_sem_init(&fs_ctx.busy, 0, 1);

	return 0;
}

SYS_INIT(fs_mgmt_client_init, APPLICATION, 99);

/**************************************************************************************************/
/* Local Function Definitions                                                                     */
/**************************************************************************************************/
static int fs_send_cmd(struct zephyr_smp_transport *transport, struct mgmt_hdr *hdr,
		       const void *cbor_data)
{
	int r = zephyr_smp_tx_cmd(transport, hdr, cbor_data);
	fs_ctx.sequence = (r == 0) ? hdr->nh_seq : -1;
	return r;
}

static int download_rsp_handler(struct mgmt_ctxt *ctxt)
{
	return MGMT_ERR_NO_CLIENT;
}

static int upload_rsp_handler(struct mgmt_ctxt *ctxt)
{
	int r = MGMT_ERR_EBADSTATE;
	struct file_upload_rsp file_upload_rsp;
	struct error_rsp error_rsp;

	if (fs_ctx.status != 0) {
		return r;
	}

	if (zcbor_mgmt_decode(ctxt, cbor_decode_file_upload_rsp, &file_upload_rsp, true) != 0) {
		if (zcbor_mgmt_decode(ctxt, cbor_decode_error_rsp, &error_rsp, false) != 0) {
			r = MGMT_ERR_DECODE;
		} else {
			r = error_rsp.rc;
		}
	} else {
		LOG_DBG("rc: %d off: %u", file_upload_rsp.rc, file_upload_rsp.off);

		if ((file_upload_rsp.rc == 0) && (file_upload_rsp.off <= fs_ctx.size) &&
		    (file_upload_rsp.off == fs_ctx.offset + fs_ctx.chunk_size)) {
			fs_ctx.offset = file_upload_rsp.off;
			fs_ctx.chunk_size =
				MIN(fs_ctx.chunk_size, (fs_ctx.size - file_upload_rsp.off));
			r =  MGMT_ERR_EOK;
		} else {
			r = MGMT_ERR_OFFSET;
		}
	}

	return r;
}

static void fs_mgmt_event_callback(uint8_t event, const struct mgmt_hdr *hdr, void *arg)
{
	/* Short circuit if event is a don't care */
	if (is_cmd(hdr) || (hdr->nh_group != MGMT_GROUP_ID_FS)) {
		return;
	}

	if (event == MGMT_EVT_OP_CLIENT_DONE) {
		if (hdr->nh_seq != fs_ctx.sequence) {
			LOG_ERR("Unexpected sequence");
		}

		fs_ctx.status =
			arg ? ((struct mgmt_evt_op_cmd_done_arg *)arg)->err : MGMT_ERR_EUNKNOWN;
		k_sem_give(&fs_ctx.busy);
	}
}

static int upload_chunk(struct zephyr_smp_transport *transport)
{
	int r;
	size_t cmd_len = 0;
	/* clang-format off */
	struct file_upload_cmd file_upload_cmd = {
		.offset = fs_ctx.offset,
		.data.value = fs_ctx.data + fs_ctx.offset,
		.data.len = fs_ctx.chunk_size,
		.len = fs_ctx.size,
		.name.value = fs_ctx.name,
		.name.len = strlen(fs_ctx.name)
	};
	/* clang-format on */

	r = cbor_encode_file_upload_cmd(fs_ctx.cmd, sizeof(fs_ctx.cmd), &file_upload_cmd, &cmd_len);
	if (r != 0) {
		LOG_ERR("Unable to encode fs upload chunk %s", zcbor_get_error_string(r));
		return -EINVAL;
	}

	fs_ctx.cmd_hdr = BUILD_NETWORK_HEADER(MGMT_OP_WRITE, cmd_len, FS_MGMT_ID_FILE);
	return fs_send_cmd(transport, &fs_ctx.cmd_hdr, fs_ctx.cmd);
}

static int wait_for_response(void)
{
	return k_sem_take(&fs_ctx.busy, K_SECONDS(CONFIG_FS_CMD_TIMEOUT_SECONDS));
}

/**************************************************************************************************/
/* Global Function Definitions                                                                    */
/**************************************************************************************************/
int fs_mgmt_client_download(struct zephyr_smp_transport *transport, const char *name)
{
	/* destination buffer and size
	if (name == NULL || data == NULL || size == 0) {
		return -EINVAL;
	}
	*/

	int r;
	size_t cmd_len = 0;
	struct file_download_cmd file_download_cmd;

	r = k_mutex_lock(&fs_client, K_NO_WAIT);
	if (r < 0) {
		return r;
	}

	fs_ctx.status = 0;
	fs_ctx.offset = 0;
	fs_ctx.size = 0;
	fs_ctx.name = name;

	/* clang-format off */
	file_download_cmd = (struct file_download_cmd) {
		.offset = 0,
		.name.value = name,
		.name.len = strlen(name)
	};
	/* clang-format on */

	if (cbor_encode_file_download_cmd(fs_ctx.cmd, sizeof(fs_ctx.cmd), &file_download_cmd,
					  &cmd_len)) {
		LOG_ERR("Unable to encode %s", __func__);
		return -EINVAL;
	}

	fs_ctx.cmd_hdr = BUILD_NETWORK_HEADER(MGMT_OP_READ, cmd_len, FS_MGMT_ID_FILE);
	r = fs_send_cmd(transport, &fs_ctx.cmd_hdr, fs_ctx.cmd);

	if (r == 0) {
		if (k_sem_take(&fs_ctx.busy, K_SECONDS(CONFIG_FS_CMD_TIMEOUT_SECONDS)) != 0) {
			r = -ETIMEDOUT;
		}
	}

	k_mutex_unlock(&fs_client);
	return r;
}

int fs_mgmt_client_upload(struct zephyr_smp_transport *transport, const char *name,
			  const void *data, size_t size)
{
	int r;
	int adjusted_mtu;

#if 0
	adjusted_mtu = transport->zst_get_mtu(NULL) - MCUMGR_OVERHEAD;
	if (adjusted_mtu <= 0) {
		return -EIO;
	}
#else
	adjusted_mtu = 300;
#endif

	if (name == NULL || data == NULL || size == 0) {
		return -EINVAL;
	}

	r = k_mutex_lock(&fs_client, K_NO_WAIT);
	if (r < 0) {
		return r;
	}

	fs_ctx.status = 0;
	fs_ctx.offset = 0;
	fs_ctx.size = size;
	fs_ctx.chunk_size = MIN(size, adjusted_mtu);
	fs_ctx.name = name;
	fs_ctx.data = (uint8_t *)data;

	k_sem_reset(&fs_ctx.busy);

	do {
		r = upload_chunk(transport);

		if (r == 0) {
			if (wait_for_response() != 0) {
				r = -ETIMEDOUT;
			} else {
				r = fs_ctx.status;
			}
		}
	} while ((r == 0) && (fs_ctx.offset < fs_ctx.size));

	k_mutex_unlock(&fs_client);
	return r;
}
