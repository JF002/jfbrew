#include "DS18B20.h"
#include <OneWire.h>
#include <DallasTemperature.h>

using namespace Codingfield::Brew::Sensors::Temperature;

DS18B20::DS18B20(DallasTemperature* sensors, uint8_t index, uint8_t* addr) : sensors{sensors}, index{index} {
  Serial.print("Address : ");
  for(int i = 0; i < 8; i++) {
    address[i] = addr[i];
    Serial.print(String(String(address[i]) + " "));

  }
  Serial.println("");
}

DS18B20::~DS18B20() {
}

float DS18B20::Value() const {
  return value;
}

void DS18B20::Update() {
  auto newValue = sensors->getTempC(address);
  if(newValue > -100.0 && newValue < 100.0)
    value = newValue;
}
