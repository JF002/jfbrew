#pragma once

#include "Stub.h"
#include "Gpio.h"

namespace Codingfield {
  namespace Brew {
    namespace Actuators {
      namespace Relays {
        class Factory {
        public:
          enum class Types {Stub, Gpio};
          static Relay* Create(Types type, uint8_t pin, States initialState = States::Open) {
            switch(type) {
              case Types::Gpio : return new Gpio(pin, initialState);
              case Types::Stub:
              default:
                return new Stub();
            }
          }
        };
      }
    }
  }
}
