#
# Copyright (c) 2018-2021 mcumgr authors
#
# SPDX-License-Identifier: Apache-2.0
#

target_include_directories(MCUMGR INTERFACE
    include
)

zephyr_library_sources_ifdef(CONFIG_MCUMGR_CMD_OS_MGMT
    src/zephyr_os_mgmt.c
    src/os_mgmt.c
)

zephyr_library_sources_ifdef(CONFIG_MCUMGR_CLIENT_OS_MGMT
    src/os_mgmt_client.c
)