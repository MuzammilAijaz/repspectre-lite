/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* Includes */
#include "gatt_svc.h"
#include "common.h"
#include "sensor_data.h"

// =====================================================================================
// |                       Declarations/Structures/Variables
// =====================================================================================

// ------------------ Private function declarations ------------------

static int sensor_data_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                                 struct ble_gatt_access_ctxt *ctxt, void *arg);

// ------------------ GATT structures ------------------

// Service
static const ble_uuid128_t sensor_data_svc_uuid = // Custom UUID
    BLE_UUID128_INIT(0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
                     0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF);

// Characteristic
static SensorData sensor_data_chr_val = {0.0f, 0.0f, 0.0f, 0.0f}; // stores the actual data temporarily so it can be sent
static uint16_t sensor_data_chr_val_handle;
static const ble_uuid128_t sensor_data_chr_uuid = // Custom UUID
    BLE_UUID128_INIT(0x98, 0x76, 0x54, 0x32, 0x10, 0xFE, 0xDC, 0xBA,
                 0x98, 0x76, 0x54, 0x32, 0x10, 0xFE, 0xDC, 0xBA);

static uint16_t sensor_data_chr_conn_handle = 0;
static bool sensor_data_chr_conn_handle_inited = false;
static bool sensor_data_ind_status = false;
static bool sensor_data_notify_status = false;

// Custom Descriptor
static uint8_t sensor_data_dsc_val[] = "Acceleration data (x, y, z)";
static const ble_uuid128_t sensor_data_dsc_uuid =
    BLE_UUID128_INIT(0x23, 0xAB, 0x45, 0xCD, 0x67, 0x89, 0xEF, 0x01,
                     0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x00);


// ================== GATT Sevices Table  ==================
/* GATT services table, which defines all services, characteristics and attributes data used
 *
 * @type ble_gatt_svc_def: defines a service with type, uuid, characteristics
 *
 * @param type: primary or secondary
 * @param uuid: for uuid for service
 * @param characteristics: ble_gatt_chr_def array storing charatertistics 
 *
 * @type ble_gatt_chr_def: defines a characteristic with uuid, access_cb, flags, val_handle
 * @type ble_gatt_dec_def: defines the descriptor of the particular UUID
*/
static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    /* SERVICE 1 : Sensor Data service */
    {.type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = &sensor_data_svc_uuid.u,
     .characteristics =
         (struct ble_gatt_chr_def[]){

             { /* sensor data characteristic */
              .uuid = &sensor_data_chr_uuid.u,
              .access_cb = sensor_data_chr_access, // Passing a Callback Function which handles GATT data accessing
              .descriptors = (struct ble_gatt_dsc_def[]){

                    // ------------------------------------------------------
                    //                       Descriptors   
                    // ------------------------------------------------------
                    // What they are -> 
                    //  Descriptors can hold useful information about the 
                    //  characteristic and its configuration
                    // 
                    // Note ->
                    //  Some Desciprtors like the CCCD, are special, for ex
                    //  CCCD enables indications / notifications
                    //
                    //  The CCCD descriptor is automatically added....
                    // ------------------------------------------------------

                    { 
                    /* Discriptor: Orientation
                     *
                     * NOTE: this is descriptor, meant to allow clients to "connect/subscribe/access" the attribute value
                     *
                     * -> This attribute can only subscribed/notified and not read manually.
                     */
                      .uuid = &sensor_data_dsc_uuid.u,
                      .att_flags = BLE_ATT_F_READ, // This Descriptor can be read/discovered by the client
                      .access_cb = sensor_data_chr_access, // SAME Callback for handling descriptors as well
                    },

                    {
                      0, /* No more descriptors in this characteristic */
                    }

                },           
              //.flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_INDICATE, // BLE_GATT_CHR_F_INDICATE flag automatically adds CCCD, so no descriptor is required
              .flags = BLE_GATT_CHR_F_NOTIFY, // This value can ONLY BE notified and not read manually
              .val_handle = &sensor_data_chr_val_handle,
             },
             
             {
                 0, /* No more characteristics in this service. */
             }
         }
    },

    {
        0, /* No more services. */
    },
};

