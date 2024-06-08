#pragma once

#include <memory>
#include "displayapp/screens/Screen.h"
#include "displayapp/Colors.h"
#include "components/datetime/DateTimeController.h"
#include "systemtask/SystemTask.h"
#include "displayapp/LittleVgl.h"
#include "displayapp/widgets/Counter.h"
#include <lvgl/lvgl.h>

#include "components/timer/Timer.h"
#include "Symbols.h"

namespace Pinetime::Applications {
  namespace Screens {
    class Timer : public Screen {
    public:
      Timer(Controllers::Timer& timerController);
      ~Timer() override;
      void Refresh() override;
      void Reset();
      void ToggleRunning();
      void ButtonPressed();
      void MaskReset();

    private:
      void SetTimerRunning();
      void SetTimerStopped();
      void CreatePlayPauseBtnStyles();
      void AddPlayPauseBtnEvents();

      Pinetime::Controllers::Timer& timer;

      enum btnStates { BTN_STATE_ANY, BTN_STATE_IDLE, BTN_STATE_SHORT_PRESS, BTN_STATE_HELD, NUMBER_BTN_STATES };

      std::shared_ptr<lv_obj_t> btnPlayPause;
      std::shared_ptr<lv_obj_t> txtPlayPause;
      lv_style_t styles_btn[NUMBER_BTN_STATES];

      const lv_style_prop_t btnTransitionElements[2] = {
        LV_STYLE_BG_OPA,
        (lv_style_prop_t) 0, /*End marker*/
      };
      std::shared_ptr<lv_style_transition_dsc_t> btnTransitionDescription;

      lv_timer_t* taskRefresh;
      Widgets::Counter minuteCounter = Widgets::Counter(0, 59, jetbrains_mono_76);
      Widgets::Counter secondCounter = Widgets::Counter(0, 59, jetbrains_mono_76);

      bool buttonPressing = false;
      lv_coord_t maskPosition = 0;
      TickType_t pressTime = 0;
    };
  }

  template <>
  struct AppTraits<Apps::Timer> {
    static constexpr Apps app = Apps::Timer;
    static constexpr const char* icon = Screens::Symbols::hourGlass;

    static Screens::Screen* Create(AppControllers& controllers) {
      return new Screens::Timer(controllers.timer);
    };
  };
}
