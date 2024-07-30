#pragma once

#include <array>
#include <lvgl/src/lv_core/lv_obj.h>
#include <chrono>
#include <cstdint>
#include <lvgl/src/lv_misc/lv_area.h>
#include <lvgl/src/lv_misc/lv_color.h>
#include <memory>
#include <sys/_stdint.h>
#include "displayapp/screens/Screen.h"
#include "components/datetime/DateTimeController.h"
#include "components/ble/BleController.h"
#include "displayapp/widgets/StatusIcons.h"
#include "utility/DirtyValue.h"
#include "displayapp/apps/Apps.h"

namespace Pinetime {
  namespace Controllers {
    class Settings;
    class Battery;
    class Ble;
    class NotificationManager;
    class HeartRateController;
    class MotionController;
  }

  namespace Applications {
    namespace Screens {
      constexpr uint8_t MAX_HEARTS = 3;
      constexpr uint8_t NUM_POINTS_MOTION_METER = 50;

      class WatchFaceDigital : public Screen {
      public:
        WatchFaceDigital(Controllers::DateTime& dateTimeController,
                         const Controllers::Battery& batteryController,
                         const Controllers::Ble& bleController,
                         Controllers::NotificationManager& notificationManager,
                         Controllers::Settings& settingsController,
                         Controllers::HeartRateController& heartRateController,
                         Controllers::MotionController& motionController,
                         Controllers::SimpleWeatherService& weather);
        ~WatchFaceDigital() override;

        void Refresh() override;

      private:
        void UpdateTempGauge();
        void UpdateTempRoller();
        void UpdateMotionMeter();
        int32_t GetMotionLevel();
        void ApplySimpleMotionToLine(uint32_t motionDiff);
        void ApplyComplexMotionToLine(uint32_t motionDiff);
        void ApplyAgressiveMotionToLine(uint32_t motionDiff);
        void InitWeatherRollerObjects();
        void InitTemperatureMeter();
        void InitMotionMeter();

        bool sleepDisabled;

        uint8_t displayedHour = -1;
        uint8_t displayedMinute = -1;

        Utility::DirtyValue<uint8_t> batteryPercentRemaining {};
        Utility::DirtyValue<bool> powerPresent {};
        Utility::DirtyValue<bool> bleState {};
        Utility::DirtyValue<bool> bleRadioEnabled {};
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>> currentDateTime {};
        Utility::DirtyValue<uint32_t> stepCount {};
        Utility::DirtyValue<uint8_t> heartbeat {};
        Utility::DirtyValue<bool> heartbeatRunning {};
        Utility::DirtyValue<bool> notificationState {};
        using days = std::chrono::duration<int32_t, std::ratio<86400>>; // TODO: days is standard in c++20
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, days>> currentDate;
        Utility::DirtyValue<std::optional<Pinetime::Controllers::SimpleWeatherService::CurrentWeather>> currentWeather {};
        Utility::DirtyValue<std::optional<Pinetime::Controllers::SimpleWeatherService::Forecast>> forecastWeather {};

        const lv_color_t needleColor[1] = {LV_COLOR_CYAN};
        lv_obj_t* heart_containers[MAX_HEARTS];

        lv_obj_t* label_time;
        lv_obj_t* label_time_ampm;
        lv_obj_t* label_date;

        lv_obj_t* heartbeatIcon;
        lv_obj_t* heartbeatValue;

        lv_obj_t* stepIcon;
        lv_obj_t* stepValue;

        lv_obj_t* motionMeterBackgrnd;
        lv_obj_t* motionMeterTestLabel;
        lv_point_t motionMeterPoints[NUM_POINTS_MOTION_METER];
        int16_t motionMeterPointsTargets[NUM_POINTS_MOTION_METER];
        lv_obj_t* motionMeterLine;

        lv_obj_t* notificationIcon;

        lv_obj_t* weatherMeterBackground;
        lv_obj_t* weatherMeter;
        int32_t temperature;
        int32_t target_temp;
        bool go_to_temp = true;

        lv_obj_t* weatherRoller;
        lv_obj_t* weatherRollerText;
        lv_obj_t* weatherRollerTriangle;
        lv_obj_t* weatherRollerTextSelected;
        lv_obj_t* weatherRollerTextNext1;
        lv_obj_t* weatherRollerTextNext2;
        lv_obj_t* weatherRollerTime;
        char* lastWeatherIcon;

        lv_obj_t* sheikaSensorBle;
        // lv_obj_t* weatherMeterLabel;

        Controllers::DateTime& dateTimeController;
        Controllers::NotificationManager& notificationManager;
        Controllers::Settings& settingsController;
        Controllers::HeartRateController& heartRateController;
        Controllers::MotionController& motionController;
        Controllers::SimpleWeatherService& weatherSrvc;
        const Controllers::Ble& bleController;

        lv_task_t* taskRefresh;
        Widgets::StatusIcons statusIcons;
      };
    }

    template <>
    struct WatchFaceTraits<WatchFace::Digital> {
      static constexpr WatchFace watchFace = WatchFace::Digital;
      static constexpr const char* name = "Digital face";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::WatchFaceDigital(controllers.dateTimeController,
                                             controllers.batteryController,
                                             controllers.bleController,
                                             controllers.notificationManager,
                                             controllers.settingsController,
                                             controllers.heartRateController,
                                             controllers.motionController,
                                             *controllers.weatherController);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      }
    };
  }
}
