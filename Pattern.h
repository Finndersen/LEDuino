#ifndef Pattern_h
#define  Pattern_h
#include <FastLED.h>
#include "Point.h"

#define DEFAULT_PATTERN_DURATION 10

// Abstract Base class for patterns. Override frameAction() to implement pattern logic
class BasePattern	{
	public:
		// Constructor
		BasePattern(	
			unsigned int frame_delay,					// Delay between pattern frames (in ms)
			byte duration=DEFAULT_PATTERN_DURATION		// Duration of pattern (in s)
			): frame_delay(frame_delay), duration(duration) {} 
			
		unsigned long frame_time;					// Time of current frame in ms relative to pattern start time
		unsigned int frame_delay;							// Delay between pattern frames (in ms)
		const byte duration;						// Duration of pattern in seconds
		// Perform pattern intialisation
		virtual void init() {		
			DPRINTLN("Initialising pattern");

			start_time = millis();
			frame_time = 0;
		}
		// Determine whether pattern has expired (exceeded duration)	
		bool expired()	{
			return frame_time >= (duration*1000);
		}
		// Whether new frame is ready (frame_delay has elapsed)
		bool frameReady()	{
			return (millis() - start_time - frame_time) >= frame_delay;
		}
		
		// Called each time frame_delay expires
		// Takes byte value representing sound level from microphone to enable music-responsive patterns
		void newFrame(byte sound_level) {
			frame_time = millis()-start_time;
			frameAction(sound_level);		
		}
		
	protected:
		// Method called after every frame_delay. Contains main logic for pattern, generally populates contents of pattern_state array but can update state of pattern in other ways
		// Takes byte value representing sound level from microphone to enable music-responsive patterns
		virtual void frameAction(byte sound_level) {}
		unsigned long start_time;					// Absolute time pattern was initialised (in ms)
		
};

// Base class for patterns defined on a single linear axis
// Converts an axis positio into an LED value
class LinearPattern: public BasePattern	{
	public:
		LinearPattern(	
			unsigned int axis_len,
			unsigned int frame_delay,					// Delay between pattern frames (in ms)
			byte duration=DEFAULT_PATTERN_DURATION		// Duration of pattern (in s)
			): BasePattern(frame_delay, duration), axis_len(axis_len)  {}
		
		unsigned int axis_len;
		
		// Get value for LED at position 'i' along axis 
		virtual CRGB get_led_value(unsigned int i);
};

// Base class for linear pattern which uses an array of length t_axis_len to store state 
// Used for more complex patterns that need to use detailed state from previous frame
template<unsigned int t_axis_len> 
class LinearStatePattern : public LinearPattern	{
	public:
		// Constructor
		LinearStatePattern(	
			unsigned int frame_delay,							// Delay between pattern frames (in ms)
			byte duration=DEFAULT_PATTERN_DURATION		// Duration of pattern (in s)
			): LinearPattern(t_axis_len, frame_delay, duration) {
		}
		CRGB pattern_state[t_axis_len];					// Contains LED values for pattern
		
		virtual void init()	{
			LinearPattern::init();
			// Reset pattern state array to black
			for (byte i=0; i<axis_len; i++) {
				pattern_state[i] = CRGB::Black;
			}	
		}
		
		virtual CRGB get_led_value(unsigned int i) {
			//Read value from pattern_state
			return pattern_state[i];
		}
};


// Pattern defined in 3D space. Converts a 3D coordinate into an LED value
// The pattern occupies a 3D space constrained by attribute 'bounds'. 
// 
class SpatialPattern : public BasePattern {
	public:
		SpatialPattern(	
			Point bounds,							// Point vector defining maximum magnitude of pattern space in x, y and z directions
			unsigned int frame_delay,				// Delay between pattern frames (in ms)
			byte duration=DEFAULT_PATTERN_DURATION	// Duration of pattern (in s)
			): BasePattern(frame_delay, duration), bounds(bounds) {}
		
		Point& bounds;
		
		// Get value for LED at point coordinate. Point will be within range (+/-bounds.x, +/-bounds.y, +/-bounds.z)
		virtual CRGB get_led_value(Point point);

};
#endif