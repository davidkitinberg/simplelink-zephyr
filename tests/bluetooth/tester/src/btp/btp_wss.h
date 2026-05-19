/* btp_wss.h - Bluetooth Weight Scale Service tester headers */

/*
 * Copyright (c) 2026 Codecoup
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>

/* WSS commands */
#define BTP_WSS_READ_SUPPORTED_COMMANDS 0x01
struct btp_wss_read_supported_commands_rp {
	uint8_t data[0];
} __packed;

#define BTP_WSS_SET_FEATURE 0x02
struct btp_wss_set_feature_cmd {
	uint32_t feature;
} __packed;

#define BTP_WSS_INDICATE 0x03

/* WSS events */
#define BTP_WSS_EV_WEIGHT_MEAS_CCC_CHANGED 0x80
struct btp_wss_weight_meas_ccc_changed_ev {
	uint8_t enabled;
} __packed;

#define BTP_WSS_EV_WEIGHT_MEAS_INDICATE_DONE 0x81
struct btp_wss_weight_meas_indicate_done_ev {
	uint8_t err;
} __packed;
