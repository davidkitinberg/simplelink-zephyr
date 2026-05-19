/* btp_bps.h - Bluetooth Blood Pressure Service tester headers */

/*
 * Copyright (c) 2026 Codecoup
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>

/* BPS commands */
#define BTP_BPS_READ_SUPPORTED_COMMANDS 0x01
struct btp_bps_read_supported_commands_rp {
	uint8_t data[0];
} __packed;

#define BTP_BPS_SET_FEATURE 0x02
struct btp_bps_set_feature_cmd {
	uint16_t feature;
} __packed;

#define BTP_BPS_INDICATE 0x03

#define BTP_BPS_NOTIFY_INTERMEDIATE_CUFF 0x04

/* BPS events */
#define BTP_BPS_EV_BP_MEAS_CCC_CHANGED 0x80
struct btp_bps_bp_meas_ccc_changed_ev {
	uint8_t enabled;
} __packed;

#define BTP_BPS_EV_INTERMEDIATE_CUFF_CCC_CHANGED 0x81
struct btp_bps_intermediate_cuff_ccc_changed_ev {
	uint8_t enabled;
} __packed;

#define BTP_BPS_EV_BP_MEAS_INDICATE_DONE 0x82
struct btp_bps_bp_meas_indicate_done_ev {
	uint8_t err;
} __packed;
