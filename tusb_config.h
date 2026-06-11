#ifndef TUSB_CONFIG_H
#define TUSB_CONFIG_H

#ifndef BOARD_TUD_RHPORT
#define BOARD_TUD_RHPORT        0
#endif

#ifndef BOARD_TUD_MAX_SPEED
#define BOARD_TUD_MAX_SPEED     OPT_MODE_DEFAULT_SPEED
#endif

#define CFG_TUSB_DEBUG          0

#define CFG_TUSB_RHPORT0_MODE   OPT_MODE_DEVICE

#define CFG_TUD_ENABLED         1

#ifndef CFG_TUD_ENDPOINT0_SIZE
#define CFG_TUD_ENDPOINT0_SIZE  64
#endif

#define CFG_TUD_HID             1
#define CFG_TUD_CDC             0
#define CFG_TUD_MSC             0
#define CFG_TUD_MIDI            0
#define CFG_TUD_VENDOR          0

#define CFG_TUD_HID_EP_BUFSIZE  16

#endif
