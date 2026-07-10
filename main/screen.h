#ifndef SCREEN_H
#define SCREEN_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  UI_SCREEN_BOOT = 0,
  UI_SCREEN_HOME,
  UI_SCREEN_BT_CONNECTED,
  UI_SCREEN_ALERT,
} ui_screen_state_t;

typedef enum {
  UI_EVT_BT_CONNECTED = 0,
  UI_EVT_BT_DISCONNECTED,
  UI_EVT_BUTTON_NEXT,
  UI_EVT_BUTTON_BACK,
  UI_EVT_SENSOR_ALERT,
} ui_event_t;

void screen_init(void);
void screen_render(ui_screen_state_t state);

void ui_init(void);
bool ui_post_event(ui_event_t event);
void ui_task(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif
