import logging
import esphome.codegen as cg
from esphome.components import uart, sensor
import esphome.config_validation as cv
from esphome.const import CONF_ID

_LOGGER = logging.getLogger(__name__)

CODEOWNERS = ["@bearpawmaxim"]
DEPENDENCIES = ["uart"]
MULTI_CONF = False

CONF_PYLONTECH_ID = "pylontech_id"
CONF_VOLTAGE_SENSOR = "voltage_sensor"
CONF_CURRENT_SENSOR = "current_sensor"
CONF_SOC_SENSOR = "soc_sensor"

pylontech_ns = cg.esphome_ns.namespace("pylontech")
PylontechComponent = pylontech_ns.class_(
    "PylontechEmulatorComponent", cg.Component, uart.UARTDevice
)

PYLONTECH_COMPONENT_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_PYLONTECH_ID): cv.use_id(PylontechComponent),
    }
)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(PylontechComponent),
            cv.Required(CONF_VOLTAGE_SENSOR): cv.use_id(sensor),
            cv.Required(CONF_CURRENT_SENSOR): cv.use_id(sensor),
            cv.Required(CONF_SOC_SENSOR): cv.use_id(sensor)
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)

async def set_sensor(var, config, sensor_key, set_method):
    sensor_var = await cg.get_variable(config[sensor_key])
    cg.add(getattr(var, set_method)(sensor_var))

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    await set_sensor(var, config, CONF_VOLTAGE_SENSOR, "set_voltage_sensor")
    await set_sensor(var, config, CONF_CURRENT_SENSOR, "set_current_sensor")
    await set_sensor(var, config, CONF_SOC_SENSOR, "set_soc_sensor")
    cg.add(var.register_callbacks())