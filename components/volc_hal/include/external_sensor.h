#ifndef EXTERNAL_SERSOR_H
#define EXTERNAL_SERSOR_H

#include <esp_err.h>

/*
voltaget sensor: INA230 {0x40}
*/
#define INA230_I2CADDR_DEFAULT  0x40
#define INA230_REG_CONFIG       0x0
#define INA230_REG_SHUNT_VOL    0x1
#define INA230_REG_BUS_VOL      0x2
#define INA230_REG_POWER        0x3
#define INA230_REG_CURRENT      0x4
#define INA230_REG_CALIBRATION  0x5
#define INA230_REG_MASK         0x6
#define INA230_REG_ALERT_LIMIT  0x7        

/*
temperature sensor: TMP75 {0x48,0x49,0x4A,0x4B} zynq
*/
#define TMP75_TEMP_REG 0x00         ///< Temperature register
#define TMP75_CONFIG_REG 0x01       ///< Configuration register
#define TMP75_LOW_LIMIT 0x02        ///< Low limit register
#define TMP75_HIGH_LIMIT 0x03       ///< High limit register
#define TMP75_DEVICE_ID 0x0F        ///< Device ID register

#define TMP75_I2CADDR_DEFAULT_HASHBOARD                 0x48
#define TMP75_I2CADDR_DEFAULT_OF_CONTROL_BOARD          0x4f

#define TEMPERATURE_SENSOR_MAX_NUM  3       /*1 control board + 2 chain*/
#define MINI_CONTROLBOARD_TEMPERATURE_SENSOR_INDEX      0
#define MINI_HASHBOARD0_TEMPERATURE_SENSOR_INDEX        1
#define MINI_HASHBOARD1_TEMPERATURE_SENSOR_INDEX        2

#define MINI_HASHBOARD0_VOLTAGE_SENSOR_INDEX            0
#define MINI_HASHBOARD1_VOLTAGE_SENSOR_INDEX            1

esp_err_t TMP75_init(uint8_t i2c_master_index, uint8_t slave_addr, int temperature_sensor_index, const char* tag);
esp_err_t TMP75_installed(int);
int8_t TMP75_read_temperature(int);

esp_err_t INA230_init(uint8_t i2c_master_index, uint8_t slave_addr, int chain_index, const char* tag);
bool INA230_installed(int chain_index);
float INA230_read_current(int chain_index);
float INA230_read_voltage(int chain_index);
float INA230_read_power(int chain_index);

#endif 