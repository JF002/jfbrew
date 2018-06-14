#pragma once

#include "TemperatureSensor.h"
#include "Stub.h"
#include "DS18B20.h"
#include "DS18B20Bus.h"

namespace Codingfield {
  namespace Brew {
    namespace Sensors {
      namespace Temperature {
        class Factory {
        public:
          enum class Types {Stub, DS18B20};

          static TemperatureSensor* CreateSensor(Types type, uint8_t index, uint8_t* addr) {
            switch(type) {
              case Types::DS18B20: return new Sensors::Temperature::DS18B20(temperatureSensors, 0, addr);
              case Types::Stub:
              default:
            }
          }
        };
      }
    }
  }
}
