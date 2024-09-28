#pragma once

#include "pylontech_emulator_component.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"

namespace esphome {
namespace pylontech {

  template<typename... Ts> class EnableAction : public Action<Ts...> {
    public:
      explicit EnableAction(PylontechEmulatorComponent *component) : component_(component) {}

      void play(Ts... x) override { this->component_->set_enabled(true); }

    protected:
      PylontechEmulatorComponent *component_;
  };

  template<typename... Ts> class DisableAction : public Action<Ts...> {
    public:
      explicit DisableAction(PylontechEmulatorComponent *component) : component_(component) {}

      void play(Ts... x) override { this->component_->set_enabled(false); }

    protected:
      PylontechEmulatorComponent *component_;
  };

  class ConnectionStateChangeTrigger : public Trigger<bool> {
    public:
      ConnectionStateChangeTrigger(PylontechEmulatorComponent *component) {
        component->add_on_connection_state_changed_callback(
          [this](bool connected) { this->trigger(connected); }
        );
      }
};

}  // namespace pylontech
}  // namespace esphome