cmake_minimum_required(VERSION 3.17)

include(${CMAKE_CURRENT_LIST_DIR}/../family_support.cmake)

project(tinyuf2 C ASM)
set(CMAKE_EXECUTABLE_SUFFIX .elf)

add_executable(tinyuf2
  board_flash.c
  boards.c
  ${TOP}/lib/tinyusb/src/portable/st/stm32_fsdev/dcd_stm32_fsdev.c
  )

target_link_options(tinyuf2 PUBLIC
  "LINKER:--script=${CMAKE_CURRENT_LIST_DIR}/linker/stm32f1_boot.ld"
  )

family_configure_tinyuf2(tinyuf2 OPT_MCU_STM32F1)
family_flash_jlink(tinyuf2)
family_flash_stlink(tinyuf2)
