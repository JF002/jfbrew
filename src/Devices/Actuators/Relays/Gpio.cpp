#include "Gpio.h"
#include <Arduino.h>

using namespace Codingfield::Brew::Actuators::Relays;

Gpio::Gpio(int pin, States initialState) : Relay(initialState) {
  this->pin = pin;
  pinMode(pin, OUTPUT);
  digitalWrite(pin, StateToHighLow(state));
}

States Gpio::State() const {
  return state;
}

void Gpio::State(States s) {
  state = s;
  digitalWrite(pin, StateToHighLow(state));
}

int Gpio::StateToHighLow(States s) {
  switch(s) {
    case States::Closed:
      return HIGH;
    case States::Open:
    default:
      return LOW;
  }
}
