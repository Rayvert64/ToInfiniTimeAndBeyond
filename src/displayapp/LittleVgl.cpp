#include "displayapp/LittleVgl.h"
#include "displayapp/InfiniTimeTheme.h"

#include <FreeRTOS.h>
#include <lvgl/src/display/lv_display.h>
#include <lvgl/src/indev/lv_indev.h>
#include <lvgl/src/lv_conf_internal.h>
#include <lvgl/src/themes/default/lv_theme_default.h>
#include <task.h>
#include "drivers/St7789.h"
#include "littlefs/lfs.h"
#include "components/fs/FS.h"
#include <any>

using namespace Pinetime::Components;

namespace {
  void InitTheme(lv_display_t* disp) {
    lv_theme_t* theme = lv_pinetime_theme_init();
    lv_display_set_theme(disp, theme);
  }
}

static void disp_flush(lv_display_t* disp_drv, const lv_area_t* area, unsigned char* buffer) {
  auto* lvgl = static_cast<LittleVgl*>(lv_display_get_user_data(disp_drv));
  lvgl->FlushDisplay(area, color_p);
}

static void rounder(lv_display_t* disp_drv, lv_area_t* area) {
  auto* lvgl = static_cast<LittleVgl*>(lv_display_get_user_data(disp_drv));
  if (lvgl->GetFullRefresh()) {
    area->x1 = 0;
    area->x2 = LV_HOR_RES - 1;
    area->y1 = 0;
    area->y2 = LV_VER_RES - 1;
  }
}

static void touchpad_read(lv_indev_t* indev_drv, lv_indev_data_t* data) {
  auto* lvgl = static_cast<LittleVgl*>(lv_indev_get_user_data(indev_drv));
  (void) lvgl->GetTouchPadInfo(data);
}

LittleVgl::LittleVgl(Pinetime::Drivers::St7789& lcd, Pinetime::Controllers::FS& filesystem) : lcd {lcd}, filesystem {filesystem} {
}

void LittleVgl::Init() {
  lv_init();
  InitDisplay();
  InitTheme(disp_drv);
  InitTouchpad();
}

void LittleVgl::InitDisplay() {
  disp_drv = lv_display_create(LV_HOR_RES, LV_VER_RES);

  lv_display_set_buffers(disp_drv, buf2_1, buf2_2, sizeof(buf2_1), LV_DISPLAY_RENDER_MODE_PARTIAL);
  /*Initialize the display buffer*/

  /*Used to copy the buffer's content to the display*/
  disp_drv.flush_cb = disp_flush;
  /*Set a display buffer*/
  disp_drv.buffer = &disp_buf_2;
  disp_drv.user_data = this;
  disp_drv.rounder_cb = rounder;

  lv_display_set_user_data(disp_drv, this);
  lv_display_set_flush_cb(disp_drv, disp_flush);
}

void LittleVgl::InitTouchpad() {
  lv_indev_t* indev_drv = lv_indev_create();

  lv_indev_set_type(indev_drv, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev_drv, touchpad_read);
  lv_indev_set_user_data(indev_drv, this);
}

void LittleVgl::SetFullRefresh(FullRefreshDirections direction) {
  if (scrollDirection == FullRefreshDirections::None) {
    scrollDirection = direction;
    if (scrollDirection == FullRefreshDirections::Down) {
      lv_display_set_rotation(lv_disp_get_default(), LV_DISPLAY_ROTATION_0);
    } else if (scrollDirection == FullRefreshDirections::Right) {
      lv_display_set_rotation(lv_disp_get_default(), LV_DISPLAY_ROTATION_90);
    } else if (scrollDirection == FullRefreshDirections::Left) {
      lv_display_set_rotation(lv_disp_get_default(), LV_DISPLAY_ROTATION_270);
    } else if (scrollDirection == FullRefreshDirections::RightAnim) {
      lv_display_set_rotation(lv_disp_get_default(), LV_DISPLAY_ROTATION_90);
    } else if (scrollDirection == FullRefreshDirections::LeftAnim) {
      lv_display_set_rotation(lv_disp_get_default(), LV_DISPLAY_ROTATION_270);
    }
  }
  fullRefresh = true;
}

