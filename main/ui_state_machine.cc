#include "screen.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

static QueueHandle_t ui_event_queue = NULL;
static ui_screen_state_t current_screen = UI_SCREEN_BOOT;

void ui_init(void) {
  if (ui_event_queue == NULL) {
    ui_event_queue = xQueueCreate(8, sizeof(ui_event_t));
  }
}

bool ui_post_event(ui_event_t event) {
  if (ui_event_queue == NULL) {
    return false;
  }

  return xQueueSend(ui_event_queue, &event, 0) == pdTRUE;
}

void ui_task(void *pvParameters) {
  (void)pvParameters;

  screen_init();

  while (1) {
    ui_event_t evt;

    while (xQueueReceive(ui_event_queue, &evt, 0) == pdTRUE) {
      switch (evt) {
        case UI_EVT_BT_CONNECTED:
          current_screen = UI_SCREEN_BT_CONNECTED;
          break;
        case UI_EVT_BT_DISCONNECTED:
          current_screen = UI_SCREEN_BOOT;
          break;
        case UI_EVT_BUTTON_NEXT:
          current_screen = UI_SCREEN_ALERT;
          break;
        case UI_EVT_BUTTON_BACK:
          current_screen = UI_SCREEN_HOME;
          break;
        case UI_EVT_SENSOR_ALERT:
          current_screen = UI_SCREEN_ALERT;
          break;
        default:
          break;
      }
    }

    screen_render(current_screen);
    vTaskDelay(pdMS_TO_TICKS(30));
  }
}
