#pragma once

#include "TemperatureSensor.h"

namespace Codingfield {
  namespace Brew {
    namespace Sensors {
      namespace Temperature {
        class Stub : public TemperatureSensor {
        public:
          Stub();
          virtual ~Stub();

          float Value() const override;
          void Update() override;
          void SetValue(float v);
        private:
          float value = 0.0f;
        };
      }
    }
  }
}
