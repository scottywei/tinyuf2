// SPDX-License-Identifier: MIT
// Executes the production Flash and UF2 parser against mapped, fault-injectable
// Flash. This validates protocol/storage logic, not the physical flash/USB IP.
#include <assert.h>
#include <stdio.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <sys/mman.h>
#endif
#include "uf2.h"

static unsigned erases;
static unsigned writes;
static bool locked = true;
static bool fail_erase;
static bool fail_write;

void flash_flag_clear(uint32_t flags) { (void) flags; }
void flash_unlock(void) { locked = false; }
void flash_lock(void) { locked = true; }
int flash_sector_erase(uint32_t addr) {
  assert(!locked);
  assert(addr >= BOARD_FLASH_APP_START && addr < BOARD_FLASH_END);
  assert((addr % BOARD_FLASH_SECTOR_SIZE) == 0);
  if (fail_erase) return 1;
  memset((void*) (uintptr_t) addr, 0xff, BOARD_FLASH_SECTOR_SIZE);
  ++erases;
  return FLASH_OPERATE_DONE;
}
int flash_word_program(uint32_t addr, uint32_t data) {
  assert(!locked);
  assert(addr >= BOARD_FLASH_APP_START && addr + 4 <= BOARD_FLASH_END);
  uint32_t* word = (uint32_t*) (uintptr_t) addr;
  assert(*word == 0xffffffffu);
  if (fail_write) return 1;
  *word = data;
  ++writes;
  return FLASH_OPERATE_DONE;
}

static void reset_test(WriteState* state) {
  memset((void*) BOARD_FLASH_ADDR_ZERO, 0xa5, BOARD_FLASH_SIZE);
  memset(state, 0, sizeof(*state));
  erases = writes = 0;
  fail_erase = fail_write = false;
  board_flash_init();
  uf2_init();
}

static UF2_Block block(unsigned index, unsigned count) {
  UF2_Block b = {0};
  b.magicStart0 = UF2_MAGIC_START0;
  b.magicStart1 = UF2_MAGIC_START1;
  b.magicEnd = UF2_MAGIC_END;
  b.flags = UF2_FLAG_FAMILYID;
  b.familyID = BOARD_UF2_FAMILY_ID;
  b.targetAddr = BOARD_FLASH_APP_START + index * 256;
  b.payloadSize = 256;
  b.blockNo = index;
  b.numBlocks = count;
  memset(b.data, (int) (index + 1), 256);
  if (!index) {
    uint32_t vector[] = {SRAM_BASE + BOARD_RAM_SIZE - 8, BOARD_FLASH_APP_START + 0x81};
    memcpy(b.data, vector, sizeof(vector));
  }
  return b;
}
static int send(UF2_Block* b, WriteState* state) {
  int ret = uf2_write_block(0, (uint8_t*) b, state);
  assert(locked);
  // The bootloader must survive every test, including malformed requests.
  for (unsigned i = 0; i < BOARD_FLASH_APP_START - BOARD_FLASH_ADDR_ZERO; ++i)
    assert(((uint8_t*) BOARD_FLASH_ADDR_ZERO)[i] == 0xa5);
  return ret;
}

