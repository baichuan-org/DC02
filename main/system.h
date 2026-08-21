#ifndef SYSTEM_H_
#define SYSTEM_H_

#include "esp_err.h"
#include "global_state.h"

typedef enum{
	NORMAL_MODE = 0x0,
	OVER_FREQ_MODE = 0x1,
	USER_CUSTOMIZED_MODE = 0x2,
	DEMO_DEBUG_MODE = 0x3,
	LOWER_POWER_MODE=0x4,
	SUPER_LOW_POWER_MODE=0x5,
	SLEEP_MODE=0x6,
	POWER_DEBUG_MODE = 0x100
}BOOT_MODE;

void SYSTEM_init_system(GlobalState * GLOBAL_STATE);
esp_err_t SYSTEM_init_peripherals(GlobalState * GLOBAL_STATE);
esp_err_t SYSTEM_get_config_by_boot_mode(GlobalState * GLOBAL_STATE);
char * SYSTEM_get_sn(void);
void SYSTEM_notify_get_work(GlobalState * GLOBAL_STATE);
void SYSTEM_notify_accepted_share(GlobalState * GLOBAL_STATE);
void SYSTEM_notify_rejected_share(GlobalState * GLOBAL_STATE, char * error_msg);

void SYSTEM_notify_found_nonce(GlobalState * GLOBAL_STATE, double found_diff, 
	uint8_t job_id, uint8_t chain_id, uint8_t chip_id, uint8_t core_id);
void SYSTEM_notify_hw(GlobalState * GLOBAL_STATE, 
	uint8_t chain_id, uint8_t chip_id, uint8_t core_id);
void SYSTEM_notify_mining_started(GlobalState * GLOBAL_STATE);
void SYSTEM_notify_new_ntime(GlobalState * GLOBAL_STATE, uint32_t ntime);

void SYSTEM_notify_status_change(
	GlobalState * GLOBAL_STATE, SYSTEM_SATUS new_status);
void SYSTEM_notify_error_info(
    GlobalState * GLOBAL_STATE, SYSTEM_ERROR system_err, const char* error_info);

bool format_seconds_to_string(uint32_t total_seconds, char* buffer, size_t buffer_size, bool show_seconds);

void system_task(void *pvParameters);

#endif /* SYSTEM_H_ */
