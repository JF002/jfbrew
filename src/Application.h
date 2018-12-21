#pragma once
#include "Configuration.h"


class OneWire;
class DallasTemperature;
class MiniPID;

namespace Codingfield {
  namespace Brew {
    namespace Actuators {
      class PwmRelay;
      namespace Relays {
        class Relay;
      }
    }
    namespace Sensors {
      namespace Temperature {
        class DS18B20;
        class DS18B20Bus;
        class TemperatureSensor;
      }
    }

    class Application {
      public:
        enum class States {Idle, Heating, Cooling, Pending, Error
        };
        struct PidValues {
            double P;
            double I;
            double D;
        };

        Application(const Configuration& configuration);
        void Init();
        void Update();

        States RegulationState() const { return state; }

        void EnableRegulation();
        void DisableRegulation();
        bool IsRegulationEnabled() const;

        float FridgeTemperature() const;
        float BeerTemperature() const;
        float RoomTemperature() const;

        void SetStubFridgeTemperature(float t);
        void SetStubBeerTemperature(float t);
        void SetStubRoomTemperature(float t);

        Actuators::Relays::States CoolerRelayState() const;
        Actuators::Relays::States HeaterRelayState() const;
        Actuators::Relays::States FanRelayState() const;

        void CoolerRelayState(Actuators::Relays::States state);
        void HeaterRelayState(Actuators::Relays::States state);
        void FanRelayState(Actuators::Relays::States state);

        void CoolerPwm(uint32_t p);
        void HeaterPwm(uint32_t p);

        uint32_t CoolerPwm() const;
        uint32_t HeaterPwm() const;

        void ActivateCoolerPwm(bool activate);
        void ActivateHeaterPwm(bool activate);

        bool IsCoolerPwmActivated() const;
        bool IsHeaterPwmActivated() const;

        void BeerSetPoint(float s);
        float BeerSetPoint() const;

        void HeaterKp(float kp);

        void HeaterKi(float ki);

        void HeaterKd(float kd);

        double HeaterPidOutput() const;
        PidValues HeaterPidValues() const;

        void CoolerKp(float kp);

        void CoolerKi(float ki);

        void CoolerKd(float kd);

        double CoolerPidOutput() const;
        PidValues CoolerPidValues() const;

        double BeerToFridgePidOutput() const;
        PidValues BeerToFridgePidValues() const;

        std::string RegulationStateToString(const Application::States s) const;
        void ResetPid();

        uint32_t TotalHeaterPoints() const;
        uint32_t TotalCoolerPoints() const;

    private:

        void InitHW();

        Configuration configuration;

        Actuators::Relays::Relay* coolerRelay;
        Actuators::Relays::Relay* heaterRelay;
        Actuators::Relays::Relay* fanRelay;

        Actuators::PwmRelay* coolerPwmRelay;
        Actuators::PwmRelay* heaterPwmRelay;

        Sensors::Temperature::TemperatureSensor* fridgeTempSensor;
        Sensors::Temperature::TemperatureSensor* beerTempSensor;
        Sensors::Temperature::TemperatureSensor* roomTempSensor;
        Sensors::Temperature::DS18B20Bus* tempSensorBus;

        OneWire* oneWire;
        DallasTemperature* temperatureSensors;
        MiniPID* heaterPid;
        MiniPID* coolerPid;
        MiniPID* beerToFridgePid;

        uint32_t cpt = 0;
        float beerSetPoint = 20.0f;
        States state = States::Idle;

        uint32_t minIdleTime = 1200; // 600000 = 10 minutes, seconds
        uint32_t idleTime = 0; // seconds

        uint32_t minCoolingTime = 600; // 600000 = 10 minutes, seconds
        uint32_t coolingTime = 0; // seconds

        uint32_t minHeatingTime = 600; // 600000 = 10 minutes, seconds
        uint32_t heatingTime = 0; // seconds

        float temperatureHystereis = 0.2f; // degrees

        double heaterPidOutput = 0.0;
        double coolerPidOutput = 0.0;
        double beerToFridgePidOutput = 0.0;

        bool regulationEnabled = false;

        bool TemperatureSensorsSanityCheck();
        bool AreTemperatureSensorsReady();
        void ProcessRegulation();
    };


  }
}
