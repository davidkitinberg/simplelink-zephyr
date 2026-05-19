/* btp_wss.c - Bluetooth Weight Scale Service Tester */

/*
 * Copyright (c) 2026 Codecoup
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>

#include <zephyr/bluetooth/services/wss.h>
#include <zephyr/sys/byteorder.h>

#include <zephyr/logging/log.h>
#define LOG_MODULE_NAME bttester_wss
LOG_MODULE_REGISTER(LOG_MODULE_NAME, CONFIG_BTTESTER_LOG_LEVEL);

#include "btp/btp.h"

static bool initialized;

static const uint8_t default_weight_measurement[] = {
	0x00,
	0xb0, 0x36,
};

static uint8_t wss_supported_commands(const void *cmd, uint16_t cmd_len,
				      void *rsp, uint16_t *rsp_len)
{
	struct btp_wss_read_supported_commands_rp *rp = rsp;

	tester_set_bit(rp->data, BTP_WSS_READ_SUPPORTED_COMMANDS);
	tester_set_bit(rp->data, BTP_WSS_SET_FEATURE);
	tester_set_bit(rp->data, BTP_WSS_INDICATE);

	*rsp_len = sizeof(*rp) + 1;

	return BTP_STATUS_SUCCESS;
}

static uint8_t wss_set_feature(const void *cmd, uint16_t cmd_len,
			       void *rsp, uint16_t *rsp_len)
{
	const struct btp_wss_set_feature_cmd *cp = cmd;
	int err;

	ARG_UNUSED(rsp);
	ARG_UNUSED(rsp_len);

	err = bt_wss_set_feature(sys_le32_to_cpu(cp->feature));

	return BTP_STATUS_VAL(err);
}

static uint8_t wss_indicate(const void *cmd, uint16_t cmd_len,
			    void *rsp, uint16_t *rsp_len)
{
	const uint8_t *measurement = cmd;
	uint16_t measurement_len = cmd_len;
	int err;

	ARG_UNUSED(rsp);
	ARG_UNUSED(rsp_len);

	if (measurement_len == 0U) {
		measurement = default_weight_measurement;
		measurement_len = sizeof(default_weight_measurement);
	}

	err = bt_wss_indicate(NULL, measurement, measurement_len);

	return BTP_STATUS_VAL(err);
}

static const struct btp_handler wss_handlers[] = {
	{
		.opcode = BTP_WSS_READ_SUPPORTED_COMMANDS,
		.index = BTP_INDEX_NONE,
		.expect_len = 0,
		.func = wss_supported_commands,
	},
	{
		.opcode = BTP_WSS_SET_FEATURE,
		.index = BTP_INDEX,
		.expect_len = sizeof(struct btp_wss_set_feature_cmd),
		.func = wss_set_feature,
	},
	{
		.opcode = BTP_WSS_INDICATE,
		.index = BTP_INDEX,
		.expect_len = BTP_HANDLER_LENGTH_VARIABLE,
		.func = wss_indicate,
	},
};

static void weight_meas_ccc_changed(bool enabled)
{
	struct btp_wss_weight_meas_ccc_changed_ev ev = {
		.enabled = enabled,
	};

	if (!initialized) {
		return;
	}

	tester_event(BTP_SERVICE_ID_WSS, BTP_WSS_EV_WEIGHT_MEAS_CCC_CHANGED,
		     &ev, sizeof(ev));
}

static void weight_meas_indicate_done(struct bt_conn *conn, uint8_t err)
{
	struct btp_wss_weight_meas_indicate_done_ev ev = {
		.err = err,
	};

	ARG_UNUSED(conn);

	if (!initialized) {
		return;
	}

	tester_event(BTP_SERVICE_ID_WSS, BTP_WSS_EV_WEIGHT_MEAS_INDICATE_DONE,
		     &ev, sizeof(ev));
}

static struct bt_wss_cb wss_callbacks = {
	.weight_meas_ccc_changed = weight_meas_ccc_changed,
	.weight_meas_indicate_done = weight_meas_indicate_done,
};

uint8_t tester_init_wss(void)
{
	int err;

	err = bt_wss_cb_register(&wss_callbacks);
	if (err != 0) {
		LOG_ERR("Failed to register WSS callbacks (err %d)", err);
		return BTP_STATUS_VAL(err);
	}

	tester_register_command_handlers(BTP_SERVICE_ID_WSS, wss_handlers,
					 ARRAY_SIZE(wss_handlers));

	initialized = true;

	return BTP_STATUS_SUCCESS;
}

uint8_t tester_unregister_wss(void)
{
	int err;

	initialized = false;

	err = bt_wss_cb_unregister(&wss_callbacks);
	if ((err != 0) && (err != -ENOENT)) {
		LOG_ERR("Failed to unregister WSS callbacks (err %d)", err);
		return BTP_STATUS_VAL(err);
	}

	return BTP_STATUS_SUCCESS;
}
