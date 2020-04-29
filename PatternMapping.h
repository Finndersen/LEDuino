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
		//BasePatternMapping() {}
		// Proxy methods which route to associated pattern method
		virtual void init();
		virtual bool expired();
		virtual bool frameReady();
		// Excute new frame of pattern and map results to LED array
		virtual void newFrame(CRGB* leds, byte sound_level);	
		
};

// Class to define mapping of a pattern to set of axes
// Pattern AXIS_LEN must be equal to length of axes it is mapped to
class LinearPatternMapping: public BasePatternMapping {
	public:
		// Constructor
		LinearPatternMapping(
			LinearPattern* pattern,   	// Pointer to LinearPattern object
			LEDAxis* led_axes,			// Pointer to Array of LEDAxis to map pattern to
			byte num_axes				// Number of axes (length of led_axes)
		): pattern(pattern), led_axes(led_axes), num_axes(num_axes)	{}
		
		// Proxy methods which route to associated pattern method
		void init()	{
			return pattern->init();
		}
		bool expired()	{
			return pattern->expired();
		}
		bool frameReady() {
			return pattern->frameReady();
		}
		
		// Excute new frame of pattern and map results to LED array
		void newFrame(CRGB* leds, byte sound_level)	{
			CRGB led_val;
			pattern->newFrame(sound_level);
			// Get pattern LED values and apply to axes
			unsigned int axis_len = pattern->axis_len;
			for (unsigned int axis_pos=0; axis_pos<axis_len; axis_pos++) {
				// Get LED value for axis position
				led_val = pattern->get_led_value(axis_pos);
				// Get corresponding LED strip position for each axis and apply value
				for (byte axis_id=0; axis_id<num_axes; axis_id++) {
					LEDAxis* axis = &led_axes[axis_id];
					leds[axis->get_led_id(axis_pos)] = led_val;
				}

			}
		}

	private:
		LinearPattern* pattern;
		LEDAxis* led_axes;
		byte num_axes;

};


// Class for defininig mapping configuration of 3DPattern to set of axes with spatial positioning
class SpatialPatternMapping: public BasePatternMapping {
	public:
		// Constructor
		SpatialPatternMapping(
			SpatialPattern* pattern,   	// Pointer to LinearPattern object
			SpatialAxis* spatial_axes,	// Pointer to Array of SpatialAxis to map pattern to
			byte num_axes				// Number of axes (length of led_axes)
		): BasePatternMapping(), pattern(pattern), spatial_axes(spatial_axes), num_axes(num_axes) 	{}
		
		// Proxy methods which route to associated pattern method
		void init()	{
			return pattern->init();
		}
		bool expired()	{
			return pattern->expired();
		}
		bool frameReady() {
			return pattern->frameReady();
		}
		
		// Excute new frame of pattern and map results to LED array
		void newFrame(CRGB* leds, byte sound_level)	{
			
		}
	private:
		SpatialPattern* pattern;
		SpatialAxis* spatial_axes;
		byte num_axes;
};

#endif