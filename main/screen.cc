#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "screen.h"

#include "driver/i2c_master.h"
#include "driver/gpio.h"

extern "C" {
  #include "u8g2.h"
  #include "esp32_hw_i2c.h"
  #include "u8x8.h"
}

#define PIN_SDA GPIO_NUM_6
#define PIN_CLK GPIO_NUM_7
#define OLED_ADDR 0x3C

u8g2_t u8g2; // Global instance for Lopaka graphics code

// [BEGIN lopaka generated]
#include <stdint.h>
#include "esp_timer.h"
#include "lopaka_64_64_28f_copy_1.h"
#include "lopaka_64_64_28f_copy_3.h"

void updateAnimations(void) {
    uint32_t now = (uint32_t)(esp_timer_get_time() / 1000);
    lopaka_64_64_28f_copy_1_frame = now / 83 % 24;
    lopaka_64_64_28f_copy_3_frame = (now / 83 + 7) % 24; // to avoid sync
}

void drawAnimation_lopaka_64_64_28f_copy_1(u8g2_t *u8g2) {
    u8g2_SetDrawColor(u8g2, 1);
    u8g2_DrawXBMP(u8g2, -1, 0, 64, 64, lopaka_64_64_28f_copy_1_frames[lopaka_64_64_28f_copy_1_frame]);
}
void drawAnimation_lopaka_64_64_28f_copy_3(u8g2_t *u8g2) {
    u8g2_SetDrawColor(u8g2, 1);
    u8g2_DrawXBMP(u8g2, 64, -1, 64, 64, lopaka_64_64_28f_copy_3_frames[lopaka_64_64_28f_copy_3_frame]);
}

void drawScreen_1(void) {
    u8g2_ClearBuffer(&u8g2);
    u8g2_SetBitmapMode(&u8g2, 1);
    u8g2_SetFontMode(&u8g2, 1);

    drawAnimation_lopaka_64_64_28f_copy_1(&u8g2);
    drawAnimation_lopaka_64_64_28f_copy_3(&u8g2);
    u8g2_SendBuffer(&u8g2);
}

// Call updateAnimations() before drawScreen_1() in your ESP-IDF task loop.
// [END lopaka generated]

void init_display_u8g2(void *pvParameters) {
  // Setup the structural context configuration
  u8g2_esp32_i2c_ctx_t ctx = {};
  memset(&ctx, 0, sizeof(u8g2_esp32_i2c_ctx_t));

  ctx.cfg.i2c_port       = I2C_NUM_1;
  ctx.cfg.sda_pin        = PIN_SDA;
  ctx.cfg.scl_pin        = PIN_CLK;
  ctx.cfg.dev_addr_7bit  = OLED_ADDR;
  ctx.cfg.clk_hz         = 100000;
  ctx.cfg.timeout_ms     = 1000;
  ctx.cfg.reset_pin      = -1;

  ctx.bus_handle  = NULL;
  ctx.dev_handle  = NULL;
  ctx.initialized = 0;

  // Register the default hardware hook context
  u8g2_esp32_i2c_set_default_context(&ctx);

  // Normal U8g2 allocation flow utilizing the fork's native assembly pins
  u8g2_Setup_ssd1306_i2c_128x64_noname_f(
      &u8g2,
      U8G2_R0,
      u8x8_byte_esp32_hw_i2c,         // Fork hardware link
      u8x8_gpio_and_delay_esp32_i2c   // Fork delay link
      );

  // Force 8-bit shifted address check right over the I2C configuration register
  u8x8_SetI2CAddress(&u8g2.u8x8, (OLED_ADDR << 1));

  // Fire panel boot instructions
  u8g2_InitDisplay(&u8g2);
  u8g2_SetPowerSave(&u8g2, 0); // Wake up display panel RAM mapping
  u8g2_ClearBuffer(&u8g2);

  // Execute screen UI layout
  int64_t start_time = esp_timer_get_time(); // microseconds

  while ((esp_timer_get_time() - start_time) < 5000000) { // 3s
    updateAnimations();
    drawScreen_1();
    vTaskDelay(pdMS_TO_TICKS(30)); // ~33 FPS
  }

  // final clear frame
  u8g2_ClearBuffer(&u8g2);
  u8g2_SendBuffer(&u8g2);

  vTaskDelete(NULL);
}
