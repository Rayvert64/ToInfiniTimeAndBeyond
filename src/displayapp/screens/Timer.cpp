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
  std::shared_ptr<lv_obj_t> colonLabel = std::make_shared<lv_obj_t>(lv_label_create(lv_scr_act()));
  lv_obj_set_style_text_font(colonLabel.get(), &jetbrains_mono_76, LV_STATE_DEFAULT);
  lv_obj_set_style_text_color(colonLabel.get(), lv_color_white(), LV_STATE_DEFAULT);
  lv_label_set_text_static(colonLabel.get(), ":");
  lv_obj_align(colonLabel.get(), LV_ALIGN_CENTER, 0, -29);

  CreateTimeCounters();

  btnPlayPause = std::make_shared<lv_obj_t>(lv_button_create(lv_scr_act()));
  lv_style_init(&styles_btn[BTN_STATE_IDLE]);
  lv_style_init(&styles_btn[BTN_STATE_SHORT_PRESS]);
  lv_style_init(&styles_btn[BTN_STATE_HELD]);
  CreatePlayPauseBtnStyles();
  AddPlayPauseBtnEvents();

  txtPlayPause = std::make_shared<lv_obj_t>(lv_label_create(lv_scr_act()));
  lv_obj_align_to(txtPlayPause.get(), btnPlayPause.get(), LV_ALIGN_CENTER, 0, 0);

  if (timer.IsRunning()) {
    SetTimerRunning();
  } else {
    SetTimerStopped();
  }

  taskRefresh = std::make_shared<lv_timer_t>(lv_timer_create(RefreshTaskCallback, LV_DEF_REFR_PERIOD, this));
}

inline void Timer::CreateTimeCounters() {
  minuteCounter = std::make_shared<lv_obj_t>(lv_roller_create(lv_scr_act()));
  secondCounter = std::make_shared<lv_obj_t>(lv_roller_create(lv_scr_act()));

  lv_roller_set_options(minuteCounter.get(), COUNTER_VALUES, LV_ROLLER_MODE_INFINITE);
  lv_roller_set_options(secondCounter.get(), COUNTER_VALUES, LV_ROLLER_MODE_INFINITE);

  lv_obj_align(minuteCounter.get(), LV_ALIGN_TOP_LEFT, 0, 0);
  lv_obj_align(secondCounter.get(), LV_ALIGN_TOP_RIGHT, 0, 0);
}

inline void Timer::CreatePlayPauseBtnStyles() {
  if (btnPlayPause.use_count() == 0) {
    return; // @TODO: We should change to use error returns rather than void returns
  }
  lv_obj_remove_style_all(btnPlayPause.get());
  lv_obj_set_align(btnPlayPause.get(), LV_ALIGN_BOTTOM_MID);
  lv_obj_set_pos(btnPlayPause.get(), 0, 0);
  lv_obj_set_size(btnPlayPause.get(), 240, 50);

  /* Default state */
  lv_obj_set_style_radius(btnPlayPause.get(), LV_RADIUS_CIRCLE, LV_STATE_ANY);
  lv_obj_set_style_bg_color(btnPlayPause.get(), Colors::bgAlt, LV_STATE_ANY);

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
  lv_style_transition_dsc_init(btnTransitionDescription.get(), btnTransitionElements, lv_anim_path_linear, 1000, 0, nullptr);
  lv_style_set_transition(&styles_btn[BTN_STATE_HELD], btnTransitionDescription.get());
}

inline void Timer::AddPlayPauseBtnEvents() {
  lv_obj_add_event_cb(btnPlayPause.get(), btnEventHandlerPressed, LV_EVENT_PRESSED, this);
  lv_obj_add_event_cb(btnPlayPause.get(), btnEventHandlerReleasedAndPressLost, LV_EVENT_RELEASED, this);
  lv_obj_add_event_cb(btnPlayPause.get(), btnEventHandlerReleasedAndPressLost, LV_EVENT_PRESS_LOST, this);
  lv_obj_add_event_cb(btnPlayPause.get(), btnEventHandlerPressedShortClicked, LV_EVENT_SHORT_CLICKED, this);
}

Timer::~Timer() {
  lv_timer_del(taskRefresh.get());
  lv_obj_clean(lv_scr_act());
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
    lv_label_set_text_static(txtPlayPause.get(), "Start");
  }
  maskPosition = 0;
}

void Timer::Refresh() {
  if (timer.IsRunning()) {
    auto secondsRemaining = std::chrono::duration_cast<std::chrono::seconds>(timer.GetTimeRemaining());
    lv_roller_set_selected(minuteCounter.get(), secondsRemaining.count() / 60, LV_ANIM_ON);
    lv_roller_set_selected(secondCounter.get(), secondsRemaining.count() % 60, LV_ANIM_ON);
  } else if (buttonPressing && xTaskGetTickCount() > pressTime + pdMS_TO_TICKS(150)) {
    lv_label_set_text_static(txtPlayPause.get(), "Reset");
    maskPosition += 15;
    if (maskPosition > 240) {
      MaskReset();
      Reset();
    }
  }
}

void Timer::SetTimerRunning() {
  lv_obj_remove_flag(minuteCounter.get(), LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_remove_flag(secondCounter.get(), LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
  lv_label_set_text_static(txtPlayPause.get(), "Pause");
}

void Timer::SetTimerStopped() {
  lv_obj_add_flag(minuteCounter.get(), LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(secondCounter.get(), LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
  lv_label_set_text_static(txtPlayPause.get(), "Start");
}

void Timer::ToggleRunning() {
  // The roller uses a 0-based index or chars that are exactly equal to the option string
  // how convienient!
  auto secondValue = lv_roller_get_selected(secondCounter.get());
  auto minuteValue = lv_roller_get_selected(minuteCounter.get());
  if (timer.IsRunning()) {
    auto secondsRemaining = std::chrono::duration_cast<std::chrono::seconds>(timer.GetTimeRemaining());
    lv_roller_set_selected(minuteCounter.get(), secondsRemaining.count() / 60, LV_ANIM_ON);
    lv_roller_set_selected(secondCounter.get(), secondsRemaining.count() % 60, LV_ANIM_ON);
    timer.StopTimer();
    SetTimerStopped();
  } else if (lv_roller_get_selected(secondCounter.get()) + lv_roller_get_selected(minuteCounter.get()) > 0) {
    auto timerDuration = std::chrono::minutes(minuteValue) + std::chrono::seconds(secondValue);
    timer.StartTimer(timerDuration);
    Refresh();
    SetTimerRunning();
  }
}

void Timer::Reset() {
  lv_roller_set_selected(minuteCounter.get(), 0, LV_ANIM_ON);
  lv_roller_set_selected(secondCounter.get(), 0, LV_ANIM_ON);
  SetTimerStopped();
}
