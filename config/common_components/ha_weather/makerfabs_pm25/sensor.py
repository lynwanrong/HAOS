"""makerfabs_pm25/sensor.py

This module defines the custom Makerfabs PM2.5 sensor for ESPHome.
"""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, uart
from esphome.const import (
    CONF_ID,
    CONF_PM_2_5,
    DEVICE_CLASS_PM25,
    STATE_CLASS_MEASUREMENT,
    UNIT_MICROGRAMS_PER_CUBIC_METER,
)

# 声明我们的C++组件所在的命名空间
makerfabs_pm25_ns = cg.esphome_ns.namespace("makerfabs_pm25")
# 声明我们的C++类
MakerfabsPm25Sensor = makerfabs_pm25_ns.class_(
    "MakerfabsPm25Sensor", cg.PollingComponent, sensor.Sensor, uart.UARTDevice
)

# 定义YAML配置的验证规则
CONFIG_SCHEMA = (
    sensor.sensor_schema(
        MakerfabsPm25Sensor,
        unit_of_measurement=UNIT_MICROGRAMS_PER_CUBIC_METER,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_PM25,
        state_class=STATE_CLASS_MEASUREMENT,
    )
   .extend(cv.polling_component_schema("15s"))
   .extend(uart.UART_DEVICE_SCHEMA)
)

# 定义代码生成函数
async def to_code(config):
    # 1. 创建C++类的实例
    var = cg.new_Pvariable(config[CONF_ID])
    
    # 2. 注册为轮询组件
    await cg.register_component(var, config)
    
    # 3. 注册为传感器实体
    await sensor.register_sensor(var, config)
    
    # 4. 注册为UART设备，并注入UART总线依赖
    await uart.register_uart_device(var, config)
