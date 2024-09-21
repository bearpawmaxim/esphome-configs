import logging
import esphome.codegen as cg
from esphome.components import uart, sensor
import esphome.config_validation as cv
from esphome.const import CONF_ADDRESS, CONF_ID

_LOGGER = logging.getLogger(__name__)

CODEOWNERS = ["@bearpawmaxim"]
DEPENDENCIES = ["uart"]
MULTI_CONF = False

CONF_PYLONTECH_ID = "pylontech_id"
CONF_BATTERIES = "batteries"
CONF_VOLTAGE_SENSOR = "voltage_sensor"
CONF_CURRENT_SENSOR = "current_sensor"
CONF_REMAIN_CAPACITY_SENSOR = "remain_capacity_sensor"

pylontech_ns = cg.esphome_ns.namespace("pylontech")
PylontechComponent = pylontech_ns.class_(
    "PylontechEmulatorComponent", cg.Component, uart.UARTDevice
)

BATTERY_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_ADDRESS): cv.int_range(2, 255),
        cv.Required(CONF_VOLTAGE_SENSOR): cv.use_id(sensor),
        cv.Required(CONF_CURRENT_SENSOR): cv.use_id(sensor),
        cv.Required(CONF_REMAIN_CAPACITY_SENSOR): cv.use_id(sensor)
    }
)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(PylontechComponent),
            # cv.Required(CONF_VOLTAGE_SENSOR): cv.use_id(sensor),
            # cv.Required(CONF_CURRENT_SENSOR): cv.use_id(sensor),
            # cv.Required(CONF_REMAIN_CAPACITY_SENSOR): cv.use_id(sensor),
            cv.Required(CONF_BATTERIES): cv.ensure_list(BATTERY_SCHEMA)
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