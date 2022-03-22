/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#include <logging/log.h>
LOG_MODULE_REGISTER(os_mgmt_client, CONFIG_MGMT_CLIENT_OS_LOG_LEVEL);

#include <init.h>
#include <assert.h>
#include <string.h>

#include "mgmt/mgmt.h"
#include "os_mgmt/os_mgmt.h"
#include "os_mgmt/os_mgmt_impl.h"
#include "os_mgmt/os_mgmt_config.h"

#include "echo_rsp_decode.h"
#include "zcbor_mgmt.h"

#if CONFIG_OS_MGMT_CLIENT_ECHO
static int os_mgmt_client_echo(struct mgmt_ctxt *ctxt);
#endif

#if CONFIG_OS_MGMT_CLIENT_RESET
static int os_mgmt_client_reset(struct mgmt_ctxt *ctxt);
#endif

#if CONFIG_OS_MGMT_CLIENT_TASKSTAT
static int os_mgmt_client_taskstat_read(struct mgmt_ctxt *ctxt);
#endif

static const struct mgmt_handler os_mgmt_client_group_handlers[] = {
#if CONFIG_OS_MGMT_CLIENT_ECHO
	[OS_MGMT_ID_ECHO] = { os_mgmt_client_echo, os_mgmt_client_echo },
#endif
#if CONFIG_OS_MGMT_CLIENT_TASKSTAT
	[OS_MGMT_ID_TASKSTAT] = { os_mgmt_client_taskstat_read, NULL },
#endif
#if CONFIG_OS_MGMT_CLIENT_RESET
	[OS_MGMT_ID_RESET] = { os_mgmt_client_reset, os_mgmt_client_reset },
#endif
};

#define OS_MGMT_CLIENT_GROUP_SIZE                                                       \
	(sizeof os_mgmt_client_group_handlers /                                \
	 sizeof os_mgmt_client_group_handlers[0])

static struct mgmt_group os_mgmt_client_group = {
	.mg_handlers = os_mgmt_client_group_handlers,
	.mg_handlers_count = OS_MGMT_CLIENT_GROUP_SIZE,
	.mg_group_id = MGMT_GROUP_ID_OS,
};

#if CONFIG_OS_MGMT_CLIENT_ECHO
static int os_mgmt_client_echo(struct mgmt_ctxt *ctxt)
{
	struct echo_rsp echo_rsp;

	if (zcbor_mgmt_decode(ctxt, (zcbor_mgmt_decoder_func)cbor_decode_echo_rsp, &echo_rsp, true) != 0) {
		return MGMT_ERR_DECODE;
	}

	return MGMT_ERR_EOK;
}
#endif

#if CONFIG_OS_MGMT_CLIENT_TASKSTAT
static int os_mgmt_client_taskstat_read(struct mgmt_ctxt *ctxt)
{
	return MGMT_ERR_EOK;
}
#endif

#if CONFIG_OS_MGMT_CLIENT_RESET
static int os_mgmt_client_reset(struct mgmt_ctxt *ctxt)
{
	return MGMT_ERR_EOK;
}
#endif

static int os_mgmt_client_init(const struct device *device)
{
	ARG_UNUSED(device);

	mgmt_register_client_group(&os_mgmt_client_group);

	return 0;
}

SYS_INIT(os_mgmt_client_init, APPLICATION, 99);
