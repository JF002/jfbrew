#pragma once

#include "Button.h"

namespace Codingfield {
  namespace UI {
    // TODO template / abstract this class
    // OR ne pas utiliser ceci et spécialiser les boutons (TemperatureButton, PercentButton,...) (l'intelligence next/previous/tostring est dans la spécialisation du bouton)

    class ValueProvider {
    public:
      int GetNextValue(int32_t value) const { return ++value; }
      int GetPreviousValue(int32_t value) const { return --value; }
      int GetValue() const { return value; }

      void SetValue(int32_t value) { this->value = value; }

      const std::string GetValueToString(int32_t value) const { return String(value).c_str(); }
    private:
      int32_t value;
    };
  }
}
