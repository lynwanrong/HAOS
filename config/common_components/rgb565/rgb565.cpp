#include "rgb565.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "esphome/core/helpers.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_panel_interface.h"
#include "esp_log.h"
#include <cstring>

#ifdef USE_ESP32

namespace esphome {
namespace rgb565 {

static const char *const TAG = "rgb565";

void RGB565Display::setup() {
  ESP_LOGCONFIG(TAG, "Setting up RGB565 Display...");

  // Calculate buffer size
  this->buffer_size_ = this->width_ * this->height_ * 2; // 2 bytes per pixel for RGB565
  ESP_LOGCONFIG(TAG, "  Buffer size: %d bytes", this->buffer_size_);
  
  // Allocate buffer in PSRAM if available
  if (psramFound()) {
    this->buffer_ = (uint16_t *) heap_caps_malloc(this->buffer_size_, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (this->buffer_ == nullptr) {
      ESP_LOGE(TAG, "Could not allocate buffer in PSRAM, trying internal memory");
      this->buffer_ = (uint16_t *) malloc(this->buffer_size_);
    }
  } else {
    this->buffer_ = (uint16_t *) malloc(this->buffer_size_);
  }

  if (this->buffer_ == nullptr) {
    ESP_LOGE(TAG, "Could not allocate buffer, display will not work");
    this->mark_failed();
    return;
  }

  ESP_LOGCONFIG(TAG, "  Allocated buffer at %p", this->buffer_);

  // Initialize LCD panel
  esp_lcd_rgb_panel_config_t panel_config = {
    .clk_src = LCD_CLK_SRC_PLL160M,
    .num_fbs = 2,
    .timings = {
      .pclk_hz = this->pclk_frequency_,
      .h_res = this->width_,
      .v_res = this->height_,
      .hsync_pulse_width = this->hsync_pulse_width_,
      .hsync_back_porch = this->hsync_back_porch_,
      .hsync_front_porch = this->hsync_front_porch_,
      .vsync_pulse_width = this->vsync_pulse_width_,
      .vsync_back_porch = this->vsync_back_porch_,
      .vsync_front_porch = this->vsync_front_porch_,
      .flags = {
        .hsync_idle_low = 0,
        .vsync_idle_low = 0,
        .de_idle_high = 0,
        .pclk_active_neg = 1,
        .pclk_idle_high = 0,
      }
    },
    .data_width = 16, // RGB565
    .psram_trans_align = 64,
    .hsync_gpio_num = this->hsync_pin_,
    .vsync_gpio_num = this->vsync_pin_,
    .de_gpio_num = this->de_pin_,
    .pclk_gpio_num = this->pclk_pin_,
    .disp_gpio_num = GPIO_NUM_NC,
    .data_gpio_nums = {
      this->red_pins_[4],   // B0
      this->red_pins_[3],   // B1
      this->red_pins_[2],   // B2
      this->red_pins_[1],   // B3
      this->red_pins_[0],   // B4
      this->green_pins_[5], // G0
      this->green_pins_[4], // G1
      this->green_pins_[3], // G2
      this->green_pins_[2], // G3
      this->green_pins_[1], // G4
      this->green_pins_[0], // G5
      this->blue_pins_[4],  // R0
      this->blue_pins_[3],  // R1
      this->blue_pins_[2],  // R2
      this->blue_pins_[1],  // R3
      this->blue_pins_[0],  // R4
    },
    .flags = {
      .disp_active_low = 0,
      .stream_mode = 1, // Enable streaming mode for better performance
      .refresh_on_demand = 0,
      .fb_in_psram = 1, // Put frame buffer in PSRAM
      .bb_invalidate_en = 1,
    }
  };

  esp_err_t err = esp_lcd_new_rgb_panel(&panel_config, &this->panel_handle_);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to create RGB panel: %s", esp_err_to_name(err));
    this->mark_failed();
    return;
  }

  err = esp_lcd_panel_reset(this->panel_handle_);
  if (err != ESP_OK) {
    ESP_LOGW(TAG, "Failed to reset panel: %s", esp_err_to_name(err));
  }

  err = esp_lcd_panel_init(this->panel_handle_);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize panel: %s", esp_err_to_name(err));
    this->mark_failed();
    return;
  }

