// This is an example using an LED strip split into 2 segments, and a variety of pattern mapping configurations
#include <FastLED.h>
#include <LEDuino.h>

#define LED_DATA_PIN 2
// Using a WS2812b strip of 60 LEDs which we want to map patterns to
#define NUM_LEDS 60
// Split the LED strip into 2 segments each with length of 30 LEDs
#define SEGMENT_LEN 30
#define NUM_SEGMENTS 2

// Declare LED array
CRGB leds[NUM_LEDS];

// Declare Pixel array with 2* segment length resolution for smooth motion of Pulse pattern
CRGB pixel_data[SEGMENT_LEN*2];

// Define segment and segment array
StripSegment first_segment(0, SEGMENT_LEN, NUM_LEDS);
StripSegment second_segment(SEGMENT_LEN, SEGMENT_LEN, NUM_LEDS);
StripSegment full_strip_segment(0, NUM_LEDS, NUM_LEDS);

// Create array of segments
StripSegment segments_normal[NUM_SEGMENTS] = {first_segment, second_segment};
// Create another configuration, but with the 2nd segment reversed
StripSegment segments_second_reversed[NUM_SEGMENTS] = {first_segment, -second_segment};
// 
StripSegment full_strip_segments[1] = {full_strip_segment};

// Define patterns to use
MovingPulsePattern pulse_pattern(8);
PridePattern pride_pattern(8);

// Define mapping of patterns to segment
// Pulse pattern duplicated on both segments
LinearPatternMapper pulse_mapping(pulse_pattern, pixel_data, SEGMENT_LEN*2, segments_normal, NUM_SEGMENTS);
// Pulse pattern but reversed on second segment (so pulses will meet in middle)
LinearPatternMapper pulse_reversed_mapping(pulse_pattern, pixel_data, SEGMENT_LEN*2, segments_second_reversed, NUM_SEGMENTS);
// Pride pattern on the full LED strip
LinearPatternMapper pride_mapping(pride_pattern, pixel_data, SEGMENT_LEN*2, full_strip_segments, 1);

// Define array of MappingRunners for controller to use
MappingRunner mappings[3] = {
  MappingRunner(pulse_mapping),           
  MappingRunner(pulse_reversed_mapping),
  MappingRunner(pride_mapping)
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
