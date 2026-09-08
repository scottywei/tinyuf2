cmake_minimum_required(VERSION 3.17)
include(${CMAKE_CURRENT_LIST_DIR}/../family_support.cmake)
set(ARTIFACT_PATH ${FAMILY_PATH}/_bin/${BOARD}/${AT32_DEVICE})
file(MAKE_DIRECTORY ${ARTIFACT_PATH}/apps)
project(tinyuf2 C ASM)
set(CMAKE_EXECUTABLE_SUFFIX .elf)

# Keep the TinyUSB submodule untouched. Select our controller in a build-local
# copy of the existing DWC2 sources, retaining its original license and code.
set(DWC2_SOURCE ${TOP}/lib/tinyusb/src/portable/synopsys/dwc2)
set(DWC2_BUILD ${CMAKE_CURRENT_BINARY_DIR}/dwc2)
file(MAKE_DIRECTORY ${DWC2_BUILD})
foreach(FILE dcd_dwc2.c dwc2_common.c dwc2_type.h)
  configure_file(${DWC2_SOURCE}/${FILE} ${DWC2_BUILD}/${FILE} COPYONLY)
endforeach()
file(READ ${DWC2_SOURCE}/dwc2_common.h DWC2_HEADER)
set(SELECTOR "#if defined(TUP_USBIP_DWC2_STM32)")
string(FIND "${DWC2_HEADER}" "${SELECTOR}" SELECTOR_POS)
if(SELECTOR_POS EQUAL -1)
  message(FATAL_ERROR "TinyUSB DWC2 header changed; review AT32 adapter")
endif()
string(REPLACE "${SELECTOR}" "#if 1\n  #include \"dwc2_at32.h\"\n#elif defined(TUP_USBIP_DWC2_STM32)" DWC2_HEADER "${DWC2_HEADER}")
file(WRITE ${DWC2_BUILD}/dwc2_common.h "${DWC2_HEADER}")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${DWC2_SOURCE}/dwc2_common.h)

set(FLASH_ORIGIN 0x08000000)
set(FLASH_KB 32)
configure_file(linker/at32f405.ld.in ${CMAKE_CURRENT_BINARY_DIR}/boot.ld @ONLY)
set(FLASH_ORIGIN 0x08008000)
set(FLASH_KB ${APP_FLASH_KB})
configure_file(linker/at32f405.ld.in ${CMAKE_CURRENT_BINARY_DIR}/app.ld @ONLY)

add_executable(tinyuf2 boards.c board_flash.c
  ${AT32_SDK}/libraries/cmsis/cm4/device_support/startup/gcc/startup_at32f402_405.s
  ${DWC2_BUILD}/dcd_dwc2.c ${DWC2_BUILD}/dwc2_common.c)
target_link_options(tinyuf2 PUBLIC "LINKER:--script=${CMAKE_CURRENT_BINARY_DIR}/boot.ld")
family_configure_tinyuf2(tinyuf2 OPT_MCU_NONE)
family_flash_jlink(tinyuf2)

add_executable(blinky apps/blinky.c
  ${AT32_SDK}/libraries/cmsis/cm4/device_support/startup/gcc/startup_at32f402_405.s)
target_link_options(blinky PUBLIC "LINKER:--script=${CMAKE_CURRENT_BINARY_DIR}/app.ld")
family_configure_common(blinky)
family_gen_uf2(blinky ${UF2_FAMILY_ID})
