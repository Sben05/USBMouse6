#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "bsp/board_api.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include "mpu6050.h"

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"

#define I2C_PORT    i2c0
#define I2C_SDA     4
#define I2C_SCL     5
#define PIN_BUTTON  15
#define PIN_LED     17

enum {
    BLINK_NOT_MOUNTED = 250,
    BLINK_MOUNTED     = 1000,
    BLINK_SUSPENDED   = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;
static bool circle_mode = false;
static float circle_angle = 0.0f;

void led_blinking_task(void);
void hid_task(void);

int main(void) {
    board_init();
    stdio_init_all();

    gpio_init(PIN_LED);
    gpio_set_dir(PIN_LED, GPIO_OUT);
    gpio_put(PIN_LED, 0);

    gpio_init(PIN_BUTTON);
    gpio_set_dir(PIN_BUTTON, GPIO_IN);
    gpio_pull_up(PIN_BUTTON);

    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    mpu6050_init(I2C_PORT);

    tud_init(BOARD_TUD_RHPORT);
    if (board_init_after_tusb) {
        board_init_after_tusb();
    }

    while (1) {
        tud_task();
        led_blinking_task();
        hid_task();
    }
}

void tud_mount_cb(void)   { blink_interval_ms = BLINK_MOUNTED; }
void tud_umount_cb(void)  { blink_interval_ms = BLINK_NOT_MOUNTED; }
void tud_suspend_cb(bool remote_wakeup_en) {
    (void)remote_wakeup_en;
    blink_interval_ms = BLINK_SUSPENDED;
}
void tud_resume_cb(void)  { blink_interval_ms = tud_mounted() ? BLINK_MOUNTED : BLINK_NOT_MOUNTED; }

static int8_t accel_to_delta(int16_t accel) {
    int32_t mag = accel < 0 ? -(int32_t)accel : (int32_t)accel;
    int8_t spd;
    if      (mag < 2000)  spd = 0;
    else if (mag < 5000)  spd = 2;
    else if (mag < 10000) spd = 4;
    else                  spd = 7;
    return (accel < 0) ? -spd : spd;
}

static void send_hid_report(uint8_t report_id, uint32_t btn) {
    if (!tud_hid_ready()) return;

    if (report_id == REPORT_ID_MOUSE) {
        static bool last_button = false;
        static uint32_t last_toggle_ms = 0;
        bool cur_button = !gpio_get(PIN_BUTTON);

        if (cur_button && !last_button && (board_millis() - last_toggle_ms > 200)) {
            circle_mode = !circle_mode;
            gpio_put(PIN_LED, circle_mode ? 1 : 0);
            last_toggle_ms = board_millis();
        }
        last_button = cur_button;

        int8_t dx = 0, dy = 0;

        if (circle_mode) {
            const float radius = 3.0f;
            dx = (int8_t)(radius * cosf(circle_angle));
            dy = (int8_t)(radius * sinf(circle_angle));
            circle_angle += 0.010f * (float)M_PI;
            if (circle_angle >= 2.0f * (float)M_PI)
                circle_angle -= 2.0f * (float)M_PI;
        } else {
            int16_t ax, ay, az;
            mpu6050_read_accel(I2C_PORT, &ax, &ay, &az);
            dx = accel_to_delta(ax);
            dy = accel_to_delta(ay);
        }

        tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, dx, dy, 0, 0);
    }
}

void hid_task(void) {
    const uint32_t interval_ms = 10;
    static uint32_t start_ms = 0;

    if (board_millis() - start_ms < interval_ms) return;
    start_ms += interval_ms;

    uint32_t const btn = board_button_read();

    if (tud_suspended() && btn) {
        tud_remote_wakeup();
    } else {
        send_hid_report(REPORT_ID_MOUSE, btn);
    }
}

void tud_hid_report_complete_cb(uint8_t instance, uint8_t const *report, uint16_t len) {
    (void)instance; (void)report; (void)len;
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                               hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    (void)instance; (void)report_id; (void)report_type; (void)buffer; (void)reqlen;
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                           hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
    (void)instance; (void)report_id; (void)report_type; (void)buffer; (void)bufsize;
}

void led_blinking_task(void) {
    static uint32_t start_ms = 0;
    static bool led_state = false;

    if (!blink_interval_ms) return;
    if (board_millis() - start_ms < blink_interval_ms) return;
    start_ms += blink_interval_ms;

    board_led_write(led_state);
    led_state = 1 - led_state;
}
