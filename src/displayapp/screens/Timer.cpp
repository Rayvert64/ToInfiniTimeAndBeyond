#include <memory>
#include "displayapp/screens/Timer.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"
#include <lvgl/lvgl.h>

using namespace Pinetime::Applications::Screens;

static void btnEventHandler(lv_obj_t* obj, lv_event_t event) {
  auto* screen = static_cast<Timer*>(obj->user_data);
  if (event == LV_EVENT_PRESSED) {
    screen->ButtonPressed();
  } else if (event == LV_EVENT_RELEASED || event == LV_EVENT_PRESS_LOST) {
    screen->MaskReset();
  } else if (event == LV_EVENT_SHORT_CLICKED) {
    screen->ToggleRunning();
  }
}

Timer::Timer(Controllers::Timer& timerController) : timer {timerController} {

  lv_obj_t* colonLabel = std::make_shared<lv_obj_t>(lv_label_create(lv_scr_act()));
  lv_obj_set_style_local_text_font(colonLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_obj_set_style_local_text_color(colonLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_white());
  lv_label_set_text_static(colonLabel, ":");
  lv_obj_align(colonLabel, lv_scr_act(), LV_ALIGN_CENTER, 0, -29);

  minuteCounter.Create();
  secondCounter.Create();
  lv_obj_align(minuteCounter.GetObject(), LV_ALIGN_TOP_LEFT, 0, 0);
  lv_obj_align(secondCounter.GetObject(), LV_ALIGN_TOP_RIGHT, 0, 0);

  btnPlayPause = std::make_shared<lv_obj_t>(lv_button_create(lv_scr_act()));

  /* Short clicked */
  lv_obj_add_style(btnPlayPause, &style_btn, LV_STATE_DEFAULT);
  lv_obj_add_style(btnPlayPause, &style_button_pressed, LV_EVENT_SHORT_CLICKED);

  /*
    highlightObjectMask = lv_objmask_create(lv_scr_act(), nullptr);
    lv_obj_set_size(highlightObjectMask, 240, 50);
    lv_obj_align(highlightObjectMask, lv_scr_act(), LV_ALIGN_BOTTOM_MID, 0, 0);

    lv_obj_t* btnHighlight = lv_obj_create(highlightObjectMask);;
    lv_obj_set_style_radius(btnHighlight, LV_RADIUS_CIRCLE, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btnHighlight, PINETIME_COLOR_ORANGE, LV_PART_MAIN);
    lv_obj_set_size(btnHighlight, LV_HOR_RES, 50);
    lv_obj_align(btnHighlight, lv_scr_act(), LV_ALIGN_BOTTOM_MID, 0, 0);

    btnObjectMask = lv_objmask_create(lv_scr_act(), nullptr);
    lv_obj_set_size(btnObjectMask, 240, 50);
    lv_obj_align(btnObjectMask, lv_scr_act(), LV_ALIGN_BOTTOM_MID, 0, 0);

    btnPlayPause = lv_btn_create(btnObjectMask, nullptr);
    btnPlayPause->user_data = this;
    lv_obj_set_style_radius(btnPlayPause, LV_RADIUS_CIRCLE, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btnPlayPause, Colors::bgAlt, LV_PART_MAIN);
    lv_obj_set_event_cb(btnPlayPause, btnEventHandler);
    lv_obj_set_size(btnPlayPause, LV_HOR_RES, 50);
  */

  lv_obj_add_event_cb(btnPlayPause, ButtonPressed, LV_EVENT_PRESSED, this);
  lv_obj_add_event_cb(btnPlayPause, MaskReset, LV_EVENT_RELEASED | LV_EVENT_PRESS_LOST, this);
  lv_obj_add_event_cb(btnPlayPause, ToggleRunning, LV_EVENT_SHORT_CLICKED, this);

  txtPlayPause = lv_label_create(lv_scr_act());
  lv_obj_align(txtPlayPause, btnPlayPause, LV_ALIGN_CENTER, 0, 0);

  if (timer.IsRunning()) {
    SetTimerRunning();
  } else {
    SetTimerStopped();
  }

  taskRefresh = lv_timer_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, this);
}

