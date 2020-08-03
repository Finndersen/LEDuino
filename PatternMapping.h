#ifndef PatternMapping_h
#define  PatternMapping_h
#include <FastLED.h>
#include <math.h>
#include "StripSegment.h"
#include "Pattern.h"
#include "Point.h"
#define DEFAULT_DURATION 10

// Base class for defining a mapping of a pattern to some kind of spatial configuration of LEDS
// E.g. a linear strip (single axis) or 3D spatial array of LEDs composed of multiple axes
class BasePatternMapping {
	public:
		// Constructor
		BasePatternMapping(
			uint16_t frame_delay,  // Delay between pattern frames (in ms)
			uint16_t duration=DEFAULT_DURATION,
			const char* name="BasePatternMapping"): name(name), duration(duration), frame_delay(frame_delay)  {}

		// Excute new frame of pattern and map results to LED array
		virtual void newFrame(CRGB* leds) {
			this->frame_time = millis()-this->start_time;
			// Sub-classes will need to override this method and implement further logic
		}
		
		// Initialise/Reset pattern state
		virtual void reset() {		
			DPRINT("Initialising pattern: ");
			DPRINTLN(this->name);
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
		
		// Set palette of underlying pattern(s)
		virtual void setPalette(CRGBPalette16 new_palette)	{};
		
		// Reset palette of underlying pattern(s) to one it was initialised with
		virtual void resetPalette()	{};
		
		const char* name;  // Name or description of pattern
	protected:
		unsigned long frame_time;			// Time since pattern started (in ms)
		uint16_t frame_delay;				// Delay between pattern frames (in ms)
		unsigned long start_time;			// Absolute time pattern was initialised (in ms)
		uint16_t duration;  				// Duration of pattern mapping configuration (seconds)

};


// Defines a mapping of a LinearPattern to a collection of LED Strip Segments
// The pattern will be interpolated to the length of each strip segment
class LinearPatternMapping: public BasePatternMapping {
	public:
		// Constructor
		LinearPatternMapping(
			LinearPattern& pattern,   		// LinearPattern object
			StripSegment* strip_segments,	// Pointer to Array of StripSegment to map pattern to
			byte num_segments,				// Number of axes (length of strip_segments)
			uint16_t frame_delay,  			// Delay between pattern frames (in ms)
			uint16_t duration=DEFAULT_DURATION,
			const char* name='LinearPatternMapping' // Name to give this pattern configuration
		): BasePatternMapping(frame_delay, duration, name), pattern(pattern), strip_segments(strip_segments), num_segments(num_segments) {}

		// Initialise/Reset pattern state
		virtual void reset() override {		
			BasePatternMapping::reset();
			this->pattern.reset();
		};
		
		// Excute new frame of pattern and map results to LED array
		void newFrame(CRGB* leds)	override {
			BasePatternMapping::newFrame(leds);
			
			this->pattern.frameAction(this->frame_time);
			
			uint16_t p = this->pattern.resolution;			
			// Loop through all strip segments
			for (byte seg_id=0; seg_id<this->num_segments; seg_id++) {
				StripSegment& strip_segment = this->strip_segments[seg_id];
				uint16_t s = strip_segment.segment_len;
				// Perform downsampling of pattern data to strip segment
				for (uint16_t led_ind=0; led_ind<s; led_ind++) 	{
					
					// GEt index of first pattern pixel to downsample
					uint16_t start_index = (led_ind*p)/s;
					// Weighting to use on first pattern pixel 
					uint16_t first_weight = s - (led_ind*p - start_index*s);
					uint16_t remaining_weight = p;
					uint16_t r = 0, g = 0, b = 0;
					// Add weighted values to colour components from pattern state
					uint16_t pat_ind = start_index;
					do {						
						CRGB led_val = this->pattern.getLEDValue(pat_ind);
						uint16_t weight;
						if (pat_ind == start_index) {
							weight = first_weight;
						} else if (remaining_weight > s) {
							weight = s;
						} else {
							weight = remaining_weight;
						}
						r += weight*led_val.red;
						g += weight*led_val.green;
						b += weight*led_val.blue;
						pat_ind += 1;
						remaining_weight -= weight;
						
					} while (remaining_weight>0);
					// Assign downsampled pixel value
					leds[strip_segment.getLEDId(led_ind)] = CRGB(r/p, g/p, b/p);
					
				}
				
			}

		}
		
		// Set palette of underlying pattern(s)
		virtual void setPalette(CRGBPalette16 new_palette)	{
			this->pattern.setPalette(new_palette);
		};
		
