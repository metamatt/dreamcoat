#include <Arduino.h>
#include "defs.h"
#include "routine.h"
#include "scene.h"
#include "control_pad.h"

#include "routine.h"
#include "images.h"
#include "platform.h"


#define USE(r)   r r##_instance; // declare
   Platform_DeclareRoutines
#undef USE

#define USE(r)   &r##_instance, // use in table
Routine *routineTable[] = {
   Platform_DeclareRoutines
};
#undef USE


const int numRoutines = ARRAYSIZE(routineTable);

byte stateBuf[ROUTINE_STATEBUF_SIZE];

#if DEBUG
struct {
   int beatCount;
   int beatTime;
   int frameCount;
   int frameTime;
   int tweenCount;
   int tweenTime;
} debugTimes;
#endif

void Scene::begin() {
   blink = FALSE;
   beatLength = initialBeatLength;
   nextBeatTime = millis();
   lastTimerPress = 0;

   whichRoutine = 0;
   onChooseNewRoutine(0);
}


void Scene::loop() {
   // always check for input, as often as possible
   controls.sample();

   int now = millis();
   FrameTimingInfo frameTiming;
   frameTiming.beatLength = beatLength;
   frameTiming.frameLength = frameLength;
   frameTiming.beatRelative = (now - nextBeatTime) % nextBeatTime;

   // beats are longer than frames. Check for beat timer elapse, which also implies
   // a frame advance; otherwise check for frame advance; otherwise spin till one of
   // those elapses (where by "spin", I mean check for input and allow cheap animations).

   // DebugPrint("now %ld next %ld interbeat %d\n", now, nextBeatTime, beatTime);
   if (now >= nextBeatTime) {
      DebugPrint("New beat t=%d\n", now);
      // attempt to keep beats exactly on sync, if we fall behind, catch up
      nextBeatTime += beatLength;
      if (nextBeatTime <= now + nextBeatTime / 2) {
         nextBeatTime = now + beatLength;
      }
      nextFrameTime = now + frameLength;
      curRoutine->drawOnBeatSync(&frameTiming);

#if DEBUG
      debugTimes.beatCount += 1;
      debugTimes.beatTime += millis() - now;
      DebugPrint("Timing: %d beats avg %d ms; %d frames avg %d ms",
                 debugTimes.beatCount, debugTimes.beatTime / debugTimes.beatCount,
                 debugTimes.frameCount, debugTimes.frameTime / debugTimes.frameCount);
      if (debugTimes.tweenCount) {
         DebugPrint("; %d tweens avg %d ms",
                    debugTimes.tweenCount, debugTimes.tweenTime / debugTimes.tweenCount);
      }
      Serial.print("\n");
#endif

      // for debugging/monitoring, blink internal LED at frame rate
      blink = !blink;
      digitalWrite(builtinLedPin, blink ? HIGH : LOW);
   } else if (now >= nextFrameTime) {
      // When the inter-frame time elapses, tell current scene to draw a new frame
      // DebugPrint("New frame t=%ld\n", now);
      nextFrameTime = now + frameLength;
      curRoutine->drawOnFrameSync(&frameTiming);
#if DEBUG
      debugTimes.frameCount += 1;
      debugTimes.frameTime += millis() - now;
#endif
   } else {
      // otherwise, spend the time till the next frame letting the draw routine
      // diddle around
      bool used = curRoutine->drawBetweenFrames(&frameTiming);
#if DEBUG
      if (used) {
         debugTimes.tweenCount += 1;
         debugTimes.tweenTime += millis() - now;
      }
#endif
   }
}


void Scene::onButton(Action action, BOOL pressed, int step) {
   switch (action) {
      case ACTION_MODE:
         if (pressed) {
            DebugPrint("Adjust major mode %d\n", step);
            onChooseNewRoutine(step);
#if DEBUG
            memset(&debugTimes, 0, sizeof debugTimes);
#endif
         }
         break;

      case ACTION_PARAM:
         if (pressed) {
            DebugPrint("Adjust minor mode %d\n", step);
            curRoutine->adjustParam(step);
#if DEBUG
            memset(&debugTimes, 0, sizeof debugTimes);
#endif
         }
         break;

      case ACTION_SPEED:
         DebugPrint("Adjust frame time: %s\n", pressed ? "press" : "release");
         onSetTimer(pressed);
         break;
   }
}


void Scene::onChooseNewRoutine(int step) {
   whichRoutine = (whichRoutine + step + numRoutines) % numRoutines;
   curRoutine = routineTable[whichRoutine];

   curRoutine->begin(stateBuf);
}


void Scene::onSetTimer(BOOL pressed) {
   long now = millis();

   if (pressed) {
      // on press, we adjust the current beat series, unless it's been a while
      // (default 2 seconds) since the previous press, in which case we start
      // a new beat series. That also implies there's no setting this slower than
      // 30 bpm.
      if (now - lastTimerPress > beatTimerLatchLength) {
         DebugPrint("Beat timer: timeout reset\n");
         timeSeries.reset(now);
         beatLength = initialBeatLength; // slow down to 2 bps until we get more taps
         curRoutine->startBeatMeasure();
      } else {
         // add this data point to time series and take average
         int newBeatLength = timeSeries.addAndRecalc(now);
         if (!newBeatLength) {
            // if this was ignored due to debounce, skip the next-frame-on-press
            // logic below
            return;
         }
         beatLength = newBeatLength;
         DebugPrint("Beat interval now %dms\n", beatLength);
      }

      // always start new frame on press
      // nextFrameTime = nextBeatTime = now;
      lastTimerPress = now;
   } else {
      // on release, if it was held a while (default 750 ms), we start a new beat
      // series. This gives an easier way to start a new series without waiting 2 sec.
      if (now - lastTimerPress > beatTimerUnlatchLength) {
         DebugPrint("Beat timer: press-and-hold reset\n");
         lastTimerPress = 0; // make the next press look like a fresh one
         beatLength = initialBeatLength; // slow down to 4 fps until we get more taps
      }
   }
}


void TimeSeries::reset(long time) {
   start = end = time;
   numPoints = 0;
}

int TimeSeries::addAndRecalc(long time) {
   if (time - end < beatTimerMinimum) {
      // This is kind of a cheap debounce, but you wouldn't normally expect bounce on
      // this time scale. Anyway, I was seeing a lot of spurious events around 120 ms
      // after an intentional press. I can't intentionally tap any faster than ~160 ms
      // anyway, and even 200 is 300 bpm, so let's ignore faster than that.
      DebugPrint("ignoring beat pulse %ld ms below threshold %d\n", time - end, beatTimerMinimum);
      return 0;
   } else {
      end = time;
      numPoints++;
   }
   int delta = end - start;
   DebugPrint("beat rate %d ms from %d ms / %d samples\n", delta / numPoints, delta, numPoints);
   return delta / numPoints;
}
