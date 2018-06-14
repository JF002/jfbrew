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
        };
      }
    }
  }
}