		// Reset palette of underlying pattern(s) to one it was initialised with
		virtual void resetPalette()	{
			this->pattern.resetPalette();
		};
		
		LinearPattern& pattern;
		
	protected:
		StripSegment* strip_segments;
		byte num_segments;				// Number of configured strip segments to map pattern to

};

// Class for defininig mapping configuration of 3DPattern to set of axes with spatial positioning
class SpatialPatternMapping: public BasePatternMapping {
	public:
		// Constructor
		SpatialPatternMapping(
			SpatialPattern& pattern,   				// Reference to SpatialPattern object
			SpatialStripSegment* spatial_segments,	// Pointer to Array of SpatialStripSegment to map pattern to
			byte num_segments,						// Number of SpatialStripSegments (length of spatial_segments)
			uint16_t frame_delay,  			// Delay between pattern frames (in ms)
			uint16_t duration=DEFAULT_DURATION,
			const char* name='SpatialPatternMapping' 			// Name to give this pattern configuration
		): BasePatternMapping(frame_delay, duration, name), pattern(pattern), spatial_segments(spatial_segments), num_segments(num_segments)	{}
		
		// Initialise/Reset pattern state
		virtual void reset() override {		
			BasePatternMapping::reset();
			this->pattern.reset();
		};
		
		// Excute new frame of pattern and map results to LED array
		void newFrame(CRGB* leds)	override {
			BasePatternMapping::newFrame(leds);			
			this->pattern.frameAction(this->frame_time);
			
			// Loop through every LED (axis and axis position combination), determine spatial position and get value
			for (byte axis_id=0; axis_id < this->num_segments; axis_id++) {
				SpatialStripSegment& axis = this->spatial_segments[axis_id];
				// Loop through all positions on axis
				for (uint16_t axis_pos=0; axis_pos<axis.strip_segment.segment_len; axis_pos++) {
					// Get position from spatial axis
					Point pos = axis.getSpatialPosition(axis_pos);
					// Get LED ID from strip segment
					uint16_t led_id = axis.strip_segment.getLEDId(axis_pos);
					// Get value from pattern
					leds[led_id] = this->pattern.getLEDValue(pos);
				}
			}
		}
		
		// Set palette of underlying pattern(s)
		virtual void setPalette(CRGBPalette16 new_palette)	{
			this->pattern.setPalette(new_palette);
		};
		
		// Reset palette of underlying pattern(s) to one it was initialised with
		virtual void resetPalette()	{
			this->pattern.resetPalette();
		};
		
		SpatialPattern& pattern;
	protected:
		SpatialStripSegment* spatial_segments;
		byte num_segments;						// Number of configured strip segments to map pattern to
};

// Allows for multiple pattern mappings to be applied at the same time
// Can have multiple LinearPatternMapping or SpatialPatternMappings running concurrently on different parts of the same strip of LEDS
class MultiplePatternMapping : public BasePatternMapping {
	public:
		// Constructor
		MultiplePatternMapping(
			BasePatternMapping* mappings,	// Pointer to Array of SpatialStripSegment to map pattern to
			byte num_mappings,				// Number of axes (length of spatial_segments)
			uint16_t frame_delay,  			// Delay between pattern frames (in ms)
			uint16_t duration=DEFAULT_DURATION,
			const char* name="MultiplePatternMapping" 			// Name to give this pattern configuration
		): BasePatternMapping(frame_delay, duration, name), mappings(mappings), num_mappings(num_mappings)	{}

		// Initialise/Reset pattern state
		virtual void reset() override {		
			BasePatternMapping::reset();
			for (byte i=0; i < this->num_mappings; i++) {
				this->mappings[i].reset();
			}
		};
		
		// Excute new frame of all pattern mappings
		void newFrame(CRGB* leds)	override {
			for (byte i=0; i < this->num_mappings; i++) {
				this->mappings[i].newFrame(leds);
			}
		}
		
		// Set palette of underlying pattern(s)
		virtual void setPalette(CRGBPalette16 new_palette)	{
			for (byte i=0; i < this->num_mappings; i++) {
				this->mappings[i].setPalette(new_palette);
			}
		};
		
		// Reset palette of underlying pattern(s) to one it was initialised with
		virtual void resetPalette()	{
			for (byte i=0; i < this->num_mappings; i++) {
				this->mappings[i].resetPalette();
			}
		};
	protected:
		BasePatternMapping* mappings;
		byte num_mappings;
};
#endif