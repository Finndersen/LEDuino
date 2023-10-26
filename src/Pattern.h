#ifndef Pattern_h
#define  Pattern_h
#include <FastLED.h>
#include "Point.h"
#include "utils.h"
#include "palettes.h"


// Abstract Base class for patterns. Subclasses override frameAction() to implement pattern logic
// Pattern logic can be defined in terms of frames (so that speed will be determined by framerate), 
// or by absolute time (using frame_time or FastLED beatX functions)
class BasePattern	{
	public:
		// Constructor
		BasePattern(				
			CRGBPalette16 colour_palette=White_p		// Colour palette to use for pattern (default to white)
			): 
			colour_palette(colour_palette), 
			initial_palette(colour_palette) {} 
			
			
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
		

	protected:
	
		// Select colour from current palette
		CRGB colorFromPalette(uint8_t hue, uint8_t bright=255, TBlendType blendType=LINEARBLEND) const {
			return ColorFromPalette(this->colour_palette, hue, bright, blendType);
		}
		
		CRGBPalette16 colour_palette;
		const CRGBPalette16 initial_palette;
};

// Base class for patterns defined on a simple linear axis 
// Contains logic to update state on each frame, and populate a pixel array which will then be mapped to strip segments
class LinearPattern: public BasePattern	{
	public:
		LinearPattern(	
			CRGBPalette16 colour_palette=White_p	// Colour palette to use for pattern 
		): 
		BasePattern(colour_palette) {}


		// Overidde frameAction() for updating pattern state with each frame, and setting the pixel values in pixel_data	
		virtual void frameAction(CRGB* pixel_data, uint16_t num_pixels, uint32_t frame_time) = 0;

};

// Base class for linear pattern which uses a CRGB array to store pixel state 
// Used for patterns that need to use historical LED state from previous frame, or for pre-computing LED values for efficiency 
// since getPixelValue(i) is called multiple times for same 'i' if there are multiple segments mapped
// Tradeoff between memory and CPU usage
// template<uint16_t t_resolution> 
// class LinearStatePattern : public LinearPattern	{
// 	public:
// 		// Constructor
// 		LinearStatePattern(	
// 			CRGBPalette16 colour_palette=White_p		// Colour palette to use for pattern (default to white)
// 		): LinearPattern(t_resolution, colour_palette) {}
		
// 		virtual void reset()	override {
// 			LinearPattern::reset();
// 			// Reset pattern state array to black
// 			for (uint8_t i=0; i<resolution; i++) {
// 				this->pattern_state[i] = CRGB::Black;
// 			}	
// 		}

// 		// Override frameAction() to implement pattern logic and populate LED values in pattern_state
		
// 		virtual CRGB getPixelValue(uint16_t i) const override {
// 			// Limit index to max res
// 			i = limit(i, this->resolution - 1);
// 			//Read value from pattern_state
// 			return this->pattern_state[i];
// 		}
// 	protected:
// 		CRGB pattern_state[t_resolution];					// Contains LED values for pattern
// };


// Pattern defined in 3D space. Converts a 3D coordinate of a pixel into a colour value
// The pattern occupies a 3D cube of space with boundaries at +/- 'resolution' on each axis
class SpatialPattern : public BasePattern {
	public:
		SpatialPattern(	
			CRGBPalette16 colour_palette=White_p,	// Colour palette to use for pattern (default to white)
			uint16_t resolution=256					// maximum magnitude of pattern space in +/- x, y and z directions
			): 
			BasePattern(colour_palette), 
			resolution(resolution) {}

		// Contains main logic for pattern to update state (for sublcasses to override)
		// Provided time in ms since pattern started 		
		virtual void frameAction(uint32_t frame_time) = 0;

		// Get value for pixel at point coordinate.
		virtual CRGB getPixelValue(Point point) const { return CRGB::Black; }

		const uint16_t resolution;
};
#endif