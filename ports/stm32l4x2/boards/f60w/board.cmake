set(JLINK_DEVICE stm32l412cb)

function(update_board TARGET)
  target_sources(${TARGET} PUBLIC
    ${ST_CMSIS}/Source/Templates/gcc/startup_stm32l412xx.s
    ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../../board_clock.c
    )
  target_compile_definitions(${TARGET} PUBLIC
    STM32L412xx
    )
endfunction()
