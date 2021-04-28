//
// Created by thecave3 on 19/08/20.
//
#include <stdio.h>
#include <string.h>
#include <ble_init.h>

#include "esp_log.h"
#include "nvs_flash.h"

#include "esp_ble_mesh_common_api.h"
#include "esp_ble_mesh_provisioning_api.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_ble_mesh_config_model_api.h"
#include "esp_ble_mesh_generic_model_api.h"
#include "esp_ble_mesh_low_power_api.h"

#include "client.h"
#include "common.h"

#define TAG "CLIENT"
#define CONFIG_BLE_MESH_FRIEND

esp_ble_mesh_cfg_srv_t config_server_client = {
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

static struct info_store {
    uint16_t net_idx;   /* NetKey Index */
    uint16_t app_idx;   /* AppKey Index */
    uint16_t level;     /* Level value */
    uint8_t tid;       /* Message TID (Transaction ID) */
    uint8_t trans_time; /* Transition time */
    uint8_t delay; /* Delay */
} __attribute__((packed)) store = {
        .net_idx = ESP_BLE_MESH_KEY_UNUSED,
        .app_idx = ESP_BLE_MESH_KEY_UNUSED,
        .level = 0,
        .tid = 0x0,
};

static nvs_handle_t NVS_HANDLE;
static const char *NVS_KEY = "level_client";

static esp_ble_mesh_client_t level_client;

ESP_BLE_MESH_MODEL_PUB_DEFINE(level_client_model, 2 + 1, ROLE_NODE);

static esp_ble_mesh_model_t root_models[] = {
        ESP_BLE_MESH_MODEL_CFG_SRV(
                &config_server_client), // This model is a server because it works for configuring the node
        ESP_BLE_MESH_MODEL_GEN_LEVEL_CLI(&level_client_model, &level_client),
};

static esp_ble_mesh_elem_t elements[] = {
        ESP_BLE_MESH_ELEMENT(0, root_models, ESP_BLE_MESH_MODEL_NONE),
};

static esp_ble_mesh_comp_t composition = {
        .cid = CID_ESP,
        .elements = elements,
        .element_count = ARRAY_SIZE(elements),
};

static void mesh_info_store(void) {
    ble_mesh_nvs_store(NVS_HANDLE, NVS_KEY, &store, sizeof(store));
}


static void mesh_info_restore(void) {
    esp_err_t err = ESP_OK;
    bool exist = false;

    err = ble_mesh_nvs_restore(NVS_HANDLE, NVS_KEY, &store, sizeof(store), &exist);
    if (err != ESP_OK) {
        return;
    }

    if (exist) {
        ESP_LOGI(TAG, "Restore, net_idx 0x%04x, app_idx 0x%04x, level %d, tid 0x%02x",
                 store.net_idx, store.app_idx, store.level, store.tid);
    }
}


static void prov_complete(uint16_t net_idx, uint16_t addr, uint8_t flags, uint32_t iv_index) {
    ESP_LOGI(TAG, "Provision Complete");
    ESP_LOGI(TAG, "net_idx: 0x%04x, addr: 0x%04x", net_idx, addr);
    ESP_LOGI(TAG, "flags: 0x%02x, iv_index: 0x%08x", flags, iv_index);
    store.net_idx = net_idx;
    /* mesh_example_info_store() shall not be invoked here, because if the device
     * is restarted and goes into a provisioned state, then the following events
     * will come:
     * 1st: ESP_BLE_MESH_NODE_PROV_COMPLETE_EVT
     * 2nd: ESP_BLE_MESH_PROV_REGISTER_COMP_EVT
     * So the store.net_idx will be updated here, and if we store the mesh example
     * info here, the wrong app_idx (initialized with 0xFFFF) will be stored in nvs
     * just before restoring it.
     */
}

static void ble_mesh_provisioning_cb(esp_ble_mesh_prov_cb_event_t event, esp_ble_mesh_prov_cb_param_t *param) {
    switch (event) {
        case ESP_BLE_MESH_PROV_REGISTER_COMP_EVT:
            ESP_LOGI(TAG, "ESP_BLE_MESH_PROV_REGISTER_COMP_EVT, err_code %d", param->prov_register_comp.err_code);
            mesh_info_restore();
            break;
        case ESP_BLE_MESH_NODE_PROV_ENABLE_COMP_EVT:
            ESP_LOGI(TAG, "ESP_BLE_MESH_NODE_PROV_ENABLE_COMP_EVT, err_code %d", param->node_prov_enable_comp.err_code);
            break;
        case ESP_BLE_MESH_NODE_PROV_LINK_OPEN_EVT:
            ESP_LOGI(TAG, "ESP_BLE_MESH_NODE_PROV_LINK_OPEN_EVT, bearer %s",
                     param->node_prov_link_open.bearer == ESP_BLE_MESH_PROV_ADV ? "PB-ADV" : "PB-GATT");
            break;
        case ESP_BLE_MESH_NODE_PROV_LINK_CLOSE_EVT:
            ESP_LOGI(TAG, "ESP_BLE_MESH_NODE_PROV_LINK_CLOSE_EVT, bearer %s",
                     param->node_prov_link_close.bearer == ESP_BLE_MESH_PROV_ADV ? "PB-ADV" : "PB-GATT");
            break;
        case ESP_BLE_MESH_NODE_PROV_COMPLETE_EVT:
            ESP_LOGI(TAG, "ESP_BLE_MESH_NODE_PROV_COMPLETE_EVT");
            prov_complete(param->node_prov_complete.net_idx, param->node_prov_complete.addr,
                          param->node_prov_complete.flags, param->node_prov_complete.iv_index);
            break;
        case ESP_BLE_MESH_NODE_PROV_RESET_EVT:
            break;
        case ESP_BLE_MESH_NODE_SET_UNPROV_DEV_NAME_COMP_EVT:
            ESP_LOGI(TAG, "ESP_BLE_MESH_NODE_SET_UNPROV_DEV_NAME_COMP_EVT, err_code %d",
                     param->node_set_unprov_dev_name_comp.err_code);
            break;
        case ESP_BLE_MESH_LPN_FRIENDSHIP_ESTABLISH_EVT :
            ESP_LOGI(TAG, "ESP_BLE_MESH_LPN_FRIENDSHIP_ESTABLISH_EVT, friend addr %u",
                     (unsigned int) param->lpn_friendship_establish.friend_addr);
            break;
        case ESP_BLE_MESH_LPN_POLL_COMP_EVT: 
            ESP_LOGI(TAG, "ESP_BLE_MESH_LPN_POLL_COMP_EVT, err_code %d",
                     param->lpn_poll_comp.err_code);
            break;
        case ESP_BLE_MESH_LPN_FRIENDSHIP_TERMINATE_EVT: 
            ESP_LOGI(TAG, "ESP_BLE_MESH_LPN_FRIENDSHIP_TERMINATE_EVT, friend addr %u",
                     (unsigned int) param->lpn_friendship_terminate.friend_addr);
            break;
        case ESP_BLE_MESH_FRIEND_FRIENDSHIP_ESTABLISH_EVT: 
            ESP_LOGI(TAG, "ESP_BLE_MESH_FRIEND_FRIENDSHIP_ESTABLISH_EVT, lpn addr %u",
                     (unsigned int) param->frnd_friendship_establish.lpn_addr);
            break;
        case ESP_BLE_MESH_FRIEND_FRIENDSHIP_TERMINATE_EVT: 
            ESP_LOGI(TAG, "ESP_BLE_MESH_FRIEND_FRIENDSHIP_TERMINATE_EVT, lpn addr %u, reason %d",
                     (unsigned int) param->frnd_friendship_terminate.lpn_addr,param->frnd_friendship_terminate.reason);
            break;
        default:
            break;
    }
}


void ble_mesh_send_test_gen_level_get(void) {

    esp_err_t err = ESP_OK;

    esp_ble_mesh_generic_client_get_state_t get = {0};

    esp_ble_mesh_client_common_param_t common = {0};

    common.opcode = ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_GET;
    common.model = level_client.model;
    common.ctx.net_idx = store.net_idx;
    common.ctx.app_idx = store.app_idx;
    common.ctx.addr = 0xFFFF;   /* to all nodes */
    common.ctx.send_ttl = 3;
    common.ctx.send_rel = false;
    common.msg_timeout = 0;     /* 0 indicates that timeout value from menuconfig will be used */
    common.msg_role = ROLE_NODE;

    err = esp_ble_mesh_generic_client_get_state(&common, &get);

    if (err) {
        ESP_LOGE(TAG, "Send Generic Level Get failed, %x", err);
        ESP_ERROR_CHECK_WITHOUT_ABORT(err);
        return;
    }

}


void ble_mesh_send_gen_level_set(void) {

    esp_ble_mesh_generic_client_set_state_t set = {0};
    esp_ble_mesh_client_common_param_t common = {0};
    esp_err_t err = ESP_OK;

    common.opcode = ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_SET_UNACK;
    common.model = level_client.model;

    common.ctx.net_idx = store.net_idx;
    common.ctx.app_idx = store.app_idx;
    common.ctx.addr = 0xFFFF;   /* to all nodes */
    common.ctx.send_ttl = 3;
    common.ctx.send_rel = false;
    common.msg_timeout = 0;     /* 0 indicates that timeout value from menuconfig will be used */
    common.msg_role = ROLE_NODE;

//    set.level_set.op_en = false;
    set.level_set.level = store.level;
    set.level_set.tid = store.tid++;

    set.level_set.op_en = true;
    set.level_set.delay = store.delay;
    set.level_set.trans_time = store.trans_time;

    err = esp_ble_mesh_generic_client_set_state(&common, &set);
    if (err) {
        ESP_LOGE(TAG, "Send Generic Level Set Unack failed, %x", err);
        ESP_ERROR_CHECK_WITHOUT_ABORT(err);
        return;
    }

    store.level++;

    mesh_info_store();
}

static void ble_mesh_generic_client_cb(esp_ble_mesh_generic_client_cb_event_t event,
                                       esp_ble_mesh_generic_client_cb_param_t *param) {

    ESP_LOGI(TAG, "Generic client, event %u, error code %d, opcode is 0x%04x",
             event, param->error_code, param->params->opcode);

    log_ble_mesh_client_packet(TAG, event, param);

    switch (event) {
        case ESP_BLE_MESH_GENERIC_CLIENT_GET_STATE_EVT:
            // Acknowledgement after sending GET
            ESP_LOGI(TAG, "ESP_BLE_MESH_GENERIC_CLIENT_GET_STATE_EVT");
            if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_GET) {
                ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_GET, level %d",
                         param->status_cb.level_status.present_level);
            }
            break;
        case ESP_BLE_MESH_GENERIC_CLIENT_SET_STATE_EVT:
            // Acknoledgement after sending SET
            ESP_LOGI(TAG, "ESP_BLE_MESH_GENERIC_CLIENT_SET_STATE_EVT");
            if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_SET) {
                ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_SET, level %d",
                         param->status_cb.level_status.present_level);
            }
            break;
        case ESP_BLE_MESH_GENERIC_CLIENT_PUBLISH_EVT:
            // Publish message received
            ESP_LOGI(TAG, "ESP_BLE_MESH_GENERIC_CLIENT_PUBLISH_EVT");
            break;
        case ESP_BLE_MESH_GENERIC_CLIENT_TIMEOUT_EVT:
            // Timeout on a message sent
            ESP_LOGI(TAG, "ESP_BLE_MESH_GENERIC_CLIENT_TIMEOUT_EVT");
            if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_SET) {
                /* If failed to get the response of Generic Level Set, resend Generic Level Set  */
                ble_mesh_send_gen_level_set();
            }
            break;
        default:
            break;
    }
}

