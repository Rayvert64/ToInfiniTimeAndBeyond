#include "displayapp/screens/Metronome.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"
#include <lvgl/src/core/lv_obj_style_gen.h>
#include <lvgl/src/lv_api_map_v8.h>
#include <lvgl/src/misc/lv_event.h>
#include <lvgl/src/misc/lv_types.h>

using namespace Pinetime::Applications::Screens;

namespace {
  void EventOnBpmValueChangeArc(lv_event_t* event) {
    auto* screen = static_cast<Metronome*>(event->user_data);
    screen->OnBpmValueChangeArc();
  }

  void EventOnBpmValueChangeDropDown(lv_event_t* event) {
    auto* screen = static_cast<Metronome*>(event->user_data);
    screen->OnBpmValueChangeDropDown();
  }

  void EventOnScreenTap(lv_event_t* event) {
    auto* screen = static_cast<Metronome*>(event->user_data);
    screen->OnScreenTap();
  }

  void EventOnScreenTapRelease(lv_event_t* event) {
    auto* screen = static_cast<Metronome*>(event->user_data);
    screen->OnScreenTapRelease();
  }

  void EventOnPlayPausePress(lv_event_t* event) {
    auto* screen = static_cast<Metronome*>(event->user_data);
    screen->OnPlayPausePress();
  }

  lv_obj_t* createLabel(const char* name, lv_obj_t* reference, lv_align_t align, const lv_font_t* font, uint8_t x, uint8_t y) {
    lv_obj_t* label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(label, font, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label, Colors::lightGray, LV_STATE_DEFAULT);
    lv_label_set_text(label, name);
    lv_obj_align_to(label, reference, align, x, y);

    return label;
  }
}

Metronome::Metronome(Controllers::MotorController& motorController, System::SystemTask& systemTask)
  : motorController {motorController}, systemTask {systemTask} {

  bpmArc = lv_arc_create(lv_scr_act());
  bpmArc->user_data = this;
  lv_obj_add_event_cb(bpmArc, EventOnBpmValueChangeArc, LV_EVENT_VALUE_CHANGED, this);
  lv_arc_set_bg_angles(bpmArc, 0, 270);
  lv_arc_set_rotation(bpmArc, 135);
  lv_arc_set_range(bpmArc, 40, 220);
  lv_arc_set_value(bpmArc, bpm);
  lv_obj_set_size(bpmArc, 210, 210);
  lv_obj_align(bpmArc, LV_ALIGN_TOP_MID, 0, 0);

  bpmValue = createLabel("120", bpmArc, LV_ALIGN_TOP_MID, &jetbrains_mono_76, 0, 55);
  createLabel("bpm", bpmValue, LV_ALIGN_OUT_BOTTOM_MID, &jetbrains_mono_bold_20, 0, 0);

  bpmTap = lv_button_create(lv_scr_act());
  bpmTap->user_data = this;
  lv_obj_add_event_cb(bpmTap, EventOnScreenTap, LV_EVENT_PRESSED, this);
  lv_obj_add_event_cb(bpmTap, EventOnScreenTapRelease, LV_EVENT_PRESS_LOST, this);
  lv_obj_add_event_cb(bpmTap, EventOnScreenTapRelease, LV_EVENT_RELEASED, this);
  lv_obj_set_style_bg_opa(bpmTap, LV_OPA_TRANSP, LV_STATE_DEFAULT);
  lv_obj_set_height(bpmTap, 80);
  lv_obj_align(bpmTap, LV_ALIGN_TOP_MID, 0, 0);

  bpbDropdown = lv_dropdown_create(lv_scr_act());
  bpbDropdown->user_data = this;
  lv_obj_add_event_cb(bpbDropdown, EventOnBpmValueChangeDropDown, LV_EVENT_VALUE_CHANGED, this);
  lv_obj_set_size(bpbDropdown, 115, 50);
  lv_obj_align(bpbDropdown, LV_ALIGN_BOTTOM_LEFT, 0, 0);
  lv_dropdown_set_options(bpbDropdown, "1\n2\n3\n4\n5\n6\n7\n8\n9");
  lv_dropdown_set_selected(bpbDropdown, bpb - 1);
  lv_dropdown_set_text(bpbDropdown, "");

  currentBpbText = lv_label_create(bpbDropdown);
  lv_label_set_text_fmt(currentBpbText, "%d bpb", bpb);
  lv_obj_align(currentBpbText, LV_ALIGN_CENTER, 0, 0);

  playPause = lv_button_create(lv_scr_act());
  playPause->user_data = this;
  lv_obj_add_event_cb(playPause, EventOnPlayPausePress, LV_EVENT_SHORT_CLICKED, this);
  lv_obj_set_size(playPause, 115, 50);
  lv_obj_align(playPause, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
  lblPlayPause = lv_label_create(playPause);
  lv_label_set_text_static(lblPlayPause, Symbols::play);

  taskRefresh = lv_timer_create(RefreshTaskCallback, LV_DEF_REFR_PERIOD, this);
}

Metronome::~Metronome() {
  lv_timer_del(taskRefresh);
  systemTask.PushMessage(System::Messages::EnableSleeping);
  lv_obj_clean(lv_scr_act());
}

void Metronome::Refresh() {
  if (metronomeStarted) {
    if (xTaskGetTickCount() - startTime > 60u * configTICK_RATE_HZ / static_cast<uint16_t>(bpm)) {
      startTime += 60 * configTICK_RATE_HZ / bpm;
      counter--;
      if (counter == 0) {
        counter = bpb;
        motorController.RunForDuration(90);
      } else {
        motorController.RunForDuration(30);
      }
    }
  }
}

void Metronome::OnBpmValueChangeArc() {
  bpm = lv_arc_get_value(bpmArc);
  lv_label_set_text_fmt(bpmValue, "%" PRIi32 "", bpm);
}

void Metronome::OnBpmValueChangeDropDown() {
  bpb = lv_dropdown_get_selected(bpbDropdown) + 1;
  lv_label_set_text_fmt(currentBpbText, "%d bpb", bpb);
}

void Metronome::OnScreenTap() {
  TickType_t delta = xTaskGetTickCount() - tappedTime;
  if (tappedTime != 0 && delta < configTICK_RATE_HZ * 3) {
    bpm = configTICK_RATE_HZ * 60 / delta;
    lv_arc_set_value(bpmArc, bpm);
    lv_label_set_text_fmt(bpmValue, "%" PRIi32 "", bpm);
  }
  tappedTime = xTaskGetTickCount();
  allowExit = true;
}

void Metronome::OnScreenTapRelease() {
  allowExit = false;
}

void Metronome::OnPlayPausePress() {
  metronomeStarted = !metronomeStarted;
  if (metronomeStarted) {
    lv_label_set_text_static(lblPlayPause, Symbols::pause);
    systemTask.PushMessage(System::Messages::DisableSleeping);
    startTime = xTaskGetTickCount();
    counter = 1;
  } else {
    lv_label_set_text_static(lblPlayPause, Symbols::play);
    systemTask.PushMessage(System::Messages::EnableSleeping);
  }
}

bool Metronome::OnTouchEvent(TouchEvents event) {
  if (event == TouchEvents::SwipeDown && allowExit) {
    running = false;
  }
  return true;
}
