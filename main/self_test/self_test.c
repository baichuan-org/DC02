#include <stdio.h>
#include <inttypes.h>

#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"

#include "self_test.h"
#include "internal_sensor.h"
#include "gpio_input_output.h"
#include "serial.h"
#include "volc_i2c.h"
#include "external_sensor.h"
#include "power.h"
#include "pwm_fan.h"
#include "nvs_config.h"
#include "lvgl_porting.h"
#include "vcore.h"
#include "asic.h"
#include "device.h"

static const char *TAG = "self_test";
static GlobalState * GLOBAL_STATE = NULL;
static int device_mode_is_dc04 = 0; 

static void tests_done(GlobalState * GLOBAL_STATE, bool test_result) 
{
    GLOBAL_STATE->SELF_TEST_MODULE.result = test_result;
    GLOBAL_STATE->SELF_TEST_MODULE.finished = true;

    if (test_result != true) {
        ESP_LOGW(TAG, "SELF TESTS FAIL !!!");
    }else{
        ESP_LOGI(TAG, "SELF_TEST OK !!!");
        nvs_config_set_u16(NVS_CONFIG_SELF_TEST,1);
    }
}

esp_err_t test_temperature_sensor(float *temperature)
{
    esp_err_t ret = ESP_FAIL;

    start_internal_temperature_sensor();
    ret = read_internal_temperature_sensor(temperature);
    if(ret == ESP_OK){
        ESP_LOGI(TAG, "Temperature: %.2f °C", *temperature);
    }else{
        ESP_LOGW(TAG, "Failed to read temperature");
    }
    stop_internal_temperature_sensor();

    return ret;
}

#define CONFIG_GPIO_PLUG_SENSE      10
#define GPIO_PLUG_SENSE  CONFIG_GPIO_PLUG_SENSE
#include "TPS546.h"
static TPS546_CONFIG TPS546_CONFIG_LOTTO = {
    /* vin voltage */
    .TPS546_INIT_VIN_ON = 4.8,
    .TPS546_INIT_VIN_OFF = 4.5,
    .TPS546_INIT_VIN_UV_WARN_LIMIT = 0, //Set to 0 to ignore. TI Bug in this register
    .TPS546_INIT_VIN_OV_FAULT_LIMIT = 6.5,
    /* vout voltage */
    .TPS546_INIT_SCALE_LOOP = 0.5,
    .TPS546_INIT_VOUT_MIN = 1,
    .TPS546_INIT_VOUT_MAX = 1.5,
    .TPS546_INIT_VOUT_COMMAND = 1.2,
    /* iout current */
    .TPS546_INIT_IOUT_OC_WARN_LIMIT = 25.00, /* A */
    .TPS546_INIT_IOUT_OC_FAULT_LIMIT = 30.00 /* A */  
};

esp_err_t test_power(int i2c_master_index)
{
    esp_err_t ret = ESP_FAIL;

    // configure plug sense, if present
    // Configure plug sense pin as input(barrel jack) 1 is plugged in
    gpio_config_t barrel_jack_conf = {
        .pin_bit_mask = (1ULL << GPIO_PLUG_SENSE),
        .mode = GPIO_MODE_INPUT,
    };
    gpio_config(&barrel_jack_conf);
    int barrel_jack_plugged_in = gpio_get_level(GPIO_PLUG_SENSE);
    ESP_LOGI(TAG, "TPS546 power good %d", barrel_jack_plugged_in);

    if (device_mode_is_dc04)
    {
        TPS546_CONFIG_LOTTO.TPS546_INIT_VOUT_MAX = 3;
        TPS546_CONFIG_LOTTO.TPS546_INIT_SCALE_LOOP = 0.25;
    }

    ret = TPS546_init(TPS546_CONFIG_LOTTO);
    ESP_LOGI(TAG, "test_power done.");
    return ret;      
}

esp_err_t test_power_on(float *vin, float *vout)
{
    esp_err_t ret = ESP_OK;

    /*init the power.*/
    if(device_mode_is_dc04)
    {
        GLOBAL_STATE->HEALTH_MODULE.voltage = 240;
    }
    else
    {
        GLOBAL_STATE->HEALTH_MODULE.voltage = 120;
    }
    power_on_hashboard(GLOBAL_STATE);
    volc_delay(3000);
    *vin = TPS546_get_vin();
    *vout = TPS546_get_vout();
    ESP_LOGI(TAG, "Power in %.2f, out %.2f",*vin, *vout);
    if(*vin < 11)
    {
        ret = ESP_FAIL;
    }
    if(*vout < 0.7)
    {
        ret = ESP_FAIL;
    }
    return ret;      
}

