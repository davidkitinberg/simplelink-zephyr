/* btp_bps.c - Bluetooth Blood Pressure Service Tester */

/*
 * Copyright (c) 2026 Codecoup
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>

#include <zephyr/bluetooth/services/bps.h>
#include <zephyr/sys/byteorder.h>

#include <zephyr/logging/log.h>
#define LOG_MODULE_NAME bttester_bps
LOG_MODULE_REGISTER(LOG_MODULE_NAME, CONFIG_BTTESTER_LOG_LEVEL);

#include "btp/btp.h"

static bool initialized;

static const uint8_t default_bp_measurement[] = {
	0x00,
	0x78, 0x00,
	0x50, 0x00,
	0x5f, 0x00,
};

static const uint8_t default_intermediate_cuff[] = {
	0x00,
	0x76, 0x00,
	0x4f, 0x00,
	0x5d, 0x00,
};

static uint8_t bps_supported_commands(const void *cmd, uint16_t cmd_len,
				      void *rsp, uint16_t *rsp_len)
{
	struct btp_bps_read_supported_commands_rp *rp = rsp;

	tester_set_bit(rp->data, BTP_BPS_READ_SUPPORTED_COMMANDS);
	tester_set_bit(rp->data, BTP_BPS_SET_FEATURE);
	tester_set_bit(rp->data, BTP_BPS_INDICATE);
	tester_set_bit(rp->data, BTP_BPS_NOTIFY_INTERMEDIATE_CUFF);

	*rsp_len = sizeof(*rp) + 1;

	return BTP_STATUS_SUCCESS;
}

static uint8_t bps_set_feature(const void *cmd, uint16_t cmd_len,
			       void *rsp, uint16_t *rsp_len)
{
	const struct btp_bps_set_feature_cmd *cp = cmd;
	int err;

	ARG_UNUSED(rsp);
	ARG_UNUSED(rsp_len);

	err = bt_bps_set_feature(sys_le16_to_cpu(cp->feature));

	return BTP_STATUS_VAL(err);
}

static uint8_t bps_indicate(const void *cmd, uint16_t cmd_len,
			    void *rsp, uint16_t *rsp_len)
{
	const uint8_t *measurement = cmd;
	uint16_t measurement_len = cmd_len;
	int err;

	ARG_UNUSED(rsp);
	ARG_UNUSED(rsp_len);

	if (measurement_len == 0U) {
		measurement = default_bp_measurement;
		measurement_len = sizeof(default_bp_measurement);
	}

	err = bt_bps_indicate(NULL, measurement, measurement_len);

	return BTP_STATUS_VAL(err);
}

static uint8_t bps_notify_intermediate_cuff(const void *cmd, uint16_t cmd_len,
					    void *rsp, uint16_t *rsp_len)
{
	const uint8_t *measurement = cmd;
	uint16_t measurement_len = cmd_len;
	int err;

	ARG_UNUSED(rsp);
	ARG_UNUSED(rsp_len);

	if (measurement_len == 0U) {
		measurement = default_intermediate_cuff;
		measurement_len = sizeof(default_intermediate_cuff);
	}

	err = bt_bps_notify_intermediate_cuff(NULL, measurement, measurement_len);

	return BTP_STATUS_VAL(err);
}

static const struct btp_handler bps_handlers[] = {
	{
		.opcode = BTP_BPS_READ_SUPPORTED_COMMANDS,
		.index = BTP_INDEX_NONE,
		.expect_len = 0,
		.func = bps_supported_commands,
	},
	{
		.opcode = BTP_BPS_SET_FEATURE,
		.index = BTP_INDEX,
		.expect_len = sizeof(struct btp_bps_set_feature_cmd),
		.func = bps_set_feature,
	},
	{
		.opcode = BTP_BPS_INDICATE,
		.index = BTP_INDEX,
		.expect_len = BTP_HANDLER_LENGTH_VARIABLE,
		.func = bps_indicate,
	},
	{
		.opcode = BTP_BPS_NOTIFY_INTERMEDIATE_CUFF,
		.index = BTP_INDEX,
		.expect_len = BTP_HANDLER_LENGTH_VARIABLE,
		.func = bps_notify_intermediate_cuff,
	},
};

static void bp_meas_ccc_changed(bool enabled)
{
	struct btp_bps_bp_meas_ccc_changed_ev ev = {
		.enabled = enabled,
	};

	if (!initialized) {
		return;
	}

	tester_event(BTP_SERVICE_ID_BPS, BTP_BPS_EV_BP_MEAS_CCC_CHANGED,
		     &ev, sizeof(ev));
}

static void intermediate_cuff_pressure_ccc_changed(bool enabled)
{
	struct btp_bps_intermediate_cuff_ccc_changed_ev ev = {
		.enabled = enabled,
	};

	if (!initialized) {
		return;
	}

	tester_event(BTP_SERVICE_ID_BPS,
		     BTP_BPS_EV_INTERMEDIATE_CUFF_CCC_CHANGED,
		     &ev, sizeof(ev));
}

static void bp_meas_indicate_done(struct bt_conn *conn, uint8_t err)
{
	struct btp_bps_bp_meas_indicate_done_ev ev = {
		.err = err,
	};

	ARG_UNUSED(conn);

	if (!initialized) {
		return;
	}

	tester_event(BTP_SERVICE_ID_BPS, BTP_BPS_EV_BP_MEAS_INDICATE_DONE,
		     &ev, sizeof(ev));
}

static struct bt_bps_cb bps_callbacks = {
	.bp_meas_ccc_changed = bp_meas_ccc_changed,
	.intermediate_cuff_pressure_ccc_changed =
		intermediate_cuff_pressure_ccc_changed,
	.bp_meas_indicate_done = bp_meas_indicate_done,
};

uint8_t tester_init_bps(void)
{
	int err;

	err = bt_bps_cb_register(&bps_callbacks);
	if (err != 0) {
		LOG_ERR("Failed to register BPS callbacks (err %d)", err);
		return BTP_STATUS_VAL(err);
	}

	tester_register_command_handlers(BTP_SERVICE_ID_BPS, bps_handlers,
					 ARRAY_SIZE(bps_handlers));

	initialized = true;

	return BTP_STATUS_SUCCESS;
}

uint8_t tester_unregister_bps(void)
{
	int err;

	initialized = false;

	err = bt_bps_cb_unregister(&bps_callbacks);
	if ((err != 0) && (err != -ENOENT)) {
		LOG_ERR("Failed to unregister BPS callbacks (err %d)", err);
		return BTP_STATUS_VAL(err);
	}

	return BTP_STATUS_SUCCESS;
}
