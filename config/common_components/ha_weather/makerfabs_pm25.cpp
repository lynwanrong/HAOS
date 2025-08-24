// makerfabs_pm25/makerfabs_pm25.cpp

#include "makerfabs_pm25.h"
#include "esphome/core/log.h"

namespace esphome {
namespace makerfabs_pm25 {

static const char *const TAG = "makerfabs_pm25.sensor";
static const int PM25_FRAME_LENGTH = 32;
static const int PM25_VALUE_HIGH_BYTE = 6;
static const int PM25_VALUE_LOW_BYTE = 7;

// 构造函数，设置默认轮询间隔为15秒
MakerfabsPM25Sensor::MakerfabsPM25Sensor() : PollingComponent(15000) {}

float MakerfabsPM25Sensor::get_setup_priority() const { return setup_priority::LATE; }

void MakerfabsPM25Sensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Makerfabs PM2.5 Sensor...");
}

void MakerfabsPM25Sensor::dump_config() {
  ESP_LOGCONFIG(TAG, "Makerfabs PM2.5 Sensor:");
  LOG_SENSOR("  ", "PM2.5", this);
  this->check_uart_settings(9600);
}

void MakerfabsPM25Sensor::update() {
  uint8_t buf[PM25_FRAME_LENGTH];
  
  // 清空串口缓冲区
  while (this->uart_ && this->uart_->available()) {
    this->uart_->read();
  }

  if (!this->uart_ || !this->uart_->read_array(buf, PM25_FRAME_LENGTH)) {
    ESP_LOGW(TAG, "Failed to read data from UART");
    this->publish_state(NAN); // 发布无效状态
    return;
  }

  // 校验帧头 0x42 0x4d
  if (buf[0] != 0x42 || buf[1] != 0x4d) {
    ESP_LOGW(TAG, "Invalid frame header received: %02X %02X", buf[0], buf[1]);
    this->publish_state(NAN);
    return;
  }

  // 校验和
  uint16_t checksum_calc = 0;
  for (int i = 0; i < PM25_FRAME_LENGTH - 2; i++) {
    checksum_calc += buf[i];
  }
  uint16_t checksum_recv = ((uint16_t)buf[PM25_FRAME_LENGTH - 2] << 8) | buf[PM25_FRAME_LENGTH - 1];

  if (checksum_calc != checksum_recv) {
    ESP_LOGW(TAG, "Checksum mismatch! Calculated: %u, Received: %u", checksum_calc, checksum_recv);
    this->publish_state(NAN);
    return;
  }

  // 提取PM2.5数值 (CF-1, standard particle)
  int value = (int)buf[PM25_VALUE_HIGH_BYTE] * 256 + (int)buf[PM25_VALUE_LOW_BYTE];
  ESP_LOGD(TAG, "Received PM2.5 value: %d ug/m3", value);
  this->publish_state(value);
}
}  // namespace esphome