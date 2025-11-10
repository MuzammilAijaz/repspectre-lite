#ifndef SENSOR_INIT_H
#define SENSOR_INIT_H

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

// ------------------ Public Variables ------------------
extern int init_done_flag; // tells the main.cc to wait untill setup is done before proceeding with other tasks

// ------------------ Public Functions ------------------
void init_i2c(void *ignore);
void init_mpu(void *ignore);

#ifdef __cplusplus
}
#endif

#endif
