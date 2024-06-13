#include "displayapp/screens/Dice.h"
#include "displayapp/screens/Symbols.h"
#include "components/settings/Settings.h"
#include "components/motor/MotorController.h"
#include "components/motion/MotionController.h"
#include <cstdint>
#include <any>
#include <lvgl/lvgl.h>
#include <lvgl/src/misc/lv_types.h>

#define NCOUNTER_VALUES "1\n2\n3\n4\n5\n6\n7\n8\n9"
#define DCOUNTER_VALUES                                                                                                                    \
  "2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37" \
  "\n38\n39\n40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n51\n52\n53\n54\n55\n56\n57\n58\n59\n60\n61\n62\n63\n64\n65\n66\n67\n68\n69\n70\n" \
  "71\n72\n73\n74\n75\n76\n77\n78\n79\n80\n81\n82\n83\n84\n85\n86\n87\n88\n89\n90\n91\n92\n93\n94\n95\n96\n97\n98\n99"

using namespace Pinetime::Applications::Screens;

namespace {
  lv_obj_t* MakeLabel(const lv_font_t* font,
                      lv_color_t color,
                      lv_label_long_mode_t longMode,
                      uint8_t width,
                      lv_align_t labelAlignment,
                      const char* text,
                      lv_obj_t* reference,
                      lv_align_t alignment,
                      int8_t x,
                      int8_t y) {
    lv_obj_t* label = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(label, font, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label, color, LV_STATE_DEFAULT);
    lv_label_set_long_mode(label, longMode);
    if (width != 0) {
      lv_obj_set_width(label, width);
    }
    lv_obj_set_align(label, labelAlignment);
    lv_label_set_text(label, text);
    lv_obj_align_to(label, reference, alignment, x, y);
    return label;
  }

  void btnRollEventHandler(lv_event_t* event) {
    auto* screen = static_cast<Dice*>(event->user_data);
    screen->Roll();
  }
}

