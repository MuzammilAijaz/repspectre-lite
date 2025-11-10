
/** =====================================================================================
 * |                              📌 FILE PURPOSE
 *  -------------------------------------------------------------------------------------
 *  What this file does:
 *      - deals with the sensor data directly, updating the buffer accordingly
 * 
 * ===================================================================================== */

/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "accelerometer_handler.h"
#include "sensor_data.h"

// Variables for Accel and Gyro 
// MPU_t MPU;         // create an object
// mpud::raw_axes_t accelRaw;     // holds x, y, z axes as int16
// mpud::float_axes_t accelG; 

struct SensorData accelRaw = {0.0f, 0.0f, 0.0f, 0.0f};

// Buffer to hold the last 200 sets of 3-channel values
float save_data[600] = {0.0};
int begin_index = 0; // Index of the buffer starts empty
bool pending_intial_data = true;

// =====================================================================================
// |                                 Functions
// =====================================================================================

// REFACTOR: NOT USED -> REMOVEEE!!!!!!!!!
TfLiteStatus SetupAccelerometer() {
  setupMpuSensor();
  return kTfLiteOk;
}

// ================== Get accelerometer data ==================
/** Fills amount of data requested for, in the input array, and 
 * returns : 
 *  -> false if new data is not complete,
 *  -> true if new data is retrieved */
bool ReadAccelerometer(float* input, int length, bool reset_buffer) {
  if (reset_buffer) {
    memset(save_data, 0, 600 * sizeof(float)); 
    begin_index = 0;
    pending_intial_data = true;
  }

  accelRaw = get_sensor_data(SensorValue::RAW_ACCELERATION);

//  // Try to read once
//  if (MPU.acceleration(&accelRaw) != ESP_OK) {  
//    return false;  // no data available
//  }
//
//  accelG = mpud::accelGravity(accelRaw, mpud::ACCEL_FS_4G);
//
//  // Store into circular buffer
//  save_data[begin_index++] = accelG.x * 1000;
//  save_data[begin_index++] = accelG.y * 1000;
//  save_data[begin_index++] = accelG.z * 1000;

  save_data[begin_index++] = accelRaw.x * 1000;
  save_data[begin_index++] = accelRaw.y * 1000;
  save_data[begin_index++] = accelRaw.z * 1000;

  if (begin_index >= 600) {
    begin_index = 0;
  }

  // Initial fill check
  if (pending_intial_data && begin_index >= 200) {
    pending_intial_data = false;
  }

  if (pending_intial_data) {
    return false;  // still not enough data collected
  }

  // ------------------ If enough data is present ------------------
  for (int i = 0; i < length; i++) {
    int ring_array_index = begin_index + i - length;
    if (ring_array_index < 0) {
      ring_array_index += 600;
    }
    input[i] = save_data[ring_array_index];
  }

  return true; // successfully got new data
}

//void setupMpuSensor() {
//  
//  i2c0.begin(I2C_MPU_SDA, I2C_MPU_SCL, 400000);  // initialize the I2C bus
//
//  MPU.setBus(i2c0);  // set communication bus, for SPI -> pass 'hspi'
//  MPU.setAddr(mpud::MPU_I2CADDRESS_AD0_LOW);  // set address or handle, for SPI -> pass 'mpu_spi_handle'
//  MPU.testConnection();  // test connection with the chip, return is a error code
//  MPU.initialize();  // this will initialize the chip and set default configurations
//
//  MPU.setSampleRate(25);  // in (Hz) ; !!! required for the model
//  MPU.setAccelFullScale(mpud::ACCEL_FS_4G);
//  MPU.setGyroFullScale(mpud::GYRO_FS_500DPS);
//  MPU.setDigitalLowPassFilter(mpud::DLPF_42HZ);  // smoother data
//  MPU.setInterruptEnabled(mpud::INT_EN_RAWDATA_READY);  // enable INT pin
//}
