#include <esp_err.h>
#include <nvs_flash.h>

#include "esp_log.h"
#include "ble_init.h"
#include "server.h"
#include "client.h"
#include "relay.h"

#define TAG "MAIN"

#define ROLE 1

#define SERVER 0
#define CLIENT 1
#define RELAY 2


void app_main(void) {
    esp_err_t err;

    ESP_LOGI(TAG, "Initializing...");

    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);


    err = bluetooth_init();
    if (err) {
        ESP_LOGE(TAG, "esp32_bluetooth_init failed (err %d)", err);
    }

    /* Initialize the Bluetooth Mesh Subsystem according to the predefined role */
    switch (ROLE) {
        case SERVER:
            err = ble_mesh_init_server();
            break;
        case CLIENT:
            err = ble_mesh_init_client();
            break;
        case RELAY:
            err = ble_mesh_init_relay();
            break;

        default:
            err = ESP_FAIL;
    }

    if (err) {
        ESP_LOGE(TAG, "Bluetooth mesh init failed (err %d)", err);
    }
}
