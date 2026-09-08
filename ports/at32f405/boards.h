// SPDX-License-Identifier: MIT
#pragma once
#include "at32f402_405.h"
#include "board.h"

#define BOARD_FLASH_ADDR_ZERO 0x08000000u
#define BOARD_FLASH_APP_START 0x08008000u
#define BOARD_FLASH_END (BOARD_FLASH_ADDR_ZERO + BOARD_FLASH_SIZE)
#define CFG_UF2_FLASH_SIZE BOARD_FLASH_SIZE
#define TINYUF2_DBL_TAP_DFU 1
#define TINYUF2_LED 1
#define TINYUF2_PROTECT_BOOTLOADER 0
#define TINYUF2_STRICT_UF2 1

bool board_flash_complete(void);
