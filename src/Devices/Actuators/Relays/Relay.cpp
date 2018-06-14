#include "Relay.h"

using namespace Codingfield::Brew::Actuators::Relays;

Relay::Relay(States initialState = States::Open) : state{initialState} {

}

const std::string Codingfield::Brew::Actuators::Relays::ToString(const States s) {
  switch(s) {
    case States::Open: return "OFF";
    case States::Closed: return "ON";
    case States::Unknown :
    default:
      return "Unknown";
  }
}
