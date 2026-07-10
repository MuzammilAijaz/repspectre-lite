#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "screen.h"

#include "driver/gpio.h"
#include "driver/i2c_master.h"

extern "C" {
#include "esp32_hw_i2c.h"
#include "esp_timer.h"
#include "lopaka_64_64_28f_copy_1.h"
#include "lopaka_64_64_28f_copy_3.h"
#include "u8g2.h"
#include "u8x8.h"
}

#define PIN_SDA GPIO_NUM_6
#define PIN_CLK GPIO_NUM_7
#define OLED_ADDR 0x3C

u8g2_t u8g2;

static void update_animations(void) {
    uint32_t now = (uint32_t)(esp_timer_get_time() / 1000);
    lopaka_64_64_28f_copy_1_frame = now / 83 % 24;
    lopaka_64_64_28f_copy_3_frame = (now / 83 + 7) % 24;
}

static void draw_animation_lopaka_64_64_28f_copy_1(u8g2_t *display) {
    u8g2_SetDrawColor(display, 1);
    u8g2_DrawXBMP(display, -1, 0, 64, 64,
                  lopaka_64_64_28f_copy_1_frames[lopaka_64_64_28f_copy_1_frame]);
}

static void draw_animation_lopaka_64_64_28f_copy_3(u8g2_t *display) {
    u8g2_SetDrawColor(display, 1);
    u8g2_DrawXBMP(display, 64, -1, 64, 64,
                  lopaka_64_64_28f_copy_3_frames[lopaka_64_64_28f_copy_3_frame]);
}

static void screen_init_common(void) {
    u8g2_esp32_i2c_ctx_t ctx = {};
    memset(&ctx, 0, sizeof(ctx));

    ctx.cfg.i2c_port = I2C_NUM_1;
    ctx.cfg.sda_pin = PIN_SDA;
    ctx.cfg.scl_pin = PIN_CLK;
    ctx.cfg.dev_addr_7bit = OLED_ADDR;
    ctx.cfg.clk_hz = 100000;
    ctx.cfg.timeout_ms = 1000;
    ctx.cfg.reset_pin = -1;

    u8g2_esp32_i2c_set_default_context(&ctx);

    u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2,
                                           U8G2_R0,
                                           u8x8_byte_esp32_hw_i2c,
                                           u8x8_gpio_and_delay_esp32_i2c);
    u8x8_SetI2CAddress(&u8g2.u8x8, (OLED_ADDR << 1));
    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);
}

void screen_init(void) {
    screen_init_common();
}

void screen_render(ui_screen_state_t state) {
    u8g2_ClearBuffer(&u8g2);
    u8g2_SetBitmapMode(&u8g2, 1);
    u8g2_SetFontMode(&u8g2, 1);

    if (state == UI_SCREEN_BOOT) {
        update_animations();
        draw_animation_lopaka_64_64_28f_copy_1(&u8g2);
        draw_animation_lopaka_64_64_28f_copy_3(&u8g2);
        u8g2_SendBuffer(&u8g2);
        return;
    }

    const char *title = "Unknown";
    const char *subtitle = "";

    switch (state) {
        case UI_SCREEN_HOME:
            title = "Home";
            subtitle = "Ready for events";
            break;
        case UI_SCREEN_BT_CONNECTED:
            title = "Bluetooth connected";
            subtitle = "Safe event handoff";
            break;
        case UI_SCREEN_ALERT:
            title = "Alert";
            subtitle = "Use queue-based updates";
            break;
        default:
            break;
    }

    u8g2_SetFont(&u8g2, u8g2_font_6x10_tf);
    u8g2_DrawStr(&u8g2, 0, 14, title);
    u8g2_DrawStr(&u8g2, 0, 30, subtitle);
    u8g2_DrawStr(&u8g2, 0, 50, "Events are consumed here");
    u8g2_SendBuffer(&u8g2);
}
