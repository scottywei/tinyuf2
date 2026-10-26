CFLAGS += \
  -DSTM32F411xE \
  -DHSE_VALUE=25000000U

SRC_S += \
	$(ST_CMSIS)/Source/Templates/gcc/startup_stm32f411xe.s

SRC_C += ports/stm32f4/w25q32.c

# For flash-jlink target
JLINK_DEVICE = stm32f411ce

flash: flash-dfu-util
erase: erase-dfu-util
