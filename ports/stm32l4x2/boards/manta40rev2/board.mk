CFLAGS += \
  -DSTM32L412xx \

SRC_S += \
  $(ST_CMSIS)/Source/Templates/gcc/startup_stm32l412xx.s

SRC_C += \
  $(PORT_DIR)/boards/common/clock_stm32l412_hsi48.c

# For flash-jlink target
JLINK_DEVICE = stm32l412cb

flash: flash-dfu-util
erase: erase-jlink
