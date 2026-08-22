#include "board_api.h"
#include "stm32f1xx_hal.h"

#ifndef BUILD_NO_TINYUSB
#include "tusb.h"
#endif

#define STM32_UUID ((volatile uint32_t *)UID_BASE)

#if defined(UART_DEV) && CFG_TUSB_DEBUG
static UART_HandleTypeDef uart_handle;
#endif

void board_init(void)
{
  if (!clock_init()) {
    while (1) { }
  }
  SystemCoreClockUpdate();
  board_timer_stop();

  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  GPIO_InitTypeDef gpio = {0};
#ifdef LED_PIN
  gpio.Pin = LED_PIN;
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pull = LED_STATE_ON ? GPIO_PULLDOWN : GPIO_PULLUP;
  gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(LED_PORT, &gpio);
  board_led_write(0);
#endif

#if defined(UART_DEV) && CFG_TUSB_DEBUG
  UART_CLOCK_ENABLE();
  gpio.Pin = UART_TX_PIN;
  gpio.Mode = GPIO_MODE_AF_PP;
  gpio.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(UART_GPIO_PORT, &gpio);
  gpio.Pin = UART_RX_PIN;
  gpio.Mode = GPIO_MODE_INPUT;
  HAL_GPIO_Init(UART_GPIO_PORT, &gpio);

  uart_handle.Instance = UART_DEV;
  uart_handle.Init.BaudRate = BOARD_UART_BAUDRATE;
  uart_handle.Init.WordLength = UART_WORDLENGTH_8B;
  uart_handle.Init.StopBits = UART_STOPBITS_1;
  uart_handle.Init.Parity = UART_PARITY_NONE;
  uart_handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  uart_handle.Init.Mode = UART_MODE_TX_RX;
  HAL_UART_Init(&uart_handle);
#endif
}

void board_dfu_init(void)
{
  GPIO_InitTypeDef gpio = {0};

  // Force a disconnect so USB re-enumerates even after a software reset.
  gpio.Pin = GPIO_PIN_12;
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &gpio);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
  for (volatile uint32_t i = 0; i < (SystemCoreClock / 100U); i++) {
    __NOP();
  }

  gpio.Pin = GPIO_PIN_11 | GPIO_PIN_12;
  gpio.Mode = GPIO_MODE_AF_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &gpio);
  __HAL_RCC_USB_CLK_ENABLE();
}

void board_reset(void)
{
  NVIC_SystemReset();
}

void board_dfu_complete(void)
{
  NVIC_SystemReset();
}

bool board_app_valid(void)
{
  uint32_t const sp = *(uint32_t const *)BOARD_FLASH_APP_START;
  uint32_t const entry = *(uint32_t const *)(BOARD_FLASH_APP_START + 4U);
  uint32_t const flash_end = BOARD_FLASH_ADDR_ZERO + BOARD_FLASH_SIZE;

  return sp >= BOARD_RAM_START && sp <= BOARD_RAM_START + BOARD_RAM_SIZE &&
         (sp & 3U) == 0U &&
         entry >= BOARD_FLASH_APP_START && entry < flash_end &&
         (entry & 1U) != 0U;
}

void board_app_jump(void)
{
  uint32_t const *vectors = (uint32_t const *)BOARD_FLASH_APP_START;
  uint32_t const sp = vectors[0];
  uint32_t const entry = vectors[1];

  __disable_irq();
  __HAL_RCC_USB_FORCE_RESET();
  __HAL_RCC_USB_RELEASE_RESET();
  __HAL_RCC_USB_CLK_DISABLE();

#ifdef LED_PIN
  HAL_GPIO_DeInit(LED_PORT, LED_PIN);
#endif
  HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11 | GPIO_PIN_12);

  HAL_RCC_DeInit();
  HAL_DeInit();
  SysTick->CTRL = 0;
  SysTick->LOAD = 0;
  SysTick->VAL = 0;

  for (uint32_t i = 0; i < 8; i++) {
    NVIC->ICER[i] = 0xFFFFFFFFUL;
    NVIC->ICPR[i] = 0xFFFFFFFFUL;
  }

  SCB->VTOR = BOARD_FLASH_APP_START;
  __set_MSP(sp);
  __DSB();
  __ISB();
  __enable_irq();
  ((void (*)(void))entry)();
  while (1) { }
}

uint8_t board_usb_get_serial(uint8_t serial_id[16])
{
  uint32_t *id = (uint32_t *)(uintptr_t)serial_id;
  id[0] = STM32_UUID[0];
  id[1] = STM32_UUID[1];
  id[2] = STM32_UUID[2];
  return 12;
}

void board_led_write(uint32_t state)
{
#ifdef LED_PIN
  HAL_GPIO_WritePin(LED_PORT, LED_PIN,
                    state ? LED_STATE_ON : (1 - LED_STATE_ON));
#else
  (void)state;
#endif
}

void board_rgb_write(uint8_t const rgb[])
{
  (void)rgb;
}

void board_timer_start(uint32_t ms)
{
  SysTick_Config((SystemCoreClock / 1000U) * ms);
}

void board_timer_stop(void)
{
  SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

void SysTick_Handler(void)
{
  board_timer_handler();
}

int board_uart_write(void const *buf, int len)
{
#if defined(UART_DEV) && CFG_TUSB_DEBUG
  return HAL_UART_Transmit(&uart_handle, (uint8_t *)buf, (uint16_t)len,
                           HAL_MAX_DELAY) == HAL_OK ? len : 0;
#else
  (void)buf;
  (void)len;
  return 0;
#endif
}

#ifndef BUILD_NO_TINYUSB
void USB_HP_CAN1_TX_IRQHandler(void)
{
  tud_int_handler(0);
}

void USB_LP_CAN1_RX0_IRQHandler(void)
{
  tud_int_handler(0);
}

void USBWakeUp_IRQHandler(void)
{
  tud_int_handler(0);
}
#endif

__attribute__((used)) void _init(void)
{
}
