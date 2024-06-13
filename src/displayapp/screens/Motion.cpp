#include "displayapp/screens/Motion.h"
#include "displayapp/Colors.h"
#include <lvgl/lvgl.h>
#include <lvgl/src/widgets/chart/lv_chart.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

Motion::Motion(Controllers::MotionController& motionController) : motionController {motionController} {
  chart = lv_chart_create(lv_screen_active());
  lv_obj_set_size(chart, 240, 240);
  lv_obj_align(chart, LV_ALIGN_TOP_MID, 0, 0);
  lv_chart_set_type(chart, LV_CHART_TYPE_LINE); /*Show lines and points too*/
  // lv_chart_set_series_opa(chart, LV_OPA_70);                            /*Opacity of the data series*/
  // lv_chart_set_series_width(chart, 4);                                  /*Line width and point radious*/

  lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, -1100, 1100);
  lv_chart_set_update_mode(chart, LV_CHART_UPDATE_MODE_SHIFT);
  lv_chart_set_point_count(chart, 10);

  /*Add 3 data series*/
  ser1 = lv_chart_add_series(chart, PINETIME_COLOR_RED, LV_CHART_AXIS_PRIMARY_Y);
  ser2 = lv_chart_add_series(chart, Colors::green, LV_CHART_AXIS_PRIMARY_Y);
  ser3 = lv_chart_add_series(chart, PINETIME_COLOR_YELLOW, LV_CHART_AXIS_PRIMARY_Y);

  lv_chart_refresh(chart); /*Required after direct set*/

  label = lv_label_create(lv_screen_active());
  lv_label_set_text_fmt(label, "X #FF0000 %d# Y #00B000 %d# Z #FFFF00 %d#", 0, 0, 0);
  lv_obj_set_align(label, LV_ALIGN_CENTER);
  lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 10);

  labelStep = lv_label_create(lv_screen_active());
  lv_obj_align(labelStep, LV_ALIGN_BOTTOM_LEFT, 0, 0);
  lv_label_set_text_static(labelStep, "Steps ---");

  taskRefresh = lv_timer_create(RefreshTaskCallback, LV_DEF_REFR_PERIOD, this);
}

Motion::~Motion() {
  lv_timer_del(taskRefresh);
  lv_obj_clean(lv_screen_active());
}

void Motion::Refresh() {
  lv_chart_set_next_value(chart, ser1, motionController.X());
  lv_chart_set_next_value(chart, ser2, motionController.Y());
  lv_chart_set_next_value(chart, ser3, motionController.Z());

  lv_label_set_text_fmt(labelStep, "Steps %" PRIu32 "", motionController.NbSteps());

  lv_label_set_text_fmt(label,
                        "X #FF0000 %d# Y #00B000 %d# Z #FFFF00 %d# mg",
                        motionController.X(),
                        motionController.Y(),
                        motionController.Z());
  lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 10);
}
