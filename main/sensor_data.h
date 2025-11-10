#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <driver/i2c.h>
#include <esp_log.h>
#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "MPU6050.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "sdkconfig.h"

// ----- LSB Values of MPU6050, CHANGABLE -----------------------
// source : mpu6050 datasheet
// since mpu6050 doesnt store values in float.
// #define ACCEL_LSB 16384.0f // 2G
#define ACCEL_LSB 8192.0f // 4G -> model was trained on this.
#define GYRO_LSB 131.0f

// ----- Structures ---------------------------------------------
struct SensorData {
    float x;
    float y;
    float z;
    float w = 0.0f;
};

enum class SensorValue {
    ACCELERATION,
    VELOCITY,
    ANGLE,
    QUATERNION,
    RAW_ACCELERATION,
};

// ----- Globals ------------------------------------------------
extern MPU6050 mpu;

// ----- Functions ----------------------------------------------
SensorData get_sensor_data(SensorValue val);

#ifdef __cplusplus
}
#endif

#endif

