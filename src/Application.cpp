#include "Application.h"
#include "Devices/Actuators/Relays/Factory.h"

#include "Devices/Sensors/Temperature/DS18B20.h"
#include "Devices/Sensors/Temperature/Stub.h"
#include "Devices/Sensors/Temperature/DS18B20Bus.h"

#include <OneWire.h>
#include <DallasTemperature.h> // WARNING include this one only once in headers!

using namespace Codingfield::Brew;

Application::Application(const Configuration& configuration) : configuration{configuration} {

}

void Application::Init() {
  InitHW();
}

void Application::Update() {

  if(!configuration.IsTemperatureSensorStubbed()) {
    tempSensorBus->Update();
  }
/*
  if(beerTempSensor->Value() < 22.0f) {
    heaterRelay->State(Actuators::Relays::States::Closed);
    coolerRelay->State(Actuators::Relays::States::Open);
    fanRelay->State(Actuators::Relays::States::Closed);
  } else if(beerTempSensor->Value() > 23.0f) {
    heaterRelay->State(Actuators::Relays::States::Open);
    coolerRelay->State(Actuators::Relays::States::Closed);
    fanRelay->State(Actuators::Relays::States::Closed);
  }
  else {
    heaterRelay->State(Actuators::Relays::States::Open);
    coolerRelay->State(Actuators::Relays::States::Open);
    fanRelay->State(Actuators::Relays::States::Open);
  }
  */

}

void Application::InitHW() {
  using namespace Codingfield::Brew::Actuators;
  coolerRelay = Relays::Factory::Create(configuration.RelayType(), configuration.CoolerRelayPin());
  heaterRelay = Relays::Factory::Create(configuration.RelayType(), configuration.HeaterRelayPin());
  fanRelay = Relays::Factory::Create(configuration.RelayType(), configuration.FanRelayPin());

  // TODO Stub via factory
  using namespace Codingfield::Brew::Sensors;
  if(configuration.IsTemperatureSensorStubbed()) {
    oneWire = nullptr;
    temperatureSensors = nullptr;
    tempSensorBus = nullptr;
    fridgeTempSensor = new Sensors::Temperature::Stub();
    beerTempSensor = new Sensors::Temperature::Stub();
    roomTempSensor = new Sensors::Temperature::Stub();
  } else {
    oneWire = new OneWire(configuration.OneWireTemperaturePin());
    temperatureSensors = new DallasTemperature(oneWire);
    temperatureSensors->begin();
    tempSensorBus = new Temperature::DS18B20Bus(temperatureSensors);

    uint8_t addr[8];
    auto tempSensorAddr = temperatureSensors->getAddress(addr, 0);
    fridgeTempSensor = new Sensors::Temperature::DS18B20(temperatureSensors, 0, addr);
    tempSensorBus->Add(static_cast<Sensors::Temperature::DS18B20*>(fridgeTempSensor));

    tempSensorAddr = temperatureSensors->getAddress(addr, 1);
    beerTempSensor = new Sensors::Temperature::DS18B20(temperatureSensors,1, addr);
    tempSensorBus->Add(static_cast<Sensors::Temperature::DS18B20*>(beerTempSensor));

    tempSensorAddr = temperatureSensors->getAddress(addr, 2);
    roomTempSensor = new Sensors::Temperature::DS18B20(temperatureSensors, 2, addr);
    tempSensorBus->Add(static_cast<Sensors::Temperature::DS18B20*>(roomTempSensor));
  }
}

float Application::FridgeTemperature() const {
  return fridgeTempSensor->Value();
}

float Application::BeerTemperature() const {
  return beerTempSensor->Value();
}

float Application::RoomTemperature() const {
  return roomTempSensor->Value();
}

void Application::SetStubFridgeTemperature(float t) {
  if(configuration.IsTemperatureSensorStubbed()) {
    static_cast<Sensors::Temperature::Stub*>(fridgeTempSensor)->SetValue(t);
  }
}

void Application::SetStubBeerTemperature(float t) {
  if(configuration.IsTemperatureSensorStubbed()) {
    static_cast<Sensors::Temperature::Stub*>(beerTempSensor)->SetValue(t);
  }
}

void Application::SetStubRoomTemperature(float t) {
  if(configuration.IsTemperatureSensorStubbed()) {
    static_cast<Sensors::Temperature::Stub*>(roomTempSensor)->SetValue(t);
  }
}


Actuators::Relays::States Application::CoolerState() const {
  return coolerRelay->State();
}

Actuators::Relays::States Application::HeaterState() const {
  return heaterRelay->State();
}

Actuators::Relays::States Application::FanState() const {
  return fanRelay->State();
}

void Application::CoolerState(Actuators::Relays::States state) {
  coolerRelay->State(state);
}

void Application::HeaterState(Actuators::Relays::States state) {
  heaterRelay->State(state);
}

void Application::FanState(Actuators::Relays::States state){
  fanRelay->State(state);
}
