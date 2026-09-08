// SPDX-License-Identifier: MIT
// Minimal application to verify UF2 upload, relocated vectors and reset entry.
#include "at32f402_405.h"
#include "board.h"

static volatile uint32_t ticks;
void SysTick_Handler(void);
void _init(void);
void SysTick_Handler(void) { ++ticks; }
void _init(void) {}

int main(void) {
  // The vendor SystemInit resets VTOR to Flash base; relocate it before IRQs.
  SCB->VTOR = 0x08008000u;
  __DSB();
  __ISB();
  system_core_clock_update();
  crm_periph_clock_enable(CRM_GPIOC_PERIPH_CLOCK, TRUE);
  gpio_init_type gpio;
  gpio_default_para_init(&gpio);
  gpio.gpio_pins = LED_PIN;
  gpio.gpio_mode = GPIO_MODE_OUTPUT;
  gpio.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init(LED_PORT, &gpio);
  SysTick_Config(system_core_clock / 1000);
  while (1) {
    gpio_bits_toggle(LED_PORT, LED_PIN);
    uint32_t start = ticks;
    while (ticks - start < 500) {}
  }
}
