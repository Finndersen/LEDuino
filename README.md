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

![enter image description here](https://i.imgur.com/1QFJoiz.png)
It consists of a single LED strip (44 LEDs total), divided into 4 segments (one for each edge of the rectangle). The LED strip is controlled by an Arduino (the single wire shown represents the data line, te
<!--stackedit_data:
eyJoaXN0b3J5IjpbMTc1NzYzMDMwNywxNzg1MDIxNDExLC04Mj
A2MDU4NzMsMjEyMTEwNTQ0MSwtOTY1ODA0MDI3LDU3NTYzNTg2
NiwtMTYyMjAzODU5MSwzOTAwNzg5Ml19
-->