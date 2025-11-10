#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "main_functions.h"

#include "sensor_init.h"

void tf_main(void) {
  setup();
  while (true) {
    loop();
  }
}

extern "C" void app_main() {

  // ------------------ ??????????????????????? ------------------
  // I DONT KNOW WHY MAKING THE INIT FUNCTIONS INTO A FREERTOS, MAKE IT WORK?????????
  // TODO: need to make it work without creating a task; maybe try changing app_main task starting memory
  // no longer causing overflows. 
  
  xTaskCreate(init_i2c, "i2c_init_task", 8048, NULL, 5, NULL);
  // init_i2c();    
  while(init_done_flag !=1);
  vTaskDelay(pdMS_TO_TICKS(100));
  xTaskCreate(init_mpu, "mpu_init_task", 8192, NULL, 3, NULL);
  // init_mpu();
  vTaskDelay(pdMS_TO_TICKS(100));
  
  while(init_done_flag != 2);
  // ---------------------------------------------

  xTaskCreate((TaskFunction_t)&tf_main, "tensorflow", 16 * 1024, NULL, 8, NULL);
  vTaskDelete(NULL);
}
