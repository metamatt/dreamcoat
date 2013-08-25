#pragma once

class TimeSeries {
public:
   void reset(long time);
   int addAndRecalc(long time);

   const int beatTimerMinimum = 200; // ignore taps faster than 300 bpm (assume spurious/bounce)

   long start;
   long end;
   int numPoints;
};


class Scene {
public:
   typedef enum {
      ACTION_MODE,
      ACTION_PARAM,
      ACTION_SPEED
   } Action;

   const int beatTimerLatchLength = 2000;
   const int beatTimerUnlatchLength = 750;
   const int initialBeatLength = 500; // start at 2 bps == 120 bpm
   const int frameLength = 33; // ~30 fps for animations

   void begin();
   void loop();

   void onButton(Scene::Action action, BOOL pressed, int step);
   void onChooseNewRoutine(int step);
   void onSetTimer(BOOL pressed);

   int whichRoutine;
   Routine *curRoutine;
   BOOL blink;
   long nextFrameTime;
   long nextBeatTime;
   int beatLength;

   // state for timer-rate tracking
   long lastTimerPress;
   TimeSeries timeSeries;
};
