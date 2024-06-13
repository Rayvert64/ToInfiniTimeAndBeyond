#include "displayapp/widgets/DotIndicator.h"
#include "displayapp/InfiniTimeTheme.h"
#include "displayapp/Colors.h"
#include <vector>
#include <cstddef>

using namespace Pinetime::Applications::Widgets;

DotIndicator::DotIndicator(uint8_t nCurrentScreen, uint8_t nScreens) : nCurrentScreen {nCurrentScreen}, nScreens {nScreens} {
}

void DotIndicator::Create() {
  const auto nbscreens = static_cast<size_t>(nScreens);
  std::vector<lv_obj_t*> dotIndicator;
  dotIndicator.resize(nbscreens);

  lv_obj_t* container = lv_obj_create(lv_screen_active());
  lv_obj_set_style_text_align(container, LV_ALIGN_TOP_LEFT, LV_STATE_ANY);
  lv_obj_set_style_text_align(container, LV_ALIGN_CENTER, LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, LV_STATE_DEFAULT);

  for (int i = 0; i < nScreens; i++) {
    dotIndicator[i] = lv_obj_create(container);
    lv_obj_set_size(dotIndicator[i], dotSize, dotSize);
    lv_obj_set_style_bg_color(dotIndicator[i], PINETIME_COLOR_GRAY, LV_PART_MAIN);
  }

  lv_obj_set_style_bg_color(dotIndicator[nCurrentScreen], lv_color_white(), LV_PART_MAIN);

  lv_obj_align(container, LV_ALIGN_RIGHT_MID, 0, 0);
}
