#include "displayapp/screens/WatchFaceInfineat.h"

#include <cstdint>
#include <lvgl/lvgl.h>
#include "lvgl/src/core/lv_obj.h"
#include <cstdio>
#include <lvgl/src/core/lv_obj_event.h>
#include <lvgl/src/misc/lv_event.h>
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/BleIcon.h"
#include "components/settings/Settings.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/motion/MotionController.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void event_handler(lv_event_t* event) {
    auto* screen = static_cast<WatchFaceInfineat*>(event->user_data);
    screen->UpdateSelected(lv_event_get_target_obj(event), event);
  }

  enum class colors {
    orange,
    blue,
    green,
    rainbow,
    gray,
    nordBlue,
    nordGreen,
  };

  constexpr int nColors = 7; // must match number of colors in InfineatColors

  constexpr int nLines = WatchFaceInfineat::nLines;

  constexpr std::array<lv_color_t, nLines> orangeColors = {lv_color_t LV_COLOR_MAKE(0xfd, 0x87, 0x2b),
                                                           lv_color_t LV_COLOR_MAKE(0xdb, 0x33, 0x16),
                                                           lv_color_t LV_COLOR_MAKE(0x6f, 0x10, 0x00),
                                                           lv_color_t LV_COLOR_MAKE(0xfd, 0x7a, 0x0a),
                                                           lv_color_t LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                           lv_color_t LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                           lv_color_t LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                           lv_color_t LV_COLOR_MAKE(0xe8, 0x51, 0x02),
                                                           lv_color_t LV_COLOR_MAKE(0xea, 0x1c, 0x00)};
  constexpr std::array<lv_color_t, nLines> blueColors = {lv_color_t LV_COLOR_MAKE(0xe7, 0xf8, 0xff),
                                                         lv_color_t LV_COLOR_MAKE(0x22, 0x32, 0xd0),
                                                         lv_color_t LV_COLOR_MAKE(0x18, 0x2a, 0x8b),
                                                         lv_color_t LV_COLOR_MAKE(0xe7, 0xf8, 0xff),
                                                         lv_color_t LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                         lv_color_t LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                         lv_color_t LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                         lv_color_t LV_COLOR_MAKE(0x59, 0x91, 0xff),
                                                         lv_color_t LV_COLOR_MAKE(0x16, 0x36, 0xff)};
  constexpr std::array<lv_color_t, nLines> greenColors = {lv_color_t LV_COLOR_MAKE(0xb8, 0xff, 0x9b),
                                                          lv_color_t LV_COLOR_MAKE(0x08, 0x86, 0x08),
                                                          lv_color_t LV_COLOR_MAKE(0x00, 0x4a, 0x00),
                                                          lv_color_t LV_COLOR_MAKE(0xb8, 0xff, 0x9b),
                                                          lv_color_t LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                          lv_color_t LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                          lv_color_t LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                          lv_color_t LV_COLOR_MAKE(0x62, 0xd5, 0x15),
                                                          lv_color_t LV_COLOR_MAKE(0x00, 0x74, 0x00)};
  constexpr std::array<lv_color_t, nLines> rainbowColors = {lv_color_t LV_COLOR_MAKE(0x2d, 0xa4, 0x00),
                                                            lv_color_t LV_COLOR_MAKE(0xac, 0x09, 0xc4),
                                                            lv_color_t LV_COLOR_MAKE(0xfe, 0x03, 0x03),
                                                            lv_color_t LV_COLOR_MAKE(0x0d, 0x57, 0xff),
                                                            lv_color_t LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                            lv_color_t LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                            lv_color_t LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                            lv_color_t LV_COLOR_MAKE(0xe0, 0xb9, 0x00),
                                                            lv_color_t LV_COLOR_MAKE(0xe8, 0x51, 0x02)};
  constexpr std::array<lv_color_t, nLines> grayColors = {lv_color_t LV_COLOR_MAKE(0xee, 0xee, 0xee),
                                                         lv_color_t LV_COLOR_MAKE(0x98, 0x95, 0x9b),
                                                         lv_color_t LV_COLOR_MAKE(0x19, 0x19, 0x19),
                                                         lv_color_t LV_COLOR_MAKE(0xee, 0xee, 0xee),
                                                         lv_color_t LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                         lv_color_t LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                         lv_color_t LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                         lv_color_t LV_COLOR_MAKE(0x91, 0x91, 0x91),
                                                         lv_color_t LV_COLOR_MAKE(0x3a, 0x3a, 0x3a)};
  constexpr std::array<lv_color_t, nLines> nordBlueColors = {lv_color_t LV_COLOR_MAKE(0xc3, 0xda, 0xf2),
                                                             lv_color_t LV_COLOR_MAKE(0x4d, 0x78, 0xce),
                                                             lv_color_t LV_COLOR_MAKE(0x15, 0x34, 0x51),
                                                             lv_color_t LV_COLOR_MAKE(0xc3, 0xda, 0xf2),
                                                             lv_color_t LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                             lv_color_t LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                             lv_color_t LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                             lv_color_t LV_COLOR_MAKE(0x5d, 0x8a, 0xd2),
                                                             lv_color_t LV_COLOR_MAKE(0x21, 0x51, 0x8a)};
  constexpr std::array<lv_color_t, nLines> nordGreenColors = {lv_color_t LV_COLOR_MAKE(0xd5, 0xf0, 0xe9),
                                                              lv_color_t LV_COLOR_MAKE(0x23, 0x83, 0x73),
                                                              lv_color_t LV_COLOR_MAKE(0x1d, 0x41, 0x3f),
                                                              lv_color_t LV_COLOR_MAKE(0xd5, 0xf0, 0xe9),
                                                              lv_color_t LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                              lv_color_t LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                              lv_color_t LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                              lv_color_t LV_COLOR_MAKE(0x2f, 0xb8, 0xa2),
                                                              lv_color_t LV_COLOR_MAKE(0x11, 0x70, 0x5a)};

  constexpr const std::array<lv_color_t, nLines>* returnColor(colors color) {
    if (color == colors::orange) {
      return &orangeColors;
    }
    if (color == colors::blue) {
      return &blueColors;
    }
    if (color == colors::green) {
      return &greenColors;
    }
    if (color == colors::rainbow) {
      return &rainbowColors;
    }
    if (color == colors::gray) {
      return &grayColors;
    }
    if (color == colors::nordBlue) {
      return &nordBlueColors;
    }
    return &nordGreenColors;
  }
}

