#include <stdbool.h>

#include "esp_log.h"
#include "esp_check.h"

#include "miner.h"
#include "volc_i2c.h"
#include "external_sensor.h"

static const char *TAG = "external_sensor";


/*voltaget sensor : INA230*/
static i2c_master_dev_handle_t ina230_dev_handles[MAX_CHAIN_NUM];

static i2c_master_dev_handle_t tmp75_dev_handles[TEMPERATURE_SENSOR_MAX_NUM];

/**
 * @brief Initialize the TMP75 sensor.
 *
 * @return esp_err_t ESP_OK on success, or an error code on failure.
 */
esp_err_t TMP75_init(uint8_t i2c_master_index, uint8_t slave_addr, int temperature_sensor_index, const char* tag) {
    return volc_i2c_add_device(i2c_master_index, slave_addr, tmp75_dev_handles+temperature_sensor_index, tag);
}

esp_err_t TMP75_installed(int temperature_sensor_index)
{
    uint8_t data[2] = {0, 0};
    esp_err_t result = ESP_FAIL;

    // read the configuration register
    ESP_LOGI(TAG, "Reading configuration register");
    result = volc_i2c_register_read(tmp75_dev_handles[temperature_sensor_index], TMP75_CONFIG_REG, data, 2);
    ESP_LOGI(TAG, "Configuration[%d] = %02X %02X", temperature_sensor_index, data[0], data[1]);

    return result;
}

int8_t TMP75_read_temperature(int temperature_sensor_index)
{
    uint8_t data[2] = {0, 0};
    int8_t temperature;

    if(volc_i2c_register_read(tmp75_dev_handles[temperature_sensor_index], TMP75_TEMP_REG, data, 2))
    {
        ESP_LOGW(TAG, "Read Temperature fail");
        return -60;
    }
    ESP_LOGD(TAG, "Raw Temperature = %02X %02X", data[0], data[1]);
    if(data[0] & 0x80){
        temperature = (int8_t)data[0];
    }else{
        temperature = data[0] & 0x7f;
    }
    ESP_LOGD(TAG, "Temperature[%d] = %"PRId8"", temperature_sensor_index, temperature);

    return temperature;
}

/**
 * @brief Initialize the INA230 sensor.
 *
 * @return esp_err_t ESP_OK on success, or an error code on failure.
 */
esp_err_t INA230_init(uint8_t i2c_master_index, uint8_t slave_addr, int chain_index, const char* tag) {
    return volc_i2c_add_device(i2c_master_index, slave_addr, ina230_dev_handles+chain_index, tag);
}

bool INA230_installed(int chain_index)
{   
    uint8_t data[2];

    return volc_i2c_register_read(ina230_dev_handles[chain_index], INA230_REG_BUS_VOL, data, 2) == ESP_OK;
}

float INA230_read_current(int chain_index)
{
    uint8_t data[2];

    ESP_ERROR_CHECK(volc_i2c_register_read(ina230_dev_handles[chain_index], INA230_REG_CURRENT, data, 2));
    // ESP_LOGI(TAG, "Raw Current = %02X %02X", data[1], data[0]);

    return (uint16_t)(data[1] | (data[0] << 8)) * 1.25 / 1000.0;
}

float INA230_read_voltage(int chain_index)
{
    uint8_t data[2];

    ESP_ERROR_CHECK(volc_i2c_register_read(ina230_dev_handles[chain_index], INA230_REG_BUS_VOL, data, 2));
    // ESP_LOGI(TAG, "Raw Voltage = %02X %02X", data[1], data[0]);

    return (uint16_t)(data[1] | (data[0] << 8)) * 1.25 / 1000.0;
}

float INA230_read_power(int chain_index)
{
    uint8_t data[2];

    ESP_ERROR_CHECK(volc_i2c_register_read(ina230_dev_handles[chain_index], INA230_REG_POWER, data, 2));
    // ESP_LOGI(TAG, "Raw Power = %02X %02X", data[1], data[0]);

    return (data[1] | (data[0] << 8)) * 10 / 1000.0;
}

