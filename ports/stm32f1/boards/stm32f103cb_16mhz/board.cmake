set(JLINK_DEVICE STM32F103CB)

function(update_board TARGET)
  target_sources(${TARGET} PUBLIC
    ${ST_CMSIS}/Source/Templates/gcc/startup_stm32f103xb.s
    )
  target_compile_definitions(${TARGET} PUBLIC
    STM32F103xB
    HSE_VALUE=16000000U
    )
endfunction()
