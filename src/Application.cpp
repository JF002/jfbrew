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

#include <cmath>
void Application::Update() {

  if(!configuration.IsTemperatureSensorStubbed()) {
    tempSensorBus->Update();
  }

  if(regulationEnabled) {

    if ((cpt % 100) == 0) {
      if ((state == States::Idle && idleTime >= minIdleTime) ||
          (state == States::Cooling && coolingTime >= minCoolingTime) ||
          (state == States::Heating && heatingTime >= minHeatingTime)) {
        if (fridgeTempSensor->Value() < beerSetPoint - temperatureHystereis) {
          if (state != States::Heating) {
            heaterPwmRelay->Reset();
            coolerPwmRelay->Reset();
            /*
            coolerPid->reset();
            heaterPid->reset();
            heaterPidOutput = 0;
            coolerPidOutput = 0;
             */
          }
          state = States::Heating;
        } else if (fridgeTempSensor->Value() > beerSetPoint + temperatureHystereis) {
          if (state != States::Cooling) {
            heaterPwmRelay->Reset();
            coolerPwmRelay->Reset();
            /*
            coolerPid->reset();
            heaterPid->reset();
            heaterPidOutput = 0;
            coolerPidOutput = 0;
             */
          }
          state = States::Cooling;
        } else {
          state = States::Idle;
        }
      }

      double heaterConsign = 0.0f;
      double coolerConsign = 0.0f;

      heaterConsign = heaterPid->getOutput(fridgeTempSensor->Value(), beerSetPoint);
      heaterPidOutput = heaterConsign;

      coolerConsign = coolerPid->getOutput(fridgeTempSensor->Value(), beerSetPoint);
      coolerPidOutput = coolerConsign;

      switch (state) {
        case States::Heating:
          if (heaterConsign < 0.0)
            heaterConsign = 0;
              heaterPwmRelay->Consign(heaterConsign);
              coolerPwmRelay->Consign(0);
              heatingTime++;
              coolingTime = 0;
              idleTime = 0;
              break;
        case States::Cooling:
          if (coolerConsign > 0.0)
            coolerConsign = 0;
          else
            coolerConsign = -1 * coolerConsign;

              heaterPwmRelay->Consign(0);
              coolerPwmRelay->Consign(coolerConsign);
              heatingTime = 0;
              coolingTime++;
              idleTime = 0;
              break;
        case States::Idle:
          heaterPwmRelay->Consign(0);
              coolerPwmRelay->Consign(0);
              heatingTime = 0;
              coolingTime = 0;
              idleTime++;
              break;
        default:
          break;
      }

      std::stringstream ss;
      ss << "State: ";
      switch (state) {
        case States::Idle:
          ss << "Idle";
              break;
        case States::Cooling:
          ss << "Cooling";
              break;
        case States::Heating:
          ss << "Heating";
              break;
        default:
          ss << "Unknown";
              break;
      }
      ss << " - IdleTime: " << idleTime << " - HeatingTime: " << heatingTime << " - CoolingTime: " << coolingTime;
      ss << " - Fridge temp: " << fridgeTempSensor->Value();
      ss << " - Consign:  " << beerSetPoint;
      ss << " - HeaterPID: " << heaterConsign;
      ss << " - CoolerPID: " << coolerConsign;
      Serial.println(ss.str().c_str());
    }
  }

  heaterPwmRelay->Update();
  coolerPwmRelay->Update();
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
  coolerPwmRelay->Consign(0);
  //pwmRelay->Activate();

  heaterPwmRelay = new Actuators::PwmRelay(heaterRelay, Relays::States::Open, Relays::States::Closed);
  heaterPwmRelay->Period(4 * 100);
  heaterPwmRelay->MinimumActivatedTime(0 * 100);
  heaterPwmRelay->MinimumIdleTime(0 * 100);
  heaterPwmRelay->Consign(0);

  //heaterPid = new MiniPID(30.0, 1800, 60);
  heaterPid = new MiniPID(20.0, 0.3, 40.0);
  heaterPid->setOutputLimits(0.0, heaterPwmRelay->Period());

  coolerPid = new MiniPID(3600.0, 7.0, 2500.0);
  coolerPid->setOutputLimits(-coolerPwmRelay->Period(), 0.0);
  coolerPid->setDirection(true);

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
  Serial.println("New beer setpoint : " + String(s));
  if(regulationEnabled) {
    this->coolingTime = 0;
    this->heatingTime = 0;
    this->idleTime = 0;

    if (fridgeTempSensor->Value() < (beerSetPoint - temperatureHystereis)) {
      state = States::Heating;
    } else if (fridgeTempSensor->Value() > (beerSetPoint + temperatureHystereis)) {
      state = States::Cooling;
    } else {
      state = States::Idle;
    }
  }

  this->beerSetPoint = s;
}

float Application::BeerSetPoint() const {
  return this->beerSetPoint;
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

void Application::CoolerKp(float kp) {
  coolerPid->setP(kp);
}

void Application::CoolerKi(float ki) {
  coolerPid->setI(ki);
}

void Application::CoolerKd(float kd) {
  coolerPid->setD(kd);
}

void Application::ResetPid() {
  heaterPid->reset();
  coolerPid->reset();
}

double Application::HeaterPidOutput() const {
  return heaterPidOutput;
}

double Application::CoolerPidOutput() const{
  return coolerPidOutput;
}

Application::PidValues Application::HeaterPidValues() const {
  return PidValues {heaterPid->GetP(), heaterPid->GetI(), heaterPid->GetD()};
}

Application::PidValues Application::CoolerPidValues() const {
  return PidValues {coolerPid->GetP(), coolerPid->GetI(), coolerPid->GetD()};
}

std::string Application::RegulationStateToString(const Application::States s) const {
  switch (s) {
    case Application::States::Idle:
      return "Idle";
          break;
    case Application::States::Cooling:
      return "Cooling";
          break;
    case Application::States::Heating:
      return "Heating";
          break;
    default:
      return "Unknown";
  }
}

bool Application::IsRegulationEnabled() const {
  return regulationEnabled;
}

void Application::EnableRegulation()  {
  Serial.println("Enabling Regulation...");
  this->ResetPid();
  this->ActivateCoolerPwm(true);
  this->ActivateHeaterPwm(true);
  regulationEnabled = true;
  this->BeerSetPoint(beerSetPoint);
}

void Application::DisableRegulation() {
  Serial.println("Disabling Regulation...");
  this->ActivateCoolerPwm(false);
  this->ActivateHeaterPwm(false);
  state = States::Idle;
  regulationEnabled = false;
}
