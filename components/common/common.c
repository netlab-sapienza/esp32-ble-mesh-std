//
// Created by thecave3 on 22/08/20.
//

#include <esp_ble_mesh_config_model_api.h>
#include <esp_ble_mesh_generic_model_api.h>
#include <math.h>

#include "common.h"

#define TAG "COMMON"
#define TAG_LOG "BenchMark"
#define NVS_NAME "mesh_levl_nvs"

#define DEV_UUID_BUFFER_SIZE 16
#define LOG_BUF_SIZE 512
#define CONFIG_BLE_MESH_FRIEND 


uint8_t dev_uuid[DEV_UUID_BUFFER_SIZE] = {0xdd, 0xdd};


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


int print_address(char *buf, uint8_t *address, int size) {
    int wb = 0;
    for (int i = 0; i < size; ++i) {
        int ret = sprintf(buf + wb, "%X", address[i]);
        wb += ret;
    }
    return wb;
}

int print_dev_addr(char *buf) {
    return print_address(buf, dev_uuid, DEV_UUID_BUFFER_SIZE);
}

// TODO rewrite doc
/**
 * Write into buf the common ble mesh model fields
 *
 * @param uuid UUID representing the device composed by definition of 2 bytes {0xdd, 0xdd} + 6 bytes of BDA
 * @param role representing the ROLE of the device within the network
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
int write_ble_mesh_model_packet_common(char *buf, char *role, uint16_t net_idx, uint16_t app_idx, uint16_t addr,
                                       uint16_t recv_dst,
                                       int8_t recv_rssi, uint8_t recv_ttl, uint8_t send_rel, uint8_t send_ttl,
                                       bool srv_send, uint32_t recv_op) {
    int n = 0;
    n += sprintf(buf + n, " M ");
    n += print_dev_addr(buf + n);
    n += sprintf(buf + n, " %s ", role);
    n += sprintf(buf + n, " 0x%04x ", net_idx);
    n += sprintf(buf + n, " 0x%04x ", app_idx);
    n += sprintf(buf + n, " 0x%04x ", addr);
    n += sprintf(buf + n, " 0x%04x ", recv_dst);
    n += sprintf(buf + n, " %d ", recv_rssi);
    n += sprintf(buf + n, " %hhu ", recv_ttl);
    n += sprintf(buf + n, " %hhu ", send_rel);
    n += sprintf(buf + n, " %hhu ", send_ttl);
    n += sprintf(buf + n, " %d ", srv_send);
    n += sprintf(buf + n, " 0x%08x ", recv_op);

    return n;
}

void log_ble_mesh_generic_rcv_server_packet(char *role, esp_ble_mesh_generic_server_cb_param_t *param) {
    char buf[LOG_BUF_SIZE];
    int n = write_ble_mesh_model_packet_common(buf, role, param->ctx.net_idx, param->ctx.app_idx, param->ctx.addr,
                                               param->ctx.recv_dst,
                                               param->ctx.recv_rssi,
                                               param->ctx.recv_ttl, param->ctx.send_rel, param->ctx.send_ttl,
                                               param->ctx.srv_send, param->ctx.recv_op);

    if (param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_GET) {
        n += sprintf(buf + n, " 0x%04x ", param->value.get.admin_property.property_id);
        n += sprintf(buf + n, " 0x%04x ", param->value.get.manu_property.property_id);
        n += sprintf(buf + n, " 0x%04x ", param->value.get.client_properties.property_id);
        n += sprintf(buf + n, " 0x%04x ", param->value.get.user_property.property_id);

    } else if (param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_SET ||
               param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_SET_UNACK) {
        n += sprintf(buf + n, " %d ", param->value.set.level.level);
        n += sprintf(buf + n, " %d ", param->value.set.level.tid);
        n += sprintf(buf + n, " %d ", param->value.set.level.op_en);

        if (param->value.set.level.op_en) {
            n += sprintf(buf + n, " %d ", param->value.set.level.trans_time);
            n += sprintf(buf + n, " %d ", param->value.set.level.delay * 5); // step time of a single delay = 5ms
        }
    }

//    ESP_LOGI(TAG_LOG, "%d", n);
    ESP_LOGI(TAG_LOG, "%s", buf);

}

void log_ble_mesh_config_server_packet(char *role, esp_ble_mesh_cfg_server_cb_param_t *param) {
    char buf[LOG_BUF_SIZE];
    int n = write_ble_mesh_model_packet_common(buf, role, param->ctx.net_idx, param->ctx.app_idx, param->ctx.addr,
                                               param->ctx.recv_dst,
                                               param->ctx.recv_rssi,
                                               param->ctx.recv_ttl, param->ctx.send_rel, param->ctx.send_ttl,
                                               param->ctx.srv_send, param->ctx.recv_op);
    switch (param->ctx.recv_op) {

        case ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD:
            n += sprintf(buf + n, " 0x%04x ", param->value.state_change.appkey_add.net_idx);
            n += sprintf(buf + n, " 0x%04x ", param->value.state_change.appkey_add.app_idx);
            print_address(buf + n, param->value.state_change.appkey_add.app_key, 16);
            break;

        case ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND:
            n += sprintf(buf + n, " 0x%04x ", param->value.state_change.mod_app_bind.element_addr);
            n += sprintf(buf + n, " 0x%04x ", param->value.state_change.mod_app_bind.app_idx);
            n += sprintf(buf + n, " 0x%04x ", param->value.state_change.mod_app_bind.company_id);
            sprintf(buf + n, " 0x%04x ", param->value.state_change.mod_app_bind.model_id);
            break;

        case ESP_BLE_MESH_MODEL_OP_MODEL_SUB_ADD:
            n += sprintf(buf + n, " 0x%04x ", param->value.state_change.mod_sub_add.element_addr);
            n += sprintf(buf + n, " 0x%04x ", param->value.state_change.mod_sub_add.sub_addr);
            n += sprintf(buf + n, " 0x%04x ", param->value.state_change.mod_sub_add.company_id);
            sprintf(buf + n, " 0x%04x ", param->value.state_change.mod_sub_add.model_id);
            break;

        default:
            ESP_LOGE(TAG_LOG, "Unhandled case  0x%08x", param->ctx.recv_op);

    }

    ESP_LOGI(TAG_LOG, "%s", buf);
}

void log_ble_mesh_client_packet(char *role, esp_ble_mesh_generic_client_cb_event_t event,
                                esp_ble_mesh_generic_client_cb_param_t *param) {
    char buf[LOG_BUF_SIZE];
    int n = write_ble_mesh_model_packet_common(buf, role, param->params->ctx.net_idx, param->params->ctx.app_idx,
                                               param->params->ctx.addr,
                                               param->params->ctx.recv_dst,
                                               param->params->ctx.recv_rssi,
                                               param->params->ctx.recv_ttl, param->params->ctx.send_rel,
                                               param->params->ctx.send_ttl,
                                               param->params->ctx.srv_send, param->params->ctx.recv_op);

//  we associate arbitrarily a identifier code for each event
    switch (event) {
        case ESP_BLE_MESH_GENERIC_CLIENT_GET_STATE_EVT: // 0
            n += sprintf(buf + n, " %d ", 0);
            if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_GET) {
                n += sprintf(buf + n, " %d ", param->status_cb.level_status.present_level);
                n += sprintf(buf + n, " %d ", param->status_cb.level_status.remain_time);
                n += sprintf(buf + n, " %d ", param->status_cb.level_status.op_en);
                if (param->status_cb.level_status.op_en)
                    sprintf(buf + n, " %d ", param->status_cb.level_status.target_level);
            }
            break;
        case ESP_BLE_MESH_GENERIC_CLIENT_SET_STATE_EVT: // 1
            n += sprintf(buf + n, " %d ", 1);
            if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_SET) {
                n += sprintf(buf + n, " %d ", param->status_cb.level_status.present_level);
                n += sprintf(buf + n, " %d ", param->status_cb.level_status.remain_time);
                n += sprintf(buf + n, " %d ", param->status_cb.level_status.op_en);
                if (param->status_cb.level_status.op_en)
                    sprintf(buf + n, " %d ", param->status_cb.level_status.target_level);
            }
            break;
        case ESP_BLE_MESH_GENERIC_CLIENT_PUBLISH_EVT: // 2
            sprintf(buf + n, " %d ", 2);
            break;

        default:
            break;
    }

    ESP_LOGI(TAG_LOG, "%s", buf);
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

//    ESP_LOGI(TAG, "Store, key \"%s\", length %u", key, length);
//    ESP_LOG_BUFFER_HEX("EXAMPLE_NVS: Store, data", data, length);
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




// this portion of the code has to be put inside /esp-idf/components/bt/esp_ble_mesh/mesh_core/net.c


// line 1157 funzione di log:


//void log_ble_mesh_network_packet(char *role, struct bt_mesh_net_rx *rx, struct net_buf_simple *buf) {
//
//    // rx->old_iv solo a scopo di test da eliminare in forma finale
//
//    ESP_LOGI("BenchMark", " N %s %u 0x%04x 0x%04x 0x%04x 0x%04x %hhd %hhu %hu",
//             role, rx->seq, rx->ctx.net_idx,
//             rx->ctx.app_idx, rx->ctx.addr, rx->ctx.recv_dst,
//             rx->ctx.recv_rssi, rx->ctx.recv_ttl, buf->len);
//}

// line 1196: eventuale funzione di drop e di attacco pacchetto

//true random distribution used to decide whenever drop the packet
bool drop_the_packet() {
    uint32_t threshold = UINT32_MAX / 2;

    return (esp_random() > threshold);

}

// pseudo random normal distribution (mean 0 stddev 1)
double sampleNormal() {
    double u = ((double) esp_random() / (UINT32_MAX)) * 2 - 1;
    double v = ((double) esp_random() / (UINT32_MAX)) * 2 - 1;
    double r = u * u + v * v;
    if (r == 0 || r > 1) return sampleNormal();
    double c = sqrt(-2 * log(r) / r);
    return u * c;
}


bool pseudo_random_drop_the_packet() {
    return ((int) sampleNormal()) % 2;
}

// line 1411 circa
// targeted attack versus a given client
//bool drop_targeted(struct bt_mesh_net_rx *rx, uint16_t target_addr) {
//    uint32_t threshold = UINT32_MAX / 2;
//    return esp_random() > threshold && rx->ctx.addr == target_addr;
//}
//// inside bt_mesh_net_recv
//if(drop_targeted(rx,)){
//ESP_LOGI("ATTACK","Dropped packet!");
//return;
//}
// line 1354: log
//log_ble_mesh_network_packet("RELAY", rx);
//log_ble_mesh_network_packet("SERVER", rx);
//log_ble_mesh_network_packet("CLIENT", rx);
