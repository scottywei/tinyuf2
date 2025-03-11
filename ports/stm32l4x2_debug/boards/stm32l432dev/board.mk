CFLAGS += \
  -DSTM32L432xx \
  -DLSE_VALUE=32768U

SRC_S += \
  $(ST_CMSIS)/Source/Templates/gcc/startup_stm32l432xx.s

SRC_C += \
  $(BOARD_DIR)/board.c

# For flash-jlink target
JLINK_DEVICE = stm32l432kb

flash: flash-dfu-util
erase: erase-jlink
