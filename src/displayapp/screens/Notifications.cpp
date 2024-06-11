#include "displayapp/screens/Notifications.h"
#include "displayapp/DisplayApp.h"
#include "components/ble/MusicService.h"
#include "components/ble/AlertNotificationService.h"
#include "displayapp/screens/Symbols.h"
#include <algorithm>
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;
extern lv_font_t jetbrains_mono_extrabold_compressed;
extern lv_font_t jetbrains_mono_bold_20;

Notifications::Notifications(DisplayApp* app,
                             Pinetime::Controllers::NotificationManager& notificationManager,
                             Pinetime::Controllers::AlertNotificationService& alertNotificationService,
                             Pinetime::Controllers::MotorController& motorController,
                             System::SystemTask& systemTask,
                             Modes mode)
  : app {app},
    notificationManager {notificationManager},
    alertNotificationService {alertNotificationService},
    motorController {motorController},
    systemTask {systemTask},
    mode {mode} {

  notificationManager.ClearNewNotificationFlag();
  auto notification = notificationManager.GetLastNotification();
  if (notification.valid) {
    currentId = notification.id;
    currentItem = std::make_unique<NotificationItem>(notification.Title(),
                                                     notification.Message(),
                                                     1,
                                                     notification.category,
                                                     notificationManager.NbNotifications(),
                                                     alertNotificationService,
                                                     motorController);
    validDisplay = true;
  } else {
    currentItem = std::make_unique<NotificationItem>(alertNotificationService, motorController);
    validDisplay = false;
  }
  if (mode == Modes::Preview) {
    systemTask.PushMessage(System::Messages::DisableSleeping);
    if (notification.category == Controllers::NotificationManager::Categories::IncomingCall) {
      motorController.StartRinging();
    } else {
      motorController.RunForDuration(35);
    }

    timeoutLine = lv_line_create(lv_scr_act(), nullptr);

    lv_obj_set_style_local_line_width(timeoutLine, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 3);
    lv_obj_set_style_local_line_color(timeoutLine, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, lv_color_white());
    lv_obj_set_style_local_line_rounded(timeoutLine, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, true);

    lv_line_set_points(timeoutLine, timeoutLinePoints, 2);
    timeoutTickCountStart = xTaskGetTickCount();
    interacted = false;
  }

  taskRefresh = lv_timer_create(RefreshTaskCallback, LV_DEF_REFR_PERIOD, this);
}

Notifications::~Notifications() {
  lv_timer_del(taskRefresh);
  // make sure we stop any vibrations before exiting
  motorController.StopRinging();
  systemTask.PushMessage(System::Messages::EnableSleeping);
  lv_obj_clean(lv_scr_act());
}

void Notifications::Refresh() {
  if (mode == Modes::Preview && timeoutLine != nullptr) {
    TickType_t tick = xTaskGetTickCount();
    int32_t pos = LV_HOR_RES - ((tick - timeoutTickCountStart) / (timeoutLength / LV_HOR_RES));
    if (pos <= 0) {
      running = false;
    } else {
      timeoutLinePoints[1].x = pos;
      lv_line_set_points(timeoutLine, timeoutLinePoints, 2);
    }

  } else if (mode == Modes::Preview && dismissingNotification) {
    running = false;
    currentItem = std::make_unique<NotificationItem>(alertNotificationService, motorController);

  } else if (dismissingNotification) {
    dismissingNotification = false;
    auto notification = notificationManager.Get(currentId);
    if (!notification.valid) {
      notification = notificationManager.GetLastNotification();
    }
    currentId = notification.id;

    if (!notification.valid) {
      validDisplay = false;
    }

    currentItem.reset(nullptr);
    if (afterDismissNextMessageFromAbove) {
      app->SetFullRefresh(DisplayApp::FullRefreshDirections::Down);
    } else {
      app->SetFullRefresh(DisplayApp::FullRefreshDirections::Up);
    }

    if (validDisplay) {
      Controllers::NotificationManager::Notification::Idx currentIdx = notificationManager.IndexOf(currentId);
      currentItem = std::make_unique<NotificationItem>(notification.Title(),
                                                       notification.Message(),
                                                       currentIdx + 1,
                                                       notification.category,
                                                       notificationManager.NbNotifications(),
                                                       alertNotificationService,
                                                       motorController);
    } else {
      currentItem = std::make_unique<NotificationItem>(alertNotificationService, motorController);
    }
  }

  running = currentItem->IsRunning() && running;
}

void Notifications::OnPreviewInteraction() {
  systemTask.PushMessage(System::Messages::EnableSleeping);
  motorController.StopRinging();
  if (timeoutLine != nullptr) {
    lv_obj_del(timeoutLine);
    timeoutLine = nullptr;
  }
}

