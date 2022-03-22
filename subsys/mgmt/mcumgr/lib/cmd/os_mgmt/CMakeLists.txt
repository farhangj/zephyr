#
# Copyright (c) 2018-2022 mcumgr authors
#
# SPDX-License-Identifier: Apache-2.0
#

target_include_directories(MCUMGR INTERFACE
    include
)

target_include_directories(MCUMGR INTERFACE
    zcbor/include
)

if(CONFIG_MCUMGR_CMD_OS_MGMT)
zephyr_library_sources(
    src/zephyr_os_mgmt.c
    src/os_mgmt.c
)

if(CONFIG_OS_MGMT_ZCBOR)
zephyr_library_sources_ifdef(CONFIG_OS_MGMT_ECHO
    zcbor/source/echo_cmd_decode.c
    zcbor/source/echo_rsp_encode.c
)
endif()
endif()

if(CONFIG_MCUMGR_CLIENT_OS_MGMT)
zephyr_library_sources(
    src/os_mgmt_client.c
)

zephyr_library_sources_ifdef(CONFIG_OS_MGMT_ECHO
    zcbor/source/echo_cmd_encode.c
    zcbor/source/echo_rsp_decode.c
)
endif()
