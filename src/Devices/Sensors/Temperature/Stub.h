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

            int32_t ErrorCounter() override;

            int32_t RetryCounter() override;

            bool IsReady() override;

            void Reset() override;

            void ResetCounters() override;

        private:
          float value = 0.0f;
        };
      }
    }
  }
}
