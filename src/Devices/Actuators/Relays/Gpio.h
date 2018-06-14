#pragma once


#include "Relay.h"

namespace Codingfield {
  namespace Brew {
    namespace Actuators {
      namespace Relays {
        class Gpio : public Relay {
        public:
          Gpio(int pin, States intialState = States::Open);
          virtual ~Gpio() = default;

          virtual States State() const override;
          virtual void State(States s) override;

        private:
          int pin = 22;
          int StateToHighLow(States s);
        };
      }
    }
  }
}
