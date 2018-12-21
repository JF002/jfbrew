#include "PwmRelay.h"
#include <iostream>
#include <sstream>
#include <Arduino.h>
using namespace Codingfield::Brew::Actuators;

PwmRelay::PwmRelay(Relays::Relay* relay, Relays::States idleState, Relays::States activateState) : actualRelay{relay},
                                                                                         idleState{idleState},
                                                                                         activateState{activateState} {

}

void PwmRelay::Update() {
  if(!activated) return;

  Relays::States targetState;
  Relays::States currentState = actualRelay->State();

  if(value < period) {
    if(currentState == activateState || value == 0) {
      consign = newConsign;
    }
    value++;
  }
  else {
    value = 0;
    consign = newConsign;
  }



  if(value < consign)
    targetState = activateState;
  else {
    targetState = idleState;
  }


  if(currentState == activateState) {
    activatedTime++;
    totalActivatedTime++;
    if(targetState == idleState &&  activatedTime >= minActivatedTime) {
      actualRelay->State(idleState);
      activatedTime = 0;
    }
  }

  if(currentState == idleState) {
    idleTime++;
    if(targetState == activateState && idleTime >= minIdleTime) {
      actualRelay->State(activateState);
      idleTime = 0;
    }
  }
}

void PwmRelay::Reset() {
  value = 0;
  activatedTime = 0;
  actualRelay->State(idleState);
  idleTime = minActivatedTime+1;
}

void PwmRelay::Activate() {
  activated = true;
  Reset();
  Update();
}

void PwmRelay::Deactivate() {
  activated = false;
  actualRelay->State(idleState);
}

void PwmRelay::Period(uint32_t p) {
  period = p;
}

uint32_t PwmRelay::Period() const {
  return period;
}

void PwmRelay::Consign(uint32_t c) {
  newConsign = c;
}

uint32_t PwmRelay::Consign() const {
  return consign;
}

void PwmRelay::MinimumActivatedTime(uint32_t m) {
  minActivatedTime = m;
}

uint32_t PwmRelay::MinimumActivatedTime() const {
  return minActivatedTime;
}

void PwmRelay::MinimumIdleTime(uint32_t m) {
  minIdleTime = m;
}

uint32_t PwmRelay::MinimumIdleTime() const {
  return minIdleTime;
}

bool PwmRelay::IsActivated() const {
  return activated;
}

uint32_t PwmRelay::TotalActivatedTime() const {
  return totalActivatedTime;
}
