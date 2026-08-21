#include <string.h>
#include "esp_log.h"

#include "lt0051.h"
#include "asic.h"

static const char *TAG = "asic";

esp_err_t ASIC_detect(GlobalState * GLOBAL_STATE){
    esp_err_t ret = ESP_FAIL;
    uint8_t detetecd_asic = 0;

    switch (GLOBAL_STATE->device_model) 
    {
        case DEVICE_LOTTO:
            GLOBAL_STATE->tmmusk = VOLCMINER_LOTTO_TM_DEFINE;
            
            ret = SERIAL_init(0);

            if(ESP_OK == ret){
                detetecd_asic = ASIC_init(GLOBAL_STATE);
                ESP_LOGI(TAG, "Model LOTTO, Detect %"PRIu8" asic.", detetecd_asic);
            }

            if(VOLCMINER_LOTTO_ASIC_COUNT == detetecd_asic){
                //SERIAL_set_baud(ASIC_set_max_baud(&GLOBAL_STATE));
                SERIAL_clear_buffer(0);
                GLOBAL_STATE->ASIC_initalized = true;
                ret = ESP_OK;
            }else{
                ret = ESP_FAIL;
            }
            break;

        case DEVICE_DC02:
            GLOBAL_STATE->tmmusk = DC02_LOTTO_TM_DEFINE;
            
            ret = SERIAL_init(0);

            if(ESP_OK == ret){
                detetecd_asic = ASIC_init(GLOBAL_STATE);
                ESP_LOGI(TAG, "Model DC02, Detect %"PRIu8" asic.", detetecd_asic);
            }

            if(DC02_LOTTO_ASIC_COUNT == detetecd_asic){
                //SERIAL_set_baud(ASIC_set_max_baud(&GLOBAL_STATE));
                SERIAL_clear_buffer(0);
                GLOBAL_STATE->ASIC_initalized = true;
                ret = ESP_OK;
            }else{
                ret = ESP_FAIL;
            }
            break;

        case DEVICE_DC04:
            GLOBAL_STATE->tmmusk = DC04_LOTTO_TM_DEFINE;
            
            ret = SERIAL_init(0);

            if(ESP_OK == ret){
                detetecd_asic = ASIC_init(GLOBAL_STATE);
                ESP_LOGI(TAG, "Model DC04, Detect %"PRIu8" asic.", detetecd_asic);
            }

            if(DC04_LOTTO_ASIC_COUNT == detetecd_asic){
                //SERIAL_set_baud(ASIC_set_max_baud(&GLOBAL_STATE));
                SERIAL_clear_buffer(0);
                GLOBAL_STATE->ASIC_initalized = true;
                ret = ESP_OK;
            }else{
                ret = ESP_FAIL;
            }
            break;

        default:
            break;
    }

    return ret;
}

uint8_t ASIC_init(GlobalState * GLOBAL_STATE) {
    switch (GLOBAL_STATE->device_model) 
    {
        case DEVICE_LOTTO:
            if(ESP_OK == LT0051_init_by_chain(GLOBAL_STATE, 0, VOLCMINER_LOTTO_INIT_FREQUENCY, VOLCMINER_LOTTO_ASIC_COUNT))
                return VOLCMINER_LOTTO_ASIC_COUNT;
            else
                return 0;

        case DEVICE_DC02:
            if(ESP_OK == LT0051_init_by_chain(GLOBAL_STATE, 0, DC02_LOTTO_INIT_FREQUENCY, DC02_LOTTO_ASIC_COUNT))
                return DC02_LOTTO_ASIC_COUNT;
            else
                return GLOBAL_STATE->asic_count[0];

        case DEVICE_DC04:
            if(ESP_OK == LT0051_init_by_chain(GLOBAL_STATE, 0, DC04_LOTTO_INIT_FREQUENCY, DC04_LOTTO_ASIC_COUNT))
                return DC04_LOTTO_ASIC_COUNT;
            else
                return GLOBAL_STATE->asic_count[0];

        default:
            break;
    }
    return 0;
}

// .set_max_baud_fn = LT0051_set_max_baud,
int ASIC_set_max_baud(GlobalState * GLOBAL_STATE) {
    switch (GLOBAL_STATE->device_model) 
    {
        case DEVICE_LOTTO:
            return LT0051_set_max_baud_by_chain(GLOBAL_STATE, 0);
        case DEVICE_DC02:
            return LT0051_set_max_baud_by_chain(GLOBAL_STATE, 0);
        case DEVICE_DC04:
            return LT0051_set_max_baud_by_chain(GLOBAL_STATE, 0);
        default:
            return 0;
    }
}

