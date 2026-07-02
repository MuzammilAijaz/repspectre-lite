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
static const uint8_t image_BAT_percent_bits[] = {0x0c,0x03,0x1e,0x03,0x33,0x03,0xb3,0x03,0xde,0x01,0xec,0x00,0x70,0x00,0x38,0x00,0xdc,0x00,0xee,0x01,0x37,0x03,0x33,0x03,0xe3,0x01,0xc3,0x00};
static const uint8_t image_battery_charging_bits[] = {0x00,0x40,0x00,0xf0,0x27,0x7f,0x08,0x30,0x80,0x08,0x10,0x80,0x0e,0x18,0x80,0x01,0x0c,0x80,0x01,0xfc,0x81,0x01,0xfe,0x80,0x01,0xc0,0x80,0x01,0x60,0x80,0x0e,0x20,0x80,0x08,0x30,0x80,0x08,0x10,0x80,0xf0,0xcb,0x7f,0x00,0x08,0x00,0x00,0x00,0x00};
static const uint8_t image_CPU_bar_bits[] = {0xfc,0xff,0xff,0xff,0x01,0x02,0x00,0x00,0x00,0x02,0x01,0x00,0x00,0x00,0x04,0x01,0x00,0x00,0x00,0x04,0x01,0x00,0x00,0x00,0x04,0x01,0x00,0x00,0x00,0x04,0x01,0x00,0x00,0x00,0x04,0x02,0x00,0x00,0x00,0x02,0xfc,0xff,0xff,0xff,0x01};

void drawScreen_1(void) {
  u8g2_ClearBuffer(&u8g2);
  u8g2_SetBitmapMode(&u8g2, 1);
  u8g2_SetFontMode(&u8g2, 1);

  // RAM title
  u8g2_SetFont(&u8g2, u8g2_font_t0_17b_tr);
  u8g2_DrawStr(&u8g2, 51, 16, "RAM");
  // RAM used
  u8g2_DrawStr(&u8g2, 46, 37, "16");
  // RAM seperator
  u8g2_DrawStr(&u8g2, 75, 37, "/");
  // RAM total
  u8g2_DrawStr(&u8g2, 46, 55, "16");
  // RAM total gb
  u8g2_DrawStr(&u8g2, 65, 55, "GB");
  // BAT title
  u8g2_DrawStr(&u8g2, 94, 16, "BAT");
  // BAT charge
  u8g2_DrawStr(&u8g2, 103, 38, "100");
  // BAT percent
  u8g2_DrawXBM(&u8g2, 91, 25, 10, 14, image_BAT_percent_bits);
  // battery_charging
  u8g2_DrawXBM(&u8g2, 96, 45, 24, 16, image_battery_charging_bits);
  // CPU title
  u8g2_DrawStr(&u8g2, 7, 16, "CPU");
  // CPU load
  u8g2_DrawStr(&u8g2, 15, 38, "100");
  // CPU percent
  u8g2_DrawXBM(&u8g2, 3, 25, 10, 14, image_BAT_percent_bits);
  // CPU bar
  u8g2_DrawXBM(&u8g2, 4, 47, 35, 9, image_CPU_bar_bits);
  // CPU bar filling
  u8g2_DrawBox(&u8g2, 6, 49, 28, 5);
  // barrier left
  u8g2_DrawLine(&u8g2, 42, 0, 42, 64);
  // barrier right
  u8g2_DrawLine(&u8g2, 86, 0, 86, 64);
  u8g2_SendBuffer(&u8g2);
}
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

  // Execute your custom Lopaka screen UI layout
  drawScreen_1();

  vTaskDelete(NULL);
}
