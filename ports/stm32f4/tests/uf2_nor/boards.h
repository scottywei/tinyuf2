// SPDX-License-Identifier: MIT
// Host-test configuration; deliberately contains no STM32 headers.
#define BOARD_FLASH_ADDR_ZERO 0x08000000u
#define BOARD_FLASH_APP_START 0x08010000u
#define BOARD_FLASH_SIZE (512u * 1024)
#define BOARD_NOR_FLASH_ADDR 0x90000000u
#define BOARD_NOR_FLASH_SIZE (4u * 1024 * 1024)
#define BOARD_NOR_SECTOR_SIZE 4096u
#define BOARD_UF2_FAMILY_ID 0x57755a57u
#define CFG_UF2_FLASH_SIZE BOARD_NOR_FLASH_SIZE
#define UF2_PRODUCT_NAME "NOR test"
#define UF2_BOARD_ID "NOR test"
#define UF2_VOLUME_LABEL "NORTEST"
#define UF2_INDEX_URL "https://example.com"
#define UF2_VERSION "test"
