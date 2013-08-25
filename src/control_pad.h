#pragma once

class InputButton {
public:
   void begin(int pin, bool needPullup, Scene::Action action, int param);

   int pin;
   Scene::Action action;
   int param;
   bool wasPressed;

   bool isPressed();
   bool getChange(bool *pressed);
};


class ControlPad {
public:
   void begin(int pinModePrev, int pinModeNext,
              int pinParamPrev, int pinParamNext,
              int pinSpeed, bool needPullup);
   void sample();

   int numButtons;
   InputButton buttons[5];
};


extern ControlPad controls;
