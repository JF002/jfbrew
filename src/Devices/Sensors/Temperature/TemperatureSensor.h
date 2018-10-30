#pragma once

#include <cstdint>

namespace Codingfield {
  namespace Brew {
    namespace Sensors {
      namespace Temperature {
        class TemperatureSensor {
        public:
          TemperatureSensor() = default;
          virtual ~TemperatureSensor() = default;

          virtual float Value() const = 0;
          virtual void Update() = 0;
          virtual int32_t ErrorCounter() = 0;
          virtual int32_t RetryCounter() = 0;
          virtual bool IsReady() = 0;
          virtual void ResetCounters() = 0;
          virtual void Reset() = 0;
        };
      }
    }
  }
}
