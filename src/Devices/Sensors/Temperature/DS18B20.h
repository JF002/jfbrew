#pragma once

#include "TemperatureSensor.h"
class DallasTemperature;

namespace Codingfield {
  namespace Brew {
    namespace Sensors {
      namespace Temperature {
        class DS18B20 : public TemperatureSensor {
        public:
          DS18B20(DallasTemperature* sensors, uint8_t index, uint8_t* addr);
          virtual ~DS18B20();

          float Value() const override;
          void Update() override;
        private:
          DallasTemperature* sensors = nullptr;
          uint8_t index = 0;
          uint8_t address[8];
          float value;
        };
      }
    }
  }
}
