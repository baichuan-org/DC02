#include <string.h>
#include "esp_event.h"
#include "esp_log.h"
#include "esp_check.h"

#include "miner.h"
#include "volc_i2c.h"

static const char * TAG = "vloc-i2c";
static i2c_master_bus_handle_t i2c_bus_handle[I2C_NUM_MAX];


static i2c_dev_map_entry_t i2c_device_map[MAX_DEVICES];
static int i2c_device_count = 0;

static esp_err_t log_on_error(esp_err_t err, i2c_master_dev_handle_t handle) {
    if (err == ESP_OK) {
        return ESP_OK;
    }

    for (int i = 0; i < i2c_device_count; i++) {
        if (i2c_device_map[i].handle == handle) {
            ESP_LOGE(TAG, "Device %s (0x%02x)", i2c_device_map[i].device_tag, i2c_device_map[i].device_address);
            return err;
        }
    }
    
    ESP_LOGE(TAG, "Unknown device");
    return err;
}

/**
 * @brief i2c master initialization
 */
esp_err_t volc_i2c_init_master0(void)
{   
    i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = GPIO_I2C_SCL_0,
        .sda_io_num = GPIO_I2C_SDA_0,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, i2c_bus_handle));
    
    //wait for I2C to init
    vTaskDelay(100 / portTICK_PERIOD_MS);

    return ESP_OK;
}

esp_err_t volc_i2c_init_master1(void)
{
    i2c_master_bus_config_t i2c_bus_config_1 = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_1,
        .scl_io_num = GPIO_I2C_SCL_1,
        .sda_io_num = GPIO_I2C_SDA_1,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config_1, i2c_bus_handle+1));
    //wait for I2C to init
    vTaskDelay(100 / portTICK_PERIOD_MS);

    return ESP_OK;
}

/**
 * @brief Add a new I2C Device
 * @param i2c_master_index i2c_port
 * @param device_address The I2C device address
 * @param dev_handle The I2C device handle
 */
esp_err_t volc_i2c_add_device(
        uint8_t i2c_master_index, uint8_t device_address, 
        i2c_master_dev_handle_t * dev_handle, const char *device_tag
    )
{
    if (i2c_device_count >= MAX_DEVICES) {
        ESP_LOGE(TAG, "Device map full, cannot add more devices");
        return ESP_FAIL;
    }

    if(i2c_master_index >= I2C_NUM_MAX) {
        ESP_LOGE(TAG, "Error i2c master index.");
        return ESP_FAIL;
    }

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = device_address,
        .scl_speed_hz = I2C_BUS_SPEED_HZ,
    };

    ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(i2c_bus_handle[i2c_master_index], &dev_cfg, dev_handle), TAG, "Device 0x%02x", device_address);

    i2c_device_map[i2c_device_count].handle = *dev_handle;
    i2c_device_map[i2c_device_count].device_address = device_address;
    strncpy(i2c_device_map[i2c_device_count].device_tag, device_tag, sizeof(i2c_device_map[i2c_device_count].device_tag) - 1);
    i2c_device_map[i2c_device_count].device_tag[sizeof(i2c_device_map[i2c_device_count].device_tag) - 1] = '\0';
    i2c_device_count++;

    return ESP_OK;
}

esp_err_t volc_i2c_get_master_bus_handle(
    i2c_master_bus_handle_t * dev_handle, uint8_t i2c_master_index)
{
    if(i2c_master_index >= I2C_NUM_MAX) {
        ESP_LOGE(TAG, "Error i2c master index.");
        return ESP_FAIL;
    }
    
    *dev_handle = i2c_bus_handle[i2c_master_index];
    return ESP_OK;
}

/**
 * @brief Read a sequence of I2C bytes
 * @param dev_handle The I2C device handle
 * @param reg_addr The register address to read from
 * @param read_buf The buffer to store the read data
 * @param len The number of bytes to read
 */
esp_err_t volc_i2c_register_read(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint8_t * read_buf, size_t len)
{
    return log_on_error(i2c_master_transmit_receive(dev_handle, &reg_addr, 1, read_buf, len, I2C_DEFAULT_TIMEOUT), dev_handle);
}

/**
 * @brief Just write a register address to the I2C device
 * 
 * @param dev_handle 
 * @param reg_addr 
 * @return esp_err_t 
 */
esp_err_t volc_i2c_register_write_addr(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr)
{
    return log_on_error(i2c_master_transmit(dev_handle, &reg_addr, 1, I2C_DEFAULT_TIMEOUT), dev_handle);
}


/**
 * @brief Write a byte to a I2C register
 * @param dev_handle The I2C device handle
 * @param reg_addr The register address to write to
 * @param data The data to write
 */
esp_err_t volc_i2c_register_write_byte(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint8_t data)
{
    uint8_t write_buf[2] = {reg_addr, data};

    return log_on_error(i2c_master_transmit(dev_handle, write_buf, 2, I2C_DEFAULT_TIMEOUT), dev_handle);
}

/**
 * @brief Write a bytes to a I2C register
 * @param dev_handle The I2C device handle
 * @param data The data to write
 * @param len The number of bytes to write
 */
esp_err_t volc_i2c_register_write_bytes(i2c_master_dev_handle_t dev_handle, uint8_t * data, uint8_t len)
{
    return log_on_error(i2c_master_transmit(dev_handle, data, len, I2C_DEFAULT_TIMEOUT), dev_handle);
}

/**
 * @brief Write a word to a I2C register
 * @param dev_handle The I2C device handle
 * @param reg_addr The register address to write to
 * @param data The data to write
 */
esp_err_t volc_i2c_register_write_word(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint16_t data)
{
    uint8_t write_buf[3] = {reg_addr, (uint8_t)(data & 0x00FF), (uint8_t)((data & 0xFF00) >> 8)};

    return log_on_error(i2c_master_transmit(dev_handle, write_buf, 3, I2C_DEFAULT_TIMEOUT), dev_handle);
}

esp_err_t volc_i2c_write_read(
    i2c_master_dev_handle_t dev_handle, 
    uint8_t *write_buf, size_t write_len, uint8_t *read_buf, size_t read_len)
{
    return log_on_error(i2c_master_transmit_receive(dev_handle, write_buf, write_len, read_buf, read_len, I2C_DEFAULT_TIMEOUT), dev_handle);
}

esp_err_t volc_i2c_write(
    i2c_master_dev_handle_t dev_handle, uint8_t *write_buf, size_t write_len)
{
    return log_on_error(i2c_master_transmit(dev_handle, write_buf, write_len, I2C_DEFAULT_TIMEOUT), dev_handle);
}

esp_err_t volc_i2c_read(
    i2c_master_dev_handle_t dev_handle, uint8_t *read_buf, size_t read_len
)
{
    return log_on_error(i2c_master_transmit(dev_handle, read_buf, read_len, I2C_DEFAULT_TIMEOUT), dev_handle);
}