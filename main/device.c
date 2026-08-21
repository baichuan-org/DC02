#include "device.h"
#include "vcore.h"
#include "nvs_config.h"
#include "gpio_input_output.h"
#include "internal_sensor.h"
#include "miner.h"
#include "system.h"
#include "eeprom.h"
#include "asic.h"
#include "lvgl_porting.h"

#include "esp_log.h"

#define TAG "device"

esp_err_t init_all_i2c(GlobalState *GLOBAL_STATE)
{
    esp_err_t ret = ESP_FAIL;

    ESP_LOGI(TAG, "Initialize all the i2c.");

    ESP_ERROR_CHECK(volc_i2c_init_master0());
    ESP_ERROR_CHECK(volc_i2c_init_master1());

    /*check the hash board temperature.*/
    ESP_ERROR_CHECK(ret = TMP75_init(I2C_MASTER_INDEX_OF_HASHBOARD_0,
                            TMP75_I2CADDR_DEFAULT_HASHBOARD,
                            MINI_HASHBOARD0_TEMPERATURE_SENSOR_INDEX, "hashboard0_tmp75"));
    ret = TMP75_installed(MINI_HASHBOARD0_TEMPERATURE_SENSOR_INDEX);

    return ret;
}

esp_err_t init_all_fans(GlobalState *GLOBAL_STATE)
{
    esp_err_t ret = ESP_OK;
    int fan_rpm[MAX_PWM_CHANNEL] = {0};

    ESP_LOGI(TAG, "Initialize all the fans.");

    ESP_ERROR_CHECK(ret = ledc_pwm_init(LEDC_CHANNEL_0));
    ESP_ERROR_CHECK(ret = fan_pcnts_init(LEDC_CHANNEL_0));
    fan_pcnts_clear_counter(LEDC_CHANNEL_0);
    ledc_set_pwm(LEDC_CHANNEL_0, 70);
    volc_delay(100);
    fan_pcnts_clear_counter(LEDC_CHANNEL_0);
    volc_delay(2000);
    fan_pcnts_get_rpm(LEDC_CHANNEL_0, fan_rpm, 2000);
    if(fan_rpm[0])
        ESP_LOGI(TAG, "FAN PWM 70%%, RPM  %d", fan_rpm[0]);
    else
        ESP_LOGW(TAG, "FAN PWM 70%%, RPM  %d", fan_rpm[0]);
    ledc_set_pwm(LEDC_CHANNEL_0, 50);
    return ret;
}

esp_err_t init_all_eeproms(GlobalState *GLOBAL_STATE)
{
    esp_err_t ret = ESP_OK;

    return ret;
}

esp_err_t read_hash_board_temperature(GlobalState *GLOBAL_STATE)
{
    esp_err_t ret = ESP_OK;

    GLOBAL_STATE->HEALTH_MODULE.board_temperature[0] = \
        TMP75_read_temperature(MINI_HASHBOARD0_TEMPERATURE_SENSOR_INDEX);

    return ret;
}

esp_err_t power_on_hashboard(GlobalState *GLOBAL_STATE)
{
    esp_err_t ret = ESP_OK;

    ESP_LOGI(TAG, "vcore power on hashboard, set voltage %.0f", GLOBAL_STATE->HEALTH_MODULE.voltage);
    float core_voltage = GLOBAL_STATE->HEALTH_MODULE.voltage / 100.0;
    VCORE_set_voltage(core_voltage, GLOBAL_STATE);

    return ret;
}

esp_err_t power_off_hashboard(GlobalState *GLOBAL_STATE)
{
    esp_err_t ret = ESP_OK;

    GLOBAL_STATE->HEALTH_MODULE.voltage = 0;
    ESP_LOGI(TAG, "vcore power off hashboard, set voltage %.0f", GLOBAL_STATE->HEALTH_MODULE.voltage);
    float core_voltage = GLOBAL_STATE->HEALTH_MODULE.voltage / 100.0;
    VCORE_set_voltage(core_voltage, GLOBAL_STATE);

    return ret;
}

