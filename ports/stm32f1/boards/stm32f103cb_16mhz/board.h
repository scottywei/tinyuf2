#ifndef BOARD_H_
#define BOARD_H_

// BIOI CAP45 Caps Lock LED: PA3, active high. PA11/PA12 are fixed USB pins.
#define LED_PORT              GPIOA
#define LED_PIN               GPIO_PIN_3
#define LED_STATE_ON          1

#define NEOPIXEL_NUMBER       0

#define BOARD_FLASH_SIZE      (128UL * 1024UL)

// Replace the development VID/PID before distributing a product.
#define USB_VID               0xCAFE
#define USB_PID               0xF103
#define USB_MANUFACTURER      "STM32"
#define USB_PRODUCT           "STM32F103CB"

#define UF2_PRODUCT_NAME      USB_PRODUCT
#define UF2_BOARD_ID          "BIOI_UF2"
#define UF2_VOLUME_LABEL      "BIOI_UF2"
#define UF2_INDEX_URL         "https://github.com/adafruit/tinyuf2"

static inline bool clock_init(void)
{
  RCC_OscInitTypeDef osc = {0};
  RCC_ClkInitTypeDef clk = {0};
  RCC_PeriphCLKInitTypeDef periph = {0};

  // 16 MHz HSE / 2 * 9 = 72 MHz SYSCLK; 72 MHz / 1.5 = 48 MHz USB.
  osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  osc.HSEState = RCC_HSE_ON;
  osc.HSEPredivValue = RCC_HSE_PREDIV_DIV2;
  osc.PLL.PLLState = RCC_PLL_ON;
  osc.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  osc.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&osc) != HAL_OK) return false;

  periph.PeriphClockSelection = RCC_PERIPHCLK_USB;
  periph.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&periph) != HAL_OK) return false;

  clk.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
  clk.APB1CLKDivider = RCC_HCLK_DIV2;
  clk.APB2CLKDivider = RCC_HCLK_DIV1;
  return HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_2) == HAL_OK;
}

#endif