esp_err_t test_hashboard(void)
{
    esp_err_t ret = ESP_OK;

    /*reset the hashboard*/
    reset_hash_board(GLOBAL_STATE);
    volc_delay(1000);
    GLOBAL_STATE->interface_initalized = true;
    ret = ASIC_detect(GLOBAL_STATE);
    return ret;      
}

esp_err_t test_external_temperature_sensor(int8_t *t)
{
    esp_err_t ret = ESP_FAIL;

    ret = TMP75_init(I2C_MASTER_INDEX_OF_HASHBOARD_0, TMP75_I2CADDR_DEFAULT_HASHBOARD, MINI_HASHBOARD0_TEMPERATURE_SENSOR_INDEX, "hashboard0_tmp75");
    ret = TMP75_installed(MINI_HASHBOARD0_TEMPERATURE_SENSOR_INDEX);
    *t = TMP75_read_temperature(MINI_HASHBOARD0_TEMPERATURE_SENSOR_INDEX);

    ESP_LOGI(TAG, "external_temperature_sensor Temperature: %d °C", *t);
    return ret;
}

esp_err_t test_fan(int * rpm)
{
    esp_err_t ret = ESP_FAIL;
    int pwm_conf[] = {0, 50, 80, 100};
    int pulse_counter[2] = {0, 0};

    for(int i = 0; i < sizeof(pwm_conf)/sizeof(pwm_conf[0]); i++)
    {
        ret = ESP_ERROR_CHECK_WITHOUT_ABORT(ledc_set_pwm(LEDC_CHANNEL_0, pwm_conf[i]));
        vTaskDelay(pdMS_TO_TICKS(1000));
        ret = ESP_ERROR_CHECK_WITHOUT_ABORT(fan_pcnts_clear_counter(LEDC_CHANNEL_0));
        vTaskDelay(pdMS_TO_TICKS(5000));
        ret = ESP_ERROR_CHECK_WITHOUT_ABORT(fan_pcnts_get_counter(LEDC_CHANNEL_0, pulse_counter));
        *rpm = pulse_counter[0]*12/2;
        ESP_LOGI(TAG, "Fan %d%%,speed is %d rpm", pwm_conf[i],*rpm);
        if(pwm_conf[i] < 10)
        {
            if(*rpm > 1000)
            {
                return ESP_FAIL;
            }
        }
        else if(pwm_conf[i] > 90)
        {
            if(device_mode_is_dc04)
            {
                if(*rpm < 1500)
                {
                    return ESP_FAIL;
                }
            }
            else
            {
                if(*rpm < 4500)
                {
                    return ESP_FAIL;
                }
            }
        }
    }
    return ret;
}


/*api function */
bool should_test(GlobalState *global_state)
{
    bool ret = true;
    uint16_t self_test = nvs_config_get_u16(NVS_CONFIG_SELF_TEST, 0);
    
    if(0 == self_test){
        ret = true;
    }else if(1 == self_test){
        ret = false;
    }else{
        ESP_LOGW(TAG, "NVS ERROR: self_test %"PRIu16"", self_test);
    }

    return ret;
}


