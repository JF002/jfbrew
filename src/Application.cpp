#include <sstream>
#include "Application.h"
#include "Devices/Actuators/Relays/Factory.h"

#include "Devices/Sensors/Temperature/DS18B20.h"
#include "Devices/Sensors/Temperature/Stub.h"
#include "Devices/Sensors/Temperature/DS18B20Bus.h"

#include <OneWire.h>
#include <DallasTemperature.h> // WARNING include this one only once in headers!

#include "Devices/Actuators/PwmRelay.h"
#include "PID/MiniPID.h"

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

  if((cpt%100)==0) {
    std::stringstream ss;



    auto out = heaterPid->getOutput(beerTempSensor->Value(), beerSetPoint);
      //ss << "PID : Consign = " << beerSetPoint << " - Temp : " << fridgeTempSensor->Value() << " - Out = " << out;
    //Serial.println(ss.str().c_str());
    heaterPwmRelay->Consign(out);
  }

  heaterPwmRelay->Update();
  coolerPwmRelay->Update();
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
  cpt++;
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
    temperatureSensors->setWaitForConversion(false);
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

  coolerPwmRelay = new Actuators::PwmRelay(coolerRelay, Relays::States::Open, Relays::States::Closed);
  coolerPwmRelay->Period(1200 * 100);
  coolerPwmRelay->MinimumActivatedTime(120 * 100);
  coolerPwmRelay->MinimumIdleTime(180 * 100);
  coolerPwmRelay->Consign(600 * 100);
  //pwmRelay->Activate();

  heaterPwmRelay = new Actuators::PwmRelay(heaterRelay, Relays::States::Open, Relays::States::Closed);
  heaterPwmRelay->Period(4 * 100);
  heaterPwmRelay->MinimumActivatedTime(0 * 100);
  heaterPwmRelay->MinimumIdleTime(0 * 100);
  heaterPwmRelay->Consign(2 * 100);

  //heaterPid = new MiniPID(30.0, 1800, 60);
  heaterPid = new MiniPID(20.0, 0.2, 20.0);
  heaterPid->setOutputLimits(0.0, heaterPwmRelay->Period());

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


Actuators::Relays::States Application::CoolerRelayState() const {
  return coolerRelay->State();
}

Actuators::Relays::States Application::HeaterRelayState() const {
  return heaterRelay->State();
}

Actuators::Relays::States Application::FanRelayState() const {
  return fanRelay->State();
}

void Application::CoolerRelayState(Actuators::Relays::States state) {
  coolerRelay->State(state);
}

void Application::HeaterRelayState(Actuators::Relays::States state) {
  heaterRelay->State(state);
}

void Application::FanRelayState(Actuators::Relays::States state){
  fanRelay->State(state);
}

void Application::CoolerPwm(uint32_t p) {
  coolerPwmRelay->Consign(p);
}

void Application::HeaterPwm(uint32_t p) {
  heaterPwmRelay->Consign(p);
}

uint32_t Application::CoolerPwm() const {
  return coolerPwmRelay->Consign();
}

uint32_t Application::HeaterPwm() const {
  return heaterPwmRelay->Consign();
}

void Application::ActivateCoolerPwm(bool activate) {
  if(activate)
    coolerPwmRelay->Activate();
  else
    coolerPwmRelay->Deactivate();
}

void Application::ActivateHeaterPwm(bool activate) {
  if(activate)
    heaterPwmRelay->Activate();
  else
    heaterPwmRelay->Deactivate();
}

bool Application::IsHeaterPwmActivated() const {
  return heaterPwmRelay->IsActivated();
}

bool Application::IsCoolerPwmActivated() const {
  return coolerPwmRelay->IsActivated();
}

void Application::BeerSetPoint(float s) {
  this->beerSetPoint = s;
}

void Application::HeaterKp(float kp) {
  heaterPid->setP(kp);
}

void Application::HeaterKi(float ki) {
  heaterPid->setI(ki);
}

void Application::HeaterKd(float kd) {
  heaterPid->setD(kd);
}

void Application::ResetPid() {
  heaterPid->reset();
}
