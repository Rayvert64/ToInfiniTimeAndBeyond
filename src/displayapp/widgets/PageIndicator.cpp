#include "displayapp/widgets/PageIndicator.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Widgets;

PageIndicator::PageIndicator(uint8_t nCurrentScreen, uint8_t nScreens) : nCurrentScreen {nCurrentScreen}, nScreens {nScreens} {
}

void PageIndicator::Create() {
  pageIndicatorBasePoints[0].x = LV_HOR_RES - 1;
  pageIndicatorBasePoints[0].y = 0;
  pageIndicatorBasePoints[1].x = LV_HOR_RES - 1;
  pageIndicatorBasePoints[1].y = LV_VER_RES;

  pageIndicatorBase = lv_line_create(lv_screen_active());
  lv_obj_set_style_line_width(pageIndicatorBase, 3, LV_STATE_DEFAULT);
  lv_obj_set_style_line_color(pageIndicatorBase, Colors::bgDark, LV_STATE_DEFAULT);
  lv_line_set_points(pageIndicatorBase, pageIndicatorBasePoints, 2);

  const uint16_t indicatorSize = LV_VER_RES / nScreens;
  const uint16_t indicatorPos = indicatorSize * nCurrentScreen;

  pageIndicatorPoints[0].x = LV_HOR_RES - 1;
  pageIndicatorPoints[0].y = indicatorPos;
  pageIndicatorPoints[1].x = LV_HOR_RES - 1;
  pageIndicatorPoints[1].y = indicatorPos + indicatorSize;

  pageIndicator = lv_line_create(lv_screen_active());
  lv_obj_set_style_line_width(pageIndicator, 3, LV_STATE_DEFAULT);
  lv_obj_set_style_line_color(pageIndicator, Colors::lightGray, LV_STATE_DEFAULT);
  lv_line_set_points(pageIndicator, pageIndicatorPoints, 2);
}
