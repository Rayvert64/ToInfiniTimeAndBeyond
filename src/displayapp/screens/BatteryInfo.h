#pragma once

#include <cstdint>
#include "displayapp/screens/Screen.h"
#include <lvgl/lvgl.h>
#include <lvgl/src/misc/lv_types.h>

namespace Pinetime {
  namespace Controllers {
    class Battery;
  }

  namespace Applications {
    namespace Screens {

      class BatteryInfo : public Screen {
      public:
        BatteryInfo(const Pinetime::Controllers::Battery& batteryController);
        ~BatteryInfo() override;

        void Refresh() override;

      private:
        const Pinetime::Controllers::Battery& batteryController;

        lv_obj_t* voltage;
        lv_obj_t* percent;

        lv_anim_t* chargingAnim;
        lv_obj_t* chargingBar;
        lv_obj_t* status;

        lv_timer_t* taskRefresh;

        uint8_t batteryPercent = 0;
        uint16_t batteryVoltage = 0;
      };
    }
  }
}
