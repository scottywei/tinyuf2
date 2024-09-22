/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Matthew McGowan for Blues Inc.
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

#ifndef BOARD_H_
#define BOARD_H_

//--------------------------------------------------------------------+
// Button
//--------------------------------------------------------------------+
#define BUTTON_PORT           GPIOB
#define BUTTON_PIN            GPIO_PIN_0
#define BUTTON_STATE_ACTIVE   1

//--------------------------------------------------------------------+
// LED
//--------------------------------------------------------------------+

#define LED_PORT              GPIOB
#define LED_PIN               GPIO_PIN_3
#define LED_STATE_ON          1

//--------------------------------------------------------------------+
// Neopixel
//--------------------------------------------------------------------+

// Number of neopixels
#define NEOPIXEL_NUMBER       0

//--------------------------------------------------------------------+
// Flash
//--------------------------------------------------------------------+

// Flash size of the board
#define BOARD_FLASH_SIZE  (1024 * 128)

//--------------------------------------------------------------------+
// USB UF2
//--------------------------------------------------------------------+

#define USB_VID           0x0483
#define USB_PID           0x5720
#define USB_MANUFACTURER  "STMicroelectronics"
#define USB_PRODUCT       "STM32L412nucleo"

#define UF2_PRODUCT_NAME  USB_PRODUCT
#define UF2_BOARD_ID      "Nucleo-32_L412KB"
#define UF2_VOLUME_LABEL  "L412KBBOOT"
#define UF2_INDEX_URL     "https://www.st.com/en/evaluation-tools/stm32-nucleo-boards.html"

//--------------------------------------------------------------------+
// UART
//--------------------------------------------------------------------+

//#define UART_DEV              USART1
#define UART_CLK_EN           __HAL_RCC_LPUART1_CLK_ENABLE
#define UART_GPIO_PORT        GPIOA
#define UART_GPIO_AF          GPIO_AF8_LPUART1
#define UART_TX_PIN           GPIO_PIN_2
#define UART_RX_PIN           GPIO_PIN_3

#define USB_NO_VBUS_PIN
#define USB_NO_USB_ID_PIN
#define BOARD_STACK_APP_START (0x20000000U)
#define BOARD_STACK_APP_END   (BOARD_STACK_APP_START+(40*1024))



void clock_init(void);

#endif
