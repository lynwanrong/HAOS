// makerfabs_pm25/makerfabs_pm25.h

#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace makerfabs_pm25 {

class MakerfabsPm25Sensor : public PollingComponent, public sensor::Sensor, public uart::UARTDevice {
 public:
  MakerfabsPm25Sensor();

  void setup() override;
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override;

 protected:
  // Inherit UARTDevice behavior (set_uart_parent, available, read, read_array, etc.)
};

}  // namespace makerfabs_pm25
}  // namespace esphome
