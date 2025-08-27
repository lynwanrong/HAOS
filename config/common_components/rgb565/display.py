import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import display
from esphome.const import (
    CONF_HEIGHT,
    CONF_ID,
    CONF_WIDTH,
    CONF_DC_PIN,
    CONF_RESET_PIN,
    CONF_DIMENSIONS,
    CONF_HSYNC_PIN,
    CONF_VSYNC_PIN,
    CONF_DE_PIN,
    CONF_PCLK_PIN,
    CONF_DATA_PINS,
    CONF_RED,
    CONF_GREEN,
    CONF_BLUE,
    CONF_PCLK_FREQUENCY,
    CONF_HSYNC_BACK_PORCH,
    CONF_HSYNC_FRONT_PORCH,
    CONF_HSYNC_PULSE_WIDTH,
    CONF_VSYNC_BACK_PORCH,
    CONF_VSYNC_FRONT_PORCH,
    CONF_VSYNC_PULSE_WIDTH,
)

DEPENDENCIES = ["esp32"]
CODEOWNERS = ["@xiaren"]

rgb565_ns = cg.esphome_ns.namespace("rgb565")
RGB565Display = rgb565_ns.class_("RGB565Display", display.DisplayBuffer)

CONF_RED_PINS = "red_pins"
CONF_GREEN_PINS = "green_pins"
CONF_BLUE_PINS = "blue_pins"

def validate_data_pins(value):
    if CONF_DATA_PINS in value:
        # Using simplified data_pins definition
        return value
    
    if CONF_RED_PINS not in value:
        raise cv.Invalid("Either 'data_pins' or 'red_pins' must be defined")
    if CONF_GREEN_PINS not in value:
        raise cv.Invalid("Either 'data_pins' or 'green_pins' must be defined")
    if CONF_BLUE_PINS not in value:
        raise cv.Invalid("Either 'data_pins' or 'blue_pins' must be defined")
    return value

CONFIG_SCHEMA = cv.All(
    display.FULL_DISPLAY_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(RGB565Display),
            cv.Required(CONF_DIMENSIONS): cv.Any(
                cv.dimensions,
                cv.Schema({
                    cv.Required(CONF_WIDTH): cv.int_,
                    cv.Required(CONF_HEIGHT): cv.int_,
                }),
            ),
            cv.Optional(CONF_DC_PIN): pins.gpio_output_pin_schema,
            cv.Optional(CONF_RESET_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_PCLK_PIN): cv.int_range(min=0, max=48),
            cv.Required(CONF_DE_PIN): cv.int_range(min=0, max=48),
            cv.Required(CONF_HSYNC_PIN): cv.int_range(min=0, max=48),
            cv.Required(CONF_VSYNC_PIN): cv.int_range(min=0, max=48),
            cv.Optional(CONF_DATA_PINS): cv.Schema({
                cv.Required(CONF_RED): cv.All(cv.ensure_list, [cv.int_range(min=0, max=48)], cv.Length(min=5, max=5)),
                cv.Required(CONF_GREEN): cv.All(cv.ensure_list, [cv.int_range(min=0, max=48)], cv.Length(min=6, max=6)),
                cv.Required(CONF_BLUE): cv.All(cv.ensure_list, [cv.int_range(min=0, max=48)], cv.Length(min=5, max=5)),
            }),
            cv.Optional(CONF_RED_PINS): cv.All(cv.ensure_list, [cv.int_range(min=0, max=48)], cv.Length(min=5, max=5)),
            cv.Optional(CONF_GREEN_PINS): cv.All(cv.ensure_list, [cv.int_range(min=0, max=48)], cv.Length(min=6, max=6)),
            cv.Optional(CONF_BLUE_PINS): cv.All(cv.ensure_list, [cv.int_range(min=0, max=48)], cv.Length(min=5, max=5)),
            cv.Optional(CONF_PCLK_FREQUENCY, default="16MHz"): cv.frequency,
            cv.Optional(CONF_HSYNC_BACK_PORCH, default=0): cv.int_,
            cv.Optional(CONF_HSYNC_FRONT_PORCH, default=0): cv.int_,
            cv.Optional(CONF_HSYNC_PULSE_WIDTH, default=0): cv.int_,
            cv.Optional(CONF_VSYNC_BACK_PORCH, default=0): cv.int_,
            cv.Optional(CONF_VSYNC_FRONT_PORCH, default=0): cv.int_,
            cv.Optional(CONF_VSYNC_PULSE_WIDTH, default=0): cv.int_,
        }
    ).extend(cv.COMPONENT_SCHEMA),
    validate_data_pins,
)

async def to_code(config):
    # Create the display instance
    var = cg.new_Pvariable(config[CONF_ID])
    
    # Setup display dimensions
    if isinstance(config[CONF_DIMENSIONS], dict):
        cg.add(var.set_dimensions(config[CONF_DIMENSIONS][CONF_WIDTH], config[CONF_DIMENSIONS][CONF_HEIGHT]))
    else:
        (width, height) = config[CONF_DIMENSIONS]
        cg.add(var.set_dimensions(width, height))
    
    # Setup pins
    if CONF_DC_PIN in config:
        dc = await cg.gpio_pin_expression(config[CONF_DC_PIN])
        cg.add(var.set_dc_pin(dc))
        
    if CONF_RESET_PIN in config:
        reset = await cg.gpio_pin_expression(config[CONF_RESET_PIN])
        cg.add(var.set_reset_pin(reset))
        
    # RGB timing pins
    cg.add(var.set_pclk_pin(config[CONF_PCLK_PIN]))
    cg.add(var.set_de_pin(config[CONF_DE_PIN]))
    cg.add(var.set_hsync_pin(config[CONF_HSYNC_PIN]))
    cg.add(var.set_vsync_pin(config[CONF_VSYNC_PIN]))
    
    # Data pins
    if CONF_DATA_PINS in config:
        data_pins = config[CONF_DATA_PINS]
        cg.add(var.set_data_pins(data_pins[CONF_RED], data_pins[CONF_GREEN], data_pins[CONF_BLUE]))
    else:
        cg.add(var.set_data_pins(config[CONF_RED_PINS], config[CONF_GREEN_PINS], config[CONF_BLUE_PINS]))
    
    # Timing parameters
    cg.add(var.set_pclk_frequency(config[CONF_PCLK_FREQUENCY]))
    cg.add(var.set_hsync_back_porch(config[CONF_HSYNC_BACK_PORCH]))
    cg.add(var.set_hsync_front_porch(config[CONF_HSYNC_FRONT_PORCH]))
    cg.add(var.set_hsync_pulse_width(config[CONF_HSYNC_PULSE_WIDTH]))
    cg.add(var.set_vsync_back_porch(config[CONF_VSYNC_BACK_PORCH]))
    cg.add(var.set_vsync_front_porch(config[CONF_VSYNC_FRONT_PORCH]))
    cg.add(var.set_vsync_pulse_width(config[CONF_VSYNC_PULSE_WIDTH]))
    
    await cg.register_component(var, config)
    await display.register_display(var, config)