void Notifications::DismissToBlack() {
  currentItem.reset(nullptr);
  app->SetFullRefresh(DisplayApp::FullRefreshDirections::RightAnim);
  // create black transition screen to let the notification dismiss to blackness
  lv_obj_t* blackBox = lv_obj_create(lv_scr_act());
  lv_obj_set_size(blackBox, LV_HOR_RES, LV_VER_RES);
  lv_obj_set_style_bg_color(blackBox, lv_color_black(), LV_PART_MAIN);
  dismissingNotification = true;
}

void Notifications::OnPreviewDismiss() {
  notificationManager.Dismiss(currentId);
  if (timeoutLine != nullptr) {
    lv_obj_del(timeoutLine);
    timeoutLine = nullptr;
  }
  DismissToBlack();
}

bool Notifications::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  if (mode != Modes::Normal) {
    if (!interacted && event == TouchEvents::Tap) {
      interacted = true;
      OnPreviewInteraction();
      return true;
    } else if (event == Pinetime::Applications::TouchEvents::SwipeRight) {
      OnPreviewDismiss();
      return true;
    }
    return false;
  }

  switch (event) {
    case Pinetime::Applications::TouchEvents::SwipeRight:
      if (validDisplay) {
        auto previousMessage = notificationManager.GetPrevious(currentId);
        auto nextMessage = notificationManager.GetNext(currentId);
        afterDismissNextMessageFromAbove = previousMessage.valid;
        notificationManager.Dismiss(currentId);
        if (previousMessage.valid) {
          currentId = previousMessage.id;
        } else if (nextMessage.valid) {
          currentId = nextMessage.id;
        } else {
          // don't update id, won't be found be refresh and try to load latest message or no message box
        }
        DismissToBlack();
        return true;
      }
      return false;
    case Pinetime::Applications::TouchEvents::SwipeDown: {
      Controllers::NotificationManager::Notification previousNotification;
      if (validDisplay) {
        previousNotification = notificationManager.GetPrevious(currentId);
      } else {
        previousNotification = notificationManager.GetLastNotification();
      }

      if (!previousNotification.valid) {
        return true;
      }

      currentId = previousNotification.id;
      Controllers::NotificationManager::Notification::Idx currentIdx = notificationManager.IndexOf(currentId);
      validDisplay = true;
      currentItem.reset(nullptr);
      app->SetFullRefresh(DisplayApp::FullRefreshDirections::Down);
      currentItem = std::make_unique<NotificationItem>(previousNotification.Title(),
                                                       previousNotification.Message(),
                                                       currentIdx + 1,
                                                       previousNotification.category,
                                                       notificationManager.NbNotifications(),
                                                       alertNotificationService,
                                                       motorController);
    }
      return true;
    case Pinetime::Applications::TouchEvents::SwipeUp: {
      Controllers::NotificationManager::Notification nextNotification;
      if (validDisplay) {
        nextNotification = notificationManager.GetNext(currentId);
      } else {
        nextNotification = notificationManager.GetLastNotification();
      }

      if (!nextNotification.valid) {
        running = false;
        return false;
      }

      currentId = nextNotification.id;
      Controllers::NotificationManager::Notification::Idx currentIdx = notificationManager.IndexOf(currentId);
      validDisplay = true;
      currentItem.reset(nullptr);
      app->SetFullRefresh(DisplayApp::FullRefreshDirections::Up);
      currentItem = std::make_unique<NotificationItem>(nextNotification.Title(),
                                                       nextNotification.Message(),
                                                       currentIdx + 1,
                                                       nextNotification.category,
                                                       notificationManager.NbNotifications(),
                                                       alertNotificationService,
                                                       motorController);
    }
      return true;
    default:
      return false;
  }
}

namespace {
  void CallEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* item = static_cast<Notifications::NotificationItem*>(obj->user_data);
    item->OnCallButtonEvent(obj, event);
  }
}

Notifications::NotificationItem::NotificationItem(Pinetime::Controllers::AlertNotificationService& alertNotificationService,
                                                  Pinetime::Controllers::MotorController& motorController)
  : NotificationItem("Notification",
                     "No notification to display",
                     0,
                     Controllers::NotificationManager::Categories::Unknown,
                     0,
                     alertNotificationService,
                     motorController) {
}

