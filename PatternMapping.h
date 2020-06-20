#ifndef PatternMapping_h
#define  PatternMapping_h
#include <FastLED.h>
#include "Axis.h"
#include "Pattern.h"
#include "Point.h"

// Base class for defining a mapping of a pattern to some kind of spatial configuration of LEDS
// E.g. a linear strip (single axis) or 3D spatial array of LEDs composed of multiple axes
class BasePatternMapping {
	public:
		// Constructor
		BasePatternMapping(BasePattern& pattern, byte num_axes): pattern(pattern), num_axes(num_axes) {}
		// Reset state of PatternMapping underlying pattern(s)
		//virtual void reset() {
		//	
		//}
		// Excute new frame of pattern and map results to LED array
		virtual void newFrame(CRGB* leds)=0;

		BasePattern& pattern;
	protected:
		
		byte num_axes;
};

// Class to define mapping of a pattern to set of axes
// Pattern AXIS_LEN must be equal to length of axes it is mapped to
class LinearPatternMapping: public BasePatternMapping {
	public:
		// Constructor
		LinearPatternMapping(
			BasePattern& pattern,   		// Pointer to LinearPattern object
			StripSegment* strip_segments,	// Pointer to Array of StripSegment to map pattern to
			byte num_axes					// Number of axes (length of strip_segments)
		): BasePatternMapping(pattern, num_axes),  strip_segments(strip_segments)	{}

		// Excute new frame of pattern and map results to LED array
		void newFrame(CRGB* leds)	override {
			this->pattern.newFrame();
			// Get pattern LED values and apply to axes
			unsigned int axis_len = this->strip_segments[0].segment_len;
			for (unsigned int axis_pos=0; axis_pos < axis_len; axis_pos++) {
				// Get LED value for axis position
				CRGB led_val = this->pattern.getLEDValue(axis_pos);
				// Get corresponding LED strip position for each axis and apply value
				for (byte axis_id=0; axis_id<this->num_axes; axis_id++) {
					StripSegment& axis = this->strip_segments[axis_id];
					leds[axis.getLEDId(axis_pos)] = led_val;
				}
			}
		}

	private:
		//LinearPattern& pattern;
		StripSegment* strip_segments;
};


// Class for defininig mapping configuration of 3DPattern to set of axes with spatial positioning
class SpatialPatternMapping: public BasePatternMapping {
	public:
		// Constructor
		SpatialPatternMapping(
			BasePattern& pattern,   	// Reference to LinearPattern object
			SpatialAxis* spatial_axes,	// Pointer to Array of SpatialAxis to map pattern to
			byte num_axes				// Number of axes (length of spatial_axes)
		): BasePatternMapping(pattern, num_axes), spatial_axes(spatial_axes)	{}

		// Excute new frame of pattern and map results to LED array
		void newFrame(CRGB* leds)	override {
			this->pattern.newFrame();
			// Loop through every LED (axis and axis position combination), determine spatial position and get value
			for (byte axis_id=0; axis_id < this->num_axes; axis_id++) {
				SpatialAxis& axis = this->spatial_axes[axis_id];
				// Loop through all positions on axis
				for (unsigned int axis_pos=0; axis_pos<axis.strip_segment.segment_len; axis_pos++) {
					// Get position from spatial axis
					Point pos = axis.getSpatialPosition(axis_pos);
					// Get LED ID from strip segment
					unsigned int led_id = axis.strip_segment.getLEDId(axis_pos);
					// Get value from pattern
					leds[led_id] = this->pattern.getLEDValue(pos);
				}
			}
		}
	protected:
		//SpatialPattern& pattern;
		SpatialAxis* spatial_axes;
};

#endif