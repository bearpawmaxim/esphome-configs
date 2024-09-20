#include "_pylontech_rs485_low_voltage.h"

namespace esphome {
namespace pylontech {

using namespace uart;
using namespace sensor;
using namespace pylontech_lv;

class PylontechEmulatorComponent : public Component, public UARTDevice,
    public PylontechTransport {
  public:
    PylontechEmulatorComponent(esphome::uart::UARTComponent *parent): UARTDevice(parent) {}

    void set_voltage_sensor(Sensor *sensor) { this->voltage_sensor_ = sensor; }
    void set_current_sensor(Sensor *sensor) { this->current_sensor_ = sensor; }
    void set_soc_sensor(Sensor *sensor) { this->soc_sensor_ = sensor; }

    void register_callbacks();

    void setup() override;
    void write_uint8(uint8_t byte);
    uint8_t read_uint8();
    void loop() override;

  private:
    Sensor *voltage_sensor_;
    Sensor *current_sensor_;
    Sensor *soc_sensor_;
    PylontechLowVoltageProtocol *protocol_;

    void handle_get_analog_info_(PylonAnalogInfo *info);
};
}
}
