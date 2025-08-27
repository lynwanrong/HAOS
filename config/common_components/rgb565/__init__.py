import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import display
from esphome.const import (
    CONF_DC_PIN,
    CONF_HEIGHT,
    CONF_ID,
    CONF_LAMBDA,
    CONF_RESET_PIN,
    CONF_WIDTH,
)

DEPENDENCIES = ["esp32"]
CONFLICTS_WITH = ["display"]

rgb565_ns = cg.esphome_ns.namespace("rgb565")
RGB565Display = rgb565_ns.class_(
    "RGB565Display", cg.PollingComponent, display.DisplayBuffer
)

CONF_HSYNC_PIN = "hsync_pin"
CONF_VSYNC_PIN = "vsync_pin"
CONF_DE_PIN = "de_pin"
CONF_PCLK_PIN = "pclk_pin"
CONF_ENABLE_PIN = "enable_pin"
CONF_PCLK_FREQUENCY = "pclk_frequency"
CONF_HSYNC_PULSE_WIDTH = "hsync_pulse_width"
CONF_HSYNC_BACK_PORCH = "hsync_back_porch"
CONF_HSYNC_FRONT_PORCH = "hsync_front_porch"
CONF_VSYNC_PULSE_WIDTH = "vsync_pulse_width"
CONF_VSYNC_BACK_PORCH = "vsync_back_porch"
CONF_VSYNC_FRONT_PORCH = "vsync_front_porch"

CONFIG_SCHEMA = cv.All(
    display.FULL_DISPLAY_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(RGB565Display),
            cv.Required(CONF_WIDTH): cv.uint16_t,
            cv.Required(CONF_HEIGHT): cv.uint16_t,
            cv.Required(CONF_HSYNC_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_VSYNC_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_DE_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_PCLK_PIN): pins.gpio_output_pin_schema,
            cv.Optional(CONF_DC_PIN): pins.gpio_output_pin_schema,
            cv.Optional(CONF_RESET_PIN): pins.gpio_output_pin_schema,
            cv.Optional(CONF_ENABLE_PIN): pins.gpio_output_pin_schema,
            cv.Optional(CONF_PCLK_FREQUENCY, default=16000000): cv.uint32_t,
            cv.Optional(CONF_HSYNC_PULSE_WIDTH, default=10): cv.uint16_t,
            cv.Optional(CONF_HSYNC_BACK_PORCH, default=10): cv.uint16_t,
            cv.Optional(CONF_HSYNC_FRONT_PORCH, default=20): cv.uint16_t,
            cv.Optional(CONF_VSYNC_PULSE_WIDTH, default=10): cv.uint16_t,
            cv.Optional(CONF_VSYNC_BACK_PORCH, default=10): cv.uint16_t,
            cv.Optional(CONF_VSYNC_FRONT_PORCH, default=10): cv.uint16_t,
        }
    )
    .extend(cv.polling_component_schema("5s")),
    cv.has_at_least_one_key(CONF_LAMBDA),
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await display.register_display(var, config)

    cg.add(var.set_width(config[CONF_WIDTH]))
    cg.add(var.set_height(config[CONF_HEIGHT]))
    
    # Set timing parameters
    cg.add(var.set_pclk_frequency(config[CONF_PCLK_FREQUENCY]))
    cg.add(var.set_hsync_pulse_width(config[CONF_HSYNC_PULSE_WIDTH]))
    cg.add(var.set_hsync_back_porch(config[CONF_HSYNC_BACK_PORCH]))
    cg.add(var.set_hsync_front_porch(config[CONF_HSYNC_FRONT_PORCH]))
    cg.add(var.set_vsync_pulse_width(config[CONF_VSYNC_PULSE_WIDTH]))
    cg.add(var.set_vsync_back_porch(config[CONF_VSYNC_BACK_PORCH]))
    cg.add(var.set_vsync_front_porch(config[CONF_VSYNC_FRONT_PORCH]))

    # Set pins
    hsync_pin = await cg.gpio_pin_expression(config[CONF_HSYNC_PIN])
    cg.add(var.set_pin_hsync(hsync_pin))
    
    vsync_pin = await cg.gpio_pin_expression(config[CONF_VSYNC_PIN])
    cg.add(var.set_pin_vsync(vsync_pin))
    
    de_pin = await cg.gpio_pin_expression(config[CONF_DE_PIN])
    cg.add(var.set_pin_de(de_pin))
    
    pclk_pin = await cg.gpio_pin_expression(config[CONF_PCLK_PIN])
    cg.add(var.set_pin_pclk(pclk_pin))
    
    if CONF_DC_PIN in config:
        dc_pin = await cg.gpio_pin_expression(config[CONF_DC_PIN])
        cg.add(var.set_pin_dc(dc_pin))
        
    if CONF_RESET_PIN in config:
        reset_pin = await cg.gpio_pin_expression(config[CONF_RESET_PIN])
        cg.add(var.set_pin_reset(reset_pin))
        
    if CONF_ENABLE_PIN in config:
        enable_pin = await cg.gpio_pin_expression(config[CONF_ENABLE_PIN])
        cg.add(var.set_pin_enable(enable_pin))

    if CONF_LAMBDA in config:
        lambda_ = await cg.process_lambda(
            config[CONF_LAMBDA], [(display.DisplayRef, "it")], return_type=cg.void
        )
        cg.add(var.set_writer(lambda_))