import logging
import esphome.codegen as cg
from esphome import automation
from esphome.automation import maybe_simple_id
from esphome.components import uart, sensor, text_sensor
from esphome.components.sensor import SensorPtr
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
CONF_SOC_SENSOR = "soc_sensor"
CONF_CYCLES_CNT_SENSOR = "number_of_cycles_sensor"
CONF_SOH_SENSOR = "soh_sensor"
CONF_CELL_VOLTAGE_SENSORS = "cell_voltage_sensors"
CONF_CELL_TEMP_SENSORS = "cell_temperature_sensors"
CONF_MOS_TEMP_SENSOR = "mos_temperature_sensor"
CONF_BMS_TEMP_SENSOR = "bms_temperature_sensor"
CONF_UNKNOWN_COMMAND_SENSOR = "unknown_commands_text_sensor"

pylontech_ns = cg.esphome_ns.namespace("pylontech")
PylontechComponent = pylontech_ns.class_(
    "PylontechEmulatorComponent", cg.Component, uart.UARTDevice
)
BatteryConfig = pylontech_ns.struct("BatteryConfig")



BATTERY_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_ADDRESS): cv.int_range(2, 255),
        cv.Required(CONF_VOLTAGE_SENSOR): cv.use_id(sensor),
        cv.Required(CONF_CURRENT_SENSOR): cv.use_id(sensor),
        cv.Required(CONF_SOC_SENSOR): cv.use_id(sensor),
        cv.Required(CONF_CYCLES_CNT_SENSOR): cv.use_id(sensor),
        cv.Required(CONF_SOH_SENSOR): cv.use_id(sensor),
        cv.Required(CONF_CELL_VOLTAGE_SENSORS): cv.All(
            cv.ensure_list(cv.use_id(sensor)),
            cv.Length(min=1, max=16)
        ),
        cv.Required(CONF_CELL_TEMP_SENSORS): cv.All(
            cv.ensure_list(cv.use_id(sensor)),
            cv.Length(min=1, max=16)
        ),
        cv.Required(CONF_MOS_TEMP_SENSOR): cv.use_id(sensor),
        cv.Required(CONF_BMS_TEMP_SENSOR): cv.use_id(sensor)
    }
)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(PylontechComponent),
            cv.Optional(CONF_ADDRESS, default=2): cv.int_range(0, 255),
            cv.Required(CONF_BATTERIES): cv.All(
                cv.ensure_list(BATTERY_SCHEMA),
                cv.Length(min=1, max=5)
            ),
            cv.Optional(CONF_UNKNOWN_COMMAND_SENSOR): cv.use_id(text_sensor)
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)

async def set_sensor(var, config, sensor_key, set_method):
    sensor_var = await cg.get_variable(config[sensor_key])
    cg.add(getattr(var, set_method)(sensor_var))

async def battery_cell_sensors_to_code(battery, sensor_key):
    return cg.ArrayInitializer([await cg.get_variable(sensor) for sensor in battery[sensor_key]])

async def battery_to_code(var, battery):
    batt_var = cg.StructInitializer(
        BatteryConfig,
        ("address", cg.uint8(battery[CONF_ADDRESS])),
        ("voltage_sensor", await cg.get_variable(battery[CONF_VOLTAGE_SENSOR])),
        ("current_sensor", await cg.get_variable(battery[CONF_CURRENT_SENSOR])),
        ("soc_sensor", await cg.get_variable(battery[CONF_SOC_SENSOR])),
        ("number_of_cycles_sensor", await cg.get_variable(battery[CONF_CYCLES_CNT_SENSOR])),
        ("soh_sensor", await cg.get_variable(battery[CONF_SOH_SENSOR])),
        ("cell_voltage_sensors", await battery_cell_sensors_to_code(battery, CONF_CELL_VOLTAGE_SENSORS)),
        ("cell_temp_sensors", await battery_cell_sensors_to_code(battery, CONF_CELL_TEMP_SENSORS)),
        ("mos_temp_sensor", await cg.get_variable(battery[CONF_MOS_TEMP_SENSOR])),
        ("bms_temp_sensor", await cg.get_variable(battery[CONF_BMS_TEMP_SENSOR]))
    )
    cg.add(var.add_battery(batt_var))

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    cg.add(var.set_address(config[CONF_ADDRESS]))
    if CONF_UNKNOWN_COMMAND_SENSOR in config:
        await set_sensor(var, config, CONF_UNKNOWN_COMMAND_SENSOR, "set_unknown_commands_sensor")
    for battery in config[CONF_BATTERIES]:
        await battery_to_code(var, battery)
