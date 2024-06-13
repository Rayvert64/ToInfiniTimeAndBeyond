#pragma once

#include <displayapp/Controllers.h>
#include <lvgl/src/widgets/roller/lv_roller.h>
#include <memory>
#include "Apps.h"
#include "displayapp/screens/Screen.h"
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
      void CreateTimeCounters();
      void CreatePlayPauseBtnStyles();
      void AddPlayPauseBtnEvents();

      Pinetime::Controllers::Timer& timer;

      enum btnStates { BTN_STATE_IDLE, BTN_STATE_SHORT_PRESS, BTN_STATE_HELD, NUMBER_BTN_STATES };

      lv_obj_t* btnPlayPause;
      lv_obj_t* txtPlayPause;
      lv_style_t styles_btn[NUMBER_BTN_STATES];

      lv_style_prop_t btnTransitionElements[2] = {
        LV_STYLE_BG_OPA,
        (lv_style_prop_t) 0, /*End marker*/
      };
      lv_style_transition_dsc_t* btnTransitionDescription;

      lv_timer_t* taskRefresh;
      lv_obj_t* minuteCounter;
      lv_obj_t* secondCounter;

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
