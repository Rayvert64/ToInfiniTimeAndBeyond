#include "displayapp/screens/FlashLight.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"
#include <lvgl/src/core/lv_obj.h>

using namespace Pinetime::Applications::Screens;

namespace {
  void EventHandler(lv_event_t* event) {
    auto* screen = static_cast<FlashLight*>(event->user_data);
    screen->Toggle();
  }
}

FlashLight::FlashLight(System::SystemTask& systemTask, Controllers::BrightnessController& brightnessController)
  : systemTask {systemTask}, brightnessController {brightnessController} {

  brightnessController.Set(Controllers::BrightnessController::Levels::Low);

  flashLight = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_font(flashLight, &lv_font_sys_48, LV_STATE_DEFAULT);
  lv_label_set_text_static(flashLight, Symbols::flashlight);
  lv_obj_align(flashLight, LV_ALIGN_CENTER, 0, 0);

  for (auto& indicator : indicators) {
    indicator = lv_obj_create(lv_screen_active());
    ;
    lv_obj_set_size(indicator, 15, 10);
    lv_obj_set_style_border_width(indicator, 2, LV_PART_MAIN);
  }

  lv_obj_align(indicators[1], LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
  lv_obj_align(indicators[0], LV_ALIGN_OUT_LEFT_MID, -8, 0);
  lv_obj_align(indicators[2], LV_ALIGN_OUT_RIGHT_MID, 8, 0);

  SetIndicators();
  SetColors();

  backgroundAction = lv_label_create(lv_screen_active());
  lv_label_set_long_mode(backgroundAction, LV_LABEL_LONG_CLIP);
  lv_obj_set_size(backgroundAction, 240, 240);
  lv_obj_set_pos(backgroundAction, 0, 0);
  lv_label_set_text_static(backgroundAction, "");
  lv_obj_add_flag(backgroundAction, LV_OBJ_FLAG_CLICKABLE);
  backgroundAction->user_data = this;
  lv_obj_add_event_cb(backgroundAction, EventHandler, LV_EVENT_CLICKED, this);

  systemTask.PushMessage(Pinetime::System::Messages::DisableSleeping);
}

FlashLight::~FlashLight() {
  lv_obj_clean(lv_screen_active());
  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_black(), LV_PART_MAIN);
  systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
}

void FlashLight::SetColors() {
  lv_color_t bgColor = isOn ? lv_color_white() : lv_color_black();
  lv_color_t fgColor = isOn ? Colors::lightGray : lv_color_white();

  lv_obj_set_style_bg_color(lv_screen_active(), bgColor, LV_PART_MAIN);
  lv_obj_set_style_text_color(flashLight, fgColor, LV_STATE_DEFAULT);
  for (auto& indicator : indicators) {
    lv_obj_set_style_bg_color(indicator, fgColor, LV_PART_MAIN);
    lv_obj_set_style_bg_color(indicator, bgColor, LV_PART_MAIN);
    lv_obj_set_style_border_color(indicator, fgColor, LV_PART_MAIN);
  }
}

void FlashLight::SetIndicators() {
  using namespace Pinetime::Controllers;

  if (brightnessLevel == BrightnessController::Levels::High) {
    lv_obj_set_state(indicators[1], LV_STATE_DEFAULT, true);
    lv_obj_set_state(indicators[2], LV_STATE_DEFAULT, true);
  } else if (brightnessLevel == BrightnessController::Levels::Medium) {
    lv_obj_set_state(indicators[1], LV_STATE_DEFAULT, true);
    lv_obj_set_state(indicators[2], LV_STATE_DISABLED, true);
  } else {
    lv_obj_set_state(indicators[1], LV_STATE_DISABLED, true);
    lv_obj_set_state(indicators[2], LV_STATE_DISABLED, true);
  }
}

void FlashLight::Toggle() {
  isOn = !isOn;
  SetColors();
  if (isOn) {
    brightnessController.Set(brightnessLevel);
  } else {
    brightnessController.Set(Controllers::BrightnessController::Levels::Low);
  }
}

bool FlashLight::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  using namespace Pinetime::Controllers;

  auto SetState = [this]() {
    if (isOn) {
      brightnessController.Set(brightnessLevel);
    }
    SetIndicators();
  };

  if (event == TouchEvents::SwipeLeft) {
    if (brightnessLevel == BrightnessController::Levels::High) {
      brightnessLevel = BrightnessController::Levels::Medium;
      SetState();
    } else if (brightnessLevel == BrightnessController::Levels::Medium) {
      brightnessLevel = BrightnessController::Levels::Low;
      SetState();
    }
    return true;
  }
  if (event == TouchEvents::SwipeRight) {
    if (brightnessLevel == BrightnessController::Levels::Low) {
      brightnessLevel = BrightnessController::Levels::Medium;
      SetState();
    } else if (brightnessLevel == BrightnessController::Levels::Medium) {
      brightnessLevel = BrightnessController::Levels::High;
      SetState();
    }
    return true;
  }

  return false;
}
