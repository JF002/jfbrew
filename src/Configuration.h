#pragma once

#include <stdint.h>
#include "Devices/Actuators/Relays/Factory.h"

namespace Codingfield {
  namespace Brew {
    class Configuration {
    public:
      const uint8_t OneWireTemperaturePin() const { return oneWireTemperaturePin; }
      const uint8_t HeaterRelayPin() const { return heaterRelayPin; }
      const uint8_t CoolerRelayPin() const { return coolerRelayPin; }
      const uint8_t FanRelayPin() const { return fanRelayPin; }
      const bool IsTemperatureSensorStubbed() const { return stubTemperatureSensors; }
      const Actuators::Relays::Factory::Types RelayType() const { return relayType; }

    private:
      uint8_t oneWireTemperaturePin = 2;
      uint8_t heaterRelayPin = 21;
      uint8_t coolerRelayPin = 22;
      uint8_t fanRelayPin = 5;

      bool stubTemperatureSensors = false;
      Actuators::Relays::Factory::Types relayType = Actuators::Relays::Factory::Types::Gpio;
    };
  }
}
