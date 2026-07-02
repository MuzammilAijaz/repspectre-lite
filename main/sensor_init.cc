#include "sensor_init.h"
#include "sensor_data.h" // to allow access and initialize mpu object
#include "driver/gpio.h"

#define PIN_SDA 5
#define PIN_CLK 4

int init_done_flag = 0; // tells the main.cc to wait untill setup is done before proceeding with other tasks
MPU6050 mpu;
i2c_master_bus_handle_t i2c_bus = NULL;
void errorLoopBlink();

// =====================================================================================
// |                         Public initialization Functions
// -------------------------------------------------------------------------------------

void init_i2c(void *ignore) {
    i2c_master_bus_config_t conf = {}; // initialize to 0/null, important in cpp
    conf.i2c_port = I2C_NUM_0;
    conf.sda_io_num = (gpio_num_t)PIN_SDA;
    conf.scl_io_num = (gpio_num_t)PIN_CLK;
    conf.clk_source = I2C_CLK_SRC_DEFAULT;
    conf.glitch_ignore_cnt = 7;
    conf.flags.enable_internal_pullup = true;

    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&conf, &bus_handle));
    i2c_bus = bus_handle;

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
