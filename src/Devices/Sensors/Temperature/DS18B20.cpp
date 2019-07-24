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
  for(int i = 0; i < 2; i++) {
    float newValue = sensors->getTempC(address);
    if((newValue > -100.0f) && (newValue < 100.0f)) {
      value = newValue;
      nbMeasures++;
      if(nbMeasures>5) isReady = true;
      return;
    }
    else {
      nbRetry++;
      delay(1);
    }
  }
  nbError++;
}

int32_t DS18B20::ErrorCounter() {
  return nbError;
}

int32_t DS18B20::RetryCounter() {
  return nbRetry;
}

bool DS18B20::IsReady() {
  return isReady;
}

void DS18B20::Reset() {
  ResetCounters();
  value = 0;
  nbMeasures = 0;
  isReady = false;
}

void DS18B20::ResetCounters() {
  nbError = 0;
  nbRetry = 0;
}
