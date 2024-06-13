#include <lvgl/src/core/lv_obj.h>
#include <lvgl/src/widgets/roller/lv_roller.h>
#include <memory>
#include "displayapp/screens/Timer.h"
#include "displayapp/InfiniTimeTheme.h"
#include <lvgl/lvgl.h>

#define COUNTER_VALUES                                                                                                                     \
  "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n31\n32\n33" \
  "\n34\n35\n36\n37\n38\n39\n40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n51\n52\n53\n54\n55\n56\n57\n58\n59"

using namespace Pinetime::Applications::Screens;

static void btnEventHandlerPressed(lv_event_t* event) {
  auto* screen = static_cast<Timer*>(event->user_data);
  screen->ButtonPressed();
}

static void btnEventHandlerReleasedAndPressLost(lv_event_t* event) {
  auto* screen = static_cast<Timer*>(event->user_data);
  screen->MaskReset();
}

static void btnEventHandlerPressedShortClicked(lv_event_t* event) {
  auto* screen = static_cast<Timer*>(event->user_data);
  screen->ToggleRunning();
}

Timer::Timer(Controllers::Timer& timerController) : timer {timerController} {
  lv_obj_t* colonLabel = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_font(colonLabel, &jetbrains_mono_76, LV_STATE_DEFAULT);
  lv_obj_set_style_text_color(colonLabel, lv_color_white(), LV_STATE_DEFAULT);
  lv_label_set_text_static(colonLabel, ":");
  lv_obj_align(colonLabel, LV_ALIGN_CENTER, 0, -29);

  CreateTimeCounters();

  btnPlayPause = lv_button_create(lv_screen_active());
  lv_style_init(&styles_btn[BTN_STATE_IDLE]);
  lv_style_init(&styles_btn[BTN_STATE_SHORT_PRESS]);
  lv_style_init(&styles_btn[BTN_STATE_HELD]);
  CreatePlayPauseBtnStyles();
  AddPlayPauseBtnEvents();

  txtPlayPause = lv_label_create(lv_screen_active());
  lv_obj_align_to(txtPlayPause, btnPlayPause, LV_ALIGN_CENTER, 0, 0);

  if (timer.IsRunning()) {
    SetTimerRunning();
  } else {
    SetTimerStopped();
  }

  taskRefresh = lv_timer_create(RefreshTaskCallback, LV_DEF_REFR_PERIOD, this);
}

inline void Timer::CreateTimeCounters() {
  minuteCounter = lv_roller_create(lv_screen_active());
  secondCounter = lv_roller_create(lv_screen_active());

  lv_roller_set_options(minuteCounter, COUNTER_VALUES, LV_ROLLER_MODE_INFINITE);
  lv_roller_set_options(secondCounter, COUNTER_VALUES, LV_ROLLER_MODE_INFINITE);

  lv_obj_align(minuteCounter, LV_ALIGN_TOP_LEFT, 0, 0);
  lv_obj_align(secondCounter, LV_ALIGN_TOP_RIGHT, 0, 0);
}

inline void Timer::CreatePlayPauseBtnStyles() {
  lv_obj_remove_style_all(btnPlayPause);
  lv_obj_set_align(btnPlayPause, LV_ALIGN_BOTTOM_MID);
  lv_obj_set_pos(btnPlayPause, 0, 0);
  lv_obj_set_size(btnPlayPause, 240, 50);

  /* Default state */
  lv_obj_set_style_radius(btnPlayPause, LV_RADIUS_CIRCLE, LV_STATE_ANY);
  lv_obj_set_style_bg_color(btnPlayPause, Colors::bgAlt, LV_STATE_ANY);

  lv_style_set_radius(&styles_btn[BTN_STATE_IDLE], 0);
  lv_style_set_bg_color(&styles_btn[BTN_STATE_IDLE], Colors::bgAlt);
  lv_style_set_bg_opa(&styles_btn[BTN_STATE_IDLE], LV_OPA_COVER);
  lv_style_set_text_color(&styles_btn[BTN_STATE_IDLE], lv_color_white());
  lv_style_set_text_font(&styles_btn[BTN_STATE_IDLE], &jetbrains_mono_76);

  lv_style_set_radius(&styles_btn[BTN_STATE_SHORT_PRESS], 0);
  lv_style_set_bg_color(&styles_btn[BTN_STATE_SHORT_PRESS], Colors::bgAlt);
  lv_style_set_bg_opa(&styles_btn[BTN_STATE_SHORT_PRESS], LV_OPA_COVER);
  lv_style_set_text_color(&styles_btn[BTN_STATE_SHORT_PRESS], lv_color_white());
  lv_style_set_text_font(&styles_btn[BTN_STATE_SHORT_PRESS], &jetbrains_mono_76);

  lv_style_set_radius(&styles_btn[BTN_STATE_HELD], 0);
  lv_style_set_bg_color(&styles_btn[BTN_STATE_HELD], Colors::bgAlt);
  lv_style_set_bg_opa(&styles_btn[BTN_STATE_HELD], LV_OPA_COVER);
  lv_style_set_text_color(&styles_btn[BTN_STATE_HELD], lv_color_white());
  lv_style_set_text_font(&styles_btn[BTN_STATE_HELD], &jetbrains_mono_76);
  lv_style_transition_dsc_init(btnTransitionDescription, btnTransitionElements, lv_anim_path_linear, 1000, 0, nullptr);
  lv_style_set_transition(&styles_btn[BTN_STATE_HELD], btnTransitionDescription);
}

