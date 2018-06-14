#pragma once

#include "Relay.h"

namespace Codingfield {
  namespace Brew {
    namespace Actuators {
      namespace Relays {
        class Stub : public Relay {
        public:
          Stub(States initialState = States::Open);
          virtual ~Stub() = default;

          virtual States State() const override;
          virtual void State(States s) override;

        };
      }
    }
  }
}
