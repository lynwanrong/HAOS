#include "esphome.h"
#include "esp_task_wdt.h"

class WatchdogComponent : public Component {
 public:
  void setup() override {
    // 启用任务看门狗
    esp_task_wdt_config_t wdt_config = {
      .timeout_ms = 30000,  // 30秒超时
      .idle_core_mask = 0,
      .trigger_panic = true,
    };
    esp_task_wdt_reconfigure(&wdt_config);
    
    // 添加当前任务到看门狗
    esp_task_wdt_add(NULL);
  }

  void loop() override {
    // 定期喂狗
    esp_task_wdt_reset();
    delay(1000);  // 每秒喂一次狗
  }
};