/*for http_server.*/
uint8_t ASIC_get_asic_count(GlobalState * GLOBAL_STATE) {
    switch (GLOBAL_STATE->device_model) 
    {
        case DEVICE_LOTTO:
            return VOLCMINER_LOTTO_ASIC_COUNT;
        case DEVICE_DC02:
            return DC02_LOTTO_ASIC_COUNT;
        case DEVICE_DC04:
            return DC04_LOTTO_ASIC_COUNT;           
        default:
            return 0;
    }

    return 0;
}

/*for http_server.*/
uint16_t ASIC_get_small_core_count(GlobalState * GLOBAL_STATE) {
    switch (GLOBAL_STATE->device_model) 
    {
        case DEVICE_LOTTO:
            return 132;
        case DEVICE_DC02:
            return 132;
        case DEVICE_DC04:
            return 132;            
        default:
            return 0;
    }

    return 0;
}

// .receive_result_fn = LT0051_process_work,
task_result * ASIC_process_work(GlobalState * GLOBAL_STATE, uint32_t chain_num) {
    switch (GLOBAL_STATE->device_model) 
    {
        case DEVICE_LOTTO:
            return LT0051_process_work(GLOBAL_STATE, chain_num);
        case DEVICE_DC02:
            return LT0051_process_work(GLOBAL_STATE, chain_num);
        case DEVICE_DC04:
            return LT0051_process_work(GLOBAL_STATE, chain_num);                    
        default:
    }

    return NULL;
}

// .set_difficulty_mask_fn = LT0051_set_job_difficulty_mask,
void ASIC_set_job_difficulty_mask(GlobalState * GLOBAL_STATE, uint8_t mask) {
    switch (GLOBAL_STATE->device_model) {
        case DEVICE_LOTTO:
            break;
        case DEVICE_DC02:
            break;
        case DEVICE_DC04:
            break;                       
        default:
            break;
    }
}

// .send_work_fn = LT0051_send_work,
void ASIC_send_work(GlobalState * GLOBAL_STATE, void * next_job, uint32_t chain_num, uint8_t workid) {
    switch (GLOBAL_STATE->device_model) {
        case DEVICE_LOTTO:
            LT0051_send_work(GLOBAL_STATE, next_job, chain_num, workid);
            break;
        case DEVICE_DC02:
            LT0051_send_work(GLOBAL_STATE, next_job, chain_num, workid);
            break;
        case DEVICE_DC04:
            LT0051_send_work(GLOBAL_STATE, next_job, chain_num, workid);
            break;            
            //LT0051_send_work_by_chip(GLOBAL_STATE, next_job, chain_num, workid, ASIC_get_asic_count(GLOBAL_STATE));
            break;
        default:
    }
}

// .set_version_mask = LT0051_set_version_mask
void ASIC_set_version_mask(GlobalState * GLOBAL_STATE, uint32_t mask) {
    switch (GLOBAL_STATE->device_model) {
        case DEVICE_LOTTO:
            //do_nothing.
            break;
        case DEVICE_DC02:
            //do_nothing.
            break;
        case DEVICE_DC04:
            //do_nothing.
            break;
        default:
            return;
    }
}

bool ASIC_set_frequency(GlobalState * GLOBAL_STATE, float target_frequency) {
    ESP_LOGI(TAG, "Setting ASIC frequency to %.2f MHz", target_frequency);
    bool success = false;

    switch (GLOBAL_STATE->device_model) {
        case DEVICE_LOTTO:
            success = LT0051_send_hash_frequency(GLOBAL_STATE, target_frequency);
            break;
        case DEVICE_DC02:
            success = LT0051_send_hash_frequency(GLOBAL_STATE, target_frequency);
            break;
        case DEVICE_DC04:
            success = LT0051_send_hash_frequency(GLOBAL_STATE, target_frequency);
            break;            
        default:
            ESP_LOGE(TAG, "Unknown ASIC model, cannot set frequency");
            success = false;
            break;
    }

    if (success) {
        ESP_LOGI(TAG, "Successfully transitioned to new ASIC frequency: %.2f MHz", target_frequency);
    } else {
        ESP_LOGE(TAG, "Failed to transition to new ASIC frequency: %.2f MHz", target_frequency);
    }

    return success;
}

esp_err_t ASIC_set_device_model(GlobalState * GLOBAL_STATE) {

    if (GLOBAL_STATE->device_model_str == NULL) {
        ESP_LOGE(TAG, "No device model string found");
        return ESP_FAIL;
    }

    return ESP_OK;
}

int ASIC_switch_by_chain(GlobalState *state, uint32_t chain_num)
{
    return switch_by_chain(state, chain_num);
}