inline void Timer::AddPlayPauseBtnEvents() {
  lv_obj_add_event_cb(btnPlayPause, btnEventHandlerPressed, LV_EVENT_PRESSED, this);
  lv_obj_add_event_cb(btnPlayPause, btnEventHandlerReleasedAndPressLost, LV_EVENT_RELEASED, this);
  lv_obj_add_event_cb(btnPlayPause, btnEventHandlerReleasedAndPressLost, LV_EVENT_PRESS_LOST, this);
  lv_obj_add_event_cb(btnPlayPause, btnEventHandlerPressedShortClicked, LV_EVENT_SHORT_CLICKED, this);
}

Timer::~Timer() {
  lv_timer_del(taskRefresh);
  lv_obj_clean(lv_screen_active());
}

void Timer::ButtonPressed() {
  pressTime = xTaskGetTickCount();
  buttonPressing = true;
}

void Timer::MaskReset() {
  buttonPressing = false;
  // A click event is processed before a release event,
  // so the release event would override the "Pause" text without this check
  if (!timer.IsRunning()) {
    lv_label_set_text_static(txtPlayPause, "Start");
  }
  maskPosition = 0;
}

void Timer::Refresh() {
  if (timer.IsRunning()) {
    auto secondsRemaining = std::chrono::duration_cast<std::chrono::seconds>(timer.GetTimeRemaining());
    lv_roller_set_selected(minuteCounter, secondsRemaining.count() / 60, LV_ANIM_ON);
    lv_roller_set_selected(secondCounter, secondsRemaining.count() % 60, LV_ANIM_ON);
  } else if (buttonPressing && xTaskGetTickCount() > pressTime + pdMS_TO_TICKS(150)) {
    lv_label_set_text_static(txtPlayPause, "Reset");
    maskPosition += 15;
    if (maskPosition > 240) {
      MaskReset();
      Reset();
    }
  }
}

void Timer::SetTimerRunning() {
  lv_obj_remove_flag(minuteCounter, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_remove_flag(secondCounter, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
  lv_label_set_text_static(txtPlayPause, "Pause");
}

void Timer::SetTimerStopped() {
  lv_obj_add_flag(minuteCounter, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(secondCounter, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
  lv_label_set_text_static(txtPlayPause, "Start");
}

void Timer::ToggleRunning() {
  // The roller uses a 0-based index or chars that are exactly equal to the option string
  // how convienient!
  auto secondValue = lv_roller_get_selected(secondCounter);
  auto minuteValue = lv_roller_get_selected(minuteCounter);
  if (timer.IsRunning()) {
    auto secondsRemaining = std::chrono::duration_cast<std::chrono::seconds>(timer.GetTimeRemaining());
    lv_roller_set_selected(minuteCounter, secondsRemaining.count() / 60, LV_ANIM_ON);
    lv_roller_set_selected(secondCounter, secondsRemaining.count() % 60, LV_ANIM_ON);
    timer.StopTimer();
    SetTimerStopped();
  } else if (lv_roller_get_selected(secondCounter) + lv_roller_get_selected(minuteCounter) > 0) {
    auto timerDuration = std::chrono::minutes(minuteValue) + std::chrono::seconds(secondValue);
    timer.StartTimer(timerDuration);
    Refresh();
    SetTimerRunning();
  }
}

void Timer::Reset() {
  lv_roller_set_selected(minuteCounter, 0, LV_ANIM_ON);
  lv_roller_set_selected(secondCounter, 0, LV_ANIM_ON);
  SetTimerStopped();
}
