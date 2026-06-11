#include <math.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include "mpu6050.h"

#define I2C_PORT        i2c0
#define PIN_SDA         4
#define PIN_SCL         5
#define PIN_BUTTON      15
#define PIN_MODE_LED    PICO_DEFAULT_LED_PIN

#define MOUSE_INTERVAL_MS   10
#define DEBOUNCE_MS         200

static bool circle_mode   = false;
static float circle_angle = 0.0f;

static int8_t accel_to_delta(int16_t accel) {
    int32_t mag = accel < 0 ? -(int32_t)accel : (int32_t)accel;
    int8_t  spd;

    if      (mag < 2000)  spd = 0;
    else if (mag < 5000)  spd = 1;
    else if (mag < 10000) spd = 3;
    else                  spd = 5;

    return (accel < 0) ? -spd : spd;
}

static void send_mouse_report(void) {
    if (!tud_hid_ready()) return;

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

int main(void) {
    stdio_init_all();

    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_SDA);
    gpio_pull_up(PIN_SCL);
    mpu6050_init(I2C_PORT);

    gpio_init(PIN_MODE_LED);
    gpio_set_dir(PIN_MODE_LED, GPIO_OUT);
    gpio_put(PIN_MODE_LED, 0);

    gpio_init(PIN_BUTTON);
    gpio_set_dir(PIN_BUTTON, GPIO_IN);
    gpio_pull_up(PIN_BUTTON);

    tusb_init();

    uint32_t last_mouse_ms  = 0;
    uint32_t last_btn_ms    = 0;
    bool     prev_btn_state = true;

    while (true) {
        tud_task();

        uint32_t now = to_ms_since_boot(get_absolute_time());

        bool btn_now = gpio_get(PIN_BUTTON);
        if (!btn_now && prev_btn_state && (now - last_btn_ms > DEBOUNCE_MS)) {
            last_btn_ms   = now;
            circle_mode   = !circle_mode;
            gpio_put(PIN_MODE_LED, circle_mode ? 1 : 0);
        }
        prev_btn_state = btn_now;

        if (tud_mounted() && (now - last_mouse_ms >= MOUSE_INTERVAL_MS)) {
            last_mouse_ms = now;
            send_mouse_report();
        }
    }

    return 0;
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                               hid_report_type_t report_type,
                               uint8_t *buffer, uint16_t reqlen) {
    (void)instance; (void)report_id; (void)report_type;
    (void)buffer;   (void)reqlen;
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                           hid_report_type_t report_type,
                           uint8_t const *buffer, uint16_t bufsize) {
    (void)instance; (void)report_id; (void)report_type;
    (void)buffer;   (void)bufsize;
}
