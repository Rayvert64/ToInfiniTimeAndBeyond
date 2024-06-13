#include "displayapp/screens/FirmwareValidation.h"
#include "displayapp/Colors.h"
#include <lvgl/lvgl.h>
#include <lvgl/src/misc/lv_event.h>
#include "Version.h"
#include "components/firmwarevalidator/FirmwareValidator.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void ButtonEventHandlerValidate(lv_event_t* event) {
    auto* screen = static_cast<FirmwareValidation*>(event->user_data);
    screen->Validate();
  }

  void ButtonEventHandlerReset(lv_event_t* event) {
    auto* screen = static_cast<FirmwareValidation*>(event->user_data);
    screen->Reset();
  }
}

FirmwareValidation::FirmwareValidation(Pinetime::Controllers::FirmwareValidator& validator) : validator {validator} {
  labelVersion = lv_label_create(lv_screen_active());
  lv_label_set_text_fmt(labelVersion,
                        "Version : %" PRIu32 ".%" PRIu32 ".%" PRIu32 "\n"
                        "ShortRef : %s",
                        Version::Major(),
                        Version::Minor(),
                        Version::Patch(),
                        Version::GitCommitHash());
  lv_obj_align(labelVersion, LV_ALIGN_TOP_LEFT, 0, 0);

  labelIsValidated = lv_label_create(lv_screen_active());
  lv_obj_align(labelIsValidated, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);

  lv_label_set_long_mode(labelIsValidated, LV_LABEL_LONG_WRAP);
  lv_obj_set_width(labelIsValidated, 240);

  if (validator.IsValidated()) {
    lv_label_set_text_static(labelIsValidated, "You have already\n#00ff00 validated# this firmware#");
  } else {
    lv_label_set_text_static(labelIsValidated,
                             "Please #00ff00 Validate# this version or\n#ff0000 Reset# to rollback to the previous version.");

    buttonValidate = lv_button_create(lv_screen_active());
    buttonValidate->user_data = this;
    lv_obj_set_size(buttonValidate, 115, 50);
    lv_obj_align(buttonValidate, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_add_event_cb(buttonValidate, ButtonEventHandlerValidate, LV_EVENT_SHORT_CLICKED, this);
    lv_obj_set_style_bg_color(buttonValidate, Colors::highlight, LV_PART_MAIN);

    labelButtonValidate = lv_label_create(buttonValidate);
    lv_label_set_text_static(labelButtonValidate, "Validate");

    buttonReset = lv_button_create(lv_screen_active());
    buttonReset->user_data = this;
    lv_obj_set_size(buttonReset, 115, 50);
    lv_obj_align(buttonReset, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_set_style_bg_color(buttonReset, PINETIME_COLOR_RED, LV_PART_MAIN);
    lv_obj_add_event_cb(buttonReset, ButtonEventHandlerReset, LV_EVENT_SHORT_CLICKED, this);

    labelButtonReset = lv_label_create(buttonReset);
    lv_label_set_text_static(labelButtonReset, "Reset");
  }
}

FirmwareValidation::~FirmwareValidation() {
  lv_obj_clean(lv_screen_active());
}

void FirmwareValidation::Validate() {
  validator.Validate();
  running = false;
}

void FirmwareValidation::Reset() {
  validator.Reset();
}
