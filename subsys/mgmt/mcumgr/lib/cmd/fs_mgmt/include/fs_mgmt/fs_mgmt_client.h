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

/* *** todo: limited to UART transport *** */

/**
 * @brief Read a file
 *
 * @param name of file to read from mgmt server
 * @return int 0 on success, negative errno
 */
int fs_mgmt_client_download(const char *name);

/**
 * @brief Write a file
 *
 * @param name of file to write to mgmt server
 * @param data to write
 * @param size of data
 * @return int 0 on success, negative errno
 */
int fs_mgmt_client_upload(const char *name, const void *data, size_t size);

#ifdef __cplusplus
}
#endif

#endif
