#include "tusb.h"
#include "usb_descriptors.h"
#include <string.h>

#define USB_VID     0xCafe
#define USB_PID     0x4004
#define USB_BCD     0x0200

static tusb_desc_device_t const device_desc = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = USB_BCD,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = USB_VID,
    .idProduct          = USB_PID,
    .bcdDevice          = 0x0100,
    .iManufacturer      = 1,
    .iProduct           = 2,
    .iSerialNumber      = 3,
    .bNumConfigurations = 1
};

uint8_t const *tud_descriptor_device_cb(void) {
    return (uint8_t const *)&device_desc;
}

static uint8_t const hid_report_desc[] = {
    TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(REPORT_ID_MOUSE))
};

uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) {
    (void)instance;
    return hid_report_desc;
}

enum { ITF_HID = 0, ITF_TOTAL };

#define EPNUM_HID           0x81
#define CFG_TOTAL_LEN       (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)

static uint8_t const config_desc[] = {
    TUD_CONFIG_DESCRIPTOR(1, ITF_TOTAL, 0, CFG_TOTAL_LEN, 0x00, 100),
    TUD_HID_DESCRIPTOR(ITF_HID, 0, HID_ITF_PROTOCOL_NONE,
                       sizeof(hid_report_desc), EPNUM_HID,
                       CFG_TUD_HID_EP_BUFSIZE, 5)
};

uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
    (void)index;
    return config_desc;
}

static char const *const string_table[] = {
    "\x09\x04",
    "ME433 NWU",
    "Pico HW6 Mouse",
    "HW6-0001",
};

static uint16_t str_buf[32];

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void)langid;

    uint8_t char_count;

    if (index == 0) {
        memcpy(&str_buf[1], string_table[0], 2);
        char_count = 1;
    } else {
        size_t table_len = sizeof(string_table) / sizeof(string_table[0]);
        if (index >= table_len) return NULL;

        const char *src = string_table[index];
        char_count = (uint8_t)strlen(src);
        if (char_count > 31) char_count = 31;

        for (uint8_t i = 0; i < char_count; i++) {
            str_buf[1 + i] = src[i];
        }
    }

    str_buf[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * char_count + 2));
    return str_buf;
}
