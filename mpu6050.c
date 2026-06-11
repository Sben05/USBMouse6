#include "mpu6050.h"

static void reg_write(i2c_inst_t *port, uint8_t reg, uint8_t val) {
    uint8_t buf[2] = {reg, val};
    i2c_write_blocking(port, MPU6050_ADDR, buf, 2, false);
}

void mpu6050_init(i2c_inst_t *i2c_port) {
    reg_write(i2c_port, MPU6050_REG_PWR_MGMT_1, 0x00);
    reg_write(i2c_port, MPU6050_REG_SMPLRT_DIV,  0x07);
    reg_write(i2c_port, MPU6050_REG_CONFIG,       0x06);
    reg_write(i2c_port, MPU6050_REG_ACCEL_CFG,   0x00);
}

void mpu6050_read_accel(i2c_inst_t *i2c_port, int16_t *ax, int16_t *ay, int16_t *az) {
    uint8_t reg = MPU6050_REG_ACCEL_XOUT;
    uint8_t raw[6];

    i2c_write_blocking(i2c_port, MPU6050_ADDR, &reg, 1, true);
    i2c_read_blocking(i2c_port, MPU6050_ADDR, raw, 6, false);

    *ax = (int16_t)((raw[0] << 8) | raw[1]);
    *ay = (int16_t)((raw[2] << 8) | raw[3]);
    *az = (int16_t)((raw[4] << 8) | raw[5]);
}
