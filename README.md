# LED Pattern Mapper
This is a framework or set of components which make it easy to define animated patterns and map them to segments of an LED strip controlled by an Arduino. It helps with developing more complex LED projects (infinity mirrors etc) that involve multiple changing patterns and arrangements of LED strips. It supports both 1-dimensional patterns (defined in terms of distance along a single axis) as well as 3D spatial patterns (defined in terms of 3D coordinates), and handles scaling and interpolation. It makes it easy to:
- Define linear or 3D animation patterns which can be initialised with parameter values to adjust behaviour
- Map a single linear pattern to multiple different segments of an LED strip
- Scale and interpolate a linear pattern to LED segments of different lengths
- Define the position of LED strip segments in space and map 3D patterns to them
- Have multiple pattern mapping configurations and cycle through them
- Have multiple different pattern mapping configurations running at the same time

NOTE: This is a project I have developed for my own personal use but figured could be quite useful to others as well, so it has not been tested extensively in many configurations. Please let me know if you have any issues or ideas!

## Requirements

 - Ardunio-compatible micocontroller. At least 32kB of RAM and decent CPU is recommended depending on the complexity of your project, a [Teensy 3.1+](https://www.pjrc.com/teensy/index.html) works great.
 - [FastLED](http://fastled.io/) Library
 - Individually addressable LED strip compatible with FastLED (e.g. Neopixel, WS2801, WS2811, WS2812B, LPD8806, TM1809, and [more](https://github.com/FastLED/FastLED/wiki/Chipset-reference))

## Installation

 1. Download library from [here](https://github.com/Finndersen/LEDController/archive/master.zip).
 2. Unzip and move to Arduino library folder (e.g. Documents/Arduino/libraries/)
 3. Rename folder to LEDPatternMapper
 4. Restart Arduino IDE

## Getting Started
The best way to get started and learn how to use the library is to check out the [tutorial](https://github.com/Finndersen/LEDController/wiki/Tutorial).

Then take a look at the [Reference](https://github.com/Finndersen/LEDController/wiki/Reference) for more in-depth details, and check out some [Examples](https://github.com/Finndersen/LEDController/wiki/Examples) to see how it might work for your project.
