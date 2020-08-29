//
// Created by thecave3 on 22/08/20.
//

#include <esp_ble_mesh_config_model_api.h>
#include <esp_ble_mesh_generic_model_api.h>

#include "common.h"

#define TAG "COMMON"
#define TAG_LOG "BenchMark"
#define NVS_NAME "mesh_levl_nvs"

uint8_t dev_uuid[16] = {0xdd, 0xdd};

esp_ble_mesh_cfg_srv_t config_server = {
        .relay = ESP_BLE_MESH_RELAY_ENABLED,
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
        .default_ttl = 7, // default ttl value is 7 it decide the number of hops in the network
        /* 3 transmissions with 20ms interval */
        .net_transmit = ESP_BLE_MESH_TRANSMIT(2, 20),
        .relay_retransmit = ESP_BLE_MESH_TRANSMIT(2, 20)
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

// TODO add on read state
// TODO complete according paper
/**
 * Log the ble mesh packet
 *
 * @param uuid UUID representing the device composed by definition of 2 bytes {0xdd, 0xdd} + 6 bytes of BDA
 * @param tag representing the ROLE of the device within the network
 * @param net_idx NetKey Index of the subnet
 * @param app_idx AppKey Index for message encryption.
 * @param addr Source address
 * @param recv_dst Destination address of a received message.
 * @param recv_rssi RSSI of received packet.
 * @param recv_ttl Received TTL value.
 * @param send_rel Force sending reliably by using segment acknowledgement
 * @param send_ttl TTL (time to live) of the packet, or ESP_BLE_MESH_TTL_DEFAULT for default TTL
 * @param recv_op Opcode of a received message.
 * @param srv_send Indicate if the message is sent by a node server model.
 */
void log_ble_mesh_packet(uint8_t *uuid, char *tag, uint16_t net_idx, uint16_t app_idx, uint16_t addr,
                         uint16_t recv_dst,
                         int8_t recv_rssi, uint8_t recv_ttl, uint8_t send_rel, uint8_t send_ttl, uint32_t recv_op,
                         bool srv_send) {
    // TODO errore stampa uuid
    ESP_LOGI(TAG_LOG,
             "uuid %x ,tag %s ,net_idx 0x%04x, app_idx 0x%04x, src 0x%04x, dest 0x%04x, rcv_rssi %d, recv_ttl %d, send_rel %d, "
             "send_ttl %d, opcode 0x%08x, srv_send %s", *uuid, tag,
             net_idx, app_idx, addr, recv_dst, recv_rssi, recv_ttl, send_rel, send_ttl,
             recv_op, srv_send ? "true" : "false");
}

void log_ble_mesh_generic_rcv_server_packet(char *tag, esp_ble_mesh_generic_server_cb_param_t *param) {
    if (param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_GET) {
        log_ble_mesh_packet(dev_uuid, tag, param->ctx.net_idx, param->ctx.app_idx, param->ctx.addr, param->ctx.recv_dst,
                            param->ctx.recv_rssi,
                            param->ctx.recv_ttl, param->ctx.send_rel, param->ctx.send_ttl,
                            param->ctx.recv_op, param->ctx.srv_send);

    } else if (param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_SET ||
               param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_SET_UNACK) {
        // TODO log

//        param->value.set.level.level;
//        param->value.set.level.tid;
//        param->value.set.level.op_en;
//        param->value.set.level.trans_time;
//        param->value.set.level.delay;

        log_ble_mesh_packet(dev_uuid, tag, param->ctx.net_idx, param->ctx.app_idx, param->ctx.addr, param->ctx.recv_dst,
                            param->ctx.recv_rssi,
                            param->ctx.recv_ttl, param->ctx.send_rel, param->ctx.send_ttl,
                            param->ctx.recv_op, param->ctx.srv_send);

    }

}

void log_ble_mesh_config_server_packet(char *tag, esp_ble_mesh_cfg_server_cb_param_t *param) {
    log_ble_mesh_packet(dev_uuid, tag, param->ctx.net_idx, param->ctx.app_idx, param->ctx.addr, param->ctx.recv_dst,
                        param->ctx.recv_rssi,
                        param->ctx.recv_ttl, param->ctx.send_rel, param->ctx.send_ttl,
                        param->ctx.recv_op, param->ctx.srv_send);
}

void log_ble_mesh_client_packet(char *tag, esp_ble_mesh_client_common_param_t *params) {
    log_ble_mesh_packet(dev_uuid, tag, params->ctx.net_idx, params->ctx.app_idx, params->ctx.addr, params->ctx.recv_dst,
                        params->ctx.recv_rssi,
                        params->ctx.recv_ttl, params->ctx.send_rel, params->ctx.send_ttl,
                        params->ctx.recv_op, params->ctx.srv_send);
}


esp_err_t ble_mesh_nvs_open(nvs_handle_t *handle) {
    esp_err_t err = ESP_OK;
    if (handle == NULL) {
        ESP_LOGE(TAG, "Open invalid nvs handle");
        return ESP_ERR_INVALID_ARG;
    }

    err = nvs_open(NVS_NAME, NVS_READWRITE, handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Open, nvs_open failed, err %d", err);
        return err;
    }

    ESP_LOGI(TAG, "Open namespace done, name \"%s\"", NVS_NAME);
    return err;
}

esp_err_t ble_mesh_nvs_store(nvs_handle_t handle, const char *key, const void *data, size_t length) {
    esp_err_t err = ESP_OK;

    if (key == NULL || data == NULL || length == 0) {
        ESP_LOGE(TAG, "Store, invalid parameter");
        return ESP_ERR_INVALID_ARG;
    }

    err = nvs_set_blob(handle, key, data, length);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Store, nvs_set_blob failed, err %d", err);
        return err;
    }

    err = nvs_commit(handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Store, nvs_commit failed, err %d", err);
        return err;
    }

    ESP_LOGI(TAG, "Store, key \"%s\", length %u", key, length);
    ESP_LOG_BUFFER_HEX("EXAMPLE_NVS: Store, data", data, length);
    return err;
}