void ble_mesh_send_test_gen_level_set(void) {

    return ble_mesh_send_gen_level_set();
}


static void ble_mesh_config_server_cb(esp_ble_mesh_cfg_server_cb_event_t event,
                                      esp_ble_mesh_cfg_server_cb_param_t *param) {
    if (event == ESP_BLE_MESH_CFG_SERVER_STATE_CHANGE_EVT) {
        log_ble_mesh_config_server_packet(TAG, param);
        switch (param->ctx.recv_op) {
            case ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD:
                ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD");
                ESP_LOGI(TAG, "net_idx 0x%04x, app_idx 0x%04x",
                         param->value.state_change.appkey_add.net_idx,
                         param->value.state_change.appkey_add.app_idx);
                ESP_LOG_BUFFER_HEX("AppKey", param->value.state_change.appkey_add.app_key, 16);
                break;
            case ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND:
                ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND");
                ESP_LOGI(TAG, "elem_addr 0x%04x, app_idx 0x%04x, cid 0x%04x, mod_id 0x%04x",
                         param->value.state_change.mod_app_bind.element_addr,
                         param->value.state_change.mod_app_bind.app_idx,
                         param->value.state_change.mod_app_bind.company_id,
                         param->value.state_change.mod_app_bind.model_id);
                if (param->value.state_change.mod_app_bind.company_id == 0xFFFF &&
                    param->value.state_change.mod_app_bind.model_id == ESP_BLE_MESH_MODEL_ID_GEN_LEVEL_CLI) {
                    store.app_idx = param->value.state_change.mod_app_bind.app_idx;
                    mesh_info_store(); /* Store proper mesh example info */
                }
                break;

            default:
                ESP_LOGI(TAG, "Unhandled case 0x%08x", param->ctx.recv_op);
                break;
        }
    }
}

