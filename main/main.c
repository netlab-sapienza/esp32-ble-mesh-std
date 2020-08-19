#include <esp_err.h>

#include "esp_log.h"
#include "ble_init.h"

#define TAG "MAIN"
#define SERVER false
#define CLIENT false

static uint8_t dev_uuid[16] = {0xdd, 0xdd};

void app_main(void) {
    esp_err_t err;

    ESP_LOGI(TAG, "Initializing...");

    err = bluetooth_init();
    if (err) {
        ESP_LOGE(TAG, "esp32_bluetooth_init failed (err %d)", err);
    }

    ble_mesh_get_dev_uuid(dev_uuid);

    /* Initialize the Bluetooth Mesh Subsystem according to the predefined role */
    if (SERVER) {
        //        server init
    } else if (CLIENT) {
        // client init
    } else {
        err = ESP_FAIL;
    }

    if (err) {
        ESP_LOGE(TAG, "Bluetooth mesh init failed (err %d)", err);
    }


}
