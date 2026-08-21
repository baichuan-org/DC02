#ifndef POWER_H
#define POWER_H

#include "esp_err.h"

#include "miner.h"

#define TOTAL_SUPPORT_POWER_TYPE            2
#define YG_500W_POWER_TYPE_INDEX            0
/*G1853*/
#define GOS_500W_POWER_TYPE_INDEX           1


#define YIGONG_500W_POWER_DEFAULT_I2C_ADDR  0x2
#define GOS_500W_POWER_DEFAULT_I2C_ADDR     0xB0

#define GOS_500W_MIN_OUTPUT_VOLTAGE         450
#define GOS_500W_MAX_OUTPUT_VOLTAGE         650

typedef struct
{
    int16_t mantissa : 11;
    int16_t exponent : 5;
} linear11_t;

typedef union
{
    linear11_t linear;
    uint16_t raw;
} linear11_val_t;

typedef enum
{
    UNKNOWN_POWER = 0,
    YG_500W,
    GOS_500W
}POWER_TYPE;

esp_err_t power_detect_type(uint8_t i2c_master_index);
esp_err_t power_on();
esp_err_t power_off();
esp_err_t power_set_voltage(uint16_t voltage);
esp_err_t power_get_nominal_voltage(int *input_voltage);
esp_err_t power_get_max_power(float *max_power);

double power_get_output_voltage();
double power_get_output_current();
double power_get_output_power();
double power_get_input_voltage();
double power_get_input_current();

esp_err_t gos_read_In16Linear(uint8_t cmd, int8_t exp, double *d_value);
esp_err_t gos_read_In11Linear(uint8_t cmd, double *d_value);

esp_err_t yg_power_init(uint8_t i2c_master_index);
esp_err_t powerOnYiGong();
esp_err_t powerOffYiGong();
esp_err_t setPowerVoltageYiGong(uint16_t voltage);
esp_err_t yg_checkPowerID();

#endif