void Timer::CreatePlayPauseBtnStyles() {
  if (!btnPlayPause.use_count()) {
    return; // @TODO: We should change to use error returns rather than void returns
  }
  lv_obj_remove_style_all(btnPlayPause);
  lv_obj_set_align(btnPlayPause, LV_ALIGN_BOTTOM_MID);
  lv_obj_set_pos(btnPlayPause, 0, 0);
  lv_obj_set_size(btnPlayPause, 240, 50);

  /* Default state */
  lv_obj_set_style_radius(btnPlayPause, LV_RADIUS_CIRCLE, LV_STATE_ANY);
  lv_obj_set_style_bg_color(btnPlayPause, Colors::bgAlt, LV_STATE_ANY);

  lv_style_transition_dsc_init(btnTransitionDescription, btnTransitionElements, lv_anim_path_linear, 1000, 0, NULL);

  lv_style_init(&styles_btn[BTN_STATE_IDLE]);
  lv_style_set_radius(&styles_btn run-- rm - it - v $ {PWD}
                      : / sources-- user $(id - u)
                      : $(id - g) infinitime - build[BTN_STATE_IDLE], LV_STATE_DEFAULT, 0);
  lv_style_set_bg_color(&styles_btn[BTN_STATE_IDLE], LV_STATE_DEFAULT, Colors::bgAlt);
  lv_style_set_bg_opa(&styles_btn[BTN_STATE_IDLE], LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_text_color(&styles_btn[BTN_STATE_IDLE], LV_STATE_DEFAULT, Colors::fg);
  lv_style_set_text_font(&styles_btn[BTN_STATE_IDLE], LV_STATE_DEFAULT, &jetbrains_mono_76);

  lv_style_init(&styles_btn[BTN_STATE_PRESSED]);
  lv_style_set_radius(&styles_btn[BTN_STATE_PRESSED], LV_STATE_DEFAULT, 0);
  lv_style_set_bg_color(&styles_btn[BTN_STATE_PRESSED], LV_STATE_DEFAULT, Colors::bgAlt);
  lv_style_set_bg_opa(&styles_btn[BTN_STATE_PRESSED], LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_text_color(&styles_btn[BTN_STATE_PRESSED], LV_STATE_DEFAULT, Colors::fg);
  lv_style_set_text_font(&styles_btn[BTN_STATE_PRESSED], LV_STATE_DEFAULT, &jetbrains_mono_76);

  lv_style_init(&styles_btn[BTN_STATE_HELD]);
  lv_style_set_radius(&styles_btn[BTN_STATE_HELD], LV_STATE_DEFAULT, 0);
  lv_style_set_bg_color(&styles_btn[BTN_STATE_HELD], LV_STATE_DEFAULT, Colors::bgAlt);
  lv_style_set_bg_opa(&styles_btn[BTN_STATE_HELD], LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_text_color(&styles_btn[BTN_STATE_HELD], LV_STATE_DEFAULT, Colors::fg);
  lv_style_set_text_font(&styles_btn[BTN_STATE_HELD], LV_STATE_DEFAULT, &jetbrains_mono_76);
}

Timer::~Timer() {
  lv_task_del(taskRefresh);
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
    lv_label_set_text_static(txtPlayPause, "Start");
  }
  maskPosition = 0;
  UpdateMask();
}

void Timer::UpdateMask() {
  lv_draw_mask_line_param_t maskLine;

  lv_draw_mask_line_points_init(&maskLine, maskPosition, 0, maskPosition, 240, LV_DRAW_MASK_LINE_SIDE_LEFT);
  lv_objmask_update_mask(highlightObjectMask, highlightMask, &maskLine);

  lv_draw_mask_line_points_init(&maskLine, maskPosition, 0, maskPosition, 240, LV_DRAW_MASK_LINE_SIDE_RIGHT);
  lv_objmask_update_mask(btnObjectMask, btnMask, &maskLine);
}

void Timer::Refresh() {
  if (timer.IsRunning()) {
    auto secondsRemaining = std::chrono::duration_cast<std::chrono::seconds>(timer.GetTimeRemaining());
    minuteCounter.SetValue(secondsRemaining.count() / 60);
    secondCounter.SetValue(secondsRemaining.count() % 60);
  } else if (buttonPressing && xTaskGetTickCount() > pressTime + pdMS_TO_TICKS(150)) {
    lv_label_set_text_static(txtPlayPause, "Reset");
    maskPosition += 15;
    if (maskPosition > 240) {
      MaskReset();
      Reset();
    } else {
      UpdateMask();
    }
  }
}

void Timer::SetTimerRunning() {
  minuteCounter.HideControls();
  secondCounter.HideControls();
  lv_label_set_text_static(txtPlayPause, "Pause");
}

void Timer::SetTimerStopped() {
  minuteCounter.ShowControls();
  secondCounter.ShowControls();
  lv_label_set_text_static(txtPlayPause, "Start");
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
