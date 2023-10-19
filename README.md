# Introduction
LEDuino is a powerful framework for defining animated patterns and mapping them to segments of an LED strip controlled by an Arduino. It consists of a collection of components which can be configured and combined to achieve a high level of customisation for your LED project.

LEDuino makes it easy to:
- Define animation patterns (linear or 3D) which can be initialised with parameter values and colour pallettes to adjust behaviour
- Split a single LED strip into segments which can have patterns mapped to them independently
- Map linear patterns to multiple different segments of an LED strip, with automatic scaling and interpolation
- Define the position of LEDs in space and map 3D patterns to them
- Run multiple different pattern mapping configurations at the same time
- Project a linear pattern along a vector in 3D space 
- Cycle through a sequence of pattern mapping configurations


## Demonstration
The below video demonstrates the same simple moving, colour changing pulse pattern being mapped in 3 different ways to my Infinity Cube project. 

[![InfinityCubePulseMapping](https://i.imgur.com/is2atVj.gif)](https://www.youtube.com/watch?v=DZlHctGWVvo "InfinityCubePulseMapping")

1. Linear mapped to each axis in positive direction (same pattern repeated on all edges, all pulses originate in same corner and move outwards then terminate at the end)
2. Same as #1 but one axis is reversed (looks like pulses are moving continuously and split or merge at corners).
3. Spatial mapping - instead of being mapped to each edge separately,  the pulse moves along a spatial vector from the top right to the bottom left corners of the cube, moving along the different edges as required

## Requirements

 - Ardunio-compatible micocontroller. At least 32kB of RAM, 128kB Flash and decent CPU is recommended depending on the complexity of your project. A [Teensy 3.1+](https://www.pjrc.com/teensy/index.html) works great (can comfortably run complex pattern configurations at 100+ FPS)
 - [FastLED](http://fastled.io/) Library
 - Individually addressable LED strip compatible with FastLED (e.g. Neopixel, WS2801, WS2811, WS2812B, LPD8806, TM1809, and [more](https://github.com/FastLED/FastLED/wiki/Chipset-reference))

## Installation
LEDuino can be installed from the Arduino or PlatformIO library manager. 

Otherwise, for manual installation:
 1. Download library from [here](https://github.com/Finndersen/LEDuino/archive/refs/heads/master.zip).
 2. Unzip and move to Arduino library folder (e.g. Documents/Arduino/libraries/) or PlatformIO project local library folder (lib/)
 3. Rename folder to LEDuino
 4. Restart Arduino IDE

## Getting Started
The best way to get started and learn how to use the library is to check out the [tutorial](https://github.com/Finndersen/LEDController/wiki/Tutorial).

Then take a look at the [Reference](https://github.com/Finndersen/LEDController/wiki/Reference) for more in-depth details, and check out some [Examples](https://github.com/Finndersen/LEDController/wiki/Examples) to see how it might work for your project.

NOTE: This is a project I have developed for my own personal use but figured could be quite useful to others as well, so it has not been tested extensively in many configurations. Please let me know if you have any issues or ideas!