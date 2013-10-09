What
====

This is the source code for some wearable electronics projects I devised for Burning Man over the summer of 2013.

Why
===

The idea basically came from the title of the musical Joseph and the Technicolor Dreamcoat; I decided that if such a coat existed it would be perfect at Burning Man, and it would obviously have to be made of glowing fur.

How
===

I wasn't familiar with much in the way of electronics or Arduino so I took advice from some friends, used LEDs with the WS2811 chipset, the Teensy 3 as a controller, and the excellent OctoWS2811 library to drive the LEDs.

While the real goal was the coat, I first started prototyping the wiring and programming against a 16x16 grid of LED pixels which I eventually mounted on a backpack. So, there are two related versions of the code here, "backpack" and "jacket".

I used the Arduino/Teensyduino programming environment since I was on a limited time budget and didn't want to mess with build configuration and that was already done for me, but I felt like I was fighting the build system as much as leveraging it. I'm probably doing it wrong. I wanted to share as much code as possible between the two projects, but the Arduino environment didn't seem to want to let me; it supports `#include` but, apparently, not relative paths in `#include`. OK, so symlinks to the rescue. I put all the code in a directory called "common", symlinked most of it into each of two directories named "jacket" and "backpack", and tried to trick the Arduino IDE into seeing a separate flat directory structure for each project. The slight differences between the two projects, I encoded in symbols exported by a "platform.h" which was separate for each project and `#include`d as necessary. This almost worked, except the Arduino IDE wouldn't even let me open the project unless the main file had the same name as the directory and an extension of .ino and was not a symlink. OK, hard links to the rescue.

This is all a long story to excuse the horrible organization of the code. If you're familiar with Arduino, you can probably figure out how to move the files you want around enough to get it to build. Maybe someday I'll revisit this and write standalone makefiles for it.

Credits
=======
* Teensy 3 is pretty awesome. I'm glad I started this project this year and not sooner; I was expecting to have to write the code a lot more carefully to get ok performance out of it, and it turns out that's not at all an issue.
* OctoWS2811 is pretty awesome. Tricking the DMA controller into generating the 800 KHz serial signal for the WS2811, on 8 lines at a time, for free... that's probably why Paul put the DMA controller in there in the first place, for all I know.
* The plasma routine is adapted from code by Edmund "Skorn" Horn, included in the OctoWS2811 distribution.
* The image data is stuff I found on the internet, converted through some scripts in the "images" directory. Hopefully there are no licensing issues with the source images. If you kno  otherwise, please contact me and I'll set it right.
