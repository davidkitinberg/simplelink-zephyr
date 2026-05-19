/* btp_sps.h - Bluetooth Scan Parameters Service tester headers */

/*
 * Copyright (c) 2026 Codecoup
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>

/* SPS commands */
#define BTP_SPS_READ_SUPPORTED_COMMANDS 0x01
struct btp_sps_read_supported_commands_rp {
	uint8_t data[0];
} __packed;

#define BTP_SPS_SET_SCAN_INTERVAL_WINDOW 0x02
struct btp_sps_set_scan_interval_window_cmd {
	uint16_t interval;
	uint16_t window;
} __packed;

#define BTP_SPS_GET_SCAN_INTERVAL_WINDOW 0x03
struct btp_sps_get_scan_interval_window_rp {
	uint16_t interval;
	uint16_t window;
} __packed;

#define BTP_SPS_REFRESH_REQUEST 0x04

/* SPS events */
#define BTP_SPS_EV_SCAN_INTERVAL_WINDOW_WRITTEN 0x80
struct btp_sps_scan_interval_window_written_ev {
	uint16_t interval;
	uint16_t window;
} __packed;

#define BTP_SPS_EV_SCAN_REFRESH_CCC_CHANGED 0x81
struct btp_sps_scan_refresh_ccc_changed_ev {
	uint8_t enabled;
} __packed;