Notifications::NotificationItem::NotificationItem(const char* title,
                                                  const char* msg,
                                                  uint8_t notifNr,
                                                  Controllers::NotificationManager::Categories category,
                                                  uint8_t notifNb,
                                                  Pinetime::Controllers::AlertNotificationService& alertNotificationService,
                                                  Pinetime::Controllers::MotorController& motorController)
  : alertNotificationService {alertNotificationService}, motorController {motorController} {
  container = lv_cont_create(lv_scr_act(), nullptr);
  lv_obj_set_size(container, LV_HOR_RES, LV_VER_RES);
  lv_obj_set_style_bg_color(container, lv_color_black(), LV_CONT_PART_MAIN);
  lv_obj_set_style_local_pad_all(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_pad_inner(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_border_width(container, 0, LV_CONT_PART_MAIN);

  subject_container = lv_cont_create(container, nullptr);
  lv_obj_set_style_bg_color(subject_container, Colors::bgAlt, LV_CONT_PART_MAIN);
  lv_obj_set_style_local_pad_all(subject_container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
  lv_obj_set_style_local_pad_inner(subject_container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_border_width(subject_container, 0, LV_CONT_PART_MAIN);

  lv_obj_set_pos(subject_container, 0, 50);
  lv_obj_set_size(subject_container, LV_HOR_RES, LV_VER_RES - 50);
  lv_cont_set_layout(subject_container, LV_LAYOUT_COLUMN_LEFT);
  lv_cont_set_fit(subject_container, LV_FIT_NONE);

  lv_obj_t* alert_count = lv_label_create(container);
  lv_label_set_text_fmt(alert_count, "%i/%i", notifNr, notifNb);
  lv_obj_align(alert_count, LV_ALIGN_TOP_RIGHT, 0, 16);

  lv_obj_t* alert_type = lv_label_create(container);
  lv_obj_set_style_text_color(alert_type, Colors::orange, LV_STATE_DEFAULT);
  if (title == nullptr) {
    lv_label_set_text_static(alert_type, "Notification");
  } else {
    // copy title to label and replace newlines with spaces
    lv_label_set_text(alert_type, title);
    char* pchar = strchr(lv_label_get_text(alert_type), '\n');
    while (pchar != nullptr) {
      *pchar = ' ';
      pchar = strchr(pchar + 1, '\n');
    }
    lv_label_refr_text(alert_type);
  }
  lv_label_set_long_mode(alert_type, LV_LABEL_LONG_SROLL_CIRC);
  lv_obj_set_width(alert_type, 180);
  lv_obj_align(alert_type, LV_ALIGN_TOP_LEFT, 0, 16);

  lv_obj_t* alert_subject = lv_label_create(subject_container);
  lv_label_set_long_mode(alert_subject, LV_LABEL_LONG_WRAP);
  lv_obj_set_width(alert_subject, LV_HOR_RES - 20);

  switch (category) {
    default:
      lv_label_set_text(alert_subject, msg);
      break;
    case Controllers::NotificationManager::Categories::IncomingCall: {
      lv_obj_set_height(subject_container, 108);
      lv_label_set_text_static(alert_subject, "Incoming call from");

      lv_obj_t* alert_caller = lv_label_create(subject_container);
      lv_obj_align(alert_caller, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
      lv_label_set_long_mode(alert_caller, LV_LABEL_LONG_WRAP);
      lv_obj_set_width(alert_caller, LV_HOR_RES - 20);
      lv_label_set_text(alert_caller, msg);

      bt_accept = lv_button_create(container);
      bt_accept->user_data = this;
      lv_obj_add_event_cb(bt_accept, CallEventHandler);
      lv_obj_set_size(bt_accept, 76, 76);
      lv_obj_align(bt_accept, LV_ALIGN_BOTTOM_LEFT, 0, 0);
      label_accept = lv_label_create(bt_accept);
      lv_label_set_text_static(label_accept, Symbols::phone);
      lv_obj_set_style_bg_color(bt_accept, Colors::highlight, LV_PART_MAIN);

      bt_reject = lv_button_create(container);
      bt_reject->user_data = this;
      lv_obj_add_event_cb(bt_reject, CallEventHandler);
      lv_obj_set_size(bt_reject, 76, 76);
      lv_obj_align(bt_reject, LV_ALIGN_BOTTOM_MID, 0, 0);
      label_reject = lv_label_create(bt_reject);
      lv_label_set_text_static(label_reject, Symbols::phoneSlash);
      lv_obj_set_style_bg_color(bt_reject, PINETIME_COLOR_RED, LV_PART_MAIN);

      bt_mute = lv_button_create(container);
      bt_mute->user_data = this;
      lv_obj_add_event_cb(bt_mute, CallEventHandler);
      lv_obj_set_size(bt_mute, 76, 76);
      lv_obj_align(bt_mute, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
      label_mute = lv_label_create(bt_mute);
      lv_label_set_text_static(label_mute, Symbols::volumMute);
      lv_obj_set_style_bg_color(bt_mute, Colors::lightGray, LV_PART_MAIN);
    } break;
  }
}

void Notifications::NotificationItem::OnCallButtonEvent(lv_obj_t* obj, lv_event_t event) {
  if (event != LV_EVENT_CLICKED) {
    return;
  }

  motorController.StopRinging();

  if (obj == bt_accept) {
    alertNotificationService.AcceptIncomingCall();
  } else if (obj == bt_reject) {
    alertNotificationService.RejectIncomingCall();
  } else if (obj == bt_mute) {
    alertNotificationService.MuteIncomingCall();
  }

  running = false;
}

Notifications::NotificationItem::~NotificationItem() {
  lv_obj_clean(lv_scr_act());
}
