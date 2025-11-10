#include "sensor_init.h"
#include "sensor_data.h" // to allow access and initialize mpu object

#define PIN_SDA 5
#define PIN_CLK 4

int init_done_flag = 0; // tells the main.cc to wait untill setup is done before proceeding with other tasks
MPU6050 mpu;
void errorLoopBlink();

// =====================================================================================
// |                         Public initialization Functions
// -------------------------------------------------------------------------------------

void init_i2c(void *ignore) {
    i2c_config_t conf = {}; // initialize to 0/null, important in cpp
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = (gpio_num_t)PIN_SDA;
    conf.scl_io_num = (gpio_num_t)PIN_CLK;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 400000;
    conf.clk_flags = 0;              // <---- important for ESP-IDF v5+
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));

    // vAFunction();
    init_done_flag = 1;
    vTaskDelay(pdMS_TO_TICKS(40));
    vTaskDelete(NULL);
    // TODO : add return for ERROR checking
}

void init_mpu(void *ignore) {
    mpu = MPU6050();
    mpu.initialize();

    // This function sets up the built in DMP so sensor fusion is done inside the DMP
    if ( mpu.dmpInitialize() != 0) {
	errorLoopBlink();
    };

    // This need to be setup individually
    // mpu.setXGyroOffset(220);
    // mpu.setYGyroOffset(76);
    // mpu.setZGyroOffset(-85);
    // mpu.setZAccelOffset(1788);
    
    // ---- calibration ----
    // Each loop itself collects ~100 sensor readings and adjusts the offset a little bit.
    
    mpu.CalibrateAccel(6); // loops = 6
    mpu.CalibrateGyro(6); // loops = 6

    mpu.setDMPEnabled(true); // enables the dmp bit
    mpu.setRate(25);
    
    while (!mpu.testConnection()) {printf("MPU6050 connection failed/n"); vTaskDelay(pdMS_TO_TICKS(200));}

    // vAFunction();
    printf("Exiting init");
    vTaskDelay(pdMS_TO_TICKS(20));
    init_done_flag = 2;
    vTaskDelete(NULL);
}

// =====================================================================================
// |                                    Private 
// -------------------------------------------------------------------------------------

void errorLoopBlink() {
    printf("ERROR");
    gpio_num_t LED_GPIO = GPIO_NUM_48;
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    while (1) {
      gpio_set_level(LED_GPIO, 1);  // LED ON
      vTaskDelay(pdMS_TO_TICKS(500));
      gpio_set_level(LED_GPIO, 0);  // LED OFF
      vTaskDelay(pdMS_TO_TICKS(500));
    }
    return;
}
