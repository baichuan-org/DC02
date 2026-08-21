#ifndef VCORE_H_
#define VCORE_H_

#include "global_state.h"

esp_err_t VCORE_init(GlobalState * global_state);
esp_err_t VCORE_set_voltage(float core_voltage, GlobalState * global_state);
int16_t   VCORE_get_voltage_mv(GlobalState * global_state);
int       VCORE_get_temp(void) ;
esp_err_t VCORE_check_fault(GlobalState * global_state);
const char* VCORE_get_fault_string(GlobalState * global_state);
esp_err_t VCORE_update_power(GlobalState * GLOBAL_STATE);
esp_err_t VCORE_get_nominal_voltage(GlobalState * GLOBAL_STATE);
esp_err_t VCORE_get_max_power(GlobalState * GLOBAL_STATE);

#endif /* VCORE_H_ */