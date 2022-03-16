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
#include "file_download_rsp_decode.h"

#if DEFAULT_MAX_QTY != 0
#error "The type file was generated with a different default_max_qty than this file"
#endif


static bool decode_repeated_file_download_rsp_len(
		zcbor_state_t *state, struct file_download_rsp_len *result)
{
	zcbor_print("%s\r\n", __func__);
	struct zcbor_string tmp_str;

	bool tmp_result = ((((zcbor_tstr_expect(state, ((tmp_str.value = (uint8_t *)"len", tmp_str.len = sizeof("len") - 1, &tmp_str)))))
	&& (zcbor_uint32_decode(state, (&(*result)._file_download_rsp_len)))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}

static bool decode_file_download_rsp(
		zcbor_state_t *state, struct file_download_rsp *result)
{
	zcbor_print("%s\r\n", __func__);
	struct zcbor_string tmp_str;

	bool tmp_result = (((zcbor_map_start_decode(state) && (((((zcbor_tstr_expect(state, ((tmp_str.value = (uint8_t *)"off", tmp_str.len = sizeof("off") - 1, &tmp_str)))))
	&& (zcbor_uint32_decode(state, (&(*result)._file_download_rsp_offset))))
	&& (((zcbor_tstr_expect(state, ((tmp_str.value = (uint8_t *)"data", tmp_str.len = sizeof("data") - 1, &tmp_str)))))
	&& (zcbor_bstr_decode(state, (&(*result)._file_download_rsp_data))))
	&& (((zcbor_tstr_expect(state, ((tmp_str.value = (uint8_t *)"rc", tmp_str.len = sizeof("rc") - 1, &tmp_str)))))
	&& (zcbor_int32_decode(state, (&(*result)._file_download_rsp_rc))))
	&& zcbor_present_decode(&((*result)._file_download_rsp_len_present), (zcbor_decoder_t *)decode_repeated_file_download_rsp_len, state, (&(*result)._file_download_rsp_len))) || (zcbor_list_map_end_force_decode(state), false)) && zcbor_map_end_decode(state))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}



uint_fast8_t cbor_decode_file_download_rsp(
		const uint8_t *payload, size_t payload_len,
		struct file_download_rsp *result,
		size_t *payload_len_out)
{
	zcbor_state_t states[3];

	zcbor_new_state(states, sizeof(states) / sizeof(zcbor_state_t), payload, payload_len, 1);

	bool ret = decode_file_download_rsp(states, result);

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
