#include <limits.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


// TODO check for examples about ble mesh :$IDF_PATH/examples/bluetooth/esp_ble_mesh/ble_mesh_node/onoff_server/main/main.c


_Noreturn void app_main(void)
{
    int i = 0;
    while (1) {
        printf("[%d] Hello world!\n", i);
        i++;
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
