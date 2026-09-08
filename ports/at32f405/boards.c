// SPDX-License-Identifier: MIT
#include "board_api.h"
#include "tusb.h"

void SysTick_Handler(void);
void OTGHS_IRQHandler(void);
void _init(void);

static void clock_init(void) {
  crm_reset();
  flash_psr_set(FLASH_WAIT_CYCLE_6);
  crm_periph_clock_enable(CRM_PWC_PERIPH_CLOCK, TRUE);
  pwc_ldo_output_voltage_set(PWC_LDO_OUTPUT_1V3);
  crm_clock_source_enable(CRM_CLOCK_SOURCE_HEXT, TRUE);
  while (crm_hext_stable_wait() == ERROR) {}
  // Official 216 MHz configuration: 12 MHz * 72 / 1 / 4.
  crm_pll_config(CRM_PLL_SOURCE_HEXT, 72, 1, CRM_PLL_FP_4);
  crm_pllu_div_set(CRM_PLL_FU_18);
  crm_clock_source_enable(CRM_CLOCK_SOURCE_PLL, TRUE);
  while (crm_flag_get(CRM_PLL_STABLE_FLAG) != SET) {}
  crm_ahb_div_set(CRM_AHB_DIV_1);
  crm_apb1_div_set(CRM_APB1_DIV_2);
  crm_apb2_div_set(CRM_APB2_DIV_1);
  crm_auto_step_mode_enable(TRUE);
  crm_sysclk_switch(CRM_SCLK_PLL);
  while (crm_sysclk_switch_status_get() != CRM_SCLK_PLL) {}
  crm_auto_step_mode_enable(FALSE);
  system_core_clock_update();
}

void board_init(void) {
  clock_init();
  board_timer_stop();
  crm_periph_clock_enable(CRM_GPIOC_PERIPH_CLOCK, TRUE);
  gpio_bits_set(LED_PORT, LED_PIN);
  gpio_init_type gpio;
  gpio_default_para_init(&gpio);
  gpio.gpio_pins = LED_PIN;
  gpio.gpio_mode = GPIO_MODE_OUTPUT;
  gpio.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init(LED_PORT, &gpio);
}

void board_dfu_init(void) {
  // Dedicated HS D-/D+ pads are configured by hardware; PA11/12 are unused.
  crm_usb_phy12_clock_select(CRM_USB_PHY12_CLOCK_HEXT_DIV_1);
  crm_periph_clock_enable(CRM_OTGHS_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_OTGHSPHY_PERIPH_CLOCK, TRUE);
  crm_pllu_output_set(TRUE);
  while (crm_flag_get(CRM_PLLU_STABLE_FLAG) != SET) {}
  crm_usb_clock_source_select(CRM_USB_CLOCK_SOURCE_PLLU);
  NVIC_SetPriority(OTGHS_IRQn, 0);
}

void board_reset(void) { NVIC_SystemReset(); }

void board_dfu_complete(void) {
  if (board_flash_complete() && board_app_valid()) {
    TINYUF2_DBL_TAP_REG = DBL_TAP_MAGIC_QUICK_BOOT;
    __DSB();
    NVIC_SystemReset();
  }
  // Keep USB alive for diagnostics after a failed final Flash operation.
  while (1) { tud_task(); }
}

bool board_app_valid(void) {
  uint32_t const* vector = (uint32_t const*) BOARD_FLASH_APP_START;
  uint32_t sp = vector[0];
  uint32_t pc = vector[1];
  return sp > SRAM_BASE && sp <= SRAM_BASE + BOARD_RAM_SIZE && !(sp & 7u) &&
         (pc & 1u) && pc >= BOARD_FLASH_APP_START + 8 && pc < BOARD_FLASH_END;
}

void board_app_jump(void) {
  uint32_t const* vector = (uint32_t const*) BOARD_FLASH_APP_START;
  __disable_irq();
  board_timer_stop();
  SysTick->LOAD = 0;
  SysTick->VAL = 0;
  SCB->ICSR = SCB_ICSR_PENDSTCLR_Msk | SCB_ICSR_PENDSVCLR_Msk;
  for (uint32_t i = 0; i < sizeof(NVIC->ICER) / sizeof(NVIC->ICER[0]); ++i) {
    NVIC->ICER[i] = 0xffffffffu;
    NVIC->ICPR[i] = 0xffffffffu;
  }
  board_led_write(0);
  crm_periph_reset(CRM_GPIOC_PERIPH_RESET, TRUE);
  crm_periph_reset(CRM_GPIOC_PERIPH_RESET, FALSE);
  crm_reset();
  SCB->VTOR = BOARD_FLASH_APP_START;
  __DSB();
  __ISB();
  // No C code may execute after changing MSP. All interrupt sources are off.
  __asm volatile (
    "msr msp, %0\n"
    "msr psp, %0\n"
    "movs r2, #0\n"
    "msr control, r2\n"
    "isb\n"
    "cpsie i\n"
    "bx %1\n" :: "r" (vector[0]), "r" (vector[1]) : "r2", "memory");
  __builtin_unreachable();
}

uint8_t board_usb_get_serial(uint8_t serial_id[16]) {
  volatile uint32_t const* uid = (volatile uint32_t const*) 0x1ffff7e8u;
  for (unsigned i = 0; i < 3; ++i) {
    uint32_t word = uid[i];
    memcpy(serial_id + i * 4, &word, 4);
  }
  return 12;
}
void board_led_write(uint32_t state) {
  gpio_bits_write(LED_PORT, LED_PIN, state ? FALSE : TRUE);
}
void board_rgb_write(uint8_t const rgb[]) { (void) rgb; }
int board_uart_write(void const* buf, int len) { (void) buf; (void) len; return 0; }
void board_timer_start(uint32_t ms) { SysTick_Config(system_core_clock / 1000 * ms); }
void board_timer_stop(void) { SysTick->CTRL = 0; }
void SysTick_Handler(void) { board_timer_handler(); }
void OTGHS_IRQHandler(void) { tud_int_handler(BOARD_TUD_RHPORT); }
void _init(void) {}
