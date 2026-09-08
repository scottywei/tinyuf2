// SPDX-License-Identifier: MIT
#include "board_api.h"

#define APP_SECTORS ((BOARD_FLASH_END - BOARD_FLASH_APP_START) / BOARD_FLASH_SECTOR_SIZE)
static uint8_t erased[APP_SECTORS];
static uint32_t pending_vector[2];
static bool have_vector;
static bool started;
static bool failed;
static bool complete;
static uint32_t written_end;

static bool erase_sector(uint32_t index) {
  flash_flag_clear(FLASH_ODF_FLAG | FLASH_PRGMERR_FLAG | FLASH_EPPERR_FLAG);
  if (flash_sector_erase(BOARD_FLASH_APP_START + index * BOARD_FLASH_SECTOR_SIZE) != FLASH_OPERATE_DONE) {
    failed = true;
    return false;
  }
  erased[index] = 1;
  return true;
}

void board_flash_init(void) {
  memset(erased, 0, sizeof(erased));
  memset(pending_vector, 0xff, sizeof(pending_vector));
  have_vector = started = failed = complete = false;
  written_end = BOARD_FLASH_APP_START;
}
uint32_t board_flash_size(void) { return BOARD_FLASH_SIZE; }
void board_flash_read(uint32_t addr, void* buffer, uint32_t len) {
  if (addr >= BOARD_FLASH_APP_START && addr <= BOARD_FLASH_END && len <= BOARD_FLASH_END - addr) {
    memcpy(buffer, (void const*) (uintptr_t) addr, len);
  } else {
    memset(buffer, 0, len);
  }
}

static bool program_word(uint32_t addr, uint32_t word) {
  uint32_t old = *(uint32_t const*) (uintptr_t) addr;
  if (old == word) return true; // Duplicate UF2 blocks are harmless.
  if (old != 0xffffffffu) return false;
  flash_flag_clear(FLASH_ODF_FLAG | FLASH_PRGMERR_FLAG | FLASH_EPPERR_FLAG);
  return flash_word_program(addr, word) == FLASH_OPERATE_DONE &&
         *(volatile uint32_t const*) (uintptr_t) addr == word;
}

bool board_flash_write(uint32_t addr, void const* data, uint32_t len) {
  // Use subtraction to avoid overflow. Only standard 256-byte UF2 is accepted.
  if (failed || len != 256 || (addr & 255u) ||
      addr < BOARD_FLASH_APP_START || addr >= BOARD_FLASH_END || len > BOARD_FLASH_END - addr) return false;
  if (complete) return memcmp((void const*) (uintptr_t) addr, data, len) == 0;

  flash_unlock();
  // Invalidate the old image before changing any other sector, even if block 0
  // arrives last. Leave the new vectors erased until the entire file is received.
  if (!started) {
    started = true;
    if (!erase_sector(0)) goto done;
  }
  uint32_t sector = (addr - BOARD_FLASH_APP_START) / BOARD_FLASH_SECTOR_SIZE;
  if (!erased[sector] && !erase_sector(sector)) goto done;

  for (uint32_t offset = 0; offset < len; offset += 4) {
    uint32_t word;
    memcpy(&word, (uint8_t const*) data + offset, sizeof(word));
    if (addr + offset < BOARD_FLASH_APP_START + 8) {
      unsigned index = offset / 4;
      if (have_vector && pending_vector[index] != word) { failed = true; break; }
      pending_vector[index] = word;
    } else if (!program_word(addr + offset, word)) {
      failed = true;
      break;
    }
  }
  if (addr == BOARD_FLASH_APP_START && !failed) have_vector = true;
  if (!failed && addr + len > written_end) written_end = addr + len;
done:
  flash_lock();
  return !failed;
}

void board_flash_flush(void) {
  if (complete || failed) return;
  uint32_t sp = pending_vector[0], pc = pending_vector[1];
  if (!have_vector || sp <= SRAM_BASE || sp > SRAM_BASE + BOARD_RAM_SIZE || (sp & 7u) ||
      !(pc & 1u) || pc < BOARD_FLASH_APP_START + 8 || pc >= written_end) {
    failed = true;
    return;
  }
  flash_unlock();
  // Commit SP last: a reset before this point still sees an invalid application.
  complete = program_word(BOARD_FLASH_APP_START + 4, pc) &&
             program_word(BOARD_FLASH_APP_START, sp);
  failed = !complete;
  flash_lock();
}
bool board_flash_complete(void) { return complete && !failed; }

void board_flash_erase_app(void) {
  board_flash_init();
  flash_unlock();
  for (uint32_t i = 0; i < APP_SECTORS; ++i) {
    if (!erase_sector(i)) break;
  }
  flash_lock();
}
bool board_flash_protect_bootloader(bool protect) {
  // UF2 address checks protect the boot region. Do not change option bytes.
  return !protect;
}
