#ifndef _DEVICE_H
#define _DEVICE_H

#include "global_state.h"

#include "gpio_input_output.h"
#include "pwm_fan.h"
#include "power.h"
#include "volc_i2c.h"
#include "external_sensor.h"

/*
#define MINI_I2C_MASTER_INDEX_OF_TMP75              0
#define MINI_I2C_MASTER_INDEX_OF_POWER              1

#define MINI_I2C_DEV_INDEX_OF_CONTROL_BOARD_TMP75   0
#define MINI_I2C_DEV_INDEX_OF_HASH_BOARD_0_TMP75    1

#define MINI_I2C_ADDRSS_OF_CONTROL_BOARD_TMP75      0x20
#define MINI_I2C_ADDRESS_OF_HASH_BOARD_0_TMP75      0x40
*/

esp_err_t read_hash_board_temperature(GlobalState *GLOBAL_STATE);
int       read_power_temp(void);
esp_err_t read_power_information(GlobalState *GLOBAL_STATE);
esp_err_t power_on_hashboard(GlobalState *GLOBAL_STATE);
esp_err_t power_off_hashboard(GlobalState *GLOBAL_STATE);
void      reset_hash_board(GlobalState *GLOBAL_STATE);

esp_err_t set_fan_pwm(GlobalState *GLOBAL_STATE, uint8_t pwm_percent);
esp_err_t read_fan_rpm(GlobalState *GLOBAL_STATE, uint32_t mseconds);

esp_err_t init_all_i2c(GlobalState *GLOBAL_STATE);
esp_err_t init_all_fans(GlobalState *GLOBAL_STATE);

esp_err_t init_all_peripherals(GlobalState *GLOBAL_STATE);

void dev_led_init(GlobalState *GLOBAL_STATE);
void dev_display_init(GlobalState *GLOBAL_STATE, bool invertScreen);

#ifdef STATISTIC_SYSTEM_FEATURE
esp_err_t statistic_init_system_by_device(GlobalState *GLOBAL_STATE);
#endif

#endif