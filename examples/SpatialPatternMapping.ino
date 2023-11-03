
// This is an example using an LED strip split into 4 segments, arranged in a square so that each segment is an edge of the square
#include <FastLED.h>
#include <LEDuino.h>

#define LED_DATA_PIN 2
// Using a WS2812b strip of 120 LEDs which we want to map patterns to
#define NUM_LEDS 120
// Split the LED strip into 4 segments each with length of 30 LEDs
#define SEGMENT_LEN 30
#define NUM_SEGMENTS 4
// Declare LED array
CRGB leds[NUM_LEDS];

// Declare Pixel array for linear pattern to use. Length depends on desired pattern resolution
#define NUM_PIXELS 40
CRGB pixel_data[NUM_PIXELS];

// Define segments
StripSegment segment1(0, SEGMENT_LEN, NUM_LEDS);
StripSegment segment2(SEGMENT_LEN, SEGMENT_LEN, NUM_LEDS);
StripSegment segment3(SEGMENT_LEN*2, SEGMENT_LEN, NUM_LEDS);
StripSegment segment4(SEGMENT_LEN*3, SEGMENT_LEN, NUM_LEDS);

// Define coordinates of corners of square (start and end positions of segments)
// We will define a coordinate system with origin at centre of square, and 100 distance to each edge
Point corner1 = Point(-100, 100, 0);
Point corner2 = Point(100, 100, 0);
Point corner3 = Point(100, -100, 0);
Point corner4 = Point(-100, -100, 0);

// Define spatial positioning of each segment
// Coordinates of LEDs will be automatically calculated using start and end position of segment
SpatialStripSegment<SEGMENT_LEN> spatial_segment1(segment1, corner1, corner2);
SpatialStripSegment<SEGMENT_LEN> spatial_segment2(segment2, corner2, corner3);
SpatialStripSegment<SEGMENT_LEN> spatial_segment3(segment3, corner3, corner4);
SpatialStripSegment<SEGMENT_LEN> spatial_segment4(segment4, corner4, corner1);

// Define array of pointers to spatial strip segments
SpatialStripSegment_T* spatial_segments[NUM_SEGMENTS] = {
  &spatial_segment1,
  &spatial_segment2,
  &spatial_segment3,
  &spatial_segment4
};

// Define linear patterns to map 
MovingPulsePattern pulse_pattern(8);
FirePattern<NUM_PIXELS> fire_pattern;

// Define mapping of linear patterns to spatial coordinates
LinearToSpatialPatternMapper spatial_pulse(
  pulse_pattern, 
  pixel_data, NUM_PIXELS, 
  Point(1,1,0),  // Direction for pulse to move (diagonally up-right from bottom left corner to opposite)
  spatial_segments, NUM_SEGMENTS);

LinearToSpatialPatternMapper spatial_fire(
  fire_pattern, 
  pixel_data, NUM_PIXELS, 
  Point(0,1,0),  // Direction for pulse to move (from bottom upwards)
  spatial_segments, NUM_SEGMENTS);


// Define array of MappingRunners for controller to use
MappingRunner mappings[2] = {
  MappingRunner(spatial_pulse),
  MappingRunner(spatial_fire)
};

// Define controller
LEDuinoController controller(leds, NUM_LEDS, mappings, 2, false);

void setup() {
  // Initialise FastLED
  FastLED.addLeds<NEOPIXEL, LED_DATA_PIN>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  controller.initialise();
}

void loop() {
  // put your main code here, to run repeatedly:
  controller.loop();
}
