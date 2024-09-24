/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Ha Thach (tinyusb.org) for Adafruit Industries
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef BOARDS_H_
#define BOARDS_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32l4xx.h"
#include "stm32l4xx_hal_conf.h"

#include "board.h"

#define BOARD_FLASH_ADDR_ZERO   0x08000000

// Flash Start Address of Application
#ifndef BOARD_FLASH_APP_START
#define BOARD_FLASH_APP_START   0x08006000
#endif

#define BOARD_PAGE_SIZE 0x800

#define BOARD_RAM_START 0x20000000
#define BOARD_RAM_SIZE 0x7FFF // SIZE_OF_RAM * 1024 - 1, (32 * 1024 - 1) = 32767 = 0x7FFF

// Double Reset tap to enter DFU
#define TINYUF2_DBL_TAP_DFU  1

// Enable write protection
#ifndef TINYUF2_PROTECT_BOOTLOADER
#define TINYUF2_PROTECT_BOOTLOADER    1
#endif

// Brightness percentage from 1 to 255
#ifndef NEOPIXEL_BRIGHTNESS
#define NEOPIXEL_BRIGHTNESS   0x10
#endif

#ifdef LED_PIN
#define TINYUF2_LED             1
#endif

bool board_reset_by_option_bytes(void);

#ifdef __cplusplus
 }
#endif

#endif /* BOARDS_H_ */
