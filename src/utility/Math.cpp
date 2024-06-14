#include "utility/Math.h"

#include <lvgl/src/misc/lv_math.h>

using namespace Pinetime::Utility;

#ifndef PINETIME_IS_RECOVERY

int32_t Pinetime::Utility::Asin(int16_t arg) {
  int32_t a = arg < 0 ? -arg : arg;

  int32_t angle = 45;
  int32_t low = 0;
  int32_t high = 90;
  while (low <= high) {
    int32_t sinAngle = lv_trigo_sin(angle);
    int32_t sinAngleSub = lv_trigo_sin(angle - 1);
    int32_t sinAngleAdd = lv_trigo_sin(angle + 1);

    if (a >= sinAngleSub && a <= sinAngleAdd) {
      if (a <= (sinAngleSub + sinAngle) / 2) {
        angle--;
      } else if (a > (sinAngle + sinAngleAdd) / 2) {
        angle++;
      }
      break;
    }

    if (a < sinAngle) {
      high = angle - 1;
    }

    else {
      low = angle + 1;
    }

    angle = (low + high) / 2;
  }

  return arg < 0 ? -angle : angle;
}

#else

int32_t Pinetime::Utility::Asin(int16_t /*arg*/) {
  return 0;
}

#endif
