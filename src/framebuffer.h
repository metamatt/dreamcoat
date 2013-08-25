#pragma once

class Framebuffer {
public:
   Framebuffer();
   void begin();

   void setGridPixel(int x, int y, int color);
   int getGridPixel(int x, int y);
   void drawGridLine(int x0, int y0, int x1, int y1, int color);

   int scalePixel(int pixel, float scale);
   float remap(float value, float from1, float from2, float to1, float to2);

   void clearScreen();
   void fillScreen(int color);
   void fadeScreenByStep(int fade, int base);
   void fadeScreenByScale(float scale);

   void showWithLimit();
   void show();

   int randomColor(int componentLimit = 0xFF);
   int randomPrimary(int componentLimit = 0xFF);
   int randomPrimaryOrSecondary(int componentLimit = 0xFF);
   bool colorHasSameComponents(int c1, int c2);
   int addPixelComponents(int p1, int p2);
   int getPixelBrightness(int pixel);

   void drawNGram(float cx, float cy, unsigned numSides, float size, float rotate, int color);
   void fillRect(int x0, int y0, int x1, int y1, int color);

   int width;
   int height;
   int ledsPerStrip;
   int numPixels;
};


extern Framebuffer fb;
