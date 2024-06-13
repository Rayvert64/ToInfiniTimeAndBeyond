#include "displayapp/screens/HeartRate.h"
#include "displayapp/Colors.h"
#include <lvgl/lvgl.h>
#include <components/heartrate/HeartRateController.h>

#include "displayapp/DisplayApp.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  const char* ToString(Pinetime::Controllers::HeartRateController::States s) {
    switch (s) {
      case Pinetime::Controllers::HeartRateController::States::NotEnoughData:
        return "Not enough data,\nplease wait...";
      case Pinetime::Controllers::HeartRateController::States::NoTouch:
        return "No touch detected";
      case Pinetime::Controllers::HeartRateController::States::Running:
        return "Measuring...";
      case Pinetime::Controllers::HeartRateController::States::Stopped:
        return "Stopped";
    }
    return "";
  }

  void btnStartStopEventHandler(lv_event_t* event) {
    auto* screen = static_cast<HeartRate*>(event->user_data);
    screen->OnStartStopEvent();
  }
}

HeartRate::HeartRate(Controllers::HeartRateController& heartRateController, System::SystemTask& systemTask)
  : heartRateController {heartRateController}, systemTask {systemTask} {
  bool isHrRunning = heartRateController.State() != Controllers::HeartRateController::States::Stopped;
  label_hr = lv_label_create(lv_screen_active());

  lv_obj_set_style_text_font(label_hr, &jetbrains_mono_76, LV_STATE_DEFAULT);

  if (isHrRunning) {
    lv_obj_set_style_text_color(label_hr, Colors::highlight, LV_STATE_DEFAULT);
  } else {
    lv_obj_set_style_text_color(label_hr, Colors::lightGray, LV_STATE_DEFAULT);
  }

  lv_label_set_text_static(label_hr, "---");
  lv_obj_align(label_hr, LV_ALIGN_CENTER, 0, -40);

  label_bpm = lv_label_create(lv_screen_active());
  lv_label_set_text_static(label_bpm, "Heart rate BPM");
  lv_obj_align(label_bpm, LV_ALIGN_OUT_TOP_MID, 0, -20);

  label_status = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(label_status, PINETIME_COLOR_GRAY, LV_STATE_DEFAULT);
  lv_label_set_text_static(label_status, ToString(Pinetime::Controllers::HeartRateController::States::NotEnoughData));

  lv_obj_align(label_status, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

  btn_startStop = lv_button_create(lv_screen_active());
  btn_startStop->user_data = this;
  lv_obj_set_height(btn_startStop, 50);
  lv_obj_add_event_cb(btn_startStop, btnStartStopEventHandler, LV_EVENT_SHORT_CLICKED, this);
  lv_obj_align(btn_startStop, LV_ALIGN_BOTTOM_MID, 0, 0);

  label_startStop = lv_label_create(btn_startStop);
  UpdateStartStopButton(isHrRunning);
  if (isHrRunning) {
    systemTask.PushMessage(Pinetime::System::Messages::DisableSleeping);
  }

  taskRefresh = lv_timer_create(RefreshTaskCallback, 100, this);
}

HeartRate::~HeartRate() {
  lv_timer_del(taskRefresh);
  lv_obj_clean(lv_screen_active());
  systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
}

void HeartRate::Refresh() {

  auto state = heartRateController.State();
  switch (state) {
    case Controllers::HeartRateController::States::NoTouch:
    case Controllers::HeartRateController::States::NotEnoughData:
      // case Controllers::HeartRateController::States::Stopped:
      lv_label_set_text_static(label_hr, "---");
      break;
    default:
      if (heartRateController.HeartRate() == 0) {
        lv_label_set_text_static(label_hr, "---");
      } else {
        lv_label_set_text_fmt(label_hr, "%03d", heartRateController.HeartRate());
      }
  }

  lv_label_set_text_static(label_status, ToString(state));
  lv_obj_align(label_status, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
}

void HeartRate::OnStartStopEvent() {
  if (heartRateController.State() == Controllers::HeartRateController::States::Stopped) {
    heartRateController.Start();
    UpdateStartStopButton(heartRateController.State() != Controllers::HeartRateController::States::Stopped);
    systemTask.PushMessage(Pinetime::System::Messages::DisableSleeping);
    lv_obj_set_style_text_color(label_hr, Colors::highlight, LV_STATE_DEFAULT);
  } else {
    heartRateController.Stop();
    UpdateStartStopButton(heartRateController.State() != Controllers::HeartRateController::States::Stopped);
    systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
    lv_obj_set_style_text_color(label_hr, Colors::lightGray, LV_STATE_DEFAULT);
  }
}

void HeartRate::UpdateStartStopButton(bool isRunning) {
  if (isRunning) {
    lv_label_set_text_static(label_startStop, "Stop");
  } else {
    lv_label_set_text_static(label_startStop, "Start");
  }
}
