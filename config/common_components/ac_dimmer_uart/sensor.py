from esphome.codegen import cg
from esphome.components import output
import esphome.config_validation as cv
from esphome.const import CONF_ID

CODEOWNERS = ["@xiaren"]

ac_dimmer_uart_ns = cg.esphome_ns.namespace('ac_dimmer_uart')
AcDimmerUart= ac_dimmer_arduino_ns.class_('AcDimmerUart', output.FloatOutput, cg.Component)

CONFIG_SCHEMA = cv.All( 
    output.FLOAT_OUTPUT_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(AcDimmerUart),
        }
    ).extend(cv.COMPONENT_SCHEMA),
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await output.register_output(var, config)