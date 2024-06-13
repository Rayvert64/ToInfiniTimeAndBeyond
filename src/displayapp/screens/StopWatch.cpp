#include "displayapp/screens/StopWatch.h"
#include "displayapp/Colors.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"
#include <lvgl/src/misc/lv_event.h>

using namespace Pinetime::Applications::Screens;

namespace {
  TimeSeparated_t convertTicksToTimeSegments(const TickType_t timeElapsed) {
    // Centiseconds
    const int timeElapsedCentis = timeElapsed * 100 / configTICK_RATE_HZ;

    const int hundredths = (timeElapsedCentis % 100);
    const int secs = (timeElapsedCentis / 100) % 60;
    const int mins = ((timeElapsedCentis / 100) / 60) % 60;
    const int hours = ((timeElapsedCentis / 100) / 60) / 60;
    return TimeSeparated_t {hours, mins, secs, hundredths};
  }

  void play_pause_event_handler(lv_event_t* event) {
    auto* stopWatch = static_cast<StopWatch*>(event->user_data);
    stopWatch->playPauseBtnEventHandler();
  }

  void stop_lap_event_handler(lv_event_t* event) {
    auto* stopWatch = static_cast<StopWatch*>(event->user_data);
    stopWatch->stopLapBtnEventHandler();
  }

  constexpr TickType_t blinkInterval = pdMS_TO_TICKS(1000);
}

