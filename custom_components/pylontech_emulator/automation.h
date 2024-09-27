#pragma once

#include "pylontech_emulator_component.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"

namespace esphome {
namespace pylontech {

  template<typename... Ts> class EnableAction : public Action<Ts...> {
    public:
      explicit EnableAction(PylontechEmulatorComponent *emulator) : emulator_(emulator) {}

      void play(Ts... x) override { this->emulator_->set_enabled(true); }

    protected:
      PylontechEmulatorComponent *emulator_;
  };

  template<typename... Ts> class DisableAction : public Action<Ts...> {
    public:
      explicit DisableAction(PylontechEmulatorComponent *emulator) : emulator_(emulator) {}

      void play(Ts... x) override { this->emulator_->set_enabled(false); }

    protected:
      PylontechEmulatorComponent *emulator_;
  };

}  // namespace pylontech
}  // namespace esphome