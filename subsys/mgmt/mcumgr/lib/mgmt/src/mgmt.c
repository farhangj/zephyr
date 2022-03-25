/*
 * Copyright (c) 2018-2021 mcumgr authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>

#include "tinycbor/cbor.h"
#include "mgmt/endian.h"
#include "mgmt/mgmt.h"
#include <sys/atomic.h>

static atomic_t evt_users = ATOMIC_INIT(0);
static mgmt_on_evt_cb evt_cb[CONFIG_MCUMGR_EVENT_CALLBACK_MAX_USERS];

static struct mgmt_group *server_list_head;
static struct mgmt_group *server_list_end;

#ifdef CONFIG_MCUMGR_CLIENT
static atomic_t mgmt_sequence = ATOMIC_INIT(0);

static struct mgmt_group *client_list_head;
static struct mgmt_group *client_list_end;
#endif


void *
mgmt_streamer_alloc_rsp(struct mgmt_streamer *streamer, const void *req)
{
	return streamer->cfg->alloc_rsp(req, streamer->cb_arg);
}

void
mgmt_streamer_trim_front(struct mgmt_streamer *streamer, void *buf, size_t len)
{
	streamer->cfg->trim_front(buf, len, streamer->cb_arg);
}

void
mgmt_streamer_reset_buf(struct mgmt_streamer *streamer, void *buf)
{
	streamer->cfg->reset_buf(buf, streamer->cb_arg);
}

int
mgmt_streamer_write_at(struct mgmt_streamer *streamer, size_t offset,
					   const void *data, int len)
{
	return streamer->cfg->write_at(streamer->writer, offset, data, len, streamer->cb_arg);
}

int
mgmt_streamer_init_reader(struct mgmt_streamer *streamer, void *buf)
{
	return streamer->cfg->init_reader(streamer->reader, buf, streamer->cb_arg);
}

int
mgmt_streamer_init_writer(struct mgmt_streamer *streamer, void *buf)
{
	return streamer->cfg->init_writer(streamer->writer, buf, streamer->cb_arg);
}

void
mgmt_streamer_free_buf(struct mgmt_streamer *streamer, void *buf)
{
	streamer->cfg->free_buf(buf, streamer->cb_arg);
}

void
mgmt_unregister_group(struct mgmt_group *group)
{
	struct mgmt_group *curr = server_list_head, *prev = NULL;

	if (!group) {
		return;
	}

	if (curr == group) {
		server_list_head = curr->mg_next;
		return;
	}

	while (curr && curr != group) {
		prev = curr;
		curr = curr->mg_next;
	}

	if (!prev || !curr) {
		return;
	}

	prev->mg_next = curr->mg_next;
	if (curr->mg_next == NULL) {
		server_list_end = curr;
	}
}

static struct mgmt_group *
mgmt_find_group(uint16_t group_id, uint16_t command_id, bool client)
{
	struct mgmt_group *group;

	if (client) {
		#ifdef CONFIG_MCUMGR_CLIENT
        	group = client_list_head;
		#else
	        group = NULL;
		#endif
	} else {
		group = server_list_head;
	}


	/*
	 * Find the group with the specified group id, if one exists
	 * check the handler for the command id and make sure
	 * that is not NULL. If that is not set, look for the group
	 * with a command id that is set
	 */
	for (; group != NULL; group = group->mg_next) {
		if (group->mg_group_id == group_id) {
			if (command_id >= group->mg_handlers_count) {
				return NULL;
			}

			if (!group->mg_handlers[command_id].mh_read &&
			    !group->mg_handlers[command_id].mh_write) {
				continue;
			}

			break;
		}
	}

	return group;
}

void
mgmt_register_group(struct mgmt_group *group)
{
	if (server_list_end == NULL) {
		server_list_head = group;
	} else {
		server_list_end->mg_next = group;
	}
	server_list_end = group;
}

#ifdef CONFIG_MCUMGR_CLIENT
void
mgmt_register_client_group(struct mgmt_group *group)
{
	if (client_list_end == NULL) {
		client_list_head = group;
	} else {
		client_list_end->mg_next = group;
	}
	client_list_end = group;
}
#endif

const struct mgmt_handler *
mgmt_find_handler(uint16_t group_id, uint16_t command_id)
{
	const struct mgmt_group *group;

	group = mgmt_find_group(group_id, command_id, false);
	if (!group) {
		return NULL;
	}

	return &group->mg_handlers[command_id];
}

const struct mgmt_handler *
mgmt_find_client_handler(uint16_t group_id, uint16_t command_id)
{
	const struct mgmt_group *group;

	group = mgmt_find_group(group_id, command_id, true);
	if (!group) {
		return NULL;
	}

	return &group->mg_handlers[command_id];
}

int
mgmt_write_rsp_status(struct mgmt_ctxt *ctxt, int errcode)
{
	int rc;

	rc = cbor_encode_text_stringz(&ctxt->encoder, "rc");
	if (rc != 0) {
		return rc;
	}

	rc = cbor_encode_int(&ctxt->encoder, errcode);
	if (rc != 0) {
		return rc;
	}

	return 0;
}

