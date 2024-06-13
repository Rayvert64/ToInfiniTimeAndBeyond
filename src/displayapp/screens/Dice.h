#pragma once

#include <memory>
#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "displayapp/Colors.h"
#include "Symbols.h"

#include <array>
#include <random>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Dice : public Screen {
      public:
        Dice(Controllers::MotionController& motionController,
             Controllers::MotorController& motorController,
             Controllers::Settings& settingsController);
        ~Dice() override;
        void Roll();
        void Refresh() override;

      private:
        lv_obj_t* btnRoll;
        lv_obj_t* btnRollLabel;
        lv_obj_t* resultTotalLabel;
        lv_obj_t* resultIndividualLabel;
        lv_timer_t* refreshTask;
        bool enableShakeForDice = false;

        std::mt19937 gen;

        std::array<lv_color_t, 3> resultColors = {PINETIME_COLOR_YELLOW, PINETIME_COLOR_MAGENTA, PINETIME_COLOR_CYAN};
        uint8_t currentColorIndex;
        void NextColor();

        lv_obj_t* nCounter;
        lv_obj_t* dCounter;

        bool openingRoll = true;
        uint8_t currentRollHysteresis = 0;
        static constexpr uint8_t rollHysteresis = 10;

        Controllers::MotorController& motorController;
        Controllers::MotionController& motionController;
        Controllers::Settings& settingsController;
      };
    }

    template <>
    struct AppTraits<Apps::Dice> {
      static constexpr Apps app = Apps::Dice;
      static constexpr const char* icon = Screens::Symbols::dice;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::Dice(controllers.motionController, controllers.motorController, controllers.settingsController);
      };
    };
  }
}
