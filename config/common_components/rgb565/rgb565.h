#pragma once

#include "esphome.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"

namespace esphome {
namespace rgb565 {

class RGB565Display : public PollingComponent, public display::DisplayBuffer {
 public:
  void set_pin_dc(GPIOPin *dc_pin) { this->dc_pin_ = dc_pin; }
  void set_pin_reset(GPIOPin *reset_pin) { this->reset_pin_ = reset_pin; }
  
  // RGB timing parameters
  void set_pclk_frequency(uint32_t frequency) { this->pclk_frequency_ = frequency; }
  void set_hsync_pulse_width(uint16_t pulse_width) { this->hsync_pulse_width_ = pulse_width; }
  void set_hsync_back_porch(uint16_t back_porch) { this->hsync_back_porch_ = back_porch; }
  void set_hsync_front_porch(uint16_t front_porch) { this->hsync_front_porch_ = front_porch; }
  void set_vsync_pulse_width(uint16_t pulse_width) { this->vsync_pulse_width_ = pulse_width; }
  void set_vsync_back_porch(uint16_t back_porch) { this->vsync_back_porch_ = back_porch; }
  void set_vsync_front_porch(uint16_t front_porch) { this->vsync_front_porch_ = front_porch; }

  // Display dimensions
  void set_width(uint16_t width) { this->width_ = width; }
  void set_height(uint16_t height) { this->height_ = height; }
  
  // RGB data pins (typically 16 bits for RGB565)
  void set_data_pins(std::vector<GPIOPin*> data_pins) { this->data_pins_ = data_pins; }
  
  // Control pins
  void set_pin_hsync(GPIOPin *hsync_pin) { this->hsync_pin_ = hsync_pin; }
  void set_pin_vsync(GPIOPin *vsync_pin) { this->vsync_pin_ = vsync_pin; }
  void set_pin_de(GPIOPin *de_pin) { this->de_pin_ = de_pin; }
  void set_pin_pclk(GPIOPin *pclk_pin) { this->pclk_pin_ = pclk_pin; }
  void set_pin_enable(GPIOPin *enable_pin) { this->enable_pin_ = enable_pin; }

  // Component methods
  void setup() override;
  void update() override;
  void draw_pixels_at(int x_start, int y_start, int w, int h, const uint8_t *ptr, display::ColorOrder order, display::ColorBitness bitness, bool big_endian, int x_offset, int y_offset, int x_pad) override;
  void fill(Color color) override;
  
  // DisplayBuffer methods
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::PROCESSOR; }
  display::DisplayType get_display_type() override { return display::DisplayType::DISPLAY_TYPE_COLOR; }
  
 protected:
  int get_width_internal() override { return this->width_; }
  int get_height_internal() override { return this->height_; }
  
 private:
  // Pins
  GPIOPin *dc_pin_{nullptr};
  GPIOPin *reset_pin_{nullptr};
  GPIOPin *hsync_pin_{nullptr};
  GPIOPin *vsync_pin_{nullptr};
  GPIOPin *de_pin_{nullptr};
  GPIOPin *pclk_pin_{nullptr};
  GPIOPin *enable_pin_{nullptr};
  std::vector<GPIOPin*> data_pins_;
  
  // Timing parameters
  uint32_t pclk_frequency_{16 * 1000 * 1000}; // 16MHz default
  uint16_t hsync_pulse_width_{10};
  uint16_t hsync_back_porch_{10};
  uint16_t hsync_front_porch_{20};
  uint16_t vsync_pulse_width_{10};
  uint16_t vsync_back_porch_{10};
  uint16_t vsync_front_porch_{10};
  
  // Display dimensions
  uint16_t width_{320};
  uint16_t height_{240};
  
  // LCD panel handle
  esp_lcd_panel_handle_t panel_handle_{nullptr};
};

} // namespace rgb565
} // namespace esphome