#include <string.h>
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs_config.h"
#include "nvs_device.h"

#include "connect.h"
#include "global_state.h"
#include "asic.h"

static const char * TAG = "nvs_device";
/*static const double NONCE_SPACE = 4294967296.0; //  2^32*/


esp_err_t NVSDevice_init(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    if(err != ESP_OK)
    {
        ESP_LOGW(TAG, "ERR %X", err);
    }
    return err;
}

esp_err_t NVSDevice_get_wifi_creds(GlobalState * GLOBAL_STATE, char ** wifi_ssid, char ** wifi_pass, char ** hostname) {
    // pull the wifi credentials and hostname out of NVS
    *wifi_ssid = nvs_config_get_string(NVS_CONFIG_WIFI_SSID, WIFI_SSID);
    *wifi_pass = nvs_config_get_string(NVS_CONFIG_WIFI_PASS, WIFI_PASS);
    *hostname  = nvs_config_get_string(NVS_CONFIG_HOSTNAME, HOSTNAME);

    // copy the wifi ssid to the global state
    strncpy(GLOBAL_STATE->SYSTEM_MODULE.ssid, *wifi_ssid, sizeof(GLOBAL_STATE->SYSTEM_MODULE.ssid));
    GLOBAL_STATE->SYSTEM_MODULE.ssid[sizeof(GLOBAL_STATE->SYSTEM_MODULE.ssid)-1] = 0;

    return ESP_OK;
}

esp_err_t NVSDevice_update_freq_voltage(GlobalState * GLOBAL_STATE)
{
    esp_err_t ret = ESP_OK;

    GLOBAL_STATE->asic_freqency = nvs_config_get_u16(NVS_CONFIG_ASIC_FREQ, CONFIG_ASIC_FREQUENCY);
    ESP_LOGI(TAG, "asic frequency %.0f", (float)GLOBAL_STATE->asic_freqency);
    
    /*for example, asicvoltage,data,u16,480*/
    uint16_t uint_voltage = nvs_config_get_u16(NVS_CONFIG_ASIC_VOLTAGE, CONFIG_ASIC_VOLTAGE);
    GLOBAL_STATE->HEALTH_MODULE.voltage = (float)uint_voltage;
    ESP_LOGI(TAG, "asic voltage %.0f", GLOBAL_STATE->HEALTH_MODULE.voltage);
    return ret;
}

esp_err_t NVSDevice_parse_config(GlobalState * GLOBAL_STATE) {
    GLOBAL_STATE->device_model_str = nvs_config_get_string(NVS_CONFIG_DEVICE_MODEL, CONFIG_DEVICE_MODULE);
    if (strcmp(GLOBAL_STATE->device_model_str, "lotto") == 0) {
        ESP_LOGD(TAG, "DEVICE: LOTTO");
        GLOBAL_STATE->device_model = DEVICE_LOTTO;
        //GLOBAL_STATE->asic_count = 1;
        GLOBAL_STATE->voltage_domain = 2;
        GLOBAL_STATE->asic_difficulty = VOLCMINER_LOTTO_ASIC_DIFFICULTY;
    }
    else if (strcmp(GLOBAL_STATE->device_model_str, "DC02") == 0) {
        ESP_LOGD(TAG, "DEVICE: DC02");
        GLOBAL_STATE->device_model = DEVICE_DC02;
        //GLOBAL_STATE->asic_count = 1;
        GLOBAL_STATE->voltage_domain = 2;
        GLOBAL_STATE->asic_difficulty = DC02_LOTTO_ASIC_DIFFICULTY;
        GLOBAL_STATE->asic_vol_max = CONFIG_TPS546_VOUT_MAX;
        GLOBAL_STATE->asic_vol_min = CONFIG_TPS546_VOUT_MIN;
    }  
    else if (strcmp(GLOBAL_STATE->device_model_str, "DC04") == 0) {
        ESP_LOGD(TAG, "DEVICE: DC02");
        GLOBAL_STATE->device_model = DEVICE_DC04;
        //GLOBAL_STATE->asic_count = 1;
        GLOBAL_STATE->voltage_domain = 2;
        GLOBAL_STATE->asic_difficulty = DC04_LOTTO_ASIC_DIFFICULTY;
        GLOBAL_STATE->asic_vol_max = CONFIG_TPS546_VOUT_MAX*2;
        GLOBAL_STATE->asic_vol_min = CONFIG_TPS546_VOUT_MIN*2;
    }  
    else {
        ESP_LOGE(TAG, "Invalid DEVICE model %s", GLOBAL_STATE->device_model_str);
        // maybe should return here to now execute anything with a faulty device parameter !
        // this stops crashes/reboots and allows dev testing without an asic
        GLOBAL_STATE->device_model = DEVICE_UNKNOWN;
        //GLOBAL_STATE->asic_count = -1;
        GLOBAL_STATE->voltage_domain = 1;

        return ESP_FAIL;
    }

    GLOBAL_STATE->asic_model_str = nvs_config_get_string(NVS_CONFIG_ASIC_MODEL, CONFIG_ASIC_MODULE);
    GLOBAL_STATE->board_version = atoi((nvs_config_get_string(NVS_CONFIG_BOARD_VERSION, "000")+1));
    ESP_LOGI(TAG, "Found Device Model: %s", GLOBAL_STATE->device_model_str);
    ESP_LOGI(TAG, "Found Board Version: %d", GLOBAL_STATE->board_version);

    if(0 == strcmp(GLOBAL_STATE->asic_model_str, "lt0051")){
        GLOBAL_STATE->asic_model = ASIC_LT0051;
        GLOBAL_STATE->asic_job_frequency_ms = 500;
    }

    return ESP_OK;
}
