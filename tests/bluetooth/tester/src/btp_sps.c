/* btp_sps.c - Bluetooth Scan Parameters Service Tester */

/*
 * Copyright (c) 2026 Codecoup
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>

#include <zephyr/bluetooth/services/sps.h>
#include <zephyr/sys/byteorder.h>

#include <zephyr/logging/log.h>
#define LOG_MODULE_NAME bttester_sps
LOG_MODULE_REGISTER(LOG_MODULE_NAME, CONFIG_BTTESTER_LOG_LEVEL);

#include "btp/btp.h"

static bool initialized;

static uint8_t sps_supported_commands(const void *cmd, uint16_t cmd_len,
				      void *rsp, uint16_t *rsp_len)
{
	struct btp_sps_read_supported_commands_rp *rp = rsp;

	tester_set_bit(rp->data, BTP_SPS_READ_SUPPORTED_COMMANDS);
	tester_set_bit(rp->data, BTP_SPS_SET_SCAN_INTERVAL_WINDOW);
	tester_set_bit(rp->data, BTP_SPS_GET_SCAN_INTERVAL_WINDOW);
	tester_set_bit(rp->data, BTP_SPS_REFRESH_REQUEST);

	*rsp_len = sizeof(*rp) + 1;

	return BTP_STATUS_SUCCESS;
}

static uint8_t sps_set_scan_interval_window(const void *cmd, uint16_t cmd_len,
					    void *rsp, uint16_t *rsp_len)
{
	const struct btp_sps_set_scan_interval_window_cmd *cp = cmd;
	struct bt_sps_scan_interval_window scan_interval_window = {
		.interval = sys_le16_to_cpu(cp->interval),
		.window = sys_le16_to_cpu(cp->window),
	};
	int err;

	ARG_UNUSED(rsp);
	ARG_UNUSED(rsp_len);

	err = bt_sps_set_scan_interval_window(&scan_interval_window);

	return BTP_STATUS_VAL(err);
}

static uint8_t sps_get_scan_interval_window(const void *cmd, uint16_t cmd_len,
					    void *rsp, uint16_t *rsp_len)
{
	struct btp_sps_get_scan_interval_window_rp *rp = rsp;
	struct bt_sps_scan_interval_window scan_interval_window;
	int err;

	err = bt_sps_get_scan_interval_window(&scan_interval_window);
	if (err != 0) {
		return BTP_STATUS_VAL(err);
	}

	rp->interval = sys_cpu_to_le16(scan_interval_window.interval);
	rp->window = sys_cpu_to_le16(scan_interval_window.window);
	*rsp_len = sizeof(*rp);

	return BTP_STATUS_SUCCESS;
}

static uint8_t sps_refresh_request(const void *cmd, uint16_t cmd_len,
				   void *rsp, uint16_t *rsp_len)
{
	int err;

	ARG_UNUSED(cmd);
	ARG_UNUSED(cmd_len);
	ARG_UNUSED(rsp);
	ARG_UNUSED(rsp_len);

	err = bt_sps_refresh_request(NULL);

	return BTP_STATUS_VAL(err);
}

static const struct btp_handler sps_handlers[] = {
	{
		.opcode = BTP_SPS_READ_SUPPORTED_COMMANDS,
		.index = BTP_INDEX_NONE,
		.expect_len = 0,
		.func = sps_supported_commands,
	},
	{
		.opcode = BTP_SPS_SET_SCAN_INTERVAL_WINDOW,
		.index = BTP_INDEX,
		.expect_len = sizeof(struct btp_sps_set_scan_interval_window_cmd),
		.func = sps_set_scan_interval_window,
	},
	{
		.opcode = BTP_SPS_GET_SCAN_INTERVAL_WINDOW,
		.index = BTP_INDEX,
		.expect_len = 0,
		.func = sps_get_scan_interval_window,
	},
	{
		.opcode = BTP_SPS_REFRESH_REQUEST,
		.index = BTP_INDEX,
		.expect_len = 0,
		.func = sps_refresh_request,
	},
};

static void scan_interval_window_written(struct bt_conn *conn,
					 const struct bt_sps_scan_interval_window
					 *scan_interval_window)
{
	struct btp_sps_scan_interval_window_written_ev ev = {
		.interval = sys_cpu_to_le16(scan_interval_window->interval),
		.window = sys_cpu_to_le16(scan_interval_window->window),
	};

	ARG_UNUSED(conn);

	if (!initialized) {
		return;
	}

	tester_event(BTP_SERVICE_ID_SPS,
		     BTP_SPS_EV_SCAN_INTERVAL_WINDOW_WRITTEN,
		     &ev, sizeof(ev));
}

static void scan_refresh_ccc_changed(bool enabled)
{
	struct btp_sps_scan_refresh_ccc_changed_ev ev = {
		.enabled = enabled,
	};

	if (!initialized) {
		return;
	}

	tester_event(BTP_SERVICE_ID_SPS, BTP_SPS_EV_SCAN_REFRESH_CCC_CHANGED,
		     &ev, sizeof(ev));
}

static struct bt_sps_cb sps_callbacks = {
	.scan_interval_window_written = scan_interval_window_written,
	.scan_refresh_ccc_changed = scan_refresh_ccc_changed,
};

uint8_t tester_init_sps(void)
{
	int err;

	err = bt_sps_cb_register(&sps_callbacks);
	if (err != 0) {
		LOG_ERR("Failed to register SPS callbacks (err %d)", err);
		return BTP_STATUS_VAL(err);
	}

	tester_register_command_handlers(BTP_SERVICE_ID_SPS, sps_handlers,
					 ARRAY_SIZE(sps_handlers));

	initialized = true;

	return BTP_STATUS_SUCCESS;
}

uint8_t tester_unregister_sps(void)
{
	int err;

	initialized = false;

	err = bt_sps_cb_unregister(&sps_callbacks);
	if ((err != 0) && (err != -ENOENT)) {
		LOG_ERR("Failed to unregister SPS callbacks (err %d)", err);
		return BTP_STATUS_VAL(err);
	}

	return BTP_STATUS_SUCCESS;
}
