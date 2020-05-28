#ifndef Pattern_h
#define  Pattern_h
#include <FastLED.h>
#include "Point.h"
#include "utils.h"
#include "palettes.h"
#define DEFAULT_PATTERN_DURATION 10

// Abstract Base class for patterns. Override frameAction() to implement pattern logic
class BasePattern	{
	public:
		// Constructor
		BasePattern(	
			unsigned int frame_delay,					// Delay between pattern frames (in ms)
			CRGBPalette16 colour_palette=White_p,		// Colour palette to use for pattern (default to white)
			byte duration=DEFAULT_PATTERN_DURATION		// Duration of pattern (in s)
			): frame_delay(frame_delay), duration(duration), colour_palette(colour_palette), initial_palette(colour_palette) {} 
			

		// Initialise/Reset pattern state
		virtual void reset() {		
			DPRINTLN("Initialising pattern");
			this->start_time = millis();
			this->frame_time = 0;
		};
		
		// Determine whether pattern has expired (exceeded duration)	
		bool expired()	{
			return this->frame_time >= (this->duration*1000);
		};
		
		// Whether new frame is ready (frame_delay has elapsed)
		bool frameReady()	{
			return (millis() - this->start_time - this->frame_time) >= this->frame_delay;
		};
		
		// Set new pattern palette
		void setPalette(CRGBPalette16 new_palette)	{
			this->colour_palette = new_palette;
		}
		
		// Reset palette to one pattern was initialised with
		void resetPalette()	{
			this->colour_palette = this->initial_palette;
		}
		
		
		// Called each time frame_delay expires
		// Takes byte value representing sound level from microphone to enable music-responsive patterns
		void newFrame(byte snd_lvl) {
			this->frame_time = millis()-this->start_time;
			this->sound_level = snd_lvl;
			this->frameAction();		
		};
		
	protected:
		// Method called after every frame_delay. Contains main logic for pattern, generally populates contents of pattern_state array but can update state of pattern in other ways
		// Takes byte value representing sound level from microphone to enable music-responsive patterns
		virtual void frameAction()=0;
		// Get colour from current palette
		CRGB colorFromPalette(byte hue, byte brightness=255, TBlendType blendType=LINEARBLEND) {
			return ColorFromPalette(this->colour_palette, hue, brightness, blendType);
		}
		
		// Get palette colour
		unsigned long frame_time;					// Time of current frame in ms relative to pattern start time
		unsigned int frame_delay;					// Delay between pattern frames (in ms)
		const byte duration;						// Duration of pattern in seconds
		unsigned long start_time;					// Absolute time pattern was initialised (in ms)
		byte sound_level=0;
		CRGBPalette16 colour_palette, initial_palette; // Current and initial colour palettes
};

// Base class for patterns defined on a single linear strip segment
// Converts an segment position into an LED value
class LinearPattern: public BasePattern	{
	public:
		LinearPattern(	
			unsigned int axis_len,						// Length of segment to apply axis to
			unsigned int frame_delay,					// Delay between pattern frames (in ms)
			CRGBPalette16 colour_palette=White_p,		// Colour palette to use for pattern 
			byte duration=DEFAULT_PATTERN_DURATION		// Duration of pattern (in s)
			): BasePattern(frame_delay, colour_palette, duration), axis_len(axis_len)  {}
		
		unsigned int axis_len;
		
		// Get value for LED at position 'i' along axis 
		virtual CRGB getLEDValue(unsigned int i);
};

// Base class for linear pattern which uses an array of length t_axis_len to store state 
// Used for more complex patterns that need to use detailed state from previous frame
template<unsigned int t_axis_len> 
class LinearStatePattern : public LinearPattern	{
	public:
		// Constructor
		LinearStatePattern(	
			unsigned int frame_delay,							// Delay between pattern frames (in ms)
			CRGBPalette16 colour_palette=White_p,		// Colour palette to use for pattern (default to white)
			byte duration=DEFAULT_PATTERN_DURATION		// Duration of pattern (in s)
			): LinearPattern(t_axis_len, frame_delay, colour_palette, duration) {
				
			}
		
		virtual void reset()	override {
			LinearPattern::reset();
			// Reset pattern state array to black
			for (byte i=0; i<axis_len; i++) {
				this->pattern_state[i] = CRGB::Black;
			}	
		}
		
		virtual CRGB getLEDValue(unsigned int i) override {
			//Read value from pattern_state
			return this->pattern_state[i];
		}
	protected:
		CRGB pattern_state[t_axis_len];					// Contains LED values for pattern
};


// Pattern defined in 3D space. Converts a 3D coordinate into an LED value
// The pattern occupies a 3D space constrained by attribute 'bounds'. 
// 
class SpatialPattern : public BasePattern {
	public:
		SpatialPattern(	
			Point bounds,							// Point vector defining maximum magnitude of pattern space in x, y and z directions
			unsigned int frame_delay,				// Delay between pattern frames (in ms)
			CRGBPalette16 colour_palette=White_p,	// Colour palette to use for pattern (default to white)
			byte duration=DEFAULT_PATTERN_DURATION	// Duration of pattern (in s)
			): BasePattern(frame_delay, colour_palette, duration), bounds(bounds) {}

		// Get value for LED at point coordinate. Point will be within range (+/-bounds.x, +/-bounds.y, +/-bounds.z)
		virtual CRGB getLEDValue(Point point);
		
	protected:
		Point bounds;  // Point vector defining maximum magnitude of pattern space in x, y and z directions

};
#endif