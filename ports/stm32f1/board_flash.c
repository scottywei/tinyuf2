#include "board_api.h"

#define FLASH_BASE_ADDR BOARD_FLASH_ADDR_ZERO

enum {
  PAGE_COUNT = BOARD_FLASH_SIZE / BOARD_PAGE_SIZE,
  APP_PAGE_FIRST = (BOARD_FLASH_APP_START - FLASH_BASE_ADDR) / BOARD_PAGE_SIZE
};

static uint8_t erased_pages[PAGE_COUNT];

static bool address_range_valid(uint32_t addr, uint32_t len)
{
  uint32_t const flash_end = FLASH_BASE_ADDR + BOARD_FLASH_SIZE;
  return len != 0U && addr >= BOARD_FLASH_APP_START && addr < flash_end &&
         len <= flash_end - addr;
}

static bool page_is_blank(uint32_t page_addr)
{
  for (uint32_t offset = 0; offset < BOARD_PAGE_SIZE; offset += 4U) {
    if (*(uint32_t const *)(page_addr + offset) != 0xFFFFFFFFUL) return false;
  }
  return true;
}

static bool erase_page_for_address(uint32_t addr)
{
  uint32_t const page = (addr - FLASH_BASE_ADDR) / BOARD_PAGE_SIZE;
  uint32_t const page_addr = FLASH_BASE_ADDR + page * BOARD_PAGE_SIZE;

  if (page >= PAGE_COUNT || page < APP_PAGE_FIRST) return false;
  if (erased_pages[page]) return true;

  if (page_is_blank(page_addr)) {
    erased_pages[page] = 1;
    return true;
  }

  FLASH_EraseInitTypeDef erase = {0};
  uint32_t page_error = 0;
  erase.TypeErase = FLASH_TYPEERASE_PAGES;
  erase.PageAddress = page_addr;
  erase.NbPages = 1;
  bool const ok = HAL_FLASHEx_Erase(&erase, &page_error) == HAL_OK &&
                  page_error == 0xFFFFFFFFUL;
  if (ok) erased_pages[page] = 1;
  return ok;
}

void board_flash_init(void)
{
  memset(erased_pages, 0, sizeof(erased_pages));
}

uint32_t board_flash_size(void)
{
  return BOARD_FLASH_SIZE;
}

void board_flash_read(uint32_t addr, void *buffer, uint32_t len)
{
  memcpy(buffer, (void const *)addr, len);
}

bool board_flash_write(uint32_t addr, void const *data, uint32_t len)
{
  if (!address_range_valid(addr, len) || (addr & 1U) || (len & 1U)) return false;

  uint8_t const *src = (uint8_t const *)data;
  bool ok = HAL_FLASH_Unlock() == HAL_OK;

  for (uint32_t offset = 0; ok && offset < len; offset += 2U) {
    uint32_t const dst = addr + offset;
    ok = erase_page_for_address(dst);
    if (ok) {
      uint16_t value;
      memcpy(&value, src + offset, sizeof(value));
      if (*(uint16_t const *)dst != value) {
        ok = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, dst, value) == HAL_OK;
      }
    }
  }

  HAL_FLASH_Lock();
  return ok && memcmp((void const *)addr, data, len) == 0;
}

void board_flash_flush(void)
{
}

void board_flash_erase_app(void)
{
  if (HAL_FLASH_Unlock() != HAL_OK) return;

  FLASH_EraseInitTypeDef erase = {0};
  uint32_t page_error = 0;
  erase.TypeErase = FLASH_TYPEERASE_PAGES;
  erase.PageAddress = BOARD_FLASH_APP_START;
  erase.NbPages = PAGE_COUNT - APP_PAGE_FIRST;
  HAL_FLASHEx_Erase(&erase, &page_error);
  HAL_FLASH_Lock();
  memset(erased_pages, 0, sizeof(erased_pages));
}

bool board_flash_protect_bootloader(bool protect)
{
  (void)protect;
  return false;
}