// the client will send a packet at least once every (max_limit * 7) seconds
void client_routine() {
    bool alt = false;
    uint32_t threshold = UINT32_MAX / 2;
    int max_limit = 5;
    int counter = 0;

    while (1) {
        if (esp_ble_mesh_node_is_provisioned()) {
            if (esp_random() > threshold || counter == max_limit) {
                if (alt) {
                    ESP_LOGI(TAG, "Sending Generic SET");
                    ble_mesh_send_test_gen_level_set();
                } else {
                    ESP_LOGI(TAG, "Sending Generic GET");
                    ble_mesh_send_test_gen_level_get();
                }
                alt = esp_random() > threshold;
                counter = 0;
            } else {
                counter++;
            }
        } else {
            ESP_LOGI(TAG, "Node still unprovisioned, gonna wait");
        }

        vTaskDelay(7000 / portTICK_PERIOD_MS);
    }

}

esp_err_t ble_mesh_init_client(void) {
    esp_err_t err = ESP_OK;

    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    err = ble_mesh_nvs_open(&NVS_HANDLE);
    if (err) {
        return err;
    }

    ble_mesh_get_dev_uuid(dev_uuid);

    esp_ble_mesh_register_prov_callback(ble_mesh_provisioning_cb);
    esp_ble_mesh_register_generic_client_callback(ble_mesh_generic_client_cb);
    esp_ble_mesh_register_config_server_callback(ble_mesh_config_server_cb);

    err = esp_ble_mesh_init(&provision, &composition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize mesh stack (err %d)", err);
        return err;
    }

    err = esp_ble_mesh_set_unprovisioned_device_name(TAG);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set correct name (err %d)", err);
        return err;
    }

    err = esp_ble_mesh_node_prov_enable(ESP_BLE_MESH_PROV_ADV | ESP_BLE_MESH_PROV_GATT);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable mesh node (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "BLE Mesh Node initialized");

    client_routine();

    return err;
}