Dice::Dice(Controllers::MotionController& motionController,
           Controllers::MotorController& motorController,
           Controllers::Settings& settingsController)
  : motorController {motorController}, motionController {motionController}, settingsController {settingsController} {
  std::seed_seq sseq {static_cast<uint32_t>(xTaskGetTickCount()),
                      static_cast<uint32_t>(motionController.X()),
                      static_cast<uint32_t>(motionController.Y()),
                      static_cast<uint32_t>(motionController.Z())};
  gen.seed(sseq);

  nCounter = lv_roller_create(lv_screen_active());
  dCounter = lv_roller_create(lv_screen_active());

  lv_roller_set_options(nCounter, NCOUNTER_VALUES, LV_ROLLER_MODE_INFINITE);
  lv_roller_set_options(dCounter, DCOUNTER_VALUES, LV_ROLLER_MODE_INFINITE);

  lv_obj_t* nCounterLabel = MakeLabel(&jetbrains_mono_bold_20,
                                      lv_color_white(),
                                      LV_LABEL_LONG_WRAP,
                                      0,
                                      LV_ALIGN_CENTER,
                                      "count",
                                      lv_screen_active(),
                                      LV_ALIGN_TOP_LEFT,
                                      0,
                                      0);

  lv_obj_t* dCounterLabel = MakeLabel(&jetbrains_mono_bold_20,
                                      lv_color_white(),
                                      LV_LABEL_LONG_WRAP,
                                      0,
                                      LV_ALIGN_CENTER,
                                      "sides",
                                      nCounterLabel,
                                      LV_ALIGN_OUT_RIGHT_MID,
                                      20,
                                      0);

  lv_obj_align_to(nCounter, nCounterLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
  lv_roller_set_selected(nCounter, 1, LV_ANIM_ON);

  lv_obj_align_to(dCounter, dCounterLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
  lv_roller_set_selected(dCounter, 6, LV_ANIM_ON);

  std::uniform_int_distribution<> distrib(0, static_cast<uint8_t>(resultColors.size()) - 1);
  currentColorIndex = distrib(gen);

  resultTotalLabel = MakeLabel(&jetbrains_mono_42,
                               resultColors[currentColorIndex],
                               LV_LABEL_LONG_WRAP,
                               120,
                               LV_ALIGN_CENTER,
                               "",
                               lv_screen_active(),
                               LV_ALIGN_TOP_RIGHT,
                               11,
                               38);
  resultIndividualLabel = MakeLabel(&jetbrains_mono_bold_20,
                                    resultColors[currentColorIndex],
                                    LV_LABEL_LONG_WRAP,
                                    90,
                                    LV_ALIGN_CENTER,
                                    "",
                                    resultTotalLabel,
                                    LV_ALIGN_OUT_BOTTOM_MID,
                                    0,
                                    10);

  Roll();
  openingRoll = false;

  btnRoll = lv_button_create(lv_screen_active());
  btnRoll->user_data = this;
  lv_obj_add_event_cb(btnRoll, btnRollEventHandler, LV_EVENT_CLICKED, this);
  lv_obj_set_size(btnRoll, 240, 50);
  lv_obj_align(btnRoll, LV_ALIGN_BOTTOM_MID, 0, 0);

  btnRollLabel = MakeLabel(&jetbrains_mono_bold_20,
                           lv_color_white(),
                           LV_LABEL_LONG_WRAP,
                           0,
                           LV_ALIGN_CENTER,
                           Symbols::dice,
                           btnRoll,
                           LV_ALIGN_CENTER,
                           0,
                           0);

  // Spagetti code in motion controller: it only updates the shake speed when shake to wake is on...
  enableShakeForDice = !settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::Shake);
  if (enableShakeForDice) {
    settingsController.setWakeUpMode(Pinetime::Controllers::Settings::WakeUpMode::Shake, true);
  }
  refreshTask = lv_timer_create(RefreshTaskCallback, LV_DEF_REFR_PERIOD, this);
}

Dice::~Dice() {
  // reset the shake to wake mode.
  if (enableShakeForDice) {
    settingsController.setWakeUpMode(Pinetime::Controllers::Settings::WakeUpMode::Shake, false);
    enableShakeForDice = false;
  }
  lv_timer_del(refreshTask);
  lv_obj_clean(lv_screen_active());
}

void Dice::Refresh() {
  // we only reset the hysteresis when at rest
  if (motionController.CurrentShakeSpeed() >= settingsController.GetShakeThreshold()) {
    if (currentRollHysteresis <= 0) {
      // this timestamp is used for the screen timeout
      lv_display_trigger_activity(nullptr);

      Roll();
    }
  } else if (currentRollHysteresis > 0) {
    --currentRollHysteresis;
  }
}

void Dice::Roll() {
  uint8_t resultIndividual = 0;
  uint16_t resultTotal = 0;
  std::uniform_int_distribution<int32_t> distrib(1, static_cast<int32_t>((lv_roller_get_selected(dCounter))));

  lv_label_set_text(resultIndividualLabel, "");

  if (lv_roller_get_selected(nCounter) == 1) {
    resultTotal = distrib(gen);
    if (lv_roller_get_selected(dCounter) == 2) {
      switch (resultTotal) {
        case 1:
          lv_label_set_text(resultIndividualLabel, "HEADS");
          break;
        case 2:
          lv_label_set_text(resultIndividualLabel, "TAILS");
          break;
        default:
          break;
      }
    }
  } else {
    for (uint32_t i = 0; i < lv_roller_get_selected(nCounter); i++) {
      resultIndividual = distrib(gen);
      resultTotal += resultIndividual;
      lv_label_ins_text(resultIndividualLabel, LV_LABEL_POS_LAST, std::to_string(resultIndividual).c_str());
      if (i < (lv_roller_get_selected(nCounter) - 1)) {
        lv_label_ins_text(resultIndividualLabel, LV_LABEL_POS_LAST, "+");
      }
    }
  }

  lv_label_set_text_fmt(resultTotalLabel, "%d", resultTotal);
  if (!openingRoll) {
    motorController.RunForDuration(30);
    NextColor();
    currentRollHysteresis = rollHysteresis;
  }
}

void Dice::NextColor() {
  currentColorIndex = (currentColorIndex + 1) % resultColors.size();
  lv_obj_set_style_text_color(resultTotalLabel, resultColors[currentColorIndex], LV_STATE_DEFAULT);
  lv_obj_set_style_text_color(resultIndividualLabel, resultColors[currentColorIndex], LV_STATE_DEFAULT);
}
