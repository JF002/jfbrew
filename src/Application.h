#pragma once
#include "Configuration.h"


class OneWire;
class DallasTemperature;

namespace Codingfield {
  namespace Brew {
    namespace Actuators {
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
        Application(const Configuration& configuration);
        void Init();
        void Update();

        float FridgeTemperature() const;
        float BeerTemperature() const;
        float RoomTemperature() const;

        void SetStubFridgeTemperature(float t);
        void SetStubBeerTemperature(float t);
        void SetStubRoomTemperature(float t);

        Actuators::Relays::States CoolerState() const;
        Actuators::Relays::States HeaterState() const;
        Actuators::Relays::States FanState() const;

        void CoolerState(Actuators::Relays::States state);
        void HeaterState(Actuators::Relays::States state);
        void FanState(Actuators::Relays::States state);


      private:

        void InitHW();

        Configuration configuration;

        Actuators::Relays::Relay* coolerRelay;
        Actuators::Relays::Relay* heaterRelay;
        Actuators::Relays::Relay* fanRelay;


        Sensors::Temperature::TemperatureSensor* fridgeTempSensor;
        Sensors::Temperature::TemperatureSensor* beerTempSensor;
        Sensors::Temperature::TemperatureSensor* roomTempSensor;
        Sensors::Temperature::DS18B20Bus* tempSensorBus;

        OneWire* oneWire;
        DallasTemperature* temperatureSensors;

    };
  }
}
