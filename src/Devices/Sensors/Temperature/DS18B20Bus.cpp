#include "DS18B20Bus.h"
#include "DS18B20.h"
#include <Arduino.h>
#include <DallasTemperature.h>

using namespace Codingfield::Brew::Sensors::Temperature;

DS18B20Bus::DS18B20Bus(DallasTemperature* bus) : bus{bus} {

}

DS18B20Bus::~DS18B20Bus() {

}

void DS18B20Bus::Add(DS18B20* sensor) {
  sensors.push_back(sensor);
}

void DS18B20Bus::Update() {
  if(prevTime == -1) {
    prevTime = millis();
    return;
  }

  int currentTime = millis();
  int delay = currentTime - prevTime;
  switch(state) {
    case States::Idle:
      idlingTime += delay;
      if(idlingTime >= timeBetweenConversion) {
        // Request conversion!
        bus->requestTemperatures();
        Serial.println("DS18B20Bus : requesting Conversion!");
        idlingTime = 0;
        state = States::ConversionPending;
      }
      break;
    case States::ConversionPending:
      waitForConversionTime += delay;
      if(waitForConversionTime >= timeNeededForConversion) {
        // Temperature available
        for(auto* sensor : sensors) {
          sensor->Update();
        }

        Serial.println("DS18B20Bus : Temperature available!");
        waitForConversionTime = 0;
        state = States::Idle;
      }
      break;
  }

  prevTime = currentTime;
}
