//
// Created by thecave3 on 22/08/20.
//

#ifndef ESP32_BLE_MESH_STD_COMMON_H
#define ESP32_BLE_MESH_STD_COMMON_H

#define CID_ESP 0x02E5

uint8_t dev_uuid[16];

esp_ble_mesh_cfg_srv_t config_server;

esp_ble_mesh_prov_t provision;

void log_ble_mesh_packet(esp_ble_mesh_generic_server_cb_param_t *param);

#endif //ESP32_BLE_MESH_STD_COMMON_H
