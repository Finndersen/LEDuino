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

<!--stackedit_data:
eyJoaXN0b3J5IjpbOTQzMTc5NTIyLDE3ODUwMjE0MTEsLTgyMD
YwNTg3MywyMTIxMTA1NDQxLC05NjU4MDQwMjcsNTc1NjM1ODY2
LC0xNjIyMDM4NTkxLDM5MDA3ODkyXX0=
-->