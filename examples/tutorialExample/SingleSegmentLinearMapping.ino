// This is a basic example using an LED strip configured as a single segment, that will cycle through 3 linear patterns
#include <FastLED.h>
#include <LEDuino.h>

#define LED_DATA_PIN 2
// Using a WS2812b strip of 60 LEDs which we want to map patterns to
#define NUM_LEDS 60
// Declare LED array
CRGB leds[NUM_LEDS];

// If we wanted to use patterns that would benefit from higher resolution and interpolation for smooth looking motion,
// we could declare another CRGB pixel array that is larger than the actual LED array and can be used by patterns 
// to create animations with higher resolution than the number of physical LEDs. 
// In this simple example we will just use the LED array directly

// Define segment and segment array
StripSegment full_strip_segment(0, NUM_LEDS, NUM_LEDS);
StripSegment segment_array[1] = {full_strip_segment};


// Define patterns to use
MovingPulsePattern pulse_pattern(15);
TwinklePattern twinkle_pattern;
PridePattern pride_pattern(8);

// Define mapping of patterns to segment
LinearPatternMapper pulse_mapping(pulse_pattern, leds, NUM_LEDS, segment_array, 1);
LinearPatternMapper twinkle_mapping(twinkle_pattern, leds, NUM_LEDS, segment_array, 1);
LinearPatternMapper pride_mapping(pride_pattern, leds, NUM_LEDS, segment_array, 1);

// Define array of MappingRunners for controller to use
MappingRunner mappings[3] = {
  MappingRunner(pulse_mapping, 15, 10), // 20ms delay between frames (66FPS), duration of 10 seconds
  MappingRunner(twinkle_mapping, 30),   // 30ms frame delay (33 FPS) and default duration (15)
  MappingRunner(pride_mapping)          // Will use default frame delay (20) and duration (15)
};

// Define controller
LEDuinoController controller(leds, NUM_LEDS, mappings, 3, false);

void setup() {
  // Initialise FastLED
  FastLED.addLeds<NEOPIXEL, LED_DATA_PIN>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  controller.initialise();
}

void loop() {
  // put your main code here, to run repeatedly:
  controller.loop();
}
