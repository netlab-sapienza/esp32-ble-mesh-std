//
// Created by thecave3 on 22/08/20.
//

#ifndef ESP32_BLE_MESH_STD_COMMON_H
#define ESP32_BLE_MESH_STD_COMMON_H

#include <nvs.h>

#define CID_ESP 0x02E5

uint8_t dev_uuid[16];

esp_ble_mesh_cfg_srv_t config_server;

esp_ble_mesh_prov_t provision;

void log_ble_mesh_generic_rcv_server_packet(char *role, esp_ble_mesh_generic_server_cb_param_t *param);

void log_ble_mesh_config_server_packet(char *role, esp_ble_mesh_cfg_server_cb_param_t *param);

void log_ble_mesh_client_packet(char *role, esp_ble_mesh_generic_client_cb_event_t event,
                                esp_ble_mesh_generic_client_cb_param_t *param);

esp_err_t ble_mesh_nvs_open(nvs_handle_t *handle);

esp_err_t ble_mesh_nvs_store(nvs_handle_t handle, const char *key, const void *data, size_t length);

esp_err_t ble_mesh_nvs_get_length(nvs_handle_t handle, const char *key, size_t *length);

esp_err_t ble_mesh_nvs_restore(nvs_handle_t handle, const char *key, void *data, size_t length, bool *exist);

esp_err_t ble_mesh_nvs_erase(nvs_handle_t handle, const char *key);

#endif //ESP32_BLE_MESH_STD_COMMON_H
