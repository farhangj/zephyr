/*
 * Copyright (c) 2018-2022 mcumgr authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef H_OS_MGMT_CLIENT_
#define H_OS_MGMT_CLIENT_

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr.h>
#include <mgmt/mgmt.h>
#include <mgmt/mcumgr/smp.h>

/**
 * @brief Send an echo command
 *
 * @param transport to send message on
 * @param msg string to have echoed back
 * @param callback Optional callback that provides the sequence number of the
 * sent message.  Can be used to correlate send message with the response received
 * callback.
 * @return int 0 on success, MGMT_ERR_[...] code on failure
 */
int os_mgmt_client_echo(struct zephyr_smp_transport *transport, const char *msg,
			void *(callback)(uint8_t sequence));

#ifdef __cplusplus
}
#endif

#endif
