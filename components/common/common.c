//
// Created by thecave3 on 22/08/20.
//

#include <esp_ble_mesh_config_model_api.h>
#include <esp_ble_mesh_generic_model_api.h>

#include "common.h"

#define TAG "COMMON"
#define TAG_LOG "BenchMark"

uint8_t dev_uuid[16] = {0xdd, 0xdd};

esp_ble_mesh_cfg_srv_t config_server = {
        .relay = ESP_BLE_MESH_RELAY_DISABLED,
        .beacon = ESP_BLE_MESH_BEACON_ENABLED,
#if defined(CONFIG_BLE_MESH_FRIEND)
        .friend_state = ESP_BLE_MESH_FRIEND_ENABLED,
#else
        .friend_state = ESP_BLE_MESH_FRIEND_NOT_SUPPORTED,
#endif
#if defined(CONFIG_BLE_MESH_GATT_PROXY_SERVER)
        .gatt_proxy = ESP_BLE_MESH_GATT_PROXY_ENABLED,
#else
        .gatt_proxy = ESP_BLE_MESH_GATT_PROXY_NOT_SUPPORTED,
#endif
        .default_ttl = 7,
        /* 3 transmissions with 20ms interval */
        .net_transmit = ESP_BLE_MESH_TRANSMIT(2, 20),
        .relay_retransmit = ESP_BLE_MESH_TRANSMIT(2, 20),
};

/* Disable OOB security for SILabs Android app */
esp_ble_mesh_prov_t provision = {
        .uuid = dev_uuid,
#if 0
        .output_size = 4,
    .output_actions = ESP_BLE_MESH_DISPLAY_NUMBER,
    .input_actions = ESP_BLE_MESH_PUSH,
    .input_size = 4,
#else
        .output_size = 0,
        .output_actions = 0,
#endif
};

// TODO upgrade to latest toolchain to get rssi
// TODO add on read state
// TODO complete according paper
void log_ble_mesh_packet(esp_ble_mesh_generic_server_cb_param_t *param) {
    ESP_LOGI(TAG_LOG,
             "net_idx 0x%04x, app_idx 0x%04x, src 0x%04x, dest 0x%04x, rcv_rssi 0x%02x, recv_ttl 0x%02x, send_rel 0x%02x, send_ttl 0x%02x, opcode 0x%08x, srv_send %s",
             param->ctx.net_idx, param->ctx.app_idx, param->ctx.addr, param->ctx.recv_dst, param->ctx.recv_rssi,
             param->ctx.recv_ttl, param->ctx.send_rel, param->ctx.send_ttl,
             param->ctx.recv_op, param->ctx.srv_send ? "true" : "false");
}
