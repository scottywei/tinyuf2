// SPDX-License-Identifier: MIT
// Compile the real parser with a fake flash backend to check protocol boundaries.
#include <assert.h>
#include <stdio.h>
#include "uf2.h"

static unsigned writes, flushes;
static bool fail_write;
uint32_t board_flash_size(void) { return BOARD_FLASH_SIZE; }
void board_flash_read(uint32_t addr, void* data, uint32_t len)
{
  (void)addr;
  memset(data, 0xff, len);
}
bool board_flash_write(uint32_t addr, void const* data, uint32_t len)
{
  (void)addr; (void)data;
  assert(len == 256);
  writes++;
  return !fail_write;
}
void board_flash_flush(void) { flushes++; }

static UF2_Block block(uint32_t start, uint32_t index, uint32_t count)
{
  UF2_Block b = {0};
  b.magicStart0 = UF2_MAGIC_START0;
  b.magicStart1 = UF2_MAGIC_START1;
  b.magicEnd = UF2_MAGIC_END;
  b.flags = UF2_FLAG_FAMILYID;
  b.familyID = BOARD_UF2_FAMILY_ID;
  b.targetAddr = start + index * 256;
  b.blockNo = index;
  b.numBlocks = count;
  b.payloadSize = 256;
  return b;
}

static int send(UF2_Block* b, WriteState* s)
{
  return uf2_write_block(0, (uint8_t*)b, s);
}

static void rejected(UF2_Block b)
{
  WriteState s = {0};
  unsigned const before = writes;
  assert(send(&b, &s) == -1);
  assert(s.aborted && writes == before && s.numWritten == 0);
}

int main(void)
{
  // Out of order and duplicate blocks count only once; flush after all blocks.
  WriteState s = {0};
  UF2_Block b = block(BOARD_NOR_FLASH_ADDR, 2, 3);
  assert(send(&b, &s) == 512 && s.numWritten == 1 && flushes == 0);
  assert(send(&b, &s) == 512 && s.numWritten == 1 && flushes == 0);
  b = block(BOARD_NOR_FLASH_ADDR, 0, 3);
  assert(send(&b, &s) == 512 && s.numWritten == 2 && flushes == 0);
  b = block(BOARD_NOR_FLASH_ADDR, 1, 3);
  assert(send(&b, &s) == 512 && s.numWritten == 3 && flushes == 1);

  // Full 4 MiB images and a sector at the very end are accepted.
  s = (WriteState){0};
  b = block(BOARD_NOR_FLASH_ADDR, 16383, 16384);
  assert(send(&b, &s) == 512);
  s = (WriteState){0};
  b = block(BOARD_NOR_FLASH_ADDR + BOARD_NOR_FLASH_SIZE - 4096, 15, 16);
  assert(send(&b, &s) == 512);

  rejected(block(BOARD_NOR_FLASH_ADDR, 0, 16385));
  rejected(block(BOARD_NOR_FLASH_ADDR + BOARD_NOR_FLASH_SIZE, 0, 1));
  rejected(block(BOARD_NOR_FLASH_ADDR + 256, 0, 1));
  rejected(block(BOARD_FLASH_ADDR_ZERO, 0, 1));
  rejected(block(BOARD_FLASH_APP_START, 0, 1793));
  rejected(block(0xffffff00u, 0, 2));
  rejected(block(BOARD_NOR_FLASH_ADDR, 1, 1));
  rejected(block(BOARD_NOR_FLASH_ADDR, 0, 0));
  b = block(BOARD_NOR_FLASH_ADDR, 0, 1);
  b.payloadSize = 476;
  rejected(b);
  b.payloadSize = 0;
  rejected(b);
  b = block(BOARD_NOR_FLASH_ADDR, 0, 1);
  b.flags |= 0x1000;
  rejected(b);

  // Wrong families/filesystem noise do not poison an update.
  s = (WriteState){0};
  b = block(BOARD_NOR_FLASH_ADDR, 0, 2);
  b.familyID = 0;
  assert(send(&b, &s) == -1 && !s.aborted && !s.numWritten);
  b.familyID = BOARD_UF2_FAMILY_ID;
  assert(send(&b, &s) == 512);
  b = block(BOARD_FLASH_APP_START, 1, 2);
  assert(send(&b, &s) == -1 && s.aborted); // mixed destinations
  b = block(BOARD_NOR_FLASH_ADDR, 1, 2);
  assert(send(&b, &s) == -1); // aborted state cannot recover silently

  s = (WriteState){0};
  b = block(BOARD_FLASH_APP_START, 1791, 1792);
  assert(send(&b, &s) == 512); // original internal application range
  s = (WriteState){0};
  b = block(BOARD_NOR_FLASH_ADDR, 0, 2);
  assert(send(&b, &s) == 512);
  b = block(BOARD_NOR_FLASH_ADDR, 1, 3);
  assert(send(&b, &s) == -1 && s.aborted); // changed count

  s = (WriteState){0};
  fail_write = true;
  b = block(BOARD_NOR_FLASH_ADDR, 0, 1);
  unsigned const before = flushes;
  assert(send(&b, &s) == -1 && s.aborted && !s.numWritten && flushes == before);
  puts("NOR UF2 parser tests passed");
  return 0;
}
