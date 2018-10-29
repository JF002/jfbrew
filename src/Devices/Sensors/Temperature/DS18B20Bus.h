#pragma once
#include <vector>


class DallasTemperature;

namespace Codingfield {
  namespace Brew {
    namespace Sensors {
      namespace Temperature {
        class DS18B20;

        class DS18B20Bus {
        public:
          enum class States {Idle, ConversionPending};
          DS18B20Bus(DallasTemperature* bus);
          virtual ~DS18B20Bus();

          void Add(DS18B20* sensor);

          void Update();
        private:
          States state = States::Idle;
          int prevTime = -1;
          int idlingTime = 0;
          int waitForConversionTime = 0;

          int timeBetweenConversion = 1000;
          int timeNeededForConversion = 200;

          DallasTemperature* bus = nullptr;
          std::vector<DS18B20*> sensors;
        };
      }
    }
  }
}