esp_err_t read_power_information(GlobalState *GLOBAL_STATE)
{
    esp_err_t ret = ESP_OK;

    ret = VCORE_update_power(GLOBAL_STATE);


    return ret;
}

int read_power_temp(void)
{
    return VCORE_get_temp();
}

esp_err_t set_fan_pwm(GlobalState *GLOBAL_STATE, uint8_t pwm_percent)
{
    esp_err_t ret = ESP_OK;

    ret = ledc_set_pwm(LEDC_CHANNEL_0, pwm_percent);

    return ret;
}

esp_err_t read_fan_rpm(GlobalState *GLOBAL_STATE, uint32_t mseconds)
{
    esp_err_t ret = ESP_OK;

    ESP_ERROR_CHECK(fan_pcnts_get_rpm(LEDC_CHANNEL_0, GLOBAL_STATE->HEALTH_MODULE.fan_rpm, mseconds));
    GLOBAL_STATE->HEALTH_MODULE.fan_rpm[1] = 0;

    return ret;
}

void reset_hash_board(GlobalState *GLOBAL_STATE)
{
    ESP_LOGI(TAG, "Reset Chain 0.");
    ESP_ERROR_CHECK(reset_pin_init(0));
    reset_pin_low(0);
    volc_delay(300);
    reset_pin_high(0);
}

void hashboard_reset_pin_init_and_low(GlobalState *GLOBAL_STATE)
{
    ESP_LOGI(TAG, " reset pin init.");
    ESP_ERROR_CHECK(reset_pin_init(0));
    reset_pin_low(0);
    volc_delay(300);
}

void dev_led_init(GlobalState *GLOBAL_STATE)
{

}

void dev_button_init(GlobalState *GLOBAL_STATE)
{

}

void dev_display_init(GlobalState *GLOBAL_STATE, bool invertScreen)
{
    uint16_t invertScreen1 = 0;
    invertScreen1 = nvs_config_get_u16(NVS_CONFIG_FLIP_SCREEN, 0);
    display_s3_init(invertScreen1);
}

/*init all the peripherals of esp32 control board, the flow should be passed.*/
esp_err_t init_all_peripherals(GlobalState *GLOBAL_STATE)
{
    esp_err_t ret = ESP_OK;

    hash_board_3v3_ctrl_init();
    hash_board_3v3_ctrl_enable();

    /*Init i2c.*/
    init_all_i2c(GLOBAL_STATE);
    start_internal_temperature_sensor();
    /*Init the fan*/
    //init_all_fans(GLOBAL_STATE);
    //init_all_eeproms(GLOBAL_STATE);
    SYSTEM_get_config_by_boot_mode(GLOBAL_STATE);

    /*vcore related sensor init*/
    ESP_ERROR_CHECK(VCORE_init(GLOBAL_STATE));

    /*init the power.*/
    power_on_hashboard(GLOBAL_STATE);
    volc_delay(3000);

 #ifdef MUTEX_SWITCH_FEATURE
    hashboard_reset_pin_init_and_low(GLOBAL_STATE);
 #else
    /*reset the hashboard*/
    reset_hash_board(GLOBAL_STATE);
#endif

    // dev_led_init(GLOBAL_STATE);
    //dev_button_init(GLOBAL_STATE);

    GLOBAL_STATE->interface_initalized = true;
    return ret;
}

#ifdef STATISTIC_SYSTEM_FEATURE
esp_err_t statistic_init_system_by_device(GlobalState *GLOBAL_STATE)
{
    esp_err_t ret = ESP_OK;

    ret = statistic_init_system(&(GLOBAL_STATE->STATISTIC_MODULE), VOLCMINER_LOTTO_TM_CACULATE_DEFINE);

}
#endif