int main(void) {
#ifdef _WIN32
  void* flash = VirtualAlloc((void*) BOARD_FLASH_ADDR_ZERO, BOARD_FLASH_SIZE,
                            MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
  void* flash = mmap((void*) BOARD_FLASH_ADDR_ZERO, BOARD_FLASH_SIZE, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
#endif
  assert(flash == (void*) BOARD_FLASH_ADDR_ZERO);
  WriteState state;
  reset_test(&state);

  // CURRENT.UF2 exports only the application and declares the correct count.
  unsigned expected = (BOARD_FLASH_END - BOARD_FLASH_APP_START) / 256;
  bool found = false;
  for (unsigned sector = 0; sector < CFG_UF2_NUM_BLOCKS; ++sector) {
    UF2_Block exported = {0};
    uf2_read_block(sector, (uint8_t*) &exported);
    if (exported.magicStart0 != UF2_MAGIC_START0) continue;
    assert(exported.blockNo == 0 && exported.numBlocks == expected);
    uf2_read_block(sector + expected - 1, (uint8_t*) &exported);
    assert(exported.blockNo == expected - 1);
    assert(exported.targetAddr + exported.payloadSize == BOARD_FLASH_END);
    found = true;
    break;
  }
  assert(found);

  // Reverse order across two sectors, duplicate blocks, commit only at end.
  for (int i = 15; i >= 0; --i) {
    UF2_Block b = block((unsigned) i, 16);
    assert(send(&b, &state) == 512);
    unsigned before = writes;
    assert(send(&b, &state) == 512);
    assert(writes == before);
    assert(state.numWritten == (unsigned) (16 - i));
    if (i) assert(*(uint32_t*) BOARD_FLASH_APP_START == 0xffffffffu);
  }
  assert(board_flash_complete());
  assert(erases == 4096 / BOARD_FLASH_SECTOR_SIZE);
  for (unsigned i = 0; i < 16; ++i) {
    UF2_Block b = block(i, 16);
    assert(memcmp((void*) (uintptr_t) b.targetAddr, b.data, 256) == 0);
  }

  // Missing final block must never create an executable image.
  reset_test(&state);
  UF2_Block b = block(0, 2);
  assert(send(&b, &state) == 512);
  assert(!board_flash_complete());
  assert(*(uint32_t*) BOARD_FLASH_APP_START == 0xffffffffu);

  // Wrong family and non-Flash metadata do not erase existing firmware.
  reset_test(&state);
  b = block(0, 1); b.familyID ^= 1;
  assert(send(&b, &state) == -1 && erases == 0);
  b = block(0, 1); b.flags |= UF2_FLAG_NOFLASH;
  assert(send(&b, &state) == -1 && erases == 0);

  // Malformed size, counts, address overflow, boot region and Flash end.
  for (unsigned test = 0; test < 9; ++test) {
    reset_test(&state); b = block(0, 1);
    switch (test) {
      case 0: b.payloadSize = 257; break;
      case 1: b.payloadSize = 0xffffffffu; break;
      case 2: b.numBlocks = 0; break;
      case 3: b.blockNo = 1; break;
      case 4: b.targetAddr = BOARD_FLASH_ADDR_ZERO; break;
      case 5: b.targetAddr = BOARD_FLASH_END; break;
      case 6: b.targetAddr = 0xffffff00u; break;
      case 7: ++b.targetAddr; break;
      case 8: b.numBlocks = 0xffffffffu; break;
    }
    assert(send(&b, &state) == -1 && state.aborted && erases == 0);
  }

  // Underlying port rejects unsafe calls independently of the UF2 parser.
  reset_test(&state); b = block(0, 1);
  assert(!board_flash_write(BOARD_FLASH_ADDR_ZERO, b.data, 256));
  assert(!board_flash_write(BOARD_FLASH_END, b.data, 256));
  assert(!board_flash_write(0xffffff00u, b.data, 256));
  assert(!board_flash_write(BOARD_FLASH_APP_START, b.data, 512));
  assert(erases == 0);

  // Hardware faults cannot advance completion or install valid vectors.
  reset_test(&state); b = block(0, 1); fail_erase = true;
  assert(send(&b, &state) == -1 && state.numWritten == 0);
  assert(!board_flash_complete());
  reset_test(&state); fail_write = true;
  assert(send(&b, &state) == -1 && state.numWritten == 0);
  assert(*(uint32_t*) BOARD_FLASH_APP_START == 0xffffffffu);

  // Final vector programming failure and invalid reset entry.
  reset_test(&state); b = block(0, 2);
  assert(send(&b, &state) == 512);
  fail_write = true; board_flash_flush();
  assert(!board_flash_complete());
  assert(*(uint32_t*) BOARD_FLASH_APP_START == 0xffffffffu);
  reset_test(&state); b = block(0, 1); b.data[4] &= 0xfe;
  assert(send(&b, &state) == 512 && !board_flash_complete());
  assert(*(uint32_t*) BOARD_FLASH_APP_START == 0xffffffffu);

  // Conflicting duplicates and changing transaction lengths abort.
  reset_test(&state); b = block(0, 2); assert(send(&b, &state) == 512);
  b.data[8] ^= 1;
  assert(send(&b, &state) == -1 && state.aborted);
  reset_test(&state); b = block(0, 2); assert(send(&b, &state) == 512);
  b = block(1, 3);
  assert(send(&b, &state) == -1 && state.aborted);
  puts("AT32F405 Flash/UF2 tests passed");
  return 0;
}
