#ifndef NIMBLE_MAIN_H
#define NIMBLE_MAIN_H

//#include "esp_err.h"     // for esp_err_t
//#include "nvs_flash.h"   // for NVS functions
//#include "nimble/port.h" // for nimble_port_init and nimble_port_run

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "gap.h"
#include "gatt_svc.h"
#include "sensor_data.h"

void setupBLE(void);

void ble_store_config_init(void);

#ifdef __cplusplus
}
#endif
#endif

