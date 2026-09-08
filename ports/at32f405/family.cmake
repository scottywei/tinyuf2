include_guard(GLOBAL)
set(UF2_FAMILY_ID 0x6b1f405c) # Private, unregistered ID; see README.
set(AT32_SDK ${TOP}/lib/mcu/artery/at32f402_405 CACHE PATH "Official AT32F402/405 SDK")
include(${CMAKE_CURRENT_LIST_DIR}/boards/${BOARD}/board.cmake)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
set(CMAKE_SYSTEM_PROCESSOR cortex-m4 CACHE INTERNAL "System Processor")
set(CMAKE_TOOLCHAIN_FILE ${TOP}/cmake/toolchain/arm_${TOOLCHAIN}.cmake)

function(family_add_board_target BOARD_TARGET)
  if(TARGET ${BOARD_TARGET})
    return()
  endif()
  if(NOT EXISTS "${AT32_SDK}/libraries/drivers/src/at32f402_405_crm.c")
    message(FATAL_ERROR "Run: python tools/get_deps.py at32f405")
  endif()
  add_library(${BOARD_TARGET} STATIC
    ${AT32_SDK}/libraries/cmsis/cm4/device_support/system_at32f402_405.c
    ${AT32_SDK}/libraries/drivers/src/at32f402_405_crm.c
    ${AT32_SDK}/libraries/drivers/src/at32f402_405_flash.c
    ${AT32_SDK}/libraries/drivers/src/at32f402_405_gpio.c
    ${AT32_SDK}/libraries/drivers/src/at32f402_405_pwc.c
  )
  target_include_directories(${BOARD_TARGET} PUBLIC
    ${CMAKE_CURRENT_FUNCTION_LIST_DIR}
    ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/boards/${BOARD}
    ${AT32_SDK}/libraries/cmsis/cm4/device_support
    ${AT32_SDK}/libraries/drivers/inc
    ${TOP}/lib/CMSIS_5/CMSIS/Core/Include
  )
  target_compile_definitions(${BOARD_TARGET} PUBLIC
    ${AT32_DEVICE} BOARD_UF2_FAMILY_ID=${UF2_FAMILY_ID} TUF2_LOG=0
  )
  target_link_options(${BOARD_TARGET} PUBLIC -nostartfiles --specs=nosys.specs --specs=nano.specs)
endfunction()
