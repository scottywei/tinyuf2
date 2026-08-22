#ifndef TINYUF2_STM32F1_BOARDS_H_
#define TINYUF2_STM32F1_BOARDS_H_

#include "stm32f1xx.h"
#include "stm32f1xx_hal_conf.h"
#include "board.h"

#define BOARD_FLASH_ADDR_ZERO 0x08000000UL
#define BOARD_FLASH_APP_START 0x08004000UL
#define BOARD_PAGE_SIZE       0x400UL
#define BOARD_RAM_START       0x20000000UL
#define BOARD_RAM_SIZE        (20UL * 1024UL)

#define TINYUF2_DBL_TAP_DFU 1

// Option-byte write protection is intentionally disabled. It makes field
// recovery and board bring-up safer, and F1 option-byte changes force a reset.
#define TINYUF2_PROTECT_BOOTLOADER 0

#ifdef LED_PIN
#define TINYUF2_LED 1
#endif

#endif