WatchFaceInfineat::WatchFaceInfineat(Controllers::DateTime& dateTimeController,
                                     const Controllers::Battery& batteryController,
                                     const Controllers::Ble& bleController,
                                     Controllers::NotificationManager& notificationManager,
                                     Controllers::Settings& settingsController,
                                     Controllers::MotionController& motionController,
                                     Controllers::FS& filesystem)
  : currentDateTime {{}},
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    notificationManager {notificationManager},
    settingsController {settingsController},
    motionController {motionController} {
  lfs_file f = {};
  if (filesystem.FileOpen(&f, "/fonts/teko.bin", LFS_O_RDONLY) >= 0) {
    filesystem.FileClose(&f);
    font_teko = lv_binfont_create("F:/fonts/teko.bin");
  }

  if (filesystem.FileOpen(&f, "/fonts/bebas.bin", LFS_O_RDONLY) >= 0) {
    filesystem.FileClose(&f);
    font_bebas = lv_binfont_create("F:/fonts/bebas.bin");
  }

  // Side Cover
  static constexpr lv_point_precise_t linePoints[nLines][2] = {{{30, 25}, {68, -8}},
                                                               {{26, 167}, {43, 216}},
                                                               {{27, 40}, {27, 196}},
                                                               {{12, 182}, {65, 249}},
                                                               {{17, 99}, {17, 144}},
                                                               {{14, 81}, {40, 127}},
                                                               {{14, 163}, {40, 118}},
                                                               {{-20, 124}, {25, -11}},
                                                               {{-29, 89}, {27, 254}}};

  static constexpr uint8_t lineWidths[nLines] = {18, 15, 14, 22, 20, 18, 18, 52, 48};

  const std::array<lv_color_t, nLines>* colors = returnColor(static_cast<enum colors>(settingsController.GetInfineatColorIndex()));
  for (int i = 0; i < nLines; i++) {
    lines[i] = lv_line_create(lv_screen_active());
    lv_obj_set_style_line_width(lines[i], lineWidths[i], LV_STATE_DEFAULT);
    lv_color_t color = (*colors)[i];
    lv_obj_set_style_line_color(lines[i], color, LV_STATE_DEFAULT);
    lv_line_set_points(lines[i], linePoints[i], 2);
  }

  logoPine = lv_img_create(lv_screen_active());
  lv_img_set_src(logoPine, "F:/images/pine_small.bin");
  lv_obj_set_pos(logoPine, 15, 106);

  lineBattery = lv_line_create(lv_screen_active());
  lv_obj_set_style_line_width(lineBattery, 24, LV_STATE_DEFAULT);
  lv_obj_set_style_line_color(lineBattery, (*colors)[4], LV_STATE_DEFAULT);
  lv_obj_set_style_line_opa(lineBattery, 190, LV_STATE_DEFAULT);
  ;
  lineBatteryPoints[0] = {27, 105};
  lineBatteryPoints[1] = {27, 106};
  lv_line_set_points(lineBattery, lineBatteryPoints, 2);
  lv_obj_move_foreground(lineBattery);

  notificationIcon = lv_obj_create(lv_screen_active());
  lv_obj_set_style_bg_color(notificationIcon, (*colors)[7], LV_PART_MAIN);
  lv_obj_set_style_radius(notificationIcon, LV_RADIUS_CIRCLE, LV_STATE_DEFAULT);
  lv_obj_set_size(notificationIcon, 13, 13);
  lv_obj_add_flag(notificationIcon, LV_OBJ_FLAG_HIDDEN);

  if (!settingsController.GetInfineatShowSideCover()) {
    ToggleBatteryIndicatorColor(false);
    for (auto& line : lines) {
      lv_obj_add_flag(line, LV_OBJ_FLAG_HIDDEN);
    }
  }

  timeContainer = lv_obj_create(lv_screen_active());
  lv_obj_set_style_bg_opa(timeContainer, LV_OPA_TRANSP, LV_STATE_DEFAULT);
  lv_obj_set_size(timeContainer, 185, 185);
  lv_obj_align(timeContainer, LV_ALIGN_CENTER, 0, -10);

  labelHour = lv_label_create(lv_screen_active());
  lv_label_set_text_static(labelHour, "01");
  lv_obj_set_style_text_font(labelHour, font_bebas, LV_STATE_DEFAULT);
  lv_obj_align(labelHour, LV_ALIGN_TOP_MID, 0, 0);

  labelMinutes = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_font(labelMinutes, font_bebas, LV_STATE_DEFAULT);
  lv_label_set_text_static(labelMinutes, "00");
  lv_obj_align(labelMinutes, LV_ALIGN_BOTTOM_MID, 0, 0);

  labelTimeAmPm = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_font(labelTimeAmPm, font_teko, LV_STATE_DEFAULT);

  lv_label_set_text_static(labelTimeAmPm, "");
  lv_obj_align(labelTimeAmPm, LV_ALIGN_OUT_RIGHT_TOP, 0, 15);

  dateContainer = lv_obj_create(lv_screen_active());
  lv_obj_set_style_bg_opa(dateContainer, LV_OPA_TRANSP, LV_STATE_DEFAULT);
  lv_obj_set_size(dateContainer, 60, 30);
  lv_obj_align(dateContainer, LV_ALIGN_RIGHT_MID, 0, 5);

  static constexpr lv_color_t grayColor = LV_COLOR_MAKE(0x99, 0x99, 0x99);
  labelDate = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(labelDate, grayColor, LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(labelDate, font_teko, LV_STATE_DEFAULT);
  lv_obj_align(labelDate, LV_ALIGN_TOP_MID, 0, 0);
  lv_label_set_text_static(labelDate, "Mon 01");

  bleIcon = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(bleIcon, grayColor, LV_STATE_DEFAULT);
  lv_label_set_text_static(bleIcon, Symbols::bluetooth);
  lv_obj_align(bleIcon, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

  stepValue = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(stepValue, grayColor, LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(stepValue, font_teko, LV_STATE_DEFAULT);
  lv_obj_align(stepValue, LV_ALIGN_BOTTOM_RIGHT, 10, 0);
  lv_label_set_text_static(stepValue, "0");

  stepIcon = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(stepIcon, grayColor, LV_STATE_DEFAULT);
  lv_label_set_text_static(stepIcon, Symbols::shoe);
  lv_obj_align(stepIcon, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  // Setting buttons
  btnClose = lv_button_create(lv_screen_active());
  btnClose->user_data = this;
  lv_obj_set_size(btnClose, 60, 60);
  lv_obj_align(btnClose, LV_ALIGN_CENTER, 0, -80);
  lv_obj_set_style_bg_opa(btnClose, LV_OPA_70, LV_STATE_DEFAULT);
  lv_obj_t* lblClose = lv_label_create(btnClose);
  lv_label_set_text_static(lblClose, "X");
  lv_obj_add_event_cb(btnClose, event_handler, LV_EVENT_CLICKED, this);
  lv_obj_add_flag(btnClose, LV_OBJ_FLAG_HIDDEN);

  btnNextColor = lv_button_create(lv_screen_active());
  btnNextColor->user_data = this;
  lv_obj_set_size(btnNextColor, 60, 60);
  lv_obj_align(btnNextColor, LV_ALIGN_RIGHT_MID, -15, 0);
  lv_obj_set_style_bg_opa(btnNextColor, LV_OPA_70, LV_STATE_DEFAULT);
  lv_obj_t* lblNextColor = lv_label_create(btnNextColor);
  lv_label_set_text_static(lblNextColor, ">");
  lv_obj_add_event_cb(btnNextColor, event_handler, LV_EVENT_CLICKED, this);
  lv_obj_add_flag(btnNextColor, LV_OBJ_FLAG_HIDDEN);

  btnPrevColor = lv_button_create(lv_screen_active());
  btnPrevColor->user_data = this;
  lv_obj_set_size(btnPrevColor, 60, 60);
  lv_obj_align(btnPrevColor, LV_ALIGN_LEFT_MID, 15, 0);
  lv_obj_set_style_bg_opa(btnPrevColor, LV_OPA_70, LV_STATE_DEFAULT);
  lv_obj_t* lblPrevColor = lv_label_create(btnPrevColor);
  lv_label_set_text_static(lblPrevColor, "<");
  lv_obj_add_event_cb(btnPrevColor, event_handler, LV_EVENT_CLICKED, this);
  lv_obj_add_flag(btnPrevColor, LV_OBJ_FLAG_HIDDEN);

  btnToggleCover = lv_button_create(lv_screen_active());
  btnToggleCover->user_data = this;
  lv_obj_set_size(btnToggleCover, 60, 60);
  lv_obj_align(btnToggleCover, LV_ALIGN_CENTER, 0, 80);
  lv_obj_set_style_bg_opa(btnToggleCover, LV_OPA_70, LV_STATE_DEFAULT);
  const char* labelToggle = settingsController.GetInfineatShowSideCover() ? "ON" : "OFF";
  lblToggle = lv_label_create(btnToggleCover);
  lv_label_set_text_static(lblToggle, labelToggle);
  lv_obj_add_event_cb(btnToggleCover, event_handler, LV_EVENT_CLICKED, this);
  lv_obj_add_flag(btnToggleCover, LV_OBJ_FLAG_HIDDEN);

  // Button to access the settings
  btnSettings = lv_button_create(lv_screen_active());
  btnSettings->user_data = this;
  lv_obj_set_size(btnSettings, 150, 150);
  lv_obj_align(btnSettings, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_radius(btnSettings, 30, LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(btnSettings, LV_OPA_70, LV_STATE_DEFAULT);
  lv_obj_add_event_cb(btnSettings, event_handler, LV_EVENT_CLICKED, this);
  labelBtnSettings = lv_label_create(btnSettings);
  lv_obj_set_style_text_font(labelBtnSettings, &lv_font_sys_48, LV_STATE_DEFAULT);
  lv_label_set_text_static(labelBtnSettings, Symbols::settings);
  lv_obj_add_flag(btnSettings, LV_OBJ_FLAG_HIDDEN);

  taskRefresh = lv_timer_create(RefreshTaskCallback, LV_DEF_REFR_PERIOD, this);
  Refresh();
}

WatchFaceInfineat::~WatchFaceInfineat() {
  lv_timer_del(taskRefresh);

  if (font_bebas != nullptr) {
    lv_binfont_destroy(font_bebas);
  }
  if (font_teko != nullptr) {
    lv_binfont_destroy(font_teko);
  }

  lv_obj_clean(lv_screen_active());
}

bool WatchFaceInfineat::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  if ((event == Pinetime::Applications::TouchEvents::LongTap) && lv_obj_has_flag(btnSettings, LV_OBJ_FLAG_HIDDEN)) {
    lv_obj_remove_flag(btnSettings, LV_OBJ_FLAG_HIDDEN);
  };
  savedTick = lv_tick_get();
  return true;

  // Prevent screen from sleeping when double tapping with settings on
  if ((event == Pinetime::Applications::TouchEvents::DoubleTap) && !lv_obj_has_flag(btnSettings, LV_OBJ_FLAG_HIDDEN)) {
    return true;
  }
  return false;
}

void WatchFaceInfineat::CloseMenu() {
  settingsController.SaveSettings();
  lv_obj_add_flag(btnClose, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(btnNextColor, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(btnPrevColor, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(btnToggleCover, LV_OBJ_FLAG_HIDDEN);
}

bool WatchFaceInfineat::OnButtonPushed() {
  if (!lv_obj_has_flag(btnSettings, LV_OBJ_FLAG_HIDDEN)) {
    CloseMenu();
    return true;
  }
  return false;
}

void WatchFaceInfineat::UpdateSelected(lv_obj_t* object, lv_event_t* event) {
  if (event->code == LV_EVENT_CLICKED) {
    bool showSideCover = settingsController.GetInfineatShowSideCover();
    int colorIndex = settingsController.GetInfineatColorIndex();

    if (object == btnSettings) {
      lv_obj_add_flag(btnSettings, LV_OBJ_FLAG_HIDDEN);
      if (lv_obj_has_flag(btnClose, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_remove_flag(btnClose, LV_OBJ_FLAG_HIDDEN);
      };
      if (!showSideCover) {
        lv_obj_add_flag(btnNextColor, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(btnPrevColor, LV_OBJ_FLAG_HIDDEN);
      } else {
        lv_obj_remove_flag(btnNextColor, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(btnPrevColor, LV_OBJ_FLAG_HIDDEN);
      }

      if (lv_obj_has_flag(btnToggleCover, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_remove_flag(btnToggleCover, LV_OBJ_FLAG_HIDDEN);
      };
    }
    if (object == btnClose) {
      CloseMenu();
    }
    if (object == btnToggleCover) {
      settingsController.SetInfineatShowSideCover(!showSideCover);
      ToggleBatteryIndicatorColor(!showSideCover);
      for (auto& line : lines) {
        if (showSideCover) {
          lv_obj_add_flag(line, LV_OBJ_FLAG_HIDDEN);
        } else {
          lv_obj_remove_flag(line, LV_OBJ_FLAG_HIDDEN);
        }
      }
      if (showSideCover) {
        lv_obj_add_flag(btnNextColor, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(btnPrevColor, LV_OBJ_FLAG_HIDDEN);
      } else {
        lv_obj_remove_flag(btnNextColor, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(btnPrevColor, LV_OBJ_FLAG_HIDDEN);
      }
      const char* labelToggle = showSideCover ? "OFF" : "ON";
      lv_label_set_text_static(lblToggle, labelToggle);
    }
    if (object == btnNextColor) {
      colorIndex = (colorIndex + 1) % nColors;
      settingsController.SetInfineatColorIndex(colorIndex);
    }
    if (object == btnPrevColor) {
      colorIndex -= 1;
      if (colorIndex < 0)
        colorIndex = nColors - 1;
      settingsController.SetInfineatColorIndex(colorIndex);
    }
    if (object == btnNextColor || object == btnPrevColor) {
      const std::array<lv_color_t, nLines>* colors = returnColor(static_cast<enum colors>(settingsController.GetInfineatColorIndex()));
      for (int i = 0; i < nLines; i++) {
        lv_color_t color = (*colors)[i];
        lv_obj_set_style_line_color(lines[i], color, LV_STATE_DEFAULT);
      }
      lv_obj_set_style_line_color(lineBattery, (*colors)[4], LV_STATE_DEFAULT);
      lv_obj_set_style_bg_color(notificationIcon, (*colors)[7], LV_PART_MAIN);
    }
  }
}

void WatchFaceInfineat::Refresh() {
  notificationState = notificationManager.AreNewNotificationsAvailable();
  if (notificationState.IsUpdated()) {
    if (!notificationState.Get()) {
      lv_obj_add_flag(notificationIcon, LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_remove_flag(notificationIcon, LV_OBJ_FLAG_HIDDEN);
    }
    lv_obj_align(notificationIcon, LV_ALIGN_TOP_RIGHT, 0, 0);
  }

  currentDateTime = std::chrono::time_point_cast<std::chrono::minutes>(dateTimeController.CurrentDateTime());
  if (currentDateTime.IsUpdated()) {
    uint8_t hour = dateTimeController.Hours();
    uint8_t minute = dateTimeController.Minutes();

    if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
      char ampmChar[3] = "AM";
      if (hour == 0) {
        hour = 12;
      } else if (hour == 12) {
        ampmChar[0] = 'P';
      } else if (hour > 12) {
        hour = hour - 12;
        ampmChar[0] = 'P';
      }
      lv_label_set_text(labelTimeAmPm, ampmChar);
    }
    lv_label_set_text_fmt(labelHour, "%02d", hour);
    lv_label_set_text_fmt(labelMinutes, "%02d", minute);

    if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
      lv_obj_align(labelTimeAmPm, LV_ALIGN_OUT_RIGHT_TOP, 0, 10);
      lv_obj_align(labelHour, LV_ALIGN_TOP_MID, 0, 5);
      lv_obj_align(labelMinutes, LV_ALIGN_BOTTOM_MID, 0, 0);
    }

    currentDate = std::chrono::time_point_cast<std::chrono::days>(currentDateTime.Get());
    if (currentDate.IsUpdated()) {
      uint8_t day = dateTimeController.Day();
      Controllers::DateTime::Days dayOfWeek = dateTimeController.DayOfWeek();
      lv_label_set_text_fmt(labelDate, "%s %02d", dateTimeController.DayOfWeekShortToStringLow(dayOfWeek), day);
    }
  }

  batteryPercentRemaining = batteryController.PercentRemaining();
  isCharging = batteryController.IsCharging();
  if (batteryController.IsCharging()) { // Charging battery animation
    chargingBatteryPercent += 1;
    if (chargingBatteryPercent > 100) {
      chargingBatteryPercent = batteryPercentRemaining.Get();
    }
    SetBatteryLevel(chargingBatteryPercent);
  } else if (isCharging.IsUpdated() || batteryPercentRemaining.IsUpdated()) {
    chargingBatteryPercent = batteryPercentRemaining.Get();
    SetBatteryLevel(chargingBatteryPercent);
  }

  bleState = bleController.IsConnected();
  bleRadioEnabled = bleController.IsRadioEnabled();
  if (bleState.IsUpdated()) {
    lv_label_set_text_static(bleIcon, BleIcon::GetIcon(bleState.Get()));
    lv_obj_align(bleIcon, LV_ALIGN_OUT_BOTTOM_MID, 0, 3);
  }

  stepCount = motionController.NbSteps();
  if (stepCount.IsUpdated()) {
    lv_label_set_text_fmt(stepValue, "%" PRIu32 "", stepCount.Get());
    lv_obj_align(stepValue, LV_ALIGN_BOTTOM_MID, 10, 0);
    lv_obj_align(stepIcon, LV_ALIGN_OUT_LEFT_MID, -5, 0);
  }

  if (!lv_obj_has_flag(btnSettings, LV_OBJ_FLAG_HIDDEN)) {
    if ((savedTick > 0) && (lv_tick_get() - savedTick > 3000)) {
      lv_obj_add_flag(btnSettings, LV_OBJ_FLAG_HIDDEN);
      savedTick = 0;
    }
  }
}

void WatchFaceInfineat::SetBatteryLevel(uint8_t batteryPercent) {
  // starting point (y) + Pine64 logo height * (100 - batteryPercent) / 100
  lineBatteryPoints[1] = {27, static_cast<lv_coord_t>(105 + 32 * (100 - batteryPercent) / 100)};
  lv_line_set_points(lineBattery, lineBatteryPoints, 2);
}

void WatchFaceInfineat::ToggleBatteryIndicatorColor(bool showSideCover) {
  if (!showSideCover) { // make indicator and notification icon color white
    lv_obj_set_style_bg_image_recolor_opa(logoPine, LV_OPA_100, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_image_recolor(logoPine, lv_color_white(), LV_STATE_DEFAULT);
    lv_obj_set_style_line_color(lineBattery, lv_color_black(), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(notificationIcon, lv_color_white(), LV_PART_MAIN);
  } else {
    lv_obj_set_style_bg_image_recolor_opa(logoPine, LV_OPA_0, LV_STATE_DEFAULT);
    const std::array<lv_color_t, nLines>* colors = returnColor(static_cast<enum colors>(settingsController.GetInfineatColorIndex()));
    lv_obj_set_style_line_color(lineBattery, (*colors)[4], LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(notificationIcon, (*colors)[7], LV_PART_MAIN);
  }
}

bool WatchFaceInfineat::IsAvailable(Pinetime::Controllers::FS& filesystem) {
  lfs_file file = {};

  if (filesystem.FileOpen(&file, "/fonts/teko.bin", LFS_O_RDONLY) < 0) {
    return false;
  }

  filesystem.FileClose(&file);
  if (filesystem.FileOpen(&file, "/fonts/bebas.bin", LFS_O_RDONLY) < 0) {
    return false;
  }

  filesystem.FileClose(&file);
  if (filesystem.FileOpen(&file, "/images/pine_small.bin", LFS_O_RDONLY) < 0) {
    return false;
  }

  filesystem.FileClose(&file);
  return true;
}