void LittleVgl::FlushDisplay(const lv_area_t* area, lv_color_t* color_p) {
  uint16_t y1, y2, width, height = 0;

  if ((scrollDirection == LittleVgl::FullRefreshDirections::Down) && (area->y2 == visibleNbLines - 1)) {
    writeOffset = ((writeOffset + totalNbLines) - visibleNbLines) % totalNbLines;
  } else if ((scrollDirection == FullRefreshDirections::Up) && (area->y1 == 0)) {
    writeOffset = (writeOffset + visibleNbLines) % totalNbLines;
  }

  y1 = (area->y1 + writeOffset) % totalNbLines;
  y2 = (area->y2 + writeOffset) % totalNbLines;

  width = (area->x2 - area->x1) + 1;
  height = (area->y2 - area->y1) + 1;

  if (scrollDirection == LittleVgl::FullRefreshDirections::Down) {

    if (area->y2 < visibleNbLines - 1) {
      uint16_t toScroll = 0;
      if (area->y1 == 0) {
        toScroll = height * 2;
        scrollDirection = FullRefreshDirections::None;
        lv_display_set_rotation(lv_disp_get_default(), LV_DISPLAY_ROTATION_0);
      } else {
        toScroll = height;
      }

      if (scrollOffset >= toScroll)
        scrollOffset -= toScroll;
      else {
        toScroll -= scrollOffset;
        scrollOffset = (totalNbLines) -toScroll;
      }
      lcd.VerticalScrollStartAddress(scrollOffset);
    }

  } else if (scrollDirection == FullRefreshDirections::Up) {

    if (area->y1 > 0) {
      if (area->y2 == visibleNbLines - 1) {
        scrollOffset += (height * 2);
        scrollDirection = FullRefreshDirections::None;
        lv_display_set_rotation(lv_disp_get_default(), LV_DISPLAY_ROTATION_0);
      } else {
        scrollOffset += height;
      }
      scrollOffset = scrollOffset % totalNbLines;
      lcd.VerticalScrollStartAddress(scrollOffset);
    }
  } else if (scrollDirection == FullRefreshDirections::Left or scrollDirection == FullRefreshDirections::LeftAnim) {
    if (area->x2 == visibleNbLines - 1) {
      scrollDirection = FullRefreshDirections::None;
      lv_display_set_rotation(lv_disp_get_default(), LV_DISPLAY_ROTATION_0);
    }
  } else if (scrollDirection == FullRefreshDirections::Right or scrollDirection == FullRefreshDirections::RightAnim) {
    if (area->x1 == 0) {
      scrollDirection = FullRefreshDirections::None;
      lv_display_set_rotation(lv_disp_get_default(), LV_DISPLAY_ROTATION_0);
    }
  }

  if (y2 < y1) {
    height = totalNbLines - y1;

    if (height > 0) {
      lcd.DrawBuffer(area->x1, y1, width, height, std::any_cast<const uint8_t*>(color_p), width * height * 2);
    }

    uint16_t pixOffset = width * height;
    height = y2 + 1;
    lcd.DrawBuffer(area->x1, 0, width, height, std::any_cast<const uint8_t*>(color_p + pixOffset), width * height * 2);

  } else {
    lcd.DrawBuffer(area->x1, y1, width, height, std::any_cast<const uint8_t*>(color_p), width * height * 2);
  }

  // IMPORTANT!!!
  // Inform the graphics library that you are ready with the flushing
  lv_display_flush_ready(disp_drv);
}

void LittleVgl::SetNewTouchPoint(int16_t x, int16_t y, bool contact) {
  if (contact) {
    if (!isCancelled) {
      touchPoint = {x, y};
      tapped = true;
    }
  } else {
    if (isCancelled) {
      touchPoint = {-1, -1};
      tapped = false;
      isCancelled = false;
    } else {
      touchPoint = {x, y};
      tapped = false;
    }
  }
}

void LittleVgl::CancelTap() {
  if (tapped) {
    isCancelled = true;
    touchPoint = {-1, -1};
  }
}

void LittleVgl::GetTouchPadInfo(lv_indev_data_t* ptr) const {
  ptr->point.x = touchPoint.x;
  ptr->point.y = touchPoint.y;
  if (tapped) {
    ptr->state = LV_INDEV_STATE_PR;
  } else {
    ptr->state = LV_INDEV_STATE_REL;
  }
}
