#include "Stub.h"

using namespace Codingfield::Brew::Actuators::Relays;

Stub::Stub(States initialState) : Relay(initialState) {

}

States Stub::State() const {
  return state;
}

void Stub::State(States s) {
  state = s;
}