int
mgmt_err_from_cbor(int cbor_status)
{
	switch (cbor_status) {
	case CborNoError:
		return MGMT_ERR_EOK;
	case CborErrorOutOfMemory:
		return MGMT_ERR_ENOMEM;
	}
	return MGMT_ERR_EUNKNOWN;
}

int
mgmt_ctxt_init(struct mgmt_ctxt *ctxt, struct mgmt_streamer *streamer,
		   const struct mgmt_hdr *hdr)
{
	int rc;

	rc = cbor_parser_init(streamer->reader, 0, &ctxt->parser, &ctxt->it);
	if (rc != CborNoError) {
		return mgmt_err_from_cbor(rc);
	}

	cbor_encoder_init(&ctxt->encoder, streamer->writer, 0);

	memcpy(&ctxt->hdr, hdr, sizeof(ctxt->hdr));

	return 0;
}

void
mgmt_ntoh_hdr(struct mgmt_hdr *hdr)
{
	hdr->nh_len = ntohs(hdr->nh_len);
	hdr->nh_group = ntohs(hdr->nh_group);
}

void
mgmt_hton_hdr(struct mgmt_hdr *hdr)
{
	hdr->nh_len = htons(hdr->nh_len);
	hdr->nh_group = htons(hdr->nh_group);
}

int
mgmt_register_evt_cb(mgmt_on_evt_cb cb)
{
	int i = (int)atomic_inc(&evt_users);

	if (i < CONFIG_MCUMGR_EVENT_CALLBACK_MAX_USERS) {
		evt_cb[i] = cb;
		return 0;
	} else {
		return MGMT_ERR_ENOMEM;
	}
}

void
mgmt_evt(uint8_t opcode, const struct mgmt_hdr *hdr, void *arg)
{
	int i;
	int users = (int)atomic_get(&evt_users);

	for (i = 0; i < users; i++) {
		if (evt_cb[i]) {
			evt_cb[i](opcode, hdr, arg);
		}
	}
}


bool
is_cmd(const struct mgmt_hdr *hdr)
{
    return ((hdr->nh_op % 2) == 0);
}

#ifdef CONFIG_MCUMGR_CLIENT
uint8_t
mgmt_get_sequence(void)
{
	return (uint8_t)atomic_inc(&mgmt_sequence);
}
#endif

#ifdef CONFIG_MCUMGR_STATUS_STRINGS
const char *
mgmt_get_string_operation(uint8_t operation)
{
	switch (operation) {
	case MGMT_OP_READ:
		return "Read";
	case MGMT_OP_READ_RSP:
		return "Read Rsp";
	case MGMT_OP_WRITE:
		return "Write";
	case MGMT_OP_WRITE_RSP:
		return "Write Rsp";
	default:
		return "?";
	}
}

const char *
mgmt_get_string_group(uint16_t group)
{
	switch (group) {
	case MGMT_GROUP_ID_OS:
		return "OS";
	case MGMT_GROUP_ID_IMAGE:
		return "Image";
	case MGMT_GROUP_ID_STAT:
		return "Stat";
	case MGMT_GROUP_ID_CONFIG:
		return "Config";
	case MGMT_GROUP_ID_LOG:
		return "Log";
	case MGMT_GROUP_ID_CRASH:
		return "Crash";
	case MGMT_GROUP_ID_SPLIT:
		return "Split";
	case MGMT_GROUP_ID_RUN:
		return "Run";
	case MGMT_GROUP_ID_FS:
		return "FS";
	case MGMT_GROUP_ID_SHELL:
		return "Shell";
	default:
		return "?";
	}
}

const char *
mgmt_get_string_err(int err)
{
	switch (err) {
	case MGMT_ERR_EOK:
		return "0-OK";
	case MGMT_ERR_EUNKNOWN:
		return "1-Unknown";
	case MGMT_ERR_ENOMEM:
		return "2-No Memory";
	case MGMT_ERR_EINVAL:
		return "3-Invalid Argument";
	case MGMT_ERR_ETIMEOUT:
		return "4-Timeout";
	case MGMT_ERR_ENOENT:
		return "5-No Entry";
	case MGMT_ERR_EBADSTATE:
		return "6-Current state disallows command";
	case MGMT_ERR_EMSGSIZE:
		return "7-Response too large";
	case MGMT_ERR_ENOTSUP:
		return "8-Command not supported";
	case MGMT_ERR_ECORRUPT:
		return "9-Corrupt ";
	case MGMT_ERR_DECODE:
		return "10-Decode";
	case MGMT_ERR_ENCODE:
		return "11-Encode";
	default:
		return "?";
	}
}

const char *
mgmt_get_string_event(uint8_t event)
{
	switch (event) {
	case MGMT_EVT_OP_CMD_RECV:
		return "Cmd Recv";
	case MGMT_EVT_OP_CMD_STATUS:
		return "Cmd Status";
	case MGMT_EVT_OP_CMD_DONE:
		return "Cmd Done";
	case MGMT_EVT_OP_RSP_RECV:
		return "Rsp Recv";
	case MGMT_EVT_OP_RSP_STATUS:
		return "Rsp Status";
	case MGMT_EVT_OP_RSP_DONE:
		return "Rsp Done";
	case MGMT_EVT_OP_CMD_SENT:
		return "Cmd Sent";
	default:
		return "?";
	}
}

#endif /* CONFIG_MCUMGR_STATUS_STRINGS */