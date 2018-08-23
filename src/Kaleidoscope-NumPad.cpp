#include "Kaleidoscope-NumPad.h"
#include "LEDUtils.h"
#include "Kaleidoscope.h"
#include "layers.h"

#define UPDATE_INTERVAL 50  // milliseconds between two LED updates to avoid overloading; 20 fps

byte NumPad_::row = 255, NumPad_::col = 255;
uint8_t NumPad_::numPadLayer;
bool NumPad_::cleanupDone = true;
bool NumPad_::originalNumLockState = false;
cRGB NumPad_::color = CRGB(160, 0, 0);
uint8_t NumPad_::lock_hue = 170;
uint16_t NumPad_::last_update = 0;

kaleidoscope::EventHandlerResult NumPad_::onSetup(void) {
  originalNumLockState = !!(kaleidoscope::hid::getKeyboardLEDs() & LED_NUM_LOCK);
  return kaleidoscope::EventHandlerResult::OK;
}

static bool getNumlockState() {
  return !!(kaleidoscope::hid::getKeyboardLEDs() & LED_NUM_LOCK);
}

static void syncNumlock(bool state) {
  bool numState = getNumlockState();
  if (numState != state) {
    kaleidoscope::hid::pressKey(Key_KeypadNumLock);
  }
}

kaleidoscope::EventHandlerResult NumPad_::afterEachCycle() {
  if (!Layer.isOn(numPadLayer)) {
    bool numState = getNumlockState();
    if (!cleanupDone) {
      LEDControl.set_mode(LEDControl.get_mode_index());

      if (!originalNumLockState) {
        syncNumlock(false);
        numState = false;
      }
      cleanupDone = true;
    }
    originalNumLockState = numState;

    return kaleidoscope::EventHandlerResult::OK;
  }

  cleanupDone = false;
  syncNumlock(true);

  uint16_t now = millis();
  if ((now - last_update) < UPDATE_INTERVAL)
    return kaleidoscope::EventHandlerResult::OK;
  last_update = now;

  LEDControl.set_mode(LEDControl.get_mode_index());

  for (uint8_t r = 0; r < ROWS; r++) {
    for (uint8_t c = 0; c < COLS; c++) {
      Key k = Layer.lookupOnActiveLayer(r, c);
      Key layer_key = Layer.getKey(numPadLayer, r, c);

      if (k == LockLayer(numPadLayer)) {
        row  = r;
        col = c;
      }

      if ((k != layer_key) || (k == Key_NoKey) || (k.flags != KEY_FLAGS)) {
        LEDControl.refreshAt(r, c);
      } else {
        LEDControl.setCrgbAt(r, c, color);
      }
    }
  }

  if (row > ROWS || col > COLS)
    return kaleidoscope::EventHandlerResult::OK;

  cRGB lock_color = breath_compute(lock_hue);
  LEDControl.setCrgbAt(row, col, lock_color);

  return kaleidoscope::EventHandlerResult::OK;
}

NumPad_ NumPad;