// =====================================================================================
// |                 Private Callback Functions for GATT Characteristics
// ===================================================================================== 
// | Callback Functions for the services, which handles access to characteristics data 
// |    Common data accessed -> descriptors (ex. CCCD), attribute values, 
// |
// | Common Parameters to most Callbacks:
// |      ble_gatt_access_ctxt* ctxt: 
// |          1. op field - identify diff. access events (read, write...)
// |              Access Events like : BLE_GATT_ACCESS_OP_WRITE_CHR for write only
// |          2. om field - length of the characteristic 
// |          3. om_data field - data  
// =====================================================================================

static int sensor_data_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                                 struct ble_gatt_access_ctxt *ctxt, void *arg) {
    /* Local variables */
    int rc;
    const ble_uuid_t *uuid;

    /* Handle access events */
    /* Note: Heart rate characteristic is read only */
    switch (ctxt->op) {

    /* Read characteristic event */
    case BLE_GATT_ACCESS_OP_READ_CHR: // if the event is a read operation
        /* Verify connection handle */
        if (conn_handle != BLE_HS_CONN_HANDLE_NONE) {
            ESP_LOGI(TAG, "characteristic read; conn_handle=%d attr_handle=%d",
                     conn_handle, attr_handle);
        } else {
            // ESP_LOGI(TAG, "characteristic read by nimble stack; attr_handle=%d",
                     // attr_handle);
        }

        uuid = ctxt->dsc->uuid;
        /* Verify attribute handle */
        if (attr_handle == sensor_data_chr_val_handle) {
            /* Update access buffer value */
            sensor_data_chr_val = get_sensor_data(SensorValue::QUATERNION); // GET value using Function and store
            rc = os_mbuf_append(ctxt->om, &sensor_data_chr_val,
                                sizeof(sensor_data_chr_val)); // SEND the data via BLE
            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        }
        goto error;

    case BLE_GATT_ACCESS_OP_READ_DSC:
        if (conn_handle != BLE_HS_CONN_HANDLE_NONE) {
            ESP_LOGI(TAG, "Descriptor read; conn_handle=%d attr_handle=%d",
                        conn_handle, attr_handle);
        } else {
            ESP_LOGI(TAG, "Descriptor read by NimBLE stack; attr_handle=%d",
                        attr_handle);
        }
        uuid = ctxt->dsc->uuid;
        if (ble_uuid_cmp(uuid, &sensor_data_dsc_uuid.u) == 0) {
            rc = os_mbuf_append(ctxt->om,
                                &sensor_data_dsc_val,
                                sizeof(sensor_data_dsc_val));
            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        }
        goto error;

    /* Unknown event */
    default:
        goto error;
    }

error:
    ESP_LOGE(
        TAG,
        "unexpected access operation to sensor data characteristic, opcode: %d",
        ctxt->op);
    return BLE_ATT_ERR_UNLIKELY;
}

// =====================================================================================
// |                                 Public Functions
// -------------------------------------------------------------------------------------

// ================== Sensor Data Update Logic ==================
void send_sensor_data_indication(void) {
    if (sensor_data_ind_status && sensor_data_chr_conn_handle_inited) {
        // Refresh your local struct data from your MPU6050 handler
        // HACK:
        sensor_data_chr_val = get_sensor_data(SensorValue::QUATERNION);

        // ESP_LOGI("BLE_GATT", "SENSOR: %f", sensor_data_chr_val.x);

        // Allocate an mbuf from the NimBLE pool and flatten struct
        struct os_mbuf *om = ble_hs_mbuf_from_flat(&sensor_data_chr_val, sizeof(sensor_data_chr_val));
        if (om != NULL) {
            // Fire the indication with the flat data block
            ble_gatts_indicate_custom(sensor_data_chr_conn_handle, sensor_data_chr_val_handle, om);
        } else {
            ESP_LOGE("BLE_GATT", "Failed to allocate mbuf for indication");
        }
    }
}

