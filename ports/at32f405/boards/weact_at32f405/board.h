// SPDX-License-Identifier: MIT
#pragma once

#if defined(AT32F405CCT7)
#define BOARD_FLASH_SIZE (256u * 1024)
#define BOARD_RAM_SIZE   (96u * 1024)
#define BOARD_FLASH_SECTOR_SIZE 2048u
#elif defined(AT32F405CBT7)
#define BOARD_FLASH_SIZE (128u * 1024)
#define BOARD_RAM_SIZE   (64u * 1024)
#define BOARD_FLASH_SECTOR_SIZE 1024u
#else
#error Unsupported WeAct AT32F405 device
#endif

// Development VID/PID, matching the Artery USB MSC example. Override for release.
#ifndef USB_VID
#define USB_VID 0x2E3C
#endif
#ifndef USB_PID
#define USB_PID 0x5720
#endif
#define USB_MANUFACTURER "ArteryTek "
#define USB_PRODUCT "AT32F405CxTx"
#define UF2_PRODUCT_NAME USB_MANUFACTURER " " USB_PRODUCT
#define UF2_BOARD_ID "BIOI_UF2"
#define UF2_VOLUME_LABEL "BIOI_UF2"
#define UF2_INDEX_URL "https://www.arterychip.com/en/product/AT32F405.jsp"

#define LED_PORT GPIOC
#define LED_PIN GPIO_PINS_13
