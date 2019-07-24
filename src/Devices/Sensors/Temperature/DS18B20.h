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

            void Reset() override;

            virtual ~DS18B20();

          float Value() const override;
          void Update() override;

            int32_t ErrorCounter() override;

            int32_t RetryCounter() override;

            bool IsReady() override;

            void ResetCounters() override;

        private:
          DallasTemperature* sensors = nullptr;
          uint8_t index = 0;
          uint8_t address[8];
          float value = 0.0;

          int32_t nbError = 0;
          int32_t nbRetry = 0;
          int32_t nbMeasures = 0;
          bool isReady = false;
        };
      }
    }
  }
}