esp_err_t ble_mesh_nvs_get_length(nvs_handle_t handle, const char *key, size_t *length) {
    esp_err_t err = ESP_OK;

    if (key == NULL || length == NULL) {
        ESP_LOGE(TAG, "Get length, invalid parameter");
        return ESP_ERR_INVALID_ARG;
    }

    err = nvs_get_blob(handle, key, NULL, length);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGI(TAG, "Get length, key \"%s\" not exists", key);
        *length = 0;
        return ESP_OK;
    }

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Get length, nvs_get_blob failed, err %d", err);
    } else {
        ESP_LOGI(TAG, "Get length, key \"%s\", length %u", key, *length);
    }

    return err;
}

esp_err_t ble_mesh_nvs_restore(nvs_handle_t handle, const char *key, void *data, size_t length, bool *exist) {
    esp_err_t err = ESP_OK;

    if (key == NULL || data == NULL || length == 0) {
        ESP_LOGE(TAG, "Restore, invalid parameter");
        return ESP_ERR_INVALID_ARG;
    }

    err = nvs_get_blob(handle, key, data, &length);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGI(TAG, "Restore, key \"%s\" not exists", key);
        if (exist) {
            *exist = false;
        }
        return ESP_OK;
    }

    if (exist) {
        *exist = true;
    }

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Restore, nvs_get_blob failed, err %d", err);
    } else {
        ESP_LOGI(TAG, "Restore, key \"%s\", length %u", key, length);
        ESP_LOG_BUFFER_HEX("EXAMPLE_NVS: Restore, data", data, length);
    }

    return err;
}

esp_err_t ble_mesh_nvs_erase(nvs_handle_t handle, const char *key) {
    esp_err_t err = ESP_OK;

    if (key) {
        err = nvs_erase_key(handle, key);
        if (err == ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGI(TAG, "Erase, key \"%s\" not exists", key);
            return ESP_OK;
        }
    } else {
        err = nvs_erase_all(handle);
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Erase, nvs_erase_%s failed, err %d", key ? "key" : "all", err);
        return err;
    }

    err = nvs_commit(handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Erase, nvs_commit failed, err %d", err);
        return err;
    }

    if (key) {
        ESP_LOGI(TAG, "Erase done, key \"%s\"", key);
    } else {
        ESP_LOGI(TAG, "Erase namespace done, name \"%s\"", NVS_NAME);
    }
    return err;
}

