// makerfabs_pm25/makerfabs_pm25.h

#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace makerfabs_pm25 {

class MakerfabsPM25Sensor : public PollingComponent, public sensor::Sensor {
 public:
  MakerfabsPM25Sensor();
  void set_uart(uart::UARTComponent *uart) { this->uart_ = uart; }

  void setup() override;
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override;

 protected:
  uart::UARTComponent *uart_;
};

}  // namespace makerfabs_pm25
}  // namespace esphome