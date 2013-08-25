#include <Arduino.h>
#include "defs.h"
#include "routine.h"
#include "scene.h"
#include "control_pad.h"

extern Scene scene;

void ControlPad::begin(int pinModePrev, int pinModeNext,
                       int pinParamPrev, int pinParamNext,
                       int pinSpeed, bool needPullup)
{
   numButtons = 0;
   buttons[numButtons++].begin(pinModeNext, needPullup, Scene::ACTION_MODE, 1);
   buttons[numButtons++].begin(pinModePrev, needPullup, Scene::ACTION_MODE, -1);
   buttons[numButtons++].begin(pinParamNext, needPullup, Scene::ACTION_PARAM, 1);
   buttons[numButtons++].begin(pinParamPrev, needPullup, Scene::ACTION_PARAM, -1);
   buttons[numButtons++].begin(pinSpeed, needPullup, Scene::ACTION_SPEED, 1);
}


void ControlPad::sample(void) {
   for (int i = 0; i < numButtons; i++) {
      InputButton *button = buttons + i;
      bool pressed;
      if (button->getChange(&pressed)) { // should change this to getChange, report both press and release
         // onTimer wants release
         scene.onButton(button->action, pressed, button->param);
      }
   }
}


void InputButton::begin(int pin, bool needPullup, Scene::Action action, int param) {
   this->pin = pin;
   this->action = action;
   this->param = param;
   this->wasPressed = FALSE;

   if (pin) {
      int mode = needPullup ? INPUT_PULLUP : INPUT;
      pinMode(pin, mode);
   }
}


bool InputButton::isPressed() {
   if (!pin) {
      return FALSE;
   }
   // DebugPrint("InputButton sample pin %d result %d\n", pin, (int) digitalRead(pin));
   return digitalRead(pin) == LOW;
}


bool InputButton::getChange(bool *out) {
   bool pressed = isPressed();
   if (wasPressed) {
      if (!pressed) {
         *out = wasPressed = FALSE;
         return TRUE;
      }
   } else {
      if (pressed) {
         *out = wasPressed = TRUE;
         return TRUE;
      }
   }

   return FALSE;
}
