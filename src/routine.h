#pragma once

static const size_t ROUTINE_STATEBUF_SIZE = 1024; // 1K oughtta be enough for anybody. Right?


class FrameTimingInfo {
public:
   int beatLength;    // ms per beat (typically 500)
   int frameLength;   // ms per frame (typically 33)
   int beatRelative;  // how long since the last beat
};


class Routine {
public:
   // subclasses must implement begin and drawOnFrameSync
   // the rest are optional
   virtual void begin(void *stateBuf) = 0;
   virtual void adjustParam(int step) {};
   virtual void startBeatMeasure() {};
   virtual void drawOnBeatSync(FrameTimingInfo *frameTiming) {};
   virtual void drawOnFrameSync(FrameTimingInfo *frameTiming) = 0;
   virtual bool drawBetweenFrames(FrameTimingInfo *frameTiming) {
      return false;
   };
};


class OrientationRoutine: public Routine {
public:
   void begin(void *stateBuf);
   void drawOnFrameSync(FrameTimingInfo *frameTiming);

   typedef struct {
      int x;
      int y;
      int oldX;
      int oldY;
      int color;
   } Data;
   Data *data;
};


class StripeRoutine: public Routine {
public:
   void begin(void *stateBuf);
   void adjustParam(int step);
   void startBeatMeasure();
   void drawOnBeatSync(FrameTimingInfo *frameTiming);
   void drawOnFrameSync(FrameTimingInfo *frameTiming);

   typedef struct {
      int row;
      int direction;
      int color;
      int beat;
      int skipErase;
   } Data;
   Data *data;
};


class PlasmaRoutine: public Routine {
public:
   void begin(void *stateBuf);
   void adjustParam(int step);
   void drawOnBeatSync(FrameTimingInfo *frameTiming);
   void drawOnFrameSync(FrameTimingInfo *frameTiming);
   bool drawBetweenFrames(FrameTimingInfo *frameTiming);

   typedef struct {
      unsigned long frameCount;
      int allowedBright;
      int maxAllowedBright;
      int step;
   } Data;
   Data *data;
};


class ColorWash: public Routine {
public:
   void begin(void *stateBuf);
   void adjustParam(int step);
   void drawOnBeatSync(FrameTimingInfo *frameTiming);
   void drawOnFrameSync(FrameTimingInfo *frameTiming);

   typedef struct {
      int step;
      int color;
      int direction;
      int mode; // 0-2, controls brightness and fade behavior
   } Data;
   Data *data;
};


class Sparkle: public Routine {
public:
   void begin(void *stateBuf);
   void adjustParam(int step);
   void drawOnBeatSync(FrameTimingInfo *frameTiming);
   void drawOnFrameSync(FrameTimingInfo *frameTiming);
   bool drawBetweenFrames(FrameTimingInfo *frameTiming);

   typedef struct {
      int mode; // 0-2, controls brightness and fade behavior
      int last;
   } Data;
   Data *data;
};


class ThrobRoutine: public Routine {
public:
   void begin(void *stateBuf);
   void drawOnFrameSync(FrameTimingInfo *frameTiming);

   typedef struct {
      int color;
      int step;
   } Data;
   Data *data;
};


class GeoGrow: public Routine {
public:
   void begin(void *stateBuf);
   void adjustParam(int step);
   void drawOnBeatSync(FrameTimingInfo *frameTiming);
   void drawOnFrameSync(FrameTimingInfo *frameTiming);

   static const int numBrightSteps = 4;

   typedef struct {
      bool active;
      int color;
      int cx;
      int cy;
      int numSides;
      float rotation;
      float size;
   } Step;

   typedef struct {
      // constant
      int end;
      float sizeStep;
      // param
      int brightness;
      int mode;
      // current pass
      Step current;
      // history
      bool reset;
      int ringIndex;
      Step historyRingBuf[4];
   } Data;
   Data *data;

private:
   void addToHistory();
   void drawFromHistory();
};


class Simon: public Routine {
public:
   void begin(void *stateBuf);
   void drawOnFrameSync(FrameTimingInfo *frameTiming);
   void drawOnBeatSync(FrameTimingInfo *frameTiming);

   typedef struct {
      int numSteps;
      int skip;
      int step;
   } Data;
   Data *data;
};


class TranslucentSquares: public Routine {
public:
   void begin(void *stateBuf);
   void adjustParam(int step);
   void drawOnFrameSync(FrameTimingInfo *frameTiming);

   static const int minSize = 4;
   static const int maxSize = 8;
   static const int numSquares = 3;
   static const int numBrightSteps = 4;

   typedef struct {
      int x;
      int y;
      int dx;
      int dy;
      int size;
      int color;
   } Square;
   typedef struct {
      Square square[numSquares];
      int brightness;
   } Data;
   Data *data;
};


class SwirlRoutine: public Routine {
public:
   void begin(void *stateBuf);
   void adjustParam(int step);
   void drawOnFrameSync(FrameTimingInfo *frameTiming);

   static const int numBrightSteps = 4;

   typedef struct {
      int lastClick;
      int olderClick;
      int brightness;
      float angleOffset;
      float cx;
      float cy;
      int dcx;
      int dcy;
   } Data;
   Data *data;

   static const int numSteps = 14; // ideally a multiple of number of colors
};


class SnakeRoutine: public Routine {
public:
   void begin(void *stateBuf);
   void adjustParam(int step);
   void drawOnFrameSync(FrameTimingInfo *frameTiming);

   typedef enum { RIGHT, UP, LEFT, DOWN } Direction;
   typedef struct {
      int lastClick;
      int x;
      int y;
      Direction direction;
      int color;
   } Data;
   Data *data;

private:
   void newDirection();
};


class DripRoutine: public Routine {
public:
   void begin(void *stateBuf);
   void adjustParam(int step);
   void drawOnBeatSync(FrameTimingInfo *frameTiming);
   void drawOnFrameSync(FrameTimingInfo *frameTiming);

   static const int maxTrails = 16;
   static const int minTrailLength = 3;
   static const int maxTrailLength = 8;

   typedef struct {
      int x;
      int y;
      int color;
      bool active;
   } PointTrail;
   typedef struct {
      int trailLength;
      PointTrail trails[maxTrails];
   } Data;
   Data *data;

private:
   void addPoint(int colorLimit);
};
