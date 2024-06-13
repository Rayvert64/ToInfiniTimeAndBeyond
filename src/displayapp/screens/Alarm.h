/*  Copyright (C) 2021 mruss77, Florian

    This file is part of InfiniTime.

    InfiniTime is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    InfiniTime is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#pragma once

#include <memory>
#include "displayapp/apps/Apps.h"
#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/widgets/Counter.h"
#include "displayapp/Controllers.h"
#include "Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Alarm : public Screen {
      public:
        explicit Alarm(Controllers::AlarmController& alarmController,
                       Controllers::Settings::ClockType clockType,
                       System::SystemTask& systemTask,
                       Controllers::MotorController& motorController);
        ~Alarm() override;
        void SetAlerting();
        [[nodiscard]] bool GetSwitchState() const;
        bool OnButtonPushed() override;
        bool OnTouchEvent(TouchEvents event) override;
        void OnValueChanged();
        void StopAlerting();
        void ShowInfo();
        void HideInfo();
        void EnableAlarm();
        void DisableAlarm();
        void ToggleRecurrence();

      private:
        Controllers::AlarmController& alarmController;
        System::SystemTask& systemTask;
        Controllers::MotorController& motorController;

        lv_obj_t *btnStop, *txtStop, *btnRecur, *txtRecur, *btnInfo, *enableSwitch;
        lv_obj_t* lblampm = nullptr;
        lv_obj_t* txtMessage = nullptr;
        lv_obj_t* btnMessage = nullptr;
        lv_timer_t* taskStopAlarm = nullptr;

        enum class EnableButtonState { On, Off, Alerting };
        void SetRecurButtonState();
        void SetSwitchState(bool state);
        void UpdateAlarmTime();
        lv_obj_t* hourCounter;
        lv_obj_t* minuteCounter;
      };
    }

    template <>
    struct AppTraits<Apps::Alarm> {
      static constexpr Apps app = Apps::Alarm;
      static constexpr const char* icon = Screens::Symbols::bell;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::Alarm(controllers.alarmController,
                                  controllers.settingsController.GetClockType(),
                                  *controllers.systemTask,
                                  controllers.motorController);
      };
    };
  }
}
