#
# Copyright (c) 2018-2021 mcumgr authors
#
# SPDX-License-Identifier: Apache-2.0
#

target_include_directories(MCUMGR INTERFACE include)

zephyr_library_sources(
	src/zephyr_fs_mgmt.c
	src/fs_mgmt.c
)

target_include_directories(MCUMGR INTERFACE zcbor/include)

#zephyr_library_sources(
#	zcbor/source/file_download_cmd_decode.c
#	zcbor/source/file_download_rsp_encode.c
#	zcbor/source/file_upload_cmd_decode.c
#	zcbor/source/file_upload_rsp_encode.c
#   zcbor/source/error_rsp_encode.c
#)

if(CONFIG_MCUMGR_CLIENT_FS_MGMT)
zephyr_library_sources(
	zcbor/source/file_download_cmd_encode.c
	zcbor/source/file_download_rsp_decode.c
	zcbor/source/file_upload_cmd_encode.c
	zcbor/source/file_upload_rsp_decode.c
	zcbor/source/error_rsp_decode.c
	src/fs_mgmt_client.c
)
endif()