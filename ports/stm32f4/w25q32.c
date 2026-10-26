// SPDX-License-Identifier: MIT
// W25Q32JV standard SPI, mode 0. Each sector is erased once per boot/transfer.
#include <string.h>
#include "boards.h"
#include "w25q32.h"

#define NOR_PAGE_SIZE 256u
#define NOR_SPI BOARD_NOR_SPI

static bool ready;
static uint8_t erased[BOARD_NOR_FLASH_SIZE / BOARD_NOR_SECTOR_SIZE / 8];
static uint8_t written[BOARD_NOR_FLASH_SIZE / NOR_PAGE_SIZE / 8];

// The port uses SysTick for its LED timer, not HAL_IncTick(). Use the Cortex-M4
// cycle counter so timeouts work before USB starts and at every LED interval.
static bool expired(uint32_t start, uint32_t ms)
{
  return (uint32_t)(DWT->CYCCNT - start) >= (SystemCoreClock / 1000u) * ms;
}

static void delay_ms(uint32_t ms)
{
  uint32_t const start = DWT->CYCCNT;
  while (!expired(start, ms)) {}
}

static void select_flash(bool selected)
{
  BOARD_NOR_PORT->BSRR = selected ? (uint32_t)BOARD_NOR_CS_PIN << 16 : BOARD_NOR_CS_PIN;
}

static bool exchange(uint8_t tx, uint8_t* rx)
{
  uint32_t const start = DWT->CYCCNT;
  while (!(NOR_SPI->SR & SPI_SR_TXE)) {
    if (expired(start, 5)) return false;
  }
  *(__IO uint8_t*)&NOR_SPI->DR = tx;
  while (!(NOR_SPI->SR & SPI_SR_RXNE)) {
    if (expired(start, 5)) return false;
  }
  uint8_t const value = *(__IO uint8_t*)&NOR_SPI->DR;
  if (rx) *rx = value;
  while (NOR_SPI->SR & SPI_SR_BSY) {
    if (expired(start, 5)) return false;
  }
  return true;
}

static bool command(uint8_t cmd, uint8_t* result, uint32_t count)
{
  select_flash(true);
  bool ok = exchange(cmd, NULL);
  for (uint32_t i = 0; ok && i < count; i++) ok = exchange(0xff, result + i);
  select_flash(false);
  return ok;
}

static bool wait_ready(uint32_t timeout_ms)
{
  uint32_t const start = DWT->CYCCNT;
  do {
    uint8_t status;
    if (!command(0x05, &status, 1)) return false;
    if (!(status & 1)) return true;
  } while (!expired(start, timeout_ms));
  return false;
}

static bool write_enable(void)
{
  uint8_t status;
  return command(0x06, NULL, 0) && command(0x05, &status, 1) && (status & 2);
}

static bool address_command(uint8_t cmd, uint32_t addr)
{
  return exchange(cmd, NULL) && exchange(addr >> 16, NULL) &&
         exchange(addr >> 8, NULL) && exchange(addr, NULL);
}

static bool read_data(uint32_t offset, uint8_t* data, uint32_t len)
{
  select_flash(true);
  bool ok = address_command(0x03, offset);
  for (uint32_t i = 0; ok && i < len; i++) ok = exchange(0xff, data + i);
  select_flash(false);
  return ok;
}

void w25q32_init(void)
{
  ready = false;
  memset(erased, 0, sizeof(erased));
  memset(written, 0, sizeof(written));
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_SPI2_CLK_ENABLE();
  __HAL_RCC_SPI2_FORCE_RESET();
  __HAL_RCC_SPI2_RELEASE_RESET();

  select_flash(false);
  GPIO_InitTypeDef gpio = {0};
  gpio.Pin = BOARD_NOR_CS_PIN;
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pull = GPIO_PULLUP;
  gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(BOARD_NOR_PORT, &gpio);
  gpio.Pin = BOARD_NOR_SCK_PIN | BOARD_NOR_MISO_PIN | BOARD_NOR_MOSI_PIN;
  gpio.Mode = GPIO_MODE_AF_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(BOARD_NOR_PORT, &gpio);

  // APB1 = 42 MHz; /8 gives a conservative 5.25 MHz SPI clock.
  NOR_SPI->CR1 = SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_BR_1;
  NOR_SPI->CR1 |= SPI_CR1_SPE;
  delay_ms(3);
  if (!command(0xab, NULL, 0)) return; // release deep power-down
  delay_ms(1);
  if (!wait_ready(500) || !command(0x66, NULL, 0) || !command(0x99, NULL, 0)) return;
  delay_ms(1);
  uint8_t id[3], sr1, sr2, sr3;
  if (!command(0x9f, id, sizeof(id)) ||
      id[0] != 0xef || id[1] != 0x40 || id[2] != 0x16) return;
  if (!command(0x05, &sr1, 1) || !command(0x35, &sr2, 1) ||
      !command(0x15, &sr3, 1)) return;
  // Refuse protected devices. Never silently rewrite protection/QE settings.
  if ((sr1 & 0x1c) || (sr2 & 0x40) || (sr3 & 0x04)) return;
  ready = true;
}

bool w25q32_read(uint32_t offset, void* data, uint32_t len)
{
  return ready && offset < BOARD_NOR_FLASH_SIZE && len <= BOARD_NOR_FLASH_SIZE - offset &&
         read_data(offset, data, len);
}

bool w25q32_write(uint32_t offset, void const* data, uint32_t len)
{
  if (!ready || len != NOR_PAGE_SIZE || (offset % NOR_PAGE_SIZE) ||
      offset >= BOARD_NOR_FLASH_SIZE || len > BOARD_NOR_FLASH_SIZE - offset) return false;
  uint32_t const sector = offset / BOARD_NOR_SECTOR_SIZE;
  uint8_t const mask = 1u << (sector % 8);
  bool ok = true;
  if (!(erased[sector / 8] & mask)) {
    ok = write_enable();
    if (ok) {
      select_flash(true);
      ok = address_command(0x20, offset & ~(BOARD_NOR_SECTOR_SIZE - 1));
      select_flash(false);
      ok = ok && wait_ready(500);
    }
    if (ok) erased[sector / 8] |= mask;
  }

  uint8_t verify[NOR_PAGE_SIZE];
  ok = ok && read_data(offset, verify, len);
  uint32_t const page = offset / NOR_PAGE_SIZE;
  uint8_t const page_mask = 1u << (page % 8);
  if (ok && (written[page / 8] & page_mask)) {
    if (!memcmp(verify, data, len)) return true;
    ready = false; // reject every conflicting duplicate, even a 1-to-0 change
    return false;
  }
  if (ok && !memcmp(verify, data, len)) {
    written[page / 8] |= page_mask;
    return true;
  }
  uint8_t const* src = data;
  for (uint32_t i = 0; ok && i < len; i++) {
    if ((verify[i] & src[i]) != src[i]) ok = false; // conflicting duplicate
  }
  ok = ok && write_enable();
  if (ok) {
    select_flash(true);
    ok = address_command(0x02, offset);
    for (uint32_t i = 0; ok && i < len; i++) ok = exchange(src[i], NULL);
    select_flash(false);
    ok = ok && wait_ready(10) && read_data(offset, verify, len) && !memcmp(verify, data, len);
  }
  // A failed operation must not be retried as though its sector were intact.
  if (!ok) ready = false;
  else written[page / 8] |= page_mask;
  return ok;
}
