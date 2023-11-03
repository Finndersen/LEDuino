#ifndef Pattern_h
#define  Pattern_h
#include <FastLED.h>
#include "Point.h"
#include "utils.h"
#include "ColorPicker.h"

// Abstract Base class for patterns. Subclasses override frameAction() to implement pattern logic
// Pattern logic can be defined in terms of frames (so that speed will be determined by framerate), 
// or by absolute time (using frame_time or FastLED beatX functions)
class BasePattern	{
	public:
		// Constructor
		BasePattern(				
			const ColorPicker& color_picker=Basic_picker		// Colour picker/palette to use for pattern 
			): 
			color_picker(color_picker) {} 
			
			
		// Initialise/Reset pattern state
		virtual void reset() {};

	protected:
	
		// Select colour from current picker/palette
		CRGB getColor(uint8_t hue, uint8_t brightness=255) const {
			return this->color_picker.getColor(hue, brightness);
		}
		
		const ColorPicker& color_picker;
};

// Base class for patterns defined on a simple linear axis 
// Contains logic to update state on each frame, and populate a pixel array which will then be mapped to strip segments
class LinearPattern: public BasePattern	{
	public:
		LinearPattern(	
			const ColorPicker& color_picker=Basic_picker	// Colour picker/palette to use for pattern 
		): 
		BasePattern(color_picker) {}


		// Overidde frameAction() for updating pattern state with each frame, and setting the pixel values in pixel_data	
		virtual void frameAction(CRGB* pixel_data, uint16_t num_pixels, uint32_t frame_time) = 0;

};

// Pattern defined in 3D space. Converts a 3D coordinate of a pixel into a colour value
// The pattern occupies a 3D cube of space with boundaries at +/- 'resolution' on each axis
class SpatialPattern : public BasePattern {
	public:
		SpatialPattern(	
			const ColorPicker& color_picker=Basic_picker,	// Colour picker/palette to use for pattern 
			uint16_t resolution=256							// maximum magnitude of pattern space in +/- x, y and z directions
			): 
			BasePattern(color_picker), 
			resolution(resolution) {}

		// Contains main logic for pattern to update state (for sublcasses to override)
		// Provided time in ms since pattern started 		
		virtual void frameAction(uint32_t frame_time) = 0;

		// Get value for pixel at point coordinate.
		virtual CRGB getPixelValue(Point point) const { return CRGB::Black; }

		const uint16_t resolution;
};
#endif