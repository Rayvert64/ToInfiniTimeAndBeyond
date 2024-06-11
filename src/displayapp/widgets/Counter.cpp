#include "displayapp/widgets/Counter.h"
#include "components/datetime/DateTimeController.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Widgets;

namespace {
  void upBtnEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* widget = static_cast<Counter*>(obj->user_data);
    if (event == LV_EVENT_SHORT_CLICKED || event == LV_EVENT_LONG_PRESSED_REPEAT) {
      widget->UpBtnPressed();
    }
  }

  void downBtnEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* widget = static_cast<Counter*>(obj->user_data);
    if (event == LV_EVENT_SHORT_CLICKED || event == LV_EVENT_LONG_PRESSED_REPEAT) {
      widget->DownBtnPressed();
    }
  }

  constexpr int digitCount(int number) {
    int digitCount = 0;
    while (number > 0) {
      digitCount++;
      number /= 10;
    }
    return digitCount;
  }
}

Counter::Counter(int min, int max, lv_font_t& font) : min {min}, max {max}, value {min}, leadingZeroCount {digitCount(max)}, font {font} {
}

void Counter::UpBtnPressed() {
  value++;
  if (value > max) {
    value = min;
  }
  UpdateLabel();

  if (ValueChangedHandler != nullptr) {
    ValueChangedHandler(userData);
  }
};

void Counter::DownBtnPressed() {
  value--;
  if (value < min) {
    value = max;
  }
  UpdateLabel();

  if (ValueChangedHandler != nullptr) {
    ValueChangedHandler(userData);
  }
};

void Counter::SetValue(int newValue) {
  value = newValue;
  UpdateLabel();
}

void Counter::HideControls() {
  lv_obj_add_flag(upBtn, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(downBtn, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(upperLine, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(lowerLine, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_style_local_bg_opa(counterContainer, LV_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
}

void Counter::ShowControls() {
  if (lv_obj_has_flag(upBtn)) {
    lv_obj_remove_flag(upBtn, LV_OBJ_FLAG_HIDDEN);
  };
  if (lv_obj_has_flag(downBtn)) {
    lv_obj_remove_flag(downBtn, LV_OBJ_FLAG_HIDDEN);
  };
  if (lv_obj_has_flag(upperLine)) {
    lv_obj_remove_flag(upperLine, LV_OBJ_FLAG_HIDDEN);
  };
  if (lv_obj_has_flag(lowerLine)) {
    lv_obj_remove_flag(lowerLine, LV_OBJ_FLAG_HIDDEN);
  };
  lv_obj_set_style_local_bg_opa(counterContainer, LV_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
}

void Counter::UpdateLabel() {
  if (twelveHourMode) {
    if (value == 0) {
      lv_label_set_text_static(number, "12");
    } else if (value <= 12) {
      lv_label_set_text_fmt(number, "%.*i", leadingZeroCount, value);
    } else {
      lv_label_set_text_fmt(number, "%.*i", leadingZeroCount, value - 12);
    }
  } else if (monthMode) {
    lv_label_set_text(number, Controllers::DateTime::MonthShortToStringLow(static_cast<Controllers::DateTime::Months>(value)));
  } else {
    lv_label_set_text_fmt(number, "%.*i", leadingZeroCount, value);
  }
}

// Value is kept between 0 and 23, but the displayed value is converted to 12-hour.
// Make sure to set the max and min values to 0 and 23. Otherwise behaviour is undefined
void Counter::EnableTwelveHourMode() {
  twelveHourMode = true;
}

// Value is kept between 1 and 12, but the displayed value is the corresponding month
// Make sure to set the max and min values to 1 and 12. Otherwise behaviour is undefined
void Counter::EnableMonthMode() {
  monthMode = true;
}

// Counter cannot be resized after creation,
// so the newMax value must have the same number of digits as the old one
void Counter::SetMax(int newMax) {
  max = newMax;
  if (value > max) {
    value = max;
    UpdateLabel();
  }
}

void Counter::SetValueChangedEventCallback(void* userData, void (*handler)(void* userData)) {
  this->userData = userData;
  this->ValueChangedHandler = handler;
}

void Counter::Create() {
  counterContainer = lv_obj_create(lv_scr_act());
  lv_obj_set_style_bg_color(counterContainer, Colors::bgAlt, LV_PART_MAIN);

  number = lv_label_create(counterContainer);
  lv_obj_set_style_text_font(number, &font, LV_STATE_DEFAULT);
  lv_obj_align(number, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_auto_realign(number, true);
  if (monthMode) {
    lv_label_set_text_static(number, "Jan");
  } else {
    lv_label_set_text_fmt(number, "%d", max);
  }

  static constexpr uint8_t padding = 5;
  const uint8_t width = std::max(lv_obj_get_width(number) + padding * 2, 58);
  static constexpr uint8_t btnHeight = 50;
  const uint8_t containerHeight = btnHeight * 2 + lv_obj_get_height(number) + padding * 2;

  lv_obj_set_size(counterContainer, width, containerHeight);

  UpdateLabel();

  upBtn = lv_button_create(counterContainer);
  lv_obj_set_style_bg_color(upBtn, Colors::bgAlt, LV_PART_MAIN);
  lv_obj_set_size(upBtn, width, btnHeight);
  lv_obj_align(upBtn, LV_ALIGN_TOP_MID, 0, 0);
  upBtn->user_data = this;
  lv_obj_add_event_cb(upBtn, upBtnEventHandler);

  lv_obj_t* upLabel = lv_label_create(upBtn);
  lv_obj_set_style_text_font(upLabel, &jetbrains_mono_42, LV_STATE_DEFAULT);
  lv_label_set_text_static(upLabel, "+");
  lv_obj_align(upLabel, LV_ALIGN_CENTER, 0, 0);

  downBtn = lv_button_create(counterContainer);
  lv_obj_set_style_bg_color(downBtn, Colors::bgAlt, LV_PART_MAIN);
  lv_obj_set_size(downBtn, width, btnHeight);
  lv_obj_align(downBtn, LV_ALIGN_BOTTOM_MID, 0, 0);
  downBtn->user_data = this;
  lv_obj_add_event_cb(downBtn, downBtnEventHandler);

  lv_obj_t* downLabel = lv_label_create(downBtn);
  lv_obj_set_style_text_font(downLabel, &jetbrains_mono_42, LV_STATE_DEFAULT);
  lv_label_set_text_static(downLabel, "-");
  lv_obj_align(downLabel, LV_ALIGN_CENTER, 0, 0);

  linePoints[0] = {0, 0};
  linePoints[1] = {width, 0};

  auto LineCreate = [&]() {
    lv_obj_t* line = lv_line_create(counterContainer, nullptr);
    lv_line_set_points(line, linePoints, 2);
    lv_obj_set_style_local_line_width(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 1);
    lv_obj_set_style_local_line_color(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, lv_color_white());
    lv_obj_set_style_local_line_opa(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_20);
    return line;
  };

  upperLine = LineCreate();
  lv_obj_align(upperLine, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

  lowerLine = LineCreate();
  lv_obj_align(lowerLine, LV_ALIGN_OUT_TOP_MID, 0, -1);
}
