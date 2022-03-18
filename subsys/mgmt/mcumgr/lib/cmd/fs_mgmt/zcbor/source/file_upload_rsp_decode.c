/*
 * Generated using zcbor version 0.3.99
 * https://github.com/NordicSemiconductor/zcbor
 * Generated with a --default-max-qty of 0
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "zcbor_decode.h"
#include "file_upload_rsp_decode.h"

#if DEFAULT_MAX_QTY != 0
#error "The type file was generated with a different default_max_qty than this file"
#endif


static bool decode_file_upload_rsp(
		zcbor_state_t *state, struct file_upload_rsp *result)
{
	zcbor_print("%s\r\n", __func__);
	struct zcbor_string tmp_str;

	bool tmp_result = (((zcbor_map_start_decode(state) && (((((zcbor_tstr_expect(state, ((tmp_str.value = (uint8_t *)"rc", tmp_str.len = sizeof("rc") - 1, &tmp_str)))))
	&& (zcbor_int32_decode(state, (&(*result).rc))))
	&& (((zcbor_tstr_expect(state, ((tmp_str.value = (uint8_t *)"off", tmp_str.len = sizeof("off") - 1, &tmp_str)))))
	&& (zcbor_uint32_decode(state, (&(*result).off))))) || (zcbor_list_map_end_force_decode(state), false)) && zcbor_map_end_decode(state))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}



uint_fast8_t cbor_decode_file_upload_rsp(
		const uint8_t *payload, size_t payload_len,
		void *v_result,
		size_t *payload_len_out)
{
	zcbor_state_t states[3];

	struct file_upload_rsp *result = v_result;

	zcbor_new_state(states, sizeof(states) / sizeof(zcbor_state_t), payload, payload_len, 1);

	bool ret = decode_file_upload_rsp(states, result);

	if (ret && (payload_len_out != NULL)) {
		*payload_len_out = MIN(payload_len,
				(size_t)states[0].payload - (size_t)payload);
	}

	if (!ret) {
		uint_fast8_t ret = zcbor_pop_error(states);
		return (ret == ZCBOR_SUCCESS) ? ZCBOR_ERR_UNKNOWN : ret;
	}
	return ZCBOR_SUCCESS;
}
