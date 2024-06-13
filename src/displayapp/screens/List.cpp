#include "displayapp/screens/List.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"
#include <displayapp/Colors.h>

using namespace Pinetime::Applications::Screens;

namespace {
  void ButtonEventHandler(lv_event_t* event) {
    auto* screen = static_cast<List*>(event->user_data);
    screen->OnButtonEvent(event);
  }
}

List::List(uint8_t screenID,
           uint8_t numScreens,
           DisplayApp* app,
           Controllers::Settings& settingsController,
           std::array<Applications, MAXLISTITEMS>& applications)
  : app {app}, settingsController {settingsController}, pageIndicator(screenID, numScreens) {

  // Set the background to Black
  lv_obj_set_style_bg_color(lv_screen_active(), PINETIME_COLOR_BLACK, LV_STATE_DEFAULT);

  settingsController.SetSettingsMenu(screenID);

  pageIndicator.Create();

  lv_obj_t* container = lv_obj_create(lv_screen_active());

  lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, LV_STATE_DEFAULT);
  static constexpr int innerPad = 4;
  lv_obj_set_style_text_align(container, LV_ALIGN_CENTER, LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(container, 0, LV_PART_MAIN);

  lv_obj_set_pos(container, 0, 0);
  lv_obj_set_width(container, LV_HOR_RES - 8);
  lv_obj_set_height(container, LV_VER_RES);
  lv_obj_set_style_text_align(container, LV_ALIGN_TOP_LEFT, LV_STATE_ANY);

  for (int i = 0; i < MAXLISTITEMS; i++) {
    apps[i] = applications[i].application;
    if (applications[i].application != Apps::None) {

      static constexpr int btnHeight = (LV_HOR_RES - ((MAXLISTITEMS - 1) * innerPad)) / MAXLISTITEMS;
      itemApps[i] = lv_button_create(container);
      lv_obj_set_style_radius(itemApps[i], btnHeight / 3, LV_STATE_DEFAULT);
      lv_obj_set_style_bg_color(itemApps[i], Colors::bgAlt, LV_PART_MAIN);
      lv_obj_set_width(itemApps[i], LV_HOR_RES - 8);
      lv_obj_set_height(itemApps[i], btnHeight);
      lv_obj_add_event_cb(itemApps[i], ButtonEventHandler, LV_EVENT_CLICKED, this);
      lv_obj_set_layout(itemApps[i], LV_LAYOUT_NONE);
      itemApps[i]->user_data = this;
      lv_obj_set_style_clip_corner(itemApps[i], true, LV_STATE_DEFAULT);

      lv_obj_t* icon = lv_label_create(itemApps[i]);
      lv_obj_set_style_text_color(icon, PINETIME_COLOR_YELLOW, LV_STATE_DEFAULT);
      lv_label_set_text_static(icon, applications[i].icon);
      lv_label_set_long_mode(icon, LV_LABEL_LONG_CLIP);
      lv_obj_set_align(icon, LV_ALIGN_CENTER);
      lv_obj_set_width(icon, btnHeight);
      lv_obj_align(icon, LV_ALIGN_LEFT_MID, 0, 0);

      lv_obj_t* text = lv_label_create(itemApps[i]);
      lv_label_set_text_fmt(text, "%s", applications[i].name);
      lv_obj_align(text, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
    }
  }
}

List::~List() {
  lv_obj_clean(lv_screen_active());
}

void List::OnButtonEvent(lv_event_t* event) {
  auto* object = lv_event_get_target_obj(event);
  for (int i = 0; i < MAXLISTITEMS; i++) {
    if (apps[i] != Apps::None && object == itemApps[i]) {
      app->StartApp(apps[i], DisplayApp::FullRefreshDirections::Up);
      running = false;
      return;
    }
  }
}
