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
