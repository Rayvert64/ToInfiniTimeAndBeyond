#include "displayapp/DisplayApp.h"
#include "displayapp/Colors.h"
#include "displayapp/screens/CheckboxList.h"
#include "displayapp/screens/Styles.h"
#include <lvgl/src/core/lv_obj.h>
#include <lvgl/src/misc/lv_event.h>

using namespace Pinetime::Applications::Screens;

namespace {
  void event_handler(lv_event_t* event) {
    CheckboxList* screen = static_cast<CheckboxList*>(event->user_data);
    screen->UpdateSelected(event);
  }
}

CheckboxList::CheckboxList(const uint8_t screenID,
                           const uint8_t numScreens,
                           const char* optionsTitle,
                           const char* optionsSymbol,
                           uint32_t originalValue,
                           std::function<void(uint32_t)> OnValueChanged,
                           std::array<Item, MaxItems> options)
  : screenID {screenID},
    OnValueChanged {std::move(OnValueChanged)},
    options {options},
    value {originalValue},
    pageIndicator(screenID, numScreens) {
  // Set the background to Black
  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_black(), LV_PART_MAIN);

  if (numScreens > 1) {
    pageIndicator.Create();
  }

  lv_obj_t* container1 = lv_obj_create(lv_screen_active());

  lv_obj_set_style_bg_opa(container1, LV_OPA_TRANSP, LV_STATE_DEFAULT);
  lv_obj_set_style_pad_all(container1, 10, LV_STATE_DEFAULT);
  lv_obj_set_style_text_align(container1, LV_ALIGN_CENTER, LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(container1, 0, LV_PART_MAIN);

  lv_obj_set_pos(container1, 10, 60);
  lv_obj_set_width(container1, LV_HOR_RES - 20);
  lv_obj_set_height(container1, LV_VER_RES - 50);
  lv_obj_set_style_text_align(container1, LV_ALIGN_TOP_LEFT, LV_STATE_ANY);

  lv_obj_t* title = lv_label_create(lv_screen_active());
  lv_label_set_text_static(title, optionsTitle);
  lv_obj_set_align(title, LV_ALIGN_CENTER);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 10, 15);

  lv_obj_t* icon = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(icon, PINETIME_COLOR_ORANGE, LV_STATE_DEFAULT);
  lv_label_set_text_static(icon, optionsSymbol);
  lv_obj_set_align(icon, LV_ALIGN_CENTER);
  lv_obj_align(icon, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  for (unsigned int i = 0; i < options.size(); i++) {
    if (strcmp(options[i].name, "")) {
      cbOption[i] = lv_checkbox_create(container1);
      lv_checkbox_set_text(cbOption[i], options[i].name);
      if (!options[i].enabled) {
        lv_obj_remove_flag(cbOption[i], LV_OBJ_FLAG_CHECKABLE);
      }
      cbOption[i]->user_data = this;
      lv_obj_add_event_cb(cbOption[i], event_handler, LV_EVENT_VALUE_CHANGED, this);
      SetRadioButtonStyle(cbOption[i]);

      if (static_cast<unsigned int>(originalValue - MaxItems * screenID) == i) {
        lv_obj_set_state(cbOption[i], LV_STATE_CHECKED, true);
      }
    }
  }
}

CheckboxList::~CheckboxList() {
  lv_obj_clean(lv_screen_active());
  OnValueChanged(value);
}

void CheckboxList::UpdateSelected(lv_event_t* event) {
  auto* object = lv_event_get_target_obj(event);
  for (unsigned int i = 0; i < options.size(); i++) {
    if (strcmp(options[i].name, "")) {
      if (object == cbOption[i]) {
        lv_obj_set_state(cbOption[i], LV_STATE_CHECKED, true);
        value = MaxItems * screenID + i;
      } else {
        lv_obj_set_state(cbOption[i], LV_STATE_CHECKED, false);
      }
      if (!options[i].enabled) {
        lv_obj_remove_flag(cbOption[i], LV_OBJ_FLAG_CHECKABLE);
      }
    }
  }
}
