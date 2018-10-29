#pragma once
#include "./Relays/Relay.h"

namespace Codingfield {
  namespace Brew {
    namespace Actuators {
      class PwmRelay {
      public:
        PwmRelay(Relays::Relay* relay, Relays::States idleState, Relays::States activateState);
        virtual ~PwmRelay() = default;

        void Activate();
        void Deactivate();
        bool IsActivated() const;

        void Period(uint32_t p);
        uint32_t Period() const;

        void Consign(uint32_t p);
        uint32_t Consign() const;

        void MinimumActivatedTime(uint32_t m);
        uint32_t MinimumActivatedTime() const;

        void MinimumIdleTime(uint32_t m);
        uint32_t MinimumIdleTime() const;


        // Should be called periodically. The calling period will
        // determine the period of the PWM
        void Update();
        void Reset();

      private:

        Relays::Relay* actualRelay = nullptr;

        uint32_t period = 1000;
        uint32_t consign = 500;
        uint32_t newConsign = 500;
        uint32_t value = 0;

        uint32_t minActivatedTime = 0; // Once activated, keep it activated at least during this time
        uint32_t activatedTime = 0;
        uint32_t minIdleTime = 0; // Once idling, keep it idle at least during this time
        uint32_t idleTime = 0;
        Relays::States idleState = Relays::States::Open;
        Relays::States activateState = Relays::States::Closed;
        bool activated = false;
      };
    }
  }
}
