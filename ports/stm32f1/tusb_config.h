#ifndef TUSB_CONFIG_H_
#define TUSB_CONFIG_H_

#ifndef CFG_TUSB_MCU
#error CFG_TUSB_MCU must be defined by the port
#endif

#define CFG_TUSB_OS               OPT_OS_NONE
#define CFG_TUD_ENABLED           1
#define BOARD_TUD_RHPORT          0

#ifndef CFG_TUSB_DEBUG
#define CFG_TUSB_DEBUG            0
#endif

#define CFG_TUSB_MEM_ALIGN        __attribute__((aligned(4)))
#define CFG_TUD_ENDPOINT0_SIZE    64
#define CFG_TUD_CDC               0
#define CFG_TUD_MSC               1
#define CFG_TUD_HID               0
#define CFG_TUD_MIDI              0
#define CFG_TUD_VENDOR            0
#define CFG_TUD_MSC_BUFSIZE       512

#endif
