// This is an example using an LED strip split into 2 segments, with 2 different patterns mapped at the same time
#include <FastLED.h>
#include <LEDuino.h>

#define LED_DATA_PIN 2
// Using a WS2812b strip of 60 LEDs which we want to map patterns to
#define NUM_LEDS 60
// Split the LED strip into 3 segments each with length of 20 LEDs
#define SEGMENT_LEN 30
#define NUM_SEGMENTS 2

// Declare LED array
CRGB leds[NUM_LEDS];

// Declare Pixel array for patterns to use
CRGB pixel_data[SEGMENT_LEN];

// Define segments
StripSegment first_segment(0, SEGMENT_LEN, NUM_LEDS);
StripSegment second_segment(SEGMENT_LEN, SEGMENT_LEN, NUM_LEDS);

// Create array of segments
StripSegment first_segment_array[1] = {first_segment};
StripSegment second_segment_array[1] = {second_segment};

// Define patterns to use
SkippingSpikePattern skipping_spike_pattern(6);
TwinklePattern twinkle_pattern;

// Define mapping of patterns to segment
// Can only reuse a pixel_data array between different mapper configurations to be run at the same time if 
// no pattern depends on (reads) the previous state of the pixel_data. If so, it will need to use its own
// dedicated array
// Pulse pattern on first segment
LinearPatternMapper skipping_spike_mapping(skipping_spike_pattern, pixel_data, SEGMENT_LEN, first_segment_array, 1);
// Twinkle pattern on second segment (does not benefit from extra resolution, so just use SEGMENT_LEN)
LinearPatternMapper twinkle_mapping(twinkle_pattern, pixel_data, SEGMENT_LEN, second_segment_array, 1);

// MultiplePatternMapping configuration to run both at the same time
BasePatternMapper* mapper_array[NUM_SEGMENTS] = {
  &skipping_spike_mapping, 
  &twinkle_mapping
}; 
MultiplePatternMapper multi_mapping(mapper_array, NUM_SEGMENTS);

// Define array of MappingRunners for controller to use
MappingRunner mappings[1] = {
  MappingRunner(multi_mapping)
};

// Define controller
LEDuinoController controller(leds, NUM_LEDS, mappings, 1, false);

void setup() {
  // Initialise FastLED
  FastLED.addLeds<NEOPIXEL, LED_DATA_PIN>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  controller.initialise();
}

void loop() {
  // put your main code here, to run repeatedly:
  controller.loop();
}
