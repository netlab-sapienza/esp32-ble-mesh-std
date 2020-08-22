#include <esp_err.h>
#include <nvs_flash.h>

#include "esp_log.h"
#include "ble_init.h"
#include "server.h"
#include "client.h"

#define TAG "MAIN"
#define SERVER true
#define CLIENT true
#define ATTACKER true


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
    if (SERVER) {
        // server init
        err = ble_mesh_init_server();
    } else if (CLIENT) {
        // client init
        err = ble_mesh_init_client();
    } else {
        err = ESP_FAIL;
    }

    if (err) {
        ESP_LOGE(TAG, "Bluetooth mesh init failed (err %d)", err);
    }


}
