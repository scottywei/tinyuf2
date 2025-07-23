#include "board_api.h"
#include "stm32l4xx_hal.h"

void Error_Handler(void) {
    for (;;) {
    }
}

#define HAL_CHECK(x) \
    if (x != HAL_OK) Error_Handler()

//--------------------------------------------------------------------+
// RCC Clock
//--------------------------------------------------------------------+
void clock_init(void) {
    // enable the debugger while sleeping. Todo move somewhere more central (kind of generally useful in a debug build)
    // SET_BIT(DBGMCU->CR, DBGMCU_CR_DBG_SLEEP);

    //  Set tick interrupt priority, default HAL value is intentionally invalid
    //  Without this, USB does not function.
    HAL_InitTick((1UL << __NVIC_PRIO_BITS) - 1UL);

    RCC_ClkInitTypeDef          RCC_ClkInitStruct       = {0};
    RCC_OscInitTypeDef          RCC_OscInitStruct       = {0};
    RCC_CRSInitTypeDef          RCC_CRSInitStruct       = {0};
    RCC_PeriphCLKInitTypeDef    PeriphClkInitStruct     = {0};

    /* Enable Power Control clock */
    // Sets the drive strength of 32kHz external crystal, in line with calculations specified in ST AN2867 sections 3.3, 3.4, and STM32L4 datasheet DS12023 Table 58. LSE oscillator characteristics.
    // The drive strength RCC_LSEDRIVE_LOW is marginal for the 32kHz crystal oscillator stability, and RCC_LSEDRIVE_MEDIUMLOW meets the calculated drive strength with a small margin for parasitic capacitance.
    // __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_MEDIUMLOW);
    __HAL_RCC_PWR_CLK_ENABLE();

    if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK) {
        Error_Handler();
    }

    /* Activate PLL with MSI , stabilizied via PLL by LSE */
    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI48 | RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
    RCC_OscInitStruct.HSI48State          = RCC_HSI48_ON;
    RCC_OscInitStruct.MSIState            = RCC_MSI_ON;
    RCC_OscInitStruct.MSIClockRange       = RCC_MSIRANGE_6;  // 备用时钟设为 4MHz
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM            = 1;
    RCC_OscInitStruct.PLL.PLLN            = 10;
    RCC_OscInitStruct.PLL.PLLR            = RCC_PLLR_DIV2;

    HAL_CHECK(HAL_RCC_OscConfig(&RCC_OscInitStruct));

    /* Enable MSI Auto-calibration through LSE */
    HAL_RCCEx_EnableMSIPLLMode();

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
       clocks dividers */
    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4);

    /** Enable the SYSCFG APB clock
     */
    __HAL_RCC_CRS_CLK_ENABLE();

    /** Configures CRS
     */
    RCC_CRSInitStruct.Prescaler = RCC_CRS_SYNC_DIV1;
    RCC_CRSInitStruct.Source = RCC_CRS_SYNC_SOURCE_USB;
    RCC_CRSInitStruct.Polarity = RCC_CRS_SYNC_POLARITY_RISING;
    RCC_CRSInitStruct.ReloadValue = __HAL_RCC_CRS_RELOADVALUE_CALCULATE(48000000,1000);
    RCC_CRSInitStruct.ErrorLimitValue = 34;
    RCC_CRSInitStruct.HSI48CalibrationValue = 32;

    HAL_RCCEx_CRSConfig(&RCC_CRSInitStruct);

    // Wait for CRS synchronization
    uint16_t timeout = 65535; // Approx 0.5s timeout assuming 80MHz clock
    while (!(CRS->ISR & CRS_ISR_SYNCOKF) && timeout--) {}
    if (timeout == 0) {
        Error_Handler();
    }

    /* Select HSI48 output as USB clock source */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USB | RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.UsbClockSelection    = RCC_USBCLKSOURCE_HSI48;
    PeriphClkInitStruct.RTCClockSelection    = RCC_RTCCLKSOURCE_LSI;
    HAL_CHECK(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct));

    /* Select PLL output as UART clock source */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART1;
    PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    HAL_CHECK(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct));
}