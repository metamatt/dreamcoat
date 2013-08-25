#pragma once

typedef struct {
   int cx;
   int cy;
   const uint8_t *data;
} ImageInfo;

class ImageRoutine: public Routine {
public:
   void begin(void *stateBuf);
   void adjustParam(int step);
   void drawOnBeatSync(FrameTimingInfo *frameTiming);
   void drawOnFrameSync(FrameTimingInfo *frameTiming);
   bool drawBetweenFrames(FrameTimingInfo *frameTiming);

   typedef struct {
      const ImageInfo *images;
      int numImages;

      int throbStep;
      int whichImage;
      bool sparkle;
      bool frameSparkled;
      bool randomizeOnBeat;
      int beatStep;
      int lastClick;
   } Data;
   Data *data;
};

class ThemeImageRoutine: public ImageRoutine {
public:
   void begin(void *stateBuf);
};

class MarioImageRoutine: public ImageRoutine {
public:
   void begin(void *stateBuf);
};
