#pragma once

#include <string>

namespace Codingfield {
  namespace Brew {
    namespace Actuators {
      namespace Relays {
        enum class States { Unknown, Open, Closed };

        class Relay {
        public:
          Relay(States initialState);
          virtual ~Relay() = default;

          virtual States State() const = 0;
          virtual void State(States s) = 0;

        protected:
          States state;
        };

        const std::string ToString(const States s);

      }
    }
  }
}