void self_test(void *pvParameters)
{
    GLOBAL_STATE = (GlobalState *) pvParameters;
    esp_err_t ret = ESP_FAIL;
    char test_str[1000] = "Self Tests : ";
    char test_item_str[64];
    strcat(test_str, GLOBAL_STATE->device_model_str);
    strcat(test_str, "\n\n");
    GLOBAL_STATE->SELF_TEST_MODULE.active = true;
    GLOBAL_STATE->SELF_TEST_MODULE.message = test_str;

    if (strcmp(GLOBAL_STATE->device_model_str, "DC04") == 0)
    {
        device_mode_is_dc04 = 1;
    }

    ESP_LOGW(TAG, "Running Self Tests");
    logMessage(GLOBAL_STATE->SELF_TEST_MODULE.message);

    float t = 0;
    if((ret = test_temperature_sensor(&t)) != ESP_OK){
        ESP_LOGI(TAG, "Internal Temperature Sensor test failed, %d, %s", ret, esp_err_to_name(ret));
        strcat(GLOBAL_STATE->SELF_TEST_MODULE.message, "interanl temp fail!\n");
        logMessage(GLOBAL_STATE->SELF_TEST_MODULE.message);
        tests_done(GLOBAL_STATE, false);
        return;
    }
    else
    {
        sprintf(test_item_str,"internal temp: %0.2f\n",t);
        strcat(GLOBAL_STATE->SELF_TEST_MODULE.message, test_item_str);
        logMessage(GLOBAL_STATE->SELF_TEST_MODULE.message);
    }

    #if 0
    if((ret = test_led()) != ESP_OK){
        ESP_LOGI(TAG, "Led test failed, %d, %s", ret, esp_err_to_name(ret));
        tests_done(GLOBAL_STATE, false);
    }
    

    if((ret = test_devkitc_serial()) != ESP_OK){
        ESP_LOGI(TAG, "Serial test failed, %d, %s", ret, esp_err_to_name(ret));
        tests_done(GLOBAL_STATE, false);
    }   
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    #endif

    #if 1
    int rpm = 0;
    if((ret = test_fan(&rpm)) != ESP_OK){
        ESP_LOGI(TAG, "Fan test failed, %d, %s", ret, esp_err_to_name(ret));
        strcat(GLOBAL_STATE->SELF_TEST_MODULE.message, "FAN fail!\n");
        logMessage(GLOBAL_STATE->SELF_TEST_MODULE.message);
        tests_done(GLOBAL_STATE, false);
        return;
    }
    else
    {
        sprintf(test_item_str,"PWM 100%%,  FAN %d rpm\n",rpm);
        strcat(GLOBAL_STATE->SELF_TEST_MODULE.message, test_item_str);
        logMessage(GLOBAL_STATE->SELF_TEST_MODULE.message);
    }
    #endif
        
    ESP_ERROR_CHECK(volc_i2c_init_master0());
    ESP_ERROR_CHECK(volc_i2c_init_master1());

    if(ESP_OK != (ret = test_power(I2C_MASTER_INDEX_OF_POWER))){
        ESP_LOGE(TAG, "power test failed, %d, %s", ret, esp_err_to_name(ret));
        strcat(GLOBAL_STATE->SELF_TEST_MODULE.message, "Power init fail!\n");
        logMessage(GLOBAL_STATE->SELF_TEST_MODULE.message);
        tests_done(GLOBAL_STATE, false);
        return;
    }
    else
    {
        strcat(GLOBAL_STATE->SELF_TEST_MODULE.message, "Power init OK\n");
        logMessage(GLOBAL_STATE->SELF_TEST_MODULE.message);
    }

    float vin,vout;
    if(ESP_OK != (ret = test_power_on(&vin, &vout))){
        ESP_LOGE(TAG, "power on test failed, %d, %s", ret, esp_err_to_name(ret));
        strcat(GLOBAL_STATE->SELF_TEST_MODULE.message, "Power on fail!\n");
        logMessage(GLOBAL_STATE->SELF_TEST_MODULE.message);
        tests_done(GLOBAL_STATE, false);
        return;
    }
    else
    {
        sprintf(test_item_str,"Power vin %0.2fV, vout %0.2fV\n",vin,vout);
        strcat(GLOBAL_STATE->SELF_TEST_MODULE.message, test_item_str);
        logMessage(GLOBAL_STATE->SELF_TEST_MODULE.message);
    }

    if(ESP_OK != (ret = test_hashboard())){
        ESP_LOGE(TAG, "hashboard test failed, %d, %s", ret, esp_err_to_name(ret));
        sprintf(test_item_str,"hashboard test fail! asic=%d\n",GLOBAL_STATE->asic_count[0]);
        strcat(GLOBAL_STATE->SELF_TEST_MODULE.message, test_item_str);
        logMessage(GLOBAL_STATE->SELF_TEST_MODULE.message);
        tests_done(GLOBAL_STATE, false);
        return;
    }
    else
    {
        sprintf(test_item_str,"hashboard OK, asic=%d\n",GLOBAL_STATE->asic_count[0]);
        strcat(GLOBAL_STATE->SELF_TEST_MODULE.message, test_item_str);
        logMessage(GLOBAL_STATE->SELF_TEST_MODULE.message);
    }

    int8_t temp = 0;
    if(ESP_OK != (ret = test_external_temperature_sensor(&temp))){
        ESP_LOGE(TAG, "temperature_sensor test failed, %d, %s", ret, esp_err_to_name(ret));
        strcat(GLOBAL_STATE->SELF_TEST_MODULE.message, "temp sensor fail!\n");
        logMessage(GLOBAL_STATE->SELF_TEST_MODULE.message);
        tests_done(GLOBAL_STATE, false);
        return;
    }
    else
    {
        sprintf(test_item_str,"temp sensor %d\n",temp);
        strcat(GLOBAL_STATE->SELF_TEST_MODULE.message, test_item_str);
        logMessage(GLOBAL_STATE->SELF_TEST_MODULE.message);
    }

    strcat(GLOBAL_STATE->SELF_TEST_MODULE.message, "\nTests ok!\n");
    logMessage(GLOBAL_STATE->SELF_TEST_MODULE.message);
    tests_done(GLOBAL_STATE, true);
}
