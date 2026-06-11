#ifndef MPU6050_H
#define MPU6050_H

#include "hardware/i2c.h"
#include <stdint.h>

#define MPU6050_ADDR            0x68

#define MPU6050_REG_SMPLRT_DIV  0x19
#define MPU6050_REG_CONFIG      0x1A
#define MPU6050_REG_ACCEL_CFG   0x1C
#define MPU6050_REG_ACCEL_XOUT  0x3B
#define MPU6050_REG_PWR_MGMT_1  0x6B

void mpu6050_init(i2c_inst_t *i2c_port);
void mpu6050_read_accel(i2c_inst_t *i2c_port, int16_t *ax, int16_t *ay, int16_t *az);

#endif
