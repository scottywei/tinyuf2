CFLAGS += \
  -DSTM32L412xx \
  -DLSE_VALUE=32768U

SRC_S += \
  $(ST_CMSIS)/Source/Templates/gcc/startup_stm32l412xx.s

# For flash-jlink target
JLINK_DEVICE = stm32l412cb

flash: flash-dfu-util
erase: erase-jlink
