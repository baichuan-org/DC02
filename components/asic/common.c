#include "common.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "serial.h"
#include "asic_abstration.h"

static const char *TAG = "asic_common";

unsigned char _reverse_bits(unsigned char num)
{
    unsigned char reversed = 0;
    int i;

    for (i = 0; i < 8; i++) {
        reversed <<= 1;      // Left shift the reversed variable by 1
        reversed |= num & 1; // Use bitwise OR to set the rightmost bit of reversed to the current bit of num
        num >>= 1;           // Right shift num by 1 to get the next bit
    }

    return reversed;
}

int _largest_power_of_two(int num)
{
    int power = 0;

    while (num > 1) {
        num = num >> 1;
        power++;
    }

    return 1 << power;
}

void volc_delay(int ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

void enc32be(void *dst, uint32_t val)
{
	((unsigned char *)dst)[0] = (val >> 24);
	((unsigned char *)dst)[1] = (val >> 16);
	((unsigned char *)dst)[2] = (val >> 8);
	((unsigned char *)dst)[3] = val;
}

// 检查 esp_err_t 返回值，如果不为 ESP_OK，则记录错误并挂起系统
void check_esp_err(esp_err_t err, const char *func, const char *file, int line)
{
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error occurred in %s at %s:%d, err: %s", func, file, line, esp_err_to_name(err));
        while (1) {
            vTaskDelay(portMAX_DELAY); // 挂起系统，无限期等待
        }
    }
}

esp_err_t receive_work(uint8_t * buffer, int buffer_size, uint32_t chain_num)
{
    int received = SERIAL_rx(chain_num, buffer, buffer_size, 10000, true);

    if (received < 0) {
        ESP_LOGE(TAG, "UART error in serial RX");
        return ESP_FAIL;
    }

    if (received == 0) {
        ESP_LOGD(TAG, "UART timeout in serial RX");
        return ESP_FAIL;
    }

    if (received != buffer_size) {
        ESP_LOGE(TAG, "Invalid response length %i", received);
        ESP_LOG_BUFFER_HEX(TAG, buffer, received);
        SERIAL_clear_buffer(chain_num);
        return ESP_FAIL;
    }

    uint16_t received_preamble = *(uint16_t *)buffer;
    if (received_preamble != MS_SYNC_TAG) {
        ESP_LOGE(TAG, "Preamble mismatch: got 0x%04x, expected 0x%04x", received_preamble, MS_SYNC_TAG);
        ESP_LOG_BUFFER_HEX(TAG, buffer, received);
        SERIAL_clear_buffer(chain_num);
        return ESP_FAIL;
    }

    if (CRC5(buffer + 2, 64) != buffer[buffer_size - 1]) {
        ESP_LOGE(TAG, "Checksum failed on response");        
        ESP_LOG_BUFFER_HEX(TAG, buffer, received);
        SERIAL_clear_buffer(chain_num);
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t receive_work_with_large_buffer(uint8_t * buffer, int buffer_size, uint32_t chain_num)
{
    uint8_t large_buffer[600];
    int received = SERIAL_rx(chain_num, large_buffer, sizeof(large_buffer)/sizeof(large_buffer[0]), 10000, true);

    if (received < 0) {
        ESP_LOGE(TAG, "UART error in serial RX");
        return ESP_FAIL;
    }

    if (received == 0) {
        ESP_LOGD(TAG, "UART timeout in serial RX");
        return ESP_FAIL;
    }

    if (received != buffer_size) {
        ESP_LOGE(TAG, "Invalid response length %i", received);
        //ESP_LOG_BUFFER_HEX(TAG, large_buffer, received);
        SERIAL_clear_buffer(chain_num);
        return ESP_FAIL;
    }else{
        memcpy(buffer, large_buffer, buffer_size);
    }

    uint16_t received_preamble = *(uint16_t *)buffer;
    if (received_preamble != MS_SYNC_TAG) {
        ESP_LOGE(TAG, "Preamble mismatch: got 0x%04x, expected 0x%04x", received_preamble, MS_SYNC_TAG);
        ESP_LOG_BUFFER_HEX(TAG, buffer, received);
        SERIAL_clear_buffer(chain_num);
        return ESP_FAIL;
    }

    if (CRC5(buffer + 2, 64) != buffer[buffer_size - 1]) {
        ESP_LOGE(TAG, "Checksum failed on response");        
        ESP_LOG_BUFFER_HEX(TAG, buffer, received);
        SERIAL_clear_buffer(chain_num);
        return ESP_FAIL;
    }

    return ESP_OK;
}
