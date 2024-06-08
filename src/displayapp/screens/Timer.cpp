#include <memory>
#include "displayapp/screens/Timer.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"
#include <lvgl/lvgl.h>

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

  minuteCounter.Create();
  secondCounter.Create();
  lv_obj_align(minuteCounter.GetObject(), LV_ALIGN_TOP_LEFT, 0, 0);
  lv_obj_align(secondCounter.GetObject(), LV_ALIGN_TOP_RIGHT, 0, 0);

  btnPlayPause = std::make_shared<lv_obj_t>(lv_button_create(lv_scr_act()));
  CreatePlayPauseBtnStyles();
  AddPlayPauseBtnEvents();

  txtPlayPause = std::make_shared<lv_obj_t>(lv_label_create(lv_scr_act()));
  lv_obj_align_to(txtPlayPause.get(), btnPlayPause.get(), LV_ALIGN_CENTER, 0, 0);

  if (timer.IsRunning()) {
    SetTimerRunning();
  } else {
    SetTimerStopped();
  }

  taskRefresh = lv_timer_create(RefreshTaskCallback, LV_DEF_REFR_PERIOD, this);
}

void Timer::CreatePlayPauseBtnStyles() {
  if (!btnPlayPause.use_count()) {
    return; // @TODO: We should change to use error returns rather than void returns
  }
  lv_obj_remove_style_all(btnPlayPause.get());
  lv_obj_set_align(btnPlayPause.get(), LV_ALIGN_BOTTOM_MID);
  lv_obj_set_pos(btnPlayPause.get(), 0, 0);
  lv_obj_set_size(btnPlayPause.get(), 240, 50);

  /* Default state */
  lv_obj_set_style_radius(btnPlayPause.get(), LV_RADIUS_CIRCLE, LV_STATE_ANY);
  lv_obj_set_style_bg_color(btnPlayPause.get(), Colors::bgAlt, LV_STATE_ANY);

  lv_style_init(&styles_btn[BTN_STATE_IDLE]);
  lv_style_set_radius(&styles_btn[BTN_STATE_IDLE], 0);
  lv_style_set_bg_color(&styles_btn[BTN_STATE_IDLE], Colors::bgAlt);
  lv_style_set_bg_opa(&styles_btn[BTN_STATE_IDLE], LV_OPA_COVER);
  lv_style_set_text_color(&styles_btn[BTN_STATE_IDLE], lv_color_white());
  lv_style_set_text_font(&styles_btn[BTN_STATE_IDLE], &jetbrains_mono_76);

  lv_style_init(&styles_btn[BTN_STATE_SHORT_PRESS]);
  lv_style_set_radius(&styles_btn[BTN_STATE_SHORT_PRESS], 0);
  lv_style_set_bg_color(&styles_btn[BTN_STATE_SHORT_PRESS], Colors::bgAlt);
  lv_style_set_bg_opa(&styles_btn[BTN_STATE_SHORT_PRESS], LV_OPA_COVER);
  lv_style_set_text_color(&styles_btn[BTN_STATE_SHORT_PRESS], lv_color_white());
  lv_style_set_text_font(&styles_btn[BTN_STATE_SHORT_PRESS], &jetbrains_mono_76);

  lv_style_init(&styles_btn[BTN_STATE_HELD]);
  lv_style_set_radius(&styles_btn[BTN_STATE_HELD], 0);
  lv_style_set_bg_color(&styles_btn[BTN_STATE_HELD], Colors::bgAlt);
  lv_style_set_bg_opa(&styles_btn[BTN_STATE_HELD], LV_OPA_COVER);
  lv_style_set_text_color(&styles_btn[BTN_STATE_HELD], lv_color_white());
  lv_style_set_text_font(&styles_btn[BTN_STATE_HELD], &jetbrains_mono_76);
  lv_style_transition_dsc_init(btnTransitionDescription.get(), btnTransitionElements, lv_anim_path_linear, 1000, 0, NULL);
  lv_style_set_transition(&styles_btn[BTN_STATE_HELD], btnTransitionDescription.get());
}

void Timer::AddPlayPauseBtnEvents() {
  lv_obj_add_event_cb(btnPlayPause.get(), btnEventHandlerPressed, LV_EVENT_PRESSED, this);
  lv_obj_add_event_cb(btnPlayPause.get(), btnEventHandlerReleasedAndPressLost, LV_EVENT_RELEASED, this);
  lv_obj_add_event_cb(btnPlayPause.get(), btnEventHandlerReleasedAndPressLost, LV_EVENT_PRESS_LOST, this);
  lv_obj_add_event_cb(btnPlayPause.get(), btnEventHandlerPressedShortClicked, LV_EVENT_SHORT_CLICKED, this);
}

Timer::~Timer() {
  lv_timer_del(taskRefresh);
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
    minuteCounter.SetValue(secondsRemaining.count() / 60);
    secondCounter.SetValue(secondsRemaining.count() % 60);
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
  minuteCounter.HideControls();
  secondCounter.HideControls();
  lv_label_set_text_static(txtPlayPause.get(), "Pause");
}

void Timer::SetTimerStopped() {
  minuteCounter.ShowControls();
  secondCounter.ShowControls();
  lv_label_set_text_static(txtPlayPause.get(), "Start");
}

void Timer::ToggleRunning() {
  if (timer.IsRunning()) {
    auto secondsRemaining = std::chrono::duration_cast<std::chrono::seconds>(timer.GetTimeRemaining());
    minuteCounter.SetValue(secondsRemaining.count() / 60);
    secondCounter.SetValue(secondsRemaining.count() % 60);
    timer.StopTimer();
    SetTimerStopped();
  } else if (secondCounter.GetValue() + minuteCounter.GetValue() > 0) {
    auto timerDuration = std::chrono::minutes(minuteCounter.GetValue()) + std::chrono::seconds(secondCounter.GetValue());
    timer.StartTimer(timerDuration);
    Refresh();
    SetTimerRunning();
  }
}

void Timer::Reset() {
  minuteCounter.SetValue(0);
  secondCounter.SetValue(0);
  SetTimerStopped();
}
