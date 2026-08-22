CFLAGS += \
  -DSTM32F103xB \
  -DHSE_VALUE=16000000U

SRC_S += \
  $(ST_CMSIS)/Source/Templates/gcc/startup_stm32f103xb.s

JLINK_DEVICE = STM32F103CB
PYOCD_TARGET = stm32f103cb

flash: flash-stlink
erase: erase-stlink
