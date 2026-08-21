#include "system.h"
#include "work_queue.h"
#include "serial.h"
#include "lt0051.h"
#include <string.h>
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "asic.h"
#include "lt0051.h"
#include "global_state.h"
#include "gpio_input_output.h"

static const char *TAG = "ASIC_task";

// static bm_job ** active_jobs; is required to keep track of the active jobs since the

#ifdef MUTEX_SWITCH_FEATURE

SemaphoreHandle_t freq_switch_sem;
SemaphoreHandle_t chain_init_done_sem[MAX_CHAIN_NUM];

void ASIC_task(void *pvParameters)
{
    uint32_t chain_num = ((AsicParam *)(pvParameters))->chain_num;
    GlobalState *GLOBAL_STATE = ((AsicParam *)(pvParameters))->p_global_state;
    uint8_t workid = 0;
    uint32_t loop_index = 0;
    int ret = -1;
    bool chain_switch_done = false;

    xSemaphoreTake(chain_init_done_sem[chain_num], portMAX_DELAY);   
    if(pdTRUE == xSemaphoreTake(freq_switch_sem, portMAX_DELAY)){
        //reset pin high.
        reset_pin_high(chain_num);
    }
    ESP_ERROR_CHECK(LT0051_init_by_chain(GLOBAL_STATE, chain_num, 
            VOLCMINER_MINI_PRE_INIT_FREQUENCY, VOLCMINER_MINI_PRE_ASIC_COUNT));
    xSemaphoreGive(chain_init_done_sem[chain_num]);
    
    ESP_LOGI(TAG, "Chain %"PRIu32", ASIC Job Interval: %.2f ms", chain_num, GLOBAL_STATE->asic_job_frequency_ms);
    SYSTEM_notify_mining_started(GLOBAL_STATE);
    ESP_LOGI(TAG, "ASIC Ready!");

    while (1)
    {
        /*switch the pll.*/
        if(0 == ++loop_index%10 && false == chain_switch_done){ 
            ret = switch_by_chain(GLOBAL_STATE, chain_num);
            if(1 == ret){
                chain_switch_done = true;
                xSemaphoreGive(freq_switch_sem);
            }
        }

        bm_job *next_bm_job = (bm_job *)queue_dequeue(&GLOBAL_STATE->ASIC_jobs_queue[chain_num]);

        if (next_bm_job->pool_diff != GLOBAL_STATE->stratum_difficulty)
        {
            ESP_LOGI(TAG, "New pool difficulty %lu", next_bm_job->pool_diff);
            GLOBAL_STATE->stratum_difficulty = next_bm_job->pool_diff;
        }

        //(*GLOBAL_STATE->ASIC_functions.send_work_fn)(GLOBAL_STATE, next_bm_job); // send the job to the ASIC
        ASIC_send_work(GLOBAL_STATE, next_bm_job, chain_num, workid);
        workid = (workid + 1) % 128;

        /*
        if(127 == workid){
            ESP_LOGI(TAG, "########bypass the pll.#########.");
            bypass_asic_freq_by_chain(GLOBAL_STATE, 0);
        }
        */

        // Time to execute the above code is ~0.3ms
        // Delay for ASIC(s) to finish the job
        //vTaskDelay((GLOBAL_STATE->asic_job_frequency_ms - 0.3) / portTICK_PERIOD_MS);
        xSemaphoreTake(GLOBAL_STATE->ASIC_TASK_MODULE[chain_num].semaphore, 
            (GLOBAL_STATE->asic_job_frequency_ms / portTICK_PERIOD_MS));
    }
}
#else
void ASIC_task(void *pvParameters)
{
    uint32_t chain_num = ((AsicParam *)(pvParameters))->chain_num;
    GlobalState *GLOBAL_STATE = ((AsicParam *)(pvParameters))->p_global_state;
    uint8_t workid = 0;
    uint32_t loop_index = 0;

    ESP_LOGI(TAG, "ASIC Job Interval: %.2f ms", GLOBAL_STATE->asic_job_frequency_ms);
    SYSTEM_notify_mining_started(GLOBAL_STATE);
    ESP_LOGI(TAG, "ASIC Ready!");

    while (1)
    {
        /*switch the pll.*/
        if(0 == ++loop_index%5) 
            switch_by_chain(GLOBAL_STATE, chain_num);
        
        bm_job *next_bm_job = (bm_job *)queue_dequeue(&GLOBAL_STATE->ASIC_jobs_queue[chain_num]);

        if (next_bm_job->pool_diff != GLOBAL_STATE->stratum_difficulty)
        {
            ESP_LOGI(TAG, "New pool difficulty %lu", next_bm_job->pool_diff);
            GLOBAL_STATE->stratum_difficulty = next_bm_job->pool_diff;
        }

        ASIC_send_work(GLOBAL_STATE, next_bm_job, chain_num, workid);
        workid = (workid + 1) % 128;

        /*
        if(127 == workid){
            ESP_LOGI(TAG, "########bypass the pll.#########.");
            bypass_asic_freq_by_chain(GLOBAL_STATE, 0);
        }
        */

        // Time to execute the above code is ~0.3ms
        // Delay for ASIC(s) to finish the job
        //vTaskDelay((GLOBAL_STATE->asic_job_frequency_ms - 0.3) / portTICK_PERIOD_MS);
        xSemaphoreTake(GLOBAL_STATE->ASIC_TASK_MODULE[chain_num].semaphore, 
            (GLOBAL_STATE->asic_job_frequency_ms / portTICK_PERIOD_MS));
    }
}
#endif