  // Clear display
  this->fill(COLOR_BLACK);
  this->display_();
  
  ESP_LOGCONFIG(TAG, "RGB565 Display setup complete");
}

void RGB565Display::dump_config() {
  ESP_LOGCONFIG(TAG, "RGB565 Display:");
  ESP_LOGCONFIG(TAG, "  Width: %u", this->width_);
  ESP_LOGCONFIG(TAG, "  Height: %u", this->height_);
  ESP_LOGCONFIG(TAG, "  PCLK Pin: %u", this->pclk_pin_);
  ESP_LOGCONFIG(TAG, "  DE Pin: %u", this->de_pin_);
  ESP_LOGCONFIG(TAG, "  HSync Pin: %u", this->hsync_pin_);
  ESP_LOGCONFIG(TAG, "  VSync Pin: %u", this->vsync_pin_);
  ESP_LOGCONFIG(TAG, "  PCLK Frequency: %u Hz", this->pclk_frequency_);
  ESP_LOGCONFIG(TAG, "  HSync Back Porch: %u", this->hsync_back_porch_);
  ESP_LOGCONFIG(TAG, "  HSync Front Porch: %u", this->hsync_front_porch_);
  ESP_LOGCONFIG(TAG, "  HSync Pulse Width: %u", this->hsync_pulse_width_);
  ESP_LOGCONFIG(TAG, "  VSync Back Porch: %u", this->vsync_back_porch_);
  ESP_LOGCONFIG(TAG, "  VSync Front Porch: %u", this->vsync_front_porch_);
  ESP_LOGCONFIG(TAG, "  VSync Pulse Width: %u", this->vsync_pulse_width_);
  ESP_LOGCONFIG(TAG, "  Red Pins: %u, %u, %u, %u, %u", 
    this->red_pins_[0], this->red_pins_[1], this->red_pins_[2], this->red_pins_[3], this->red_pins_[4]);
  ESP_LOGCONFIG(TAG, "  Green Pins: %u, %u, %u, %u, %u, %u", 
    this->green_pins_[0], this->green_pins_[1], this->green_pins_[2], 
    this->green_pins_[3], this->green_pins_[4], this->green_pins_[5]);
  ESP_LOGCONFIG(TAG, "  Blue Pins: %u, %u, %u, %u, %u", 
    this->blue_pins_[0], this->blue_pins_[1], this->blue_pins_[2], this->blue_pins_[3], this->blue_pins_[4]);
}

float RGB565Display::get_setup_priority() const {
  return setup_priority::PROCESSOR;
}

void RGB565Display::update() {
  this->do_update_();
  this->display_();
}

void RGB565Display::display_() {
  if (this->panel_handle_ == nullptr) {
    return;
  }

  // Send the buffer to the display
  esp_err_t err = esp_lcd_panel_draw_bitmap(this->panel_handle_, 0, 0, this->width_ - 1, this->height_ - 1, this->buffer_);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to draw bitmap: %s", esp_err_to_name(err));
  }
}

void RGB565Display::fill(Color color) {
  if (this->buffer_ == nullptr) {
    return;
  }

  // Convert RGB888 to RGB565
  uint16_t color565 = ((color.r & 0xF8) << 8) | ((color.g & 0xFC) << 3) | (color.b >> 3);
  
  // Fill the buffer with the color
  for (size_t i = 0; i < this->width_ * this->height_; i++) {
    this->buffer_[i] = color565;
  }
}

void RGB565Display::draw_absolute_pixel_at(int x, int y, Color color) {
  if (x >= this->width_ || x < 0 || y >= this->height_ || y < 0) {
    return;
  }

  // Convert RGB888 to RGB565
  uint16_t color565 = ((color.r & 0xF8) << 8) | ((color.g & 0xFC) << 3) | (color.b >> 3);
  
  // Draw the pixel
  this->buffer_[y * this->width_ + x] = color565;
}

int RGB565Display::get_width_internal() {
  return this->width_;
}

int RGB565Display::get_height_internal() {
  return this->height_;
}

}  // namespace rgb565
}  // namespace esphome

#endif // USE_ESP32