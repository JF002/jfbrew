#include "Stub.h"
#include <Arduino.h>

using namespace Codingfield::Brew::Sensors::Temperature;

Stub::Stub() {

}

Stub::~Stub() {

}

float Stub::Value() const {
  return value;
}

void Stub::Update() {
  //value = 20.0f + (random(-200,200) / 100.0f);
}

void Stub::SetValue(float v) {
  value = v;
}

int32_t Stub::ErrorCounter() {
    return 0;
}

int32_t Stub::RetryCounter() {
    return 0;
}

bool Stub::IsReady() {
    return true;
}

void Stub::Reset() {

}

void Stub::ResetCounters() {

}
