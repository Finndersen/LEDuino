# LED Pattern Controller
This is a framework or set of components which make it easy to define animated patterns and map them to segments of an LED strip controlled by an Arduino. It helps with developing more complex LED projects (infinity mirrors etc) that involve multiple changing patterns and arrangements of LED strips. It supports both 1-dimensional patterns (defined in terms of distance along a single axis) as well as 3D spatial patterns (defined in terms of 3D coordinate), and handles scaling and interpolation. 

## Requirements

 - Ardunio-compatible micocontroller. At least 32kB of RAM and decent CPU is recommended depending on the complexity of your project, a [Teensy 3.1+](https://www.pjrc.com/teensy/index.html) works great.
 - [FastLED](http://fastled.io/) Library
 - Individually addressable LED strip compatible with FastLED (e.g. Neopixel, WS2801, WS2811, WS2812B, LPD8806, TM1809, and [more](https://github.com/FastLED/FastLED/wiki/Chipset-reference))

## Installation

 1. Download library from [here](https://github.com/Finndersen/LEDController/archive/master.zip).
 2. Unzip and move to Arduino library folder (e.g. Documents/Arduino/libraries/)
 3. Rename folder to LEDController
 4. Restart Arduino IDE

## Tutorial
The best way to explain how to use the library is probably with a simple example, in this case a rectangular infinity mirror or picture frame with an LED strip running along each edge. 

![Example LED strip diagram](https://i.imgur.com/1QFJoiz.png)
It consists of a single LED strip (44 LEDs total), divided into 4 segments (of length 8 and 14, one for each edge of the rectangle). The LED strip is controlled by an Arduino (the single wire shown represents the data line, the two power wires are omitted).

You might want to define a collection of animation patterns which are each mapped and repeated to each LED segment for a nice consistent, symmetric effect, instead of just a single pattern applied to the entire strip. That's what this library helps you do.

**Define Segments**
The first step is to define each segment (corresponding to each edge of the rectangle) in terms of their position on the LED strip. This is done by creating instances of `StripSegment` and specifying the segment start position, length and total number of LEDs.

#include <FastLED.h>
#include <LEDController.h>

#define VERTICAL_SEGMENT_LEN 8      // Number of LEDS on vertical segments
#define HORIZONTAL_SEGMENT_LEN 14   // Number of LEDS on horizontal segments
#define TOTAL_LEDS VERTICAL_SEGMENT_LEN*2 + HORIZONTAL_SEGMENT_LEN*2

StripSegment segment_1(0, VERTICAL_SEGMENT_LEN, TOTAL_LEDS);
StripSegment segment_2(VERTICAL_SEGMENT_LEN, HORIZONTAL_SEGMENT_LEN, TOTAL_LEDS);
StripSegment segment_3(VERTICAL_SEGMENT_LEN + HORIZONTAL_SEGMENT_LEN, VERTICAL_SEGMENT_LEN, TOTAL_LEDS);
StripSegment segment_4(2*VERTICAL_SEGMENT_LEN + HORIZONTAL_SEGMENT_LEN, HORIZONTAL_SEGMENT_LEN, TOTAL_LEDS);


<!--stackedit_data:
eyJoaXN0b3J5IjpbMTE1NDI2OTA5MSwtMTA4MzEwOTU2MiwtMT
Y0MjA2NzMyNiwxNzg1MDIxNDExLC04MjA2MDU4NzMsMjEyMTEw
NTQ0MSwtOTY1ODA0MDI3LDU3NTYzNTg2NiwtMTYyMjAzODU5MS
wzOTAwNzg5Ml19
-->