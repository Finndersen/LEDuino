#ifndef Pattern_h
#define  Pattern_h
#include <FastLED.h>
#include "Point.h"
#include "utils.h"
#include "palettes.h"


// Abstract Base class for patterns. Override frameAction() to implement pattern logic
class BasePattern	{
	public:
		// Constructor
		BasePattern(	
			uint16_t resolution,					
			CRGBPalette16 colour_palette=White_p		// Colour palette to use for pattern (default to white)
			): resolution(resolution), colour_palette(colour_palette), initial_palette(colour_palette) {} 
			
			
		// Initialise/Reset pattern state
		virtual void reset() {};
		
		// Set new pattern palette
		void setPalette(CRGBPalette16 new_palette)	{
			this->colour_palette = new_palette;
		}
		
		// Reset palette to one pattern was initialised with
		void resetPalette()	{
			this->colour_palette = this->initial_palette;
		}
		
		
		// Contains main logic for pattern.
		// Provides time in ms since pattern started 		
		virtual void frameAction(unsigned long frame_time) {};
		
		uint16_t resolution;
	protected:
	
		// Select colour from current palette
		CRGB colorFromPalette(uint8_t hue, uint8_t bright=255, TBlendType blendType=LINEARBLEND) {
			return ColorFromPalette(this->colour_palette, hue, bright, blendType);
		}
		
		unsigned long frame_time;					// Time in ms since pattern started
		CRGBPalette16 colour_palette, initial_palette; // Current and initial colour palettes
};

// Base class for patterns defined on a simple linear axis 
// Converts a segment position into an LED value
// Overidde frameAction() for updating pattern state with each frame, 
// and getLEDValue() for getting value of LED at position along segment
class LinearPattern: public BasePattern	{
	public:
		LinearPattern(	
			uint16_t resolution,					// Number of virtual pixels along pattern axis		
			CRGBPalette16 colour_palette=White_p		// Colour palette to use for pattern 
		): BasePattern(resolution, colour_palette)   {}
		
		// Get value for LED at position 'i' along virtual pattern axis
		virtual CRGB getLEDValue(uint16_t i) { return CRGB::Black; }
		
};

// Base class for linear pattern which uses an array of length t_resolution to store state 
// Used for patterns that need to use historical LED state from previous frame, or for pre-computing LED values for efficiency 
// since getLEDValue(i) is called multiple times for same 'i' if there are multiple segments mapped
// Tradeoff between memory and CPU usage
template<uint16_t t_resolution> 
class LinearStatePattern : public LinearPattern	{
	public:
		// Constructor
		LinearStatePattern(	
			CRGBPalette16 colour_palette=White_p		// Colour palette to use for pattern (default to white)
		): LinearPattern(t_resolution, colour_palette) {}
		
		virtual void reset()	override {
			LinearPattern::reset();
			// Reset pattern state array to black
			for (uint8_t i=0; i<resolution; i++) {
				this->pattern_state[i] = CRGB::Black;
			}	
		}

		// Override frameAction() to implement pattern logic and populate LED values in pattern_state
		
		virtual CRGB getLEDValue(uint16_t i) override {
			//Read value from pattern_state
			return this->pattern_state[i];
		}
	protected:
		CRGB pattern_state[t_resolution];					// Contains LED values for pattern
};


// Pattern defined in 3D space. Converts a 3D coordinate of an LED into a colour value
// The pattern occupies a 3D cube of space with boundaries at +/- 'resolution' on each axis
class SpatialPattern : public BasePattern {
	public:
		SpatialPattern(	
			CRGBPalette16 colour_palette=White_p,	// Colour palette to use for pattern (default to white)
			uint16_t resolution=256				// maximum magnitude of pattern space in +/- x, y and z directions
			): BasePattern(resolution, colour_palette) {}

		// Get value for LED at point coordinate. (For SpatialPattern). 
		virtual CRGB getLEDValue(Point point) { return CRGB::Black; }

};
#endif