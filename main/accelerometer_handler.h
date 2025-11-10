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

#ifndef TENSORFLOW_LITE_EXPERIMENTAL_MICRO_EXAMPLES_MAGIC_WAND_ACCELEROMETER_HANDLER_H_
#define TENSORFLOW_LITE_EXPERIMENTAL_MICRO_EXAMPLES_MAGIC_WAND_ACCELEROMETER_HANDLER_H_

#define kChannelNumber 3

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/micro_log.h"
#include <cstring>

// #include "MPU.hpp"        // main file, provides the class itself
// #include "mpu/math.hpp"   // math helper for dealing with MPU data
// #include "mpu/types.hpp"  // MPU data types and definitions
// #include "I2Cbus.hpp"
// 
// #define I2C_MPU_SDA GPIO_NUM_5
// #define I2C_MPU_SCL GPIO_NUM_4
// ------------------ global variables ------------------
extern int begin_index;

// ------------------ functions ------------------
extern TfLiteStatus SetupAccelerometer();
extern bool ReadAccelerometer(float* input, int length, bool reset_buffer);
extern void setupMpuSensor();

#endif  // TENSORFLOW_LITE_EXPERIMENTAL_MICRO_EXAMPLES_MAGIC_WAND_ACCELEROMETER_HANDLER_H_

