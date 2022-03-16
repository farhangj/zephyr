/**
 * @file fs_mgmt_client.c
 * @brief
 *
 * Copyright (c) 2022 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <logging/log.h>
LOG_MODULE_REGISTER(fs_mgmt_client, CONFIG_FS_MGMT_CLIENT_LOG_LEVEL);

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

#include "fs_mgmt/fs_mgmt.h"

/**************************************************************************************************/
/* Global Data Definitions                                                                        */
/**************************************************************************************************/
extern struct zephyr_smp_transport smp_uart_transport;

/**************************************************************************************************/
/* Local Function Prototypes                                                                      */
/**************************************************************************************************/
static int download(struct mgmt_ctxt *ctxt);
static int upload(struct mgmt_ctxt *ctxt);

/**************************************************************************************************/
/* Local Constant, Macro and Type Definitions                                                     */
/**************************************************************************************************/
#define BUILD_NETWORK_HEADER(op, len, id) SET_NETWORK_HEADER(op, len, MGMT_GROUP_ID_FS, id)

/* clang-format off */
static const struct mgmt_handler FS_MGMT_CLIENT_HANDLERS[] = {
	[FS_MGMT_ID_FILE] = {
		.mh_write = download,
		.mh_read = upload,
		.use_custom_cbor_encoder = true
	}
};
/* clang-format on */

/**************************************************************************************************/
/* Local Data Definitions                                                                         */
/**************************************************************************************************/
static struct mgmt_group fs_mgmt_client_group = {
	.mg_handlers = FS_MGMT_CLIENT_HANDLERS,
	.mg_handlers_count = (sizeof FS_MGMT_CLIENT_HANDLERS / sizeof FS_MGMT_CLIENT_HANDLERS[0]),
	.mg_group_id = MGMT_GROUP_ID_FS,
};

/**************************************************************************************************/
/* Sys Init                                                                                       */
/**************************************************************************************************/
static int fs_mgmt_client_init(const struct device *device)
{
	ARG_UNUSED(device);

	mgmt_register_client_group(&fs_mgmt_client_group);

	return 0;
}

SYS_INIT(fs_mgmt_client_init, APPLICATION, 99);

/**************************************************************************************************/
/* Local Function Definitions                                                                     */
/**************************************************************************************************/
static void generate_cmd_sent_event(struct mgmt_hdr *nwk_hdr)
{
	struct mgmt_hdr host_hdr;

	memcpy(&host_hdr, nwk_hdr, sizeof(host_hdr));
	mgmt_ntoh_hdr(&host_hdr);
	mgmt_evt(MGMT_EVT_OP_CMD_SENT, &host_hdr, NULL);
}

static int send_cmd(struct mgmt_hdr *hdr, const void *cbor_data)
{
	int r = zephyr_smp_tx_cmd(&smp_uart_transport, hdr, cbor_data);
	if (r == 0) {
		generate_cmd_sent_event(hdr);
	}

	return (r < 0) ? r : hdr->nh_seq;
}

static int download(struct mgmt_ctxt *ctxt)
{
	return MGMT_ERR_ENOTSUP;
}

static int upload(struct mgmt_ctxt *ctxt)
{
	return MGMT_ERR_ENOTSUP;
}

/**************************************************************************************************/
/* Global Function Definitions                                                                    */
/**************************************************************************************************/
int fs_mgmt_client_download(const char *name)
{
	uint8_t cmd[128];
	size_t cmd_len = 0;
    struct file_download_cmd file_download_cmd = {
        .offset = 0,
        .name.value = name,
        .name.len = strlen(name)
    };

	if (!cbor_encode_file_download_cmd(cmd, sizeof(cmd), &file_download_cmd, &cmd_len)) {
		LOG_ERR("Unable to encode %s", __func__);
		return -EINVAL;
	}

	struct mgmt_hdr cmd_hdr = BUILD_NETWORK_HEADER(MGMT_OP_READ, cmd_len, FS_MGMT_ID_FILE);

	return send_cmd(&cmd_hdr, cmd);
}

int fs_mgmt_client_upload(const char *name, const void *data, size_t size)
{
	uint8_t cmd[128];
	size_t cmd_len = 0;
    struct file_upload_cmd file_upload_cmd = {
        .offset = 0,
        .data.value = data,
        .data.len = size,
        .len = size,
        .name.value = name,
        .name.len = strlen(name)
    };

	if (!cbor_encode_file_upload_cmd(cmd, sizeof(cmd), &file_upload_cmd, &cmd_len)) {
		LOG_ERR("Unable to encode %s", __func__);
		return -EINVAL;
	}

	struct mgmt_hdr cmd_hdr = BUILD_NETWORK_HEADER(MGMT_OP_WRITE, cmd_len, FS_MGMT_ID_FILE);
	return send_cmd(&cmd_hdr, NULL);
}
