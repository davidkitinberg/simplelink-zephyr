# Copyright (c) 2024 Texas Instruments Incorporated
# Copyright (c) 2024 BayLibre, SAS
#
# SPDX-License-Identifier: Apache-2.0

board_runner_args(jlink "--device=CC2340R53")
board_runner_args(openocd --cmd-pre-init "source [find board/ti_lp_em_cc2340r53.cfg]")
if(DEFINED ENV{TI_OPENOCD_INSTALL_DIR})
  set(OPENOCD_BASE $ENV{TI_OPENOCD_INSTALL_DIR}/openocd/bin)
  set(OPENOCD ${OPENOCD_BASE}/bin/openocd)
  set(OPENOCD_DEFAULT_PATH ${OPENOCD_BASE}/share/openocd/scripts)
else()
  find_program(TI_SYSTEM_OPENOCD openocd)
  if(TI_SYSTEM_OPENOCD)
    get_filename_component(TI_SYSTEM_OPENOCD_BIN_DIR ${TI_SYSTEM_OPENOCD} DIRECTORY)
    get_filename_component(TI_SYSTEM_OPENOCD_PREFIX ${TI_SYSTEM_OPENOCD_BIN_DIR} DIRECTORY)

    if(EXISTS "${TI_SYSTEM_OPENOCD_PREFIX}/share/openocd/scripts/board/ti_lp_em_cc2340r53.cfg")
      set(OPENOCD ${TI_SYSTEM_OPENOCD})
      set(OPENOCD_DEFAULT_PATH ${TI_SYSTEM_OPENOCD_PREFIX}/share/openocd/scripts)
    endif()
  endif()
endif()
include(${ZEPHYR_BASE}/boards/common/openocd.board.cmake)
include(${ZEPHYR_BASE}/boards/common/jlink.board.cmake)