StopWatch::StopWatch(System::SystemTask& systemTask) : systemTask {systemTask} {
  static constexpr uint8_t btnWidth = 115;
  static constexpr uint8_t btnHeight = 80;
  btnPlayPause = lv_button_create(lv_screen_active());
  btnPlayPause->user_data = this;
  lv_obj_add_event_cb(btnPlayPause, play_pause_event_handler, LV_EVENT_SHORT_CLICKED, this);
  lv_obj_set_size(btnPlayPause, btnWidth, btnHeight);
  lv_obj_align(btnPlayPause, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
  txtPlayPause = lv_label_create(btnPlayPause);

  btnStopLap = lv_button_create(lv_screen_active());
  btnStopLap->user_data = this;
  lv_obj_add_event_cb(btnStopLap, stop_lap_event_handler, LV_EVENT_SHORT_CLICKED, this);
  lv_obj_set_size(btnStopLap, btnWidth, btnHeight);
  lv_obj_align(btnStopLap, LV_ALIGN_BOTTOM_LEFT, 0, 0);
  txtStopLap = lv_label_create(btnStopLap);
  lv_obj_set_state(btnStopLap, LV_STATE_DISABLED, true);
  lv_obj_set_state(txtStopLap, LV_STATE_DISABLED, true);

  lapText = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(lapText, Colors::lightGray, LV_STATE_DEFAULT);
  lv_label_set_text_static(lapText, "\n");
  lv_label_set_long_mode(lapText, LV_LABEL_LONG_WRAP);
  lv_obj_set_align(lapText, LV_ALIGN_CENTER);
  lv_obj_set_width(lapText, LV_HOR_RES);
  lv_obj_align(lapText, LV_ALIGN_BOTTOM_MID, 0, -btnHeight);

  msecTime = lv_label_create(lv_screen_active());
  lv_label_set_text_static(msecTime, "00");
  lv_obj_set_style_text_color(msecTime, Colors::lightGray, LV_STATE_DISABLED);
  lv_obj_align(msecTime, LV_ALIGN_OUT_TOP_MID, 0, 0);

  time = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_font(time, &jetbrains_mono_76, LV_STATE_DEFAULT);
  lv_label_set_text_static(time, "00:00");
  lv_obj_set_style_text_color(time, Colors::lightGray, LV_STATE_DISABLED);
  lv_obj_align(time, LV_ALIGN_OUT_TOP_MID, 0, 0);

  SetInterfaceStopped();

  taskRefresh = lv_timer_create(RefreshTaskCallback, LV_DEF_REFR_PERIOD, this);
}

StopWatch::~StopWatch() {
  lv_timer_del(taskRefresh);
  systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
  lv_obj_clean(lv_screen_active());
}

void StopWatch::SetInterfacePaused() {
  lv_obj_set_style_bg_color(btnStopLap, PINETIME_COLOR_RED, LV_PART_MAIN);
  lv_obj_set_style_bg_color(btnPlayPause, Colors::blue, LV_PART_MAIN);
  lv_label_set_text_static(txtPlayPause, Symbols::play);
  lv_label_set_text_static(txtStopLap, Symbols::stop);
}

void StopWatch::SetInterfaceRunning() {
  lv_obj_set_state(time, LV_STATE_DEFAULT, true);
  lv_obj_set_state(msecTime, LV_STATE_DEFAULT, true);
  lv_obj_set_style_bg_color(btnPlayPause, Colors::bgAlt, LV_PART_MAIN);
  lv_obj_set_style_bg_color(btnStopLap, Colors::bgAlt, LV_PART_MAIN);

  lv_label_set_text_static(txtPlayPause, Symbols::pause);
  lv_label_set_text_static(txtStopLap, Symbols::lapsFlag);

  lv_obj_set_state(btnStopLap, LV_STATE_DEFAULT, true);
  lv_obj_set_state(txtStopLap, LV_STATE_DEFAULT, true);
}

void StopWatch::SetInterfaceStopped() {
  lv_obj_set_state(time, LV_STATE_DISABLED, true);
  lv_obj_set_state(msecTime, LV_STATE_DISABLED, true);
  lv_obj_set_style_bg_color(btnPlayPause, Colors::blue, LV_PART_MAIN);

  lv_label_set_text_static(time, "00:00");
  lv_label_set_text_static(msecTime, "00");

  if (isHoursLabelUpdated) {
    lv_obj_set_style_text_font(time, &jetbrains_mono_76, LV_STATE_DEFAULT);

    isHoursLabelUpdated = false;
  }

  lv_label_set_text_static(lapText, "");
  lv_label_set_text_static(txtPlayPause, Symbols::play);
  lv_label_set_text_static(txtStopLap, Symbols::lapsFlag);
  lv_obj_set_state(btnStopLap, LV_STATE_DISABLED, true);
  lv_obj_set_state(txtStopLap, LV_STATE_DISABLED, true);
}

void StopWatch::Reset() {
  SetInterfaceStopped();
  currentState = States::Init;
  oldTimeElapsed = 0;
  lapsDone = 0;
}

void StopWatch::Start() {
  SetInterfaceRunning();
  startTime = xTaskGetTickCount();
  currentState = States::Running;
  systemTask.PushMessage(Pinetime::System::Messages::DisableSleeping);
}

void StopWatch::Pause() {
  SetInterfacePaused();
  startTime = 0;
  // Store the current time elapsed in cache
  oldTimeElapsed = laps[lapsDone];
  blinkTime = xTaskGetTickCount() + blinkInterval;
  currentState = States::Halted;
  systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
}

void StopWatch::Refresh() {
  if (currentState == States::Running) {
    laps[lapsDone] = oldTimeElapsed + xTaskGetTickCount() - startTime;

    TimeSeparated_t currentTimeSeparated = convertTicksToTimeSegments(laps[lapsDone]);
    if (currentTimeSeparated.hours == 0) {
      lv_label_set_text_fmt(time, "%02d:%02d", currentTimeSeparated.mins, currentTimeSeparated.secs);
    } else {
      lv_label_set_text_fmt(time, "%02d:%02d:%02d", currentTimeSeparated.hours, currentTimeSeparated.mins, currentTimeSeparated.secs);
      if (!isHoursLabelUpdated) {
        lv_obj_set_style_text_font(time, &jetbrains_mono_42, LV_STATE_DEFAULT);

        isHoursLabelUpdated = true;
      }
    }
    lv_label_set_text_fmt(msecTime, "%02d", currentTimeSeparated.hundredths);
  } else if (currentState == States::Halted) {
    const TickType_t currentTime = xTaskGetTickCount();
    if (currentTime > blinkTime) {
      blinkTime = currentTime + blinkInterval;
      if (lv_obj_get_state(time)) {
        lv_obj_set_state(time, LV_STATE_DISABLED, true);
        lv_obj_set_state(msecTime, LV_STATE_DISABLED, true);
      } else {
        lv_obj_set_state(time, LV_STATE_DEFAULT, true);
        lv_obj_set_state(msecTime, LV_STATE_DEFAULT, true);
      }
    }
  }
}

void StopWatch::playPauseBtnEventHandler() {
  if (currentState == States::Init || currentState == States::Halted) {
    Start();
  } else if (currentState == States::Running) {
    Pause();
  }
}

void StopWatch::stopLapBtnEventHandler() {
  // If running, then this button is used to save laps
  if (currentState == States::Running) {
    lv_label_set_text(lapText, "");
    lapsDone = std::min(lapsDone + 1, maxLapCount);
    for (int i = lapsDone - displayedLaps; i < lapsDone; i++) {
      if (i < 0) {
        lv_label_ins_text(lapText, LV_LABEL_POS_LAST, "\n");
        continue;
      }
      TimeSeparated_t times = convertTicksToTimeSegments(laps[i]);
      char buffer[17];
      if (times.hours == 0) {
        snprintf(buffer, sizeof(buffer), "#%2d    %2d:%02d.%02d\n", i + 1, times.mins, times.secs, times.hundredths);
      } else {
        snprintf(buffer, sizeof(buffer), "#%2d %2d:%02d:%02d.%02d\n", i + 1, times.hours, times.mins, times.secs, times.hundredths);
      }
      lv_label_ins_text(lapText, LV_LABEL_POS_LAST, buffer);
    }
  } else if (currentState == States::Halted) {
    Reset();
  }
}

bool StopWatch::OnButtonPushed() {
  if (currentState == States::Running) {
    Pause();
    return true;
  }
  return false;
}
