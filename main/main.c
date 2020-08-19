#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <esp_err.h>

#include "esp_log.h"

#include "esp_ble_mesh_common_api.h"
#include "esp_ble_mesh_provisioning_api.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_ble_mesh_config_model_api.h"
#include "esp_ble_mesh_generic_model_api.h"

#include "ble_mesh_init.h"

// TODO check for examples about ble mesh :$IDF_PATH/examples/bluetooth/esp_ble_mesh/ble_mesh_node/onoff_server/main/main.c

#define TAG "MAIN"

void app_main(void) {
    esp_err_t err;

    ESP_LOGI(TAG, "Initializing...");

    err = bluetooth_init();
    if (err) {
        ESP_LOGE(TAG, "esp32_bluetooth_init failed (err %d)", err);
    }

    ble_mesh_get_dev_uuid(dev_uuid);

    /* Initialize the Bluetooth Mesh Subsystem */
    err = ble_mesh_init();
    if (err) {
        ESP_LOGE(TAG, "Bluetooth mesh init failed (err %d)", err);
    }
}
