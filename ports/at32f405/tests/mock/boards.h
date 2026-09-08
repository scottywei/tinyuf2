// SPDX-License-Identifier: MIT
#pragma once
#include "../../boards/weact_at32f405/board.h"
#define SRAM_BASE 0x20000000u
#define BOARD_FLASH_ADDR_ZERO 0x08000000u
#define BOARD_FLASH_APP_START 0x08008000u
#define BOARD_FLASH_END (BOARD_FLASH_ADDR_ZERO + BOARD_FLASH_SIZE)
#define CFG_UF2_FLASH_SIZE BOARD_FLASH_SIZE
#define TINYUF2_STRICT_UF2 1
#define BOARD_UF2_FAMILY_ID 0x6b1f405c
#define TUF2_LOG 0
#define FLASH_ODF_FLAG 1
#define FLASH_PRGMERR_FLAG 2
#define FLASH_EPPERR_FLAG 4
#define FLASH_OPERATE_DONE 0
void flash_flag_clear(uint32_t flags);
int flash_sector_erase(uint32_t addr);
int flash_word_program(uint32_t addr, uint32_t data);
void flash_unlock(void);
void flash_lock(void);
bool board_flash_complete(void);
