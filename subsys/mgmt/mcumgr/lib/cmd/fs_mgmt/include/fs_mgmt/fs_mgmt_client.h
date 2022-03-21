/*
 * Copyright (c) 2018-2022 mcumgr authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef H_FS_MGMT_CLIENT_
#define H_FS_MGMT_CLIENT_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Read a file
 *
 * @param transport to send message on
 * @param name of file to read from mgmt server
 * @return int 0 on success, MGMT_ERR_[...] code on failure
 */
int fs_mgmt_client_download(struct zephyr_smp_transport *transport, const char *name);

/**
 * @brief Write a file
 *
 * @param transport to send message on
 * @param name of file to write to mgmt server
 * @param data to write
 * @param size of data
 * @return int 0 on success, MGMT_ERR_[...] code on failure
 */
int fs_mgmt_client_upload(struct zephyr_smp_transport *transport, const char *name,
			  const void *data, size_t size);

#ifdef __cplusplus
}
#endif

#endif