void send_sensor_data_notifications(void) {
    if (sensor_data_notify_status && sensor_data_chr_conn_handle_inited) {
        // Refresh your local struct data from your MPU6050 handler
        // HACK:
        sensor_data_chr_val = get_sensor_data(SensorValue::QUATERNION);

        // ESP_LOGI("BLE_GATT", "SENSOR: %f", sensor_data_chr_val.x);

        // Allocate an mbuf from the NimBLE pool and flatten struct
        struct os_mbuf *om = ble_hs_mbuf_from_flat(&sensor_data_chr_val, sizeof(sensor_data_chr_val));
        if (om != NULL) {
            // Fire the notification with the flat data block
            ble_gatts_notify_custom(sensor_data_chr_conn_handle, sensor_data_chr_val_handle, om);
        } else {
            ESP_LOGE("BLE_GATT", "Failed to allocate mbuf for notification");
        }
    }
}

/*
 *  Handle GATT attribute register events
 *      - Service register event
 *      - Characteristic register event
 *      - Descriptor register event
 */
void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg) {
    /* Local variables */
    char buf[BLE_UUID_STR_LEN];

    /* Handle GATT attributes register events */
    switch (ctxt->op) {

    /* Service register event */
    case BLE_GATT_REGISTER_OP_SVC:
        ESP_LOGD(TAG, "registered service %s with handle=%d",
                 ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
                 ctxt->svc.handle);
        break;

    /* Characteristic register event */
    case BLE_GATT_REGISTER_OP_CHR:
        ESP_LOGD(TAG,
                 "registering characteristic %s with "
                 "def_handle=%d val_handle=%d",
                 ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                 ctxt->chr.def_handle, ctxt->chr.val_handle);
        break;

    /* Descriptor register event */
    case BLE_GATT_REGISTER_OP_DSC:
        ESP_LOGD(TAG, "registering descriptor %s with handle=%d",
                 ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                 ctxt->dsc.handle);
        break;

    /* Unknown event */
    default:
        assert(0);
        break;
    }
}


// ================== GATT Subscribe Callback on user subscription ==================
/*
 *  GATT server subscribe event callback
 *      1. Update sensor data subscription status
 *
 *  When: Called whenever a client writes to the CCCD descriptor of a characteristic,
 *      and this is done after user "subscribes" (not just after connection)
 * 
 *  Result: The server knows it can push updates to the client
 */
void gatt_svr_subscribe_cb(struct ble_gap_event *event) {
    /* Check connection handle */
    if (event->subscribe.conn_handle != BLE_HS_CONN_HANDLE_NONE) {
        ESP_LOGI(TAG, "subscribe event; conn_handle=%d attr_handle=%d",
                 event->subscribe.conn_handle, event->subscribe.attr_handle);
    } else {
       //  ESP_LOGI(TAG, "subscribe by nimble stack; attr_handle=%d",
          //       event->subscribe.attr_handle);
    }

    /* Check attribute handle */
    if (event->subscribe.attr_handle == sensor_data_chr_val_handle) { // if the handler is a sensor data handler event 
        /* Update sensor data subscription status */
        sensor_data_chr_conn_handle = event->subscribe.conn_handle; // tells the client's handle
        sensor_data_chr_conn_handle_inited = true; // Subscription is initialized
        // sensor_data_ind_status = event->subscribe.cur_indicate; // indication?
        sensor_data_notify_status = event->subscribe.cur_notify; // notification?
    }
}


// ================== GATT INIT ==================
/*
 *  GATT server initialization
 *      1. Initialize GATT service
 *      2. Update NimBLE host GATT services counter
 *      3. Add GATT services to server
 */
int gatt_svc_init(void) {
    /* Local variables */
    int rc;

    /* 1. GATT service initialization */
    ble_svc_gatt_init();

    /* 2. Update GATT services counter */
    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

    /* 3. Add GATT services */
    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

    return 0;
}
