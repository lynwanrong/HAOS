// ssap10_espidf.cpp
// ESP-IDF implementation for the SSAP10 sensor

#include "ssap10.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "SSAP10";

#define SSAP10_FRAME_LENGTH 32
#define UART_PORT_NUM      (UART_NUM_2)
#define UART_TX_PIN       (GPIO_NUM_21)
#define UART_RX_PIN       (GPIO_NUM_22)
#define UART_RTS_PIN      (UART_PIN_NO_CHANGE)
#define UART_CTS_PIN      (UART_PIN_NO_CHANGE)


namespace esphome {
namespace ssap10 {

SSAP10Sensor::SSAP10Sensor() : PollingComponent(15000) {}

void SSAP10Sensor::setup() {
    ESP_LOGI(TAG, "Setting up SSAP10 sensor...");

    const uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    // Configure UART driver
    uart_param_config(UART_PORT_NUM, &uart_config);
    uart_set_pin(UART_PORT_NUM, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM_2, SSAP10_FRAME_LENGTH * 2, 0, 0, NULL, 0);
}

void SSAP10Sensor::update() {
    uint8_t data[SSAP10_FRAME_LENGTH];
    int length = 0;

    // Read data from UART
    length = uart_read_bytes(UART_PORT_NUM, data, SSAP10_FRAME_LENGTH, pdMS_TO_TICKS(100));
    if (length < SSAP10_FRAME_LENGTH) {
        ESP_LOGW(TAG, "Incomplete data frame received");
        return;
    }

    // Validate frame header
    if (data[0] != 0x42 || data[1] != 0x4d) {
        ESP_LOGW(TAG, "Invalid frame header: %02X %02X", data[0], data[1]);
        return;
    }

    // Validate checksum
    uint8_t received_checksum = data[SSAP10_FRAME_LENGTH - 1];
    uint8_t calculated_checksum = 0;
    for (int i = 0; i < SSAP10_FRAME_LENGTH - 1; i++) {
        calculated_checksum += data[i];
    }

    if (received_checksum != calculated_checksum) {
        ESP_LOGW(TAG, "Checksum mismatch: calculated %u, received %u", calculated_checksum, received_checksum);
        return;
    }

    // Extract sensor data (e.g., PM2.5 value)
    int sensor_value = (data[6] << 8) | data[7];
    ESP_LOGI(TAG, "SSAP10 sensor value: %d", sensor_value);

    // Publish data to Home Assistant
    this->publish_state(sensor_value);
}

void SSAP10Sensor::dump_config() {
    ESP_LOGCONFIG(TAG, "SSAP10 Sensor:");
    ESP_LOGCONFIG(TAG, "  Update Interval: %d ms", this->get_update_interval());
}

float SSAP10Sensor::get_setup_priority() const {
    return setup_priority::DATA;
}

}  // namespace ssap10
}  // namespace esphome
