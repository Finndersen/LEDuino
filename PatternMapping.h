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
		unsigned long start_time;			// Absolute time pattern was initialised (in ms)
		uint16_t duration;  				// Duration of pattern mapping configuration (seconds)
		uint16_t frame_delay;				// Delay between pattern frames (in ms)

};


// Defines a mapping of a LinearPattern to a collection of LED Strip Segments
// The pattern will be interpolated to the length of each strip segment
class LinearPatternMapping: public BasePatternMapping {
	public:
		// Constructor
		LinearPatternMapping(
			LinearPattern& pattern,   		// LinearPattern object
			StripSegment* strip_segments,	// Pointer to Array of StripSegment to map pattern to
			uint8_t num_segments,				// Number of axes (length of strip_segments)
			uint16_t frame_delay,  			// Delay between pattern frames (in ms)
			const char* name="LinearPatternMapping", // Name to give this pattern configuration
			uint16_t duration=DEFAULT_DURATION
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
			for (uint8_t seg_id=0; seg_id<this->num_segments; seg_id++) {
				StripSegment& strip_segment = this->strip_segments[seg_id];
				uint16_t s = strip_segment.segment_len;
				// Perform downsampling of pattern data to strip segment. 
				// Basically trying to scale pattern of length p onto LED segment of length s
				// The value of each actual LED will be derived from a weighted average of values from a range of virtual pattern LEDs
				// To represent weights using integers, each pattern pixel will be weighted as a fraction of segment length s (for convenience)
				// A value of 's' corresponds to weighting of 1 and means the entire pattern pixel contributes to the value of the strip segment LED
				// The total weight of all pattern pixels for a strip segment LED is equal to p (pattern length)
				for (uint16_t led_ind=0; led_ind<s; led_ind++) 	{
					
					// Get index of first pattern virtual pixel to downsample
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
		
	protected:
		LinearPattern& pattern;
		StripSegment* strip_segments;
		uint8_t num_segments;				// Number of configured strip segments to map pattern to

};


// Get the bounding box of a collection of Spatial Segments
Bounds get_spatial_segment_bounds(SpatialStripSegment* spatial_segments, uint8_t num_segments) {
	Point<int16_t> min = Point<int16_t>(32767, 32767, 32767);
	Point<int16_t> max = Point<int16_t>(-32768, -32768, -32768);
	
	for (uint8_t i=0; i<num_segments; i++) {
		SpatialStripSegment spatial_segment = spatial_segments[i];
		// Update minimums
		if (spatial_segment.start_pos.x < min.x) 	min.x = spatial_segment.start_pos.x;
		if (spatial_segment.end_pos.x < min.x) 		min.x = spatial_segment.end_pos.x;
		
		if (spatial_segment.start_pos.y < min.y) 	min.y = spatial_segment.start_pos.y;
		if (spatial_segment.end_pos.y < min.y) 		min.y = spatial_segment.end_pos.y;
		
		if (spatial_segment.start_pos.z < min.z) 	min.z = spatial_segment.start_pos.z;
		if (spatial_segment.end_pos.z < min.z) 		min.z = spatial_segment.end_pos.z;
		
		// Update maximums
		if (spatial_segment.start_pos.x > max.x) 	max.x = spatial_segment.start_pos.x;
		if (spatial_segment.end_pos.x > max.x) 		max.x = spatial_segment.end_pos.x;
		
		if (spatial_segment.start_pos.y > max.y) 	max.y = spatial_segment.start_pos.y;
		if (spatial_segment.end_pos.y > max.y) 		max.y = spatial_segment.end_pos.y;
		
		if (spatial_segment.start_pos.z > max.z) 	max.z = spatial_segment.start_pos.z;
		if (spatial_segment.end_pos.z > max.z) 		max.z = spatial_segment.end_pos.z;
	}
	return Bounds(min, max);
}

// Class for defininig mapping configuration of 3DPattern to set of axes with spatial positioning
// The SpatialPattern has its own coordinate system (bounds of +/- resolution on each axis),
// and there is also the physical project coordinate system (the spatial positions of LEDS as defined in SpatialStripSegments)
// The 'scale' and 'offset' vectors are used to map the pattern coordinate system to project space
// If not specified, scale is calcualted automatically based on bounds of SpatialStripSegment, and offset is equal to project centroid
class SpatialPatternMapping: public BasePatternMapping {
	public:
		// Constructor
		SpatialPatternMapping(
			SpatialPattern& pattern,   					// Reference to SpatialPattern object
			SpatialStripSegment* spatial_segments,		// Pointer to Array of SpatialStripSegment to map pattern to
			uint8_t num_segments,							// Number of SpatialStripSegments (length of spatial_segments)
			uint16_t frame_delay,  						// Delay between pattern frames (in ms)
			const char* name="SpatialPatternMapping", 	// Name to give this pattern configuration
			uint16_t duration=DEFAULT_DURATION,			
			Point<int16_t> offset=undefinedPoint,				// Translational offset to apply to Project coordinate system before scaling
			Point<float> scale_factors=undefinedPoint			// Scaling factors to apply to Project coordinate system to map to Pattern coordinates (where 256 = 1)
		): BasePatternMapping(frame_delay, duration, name), pattern(pattern), spatial_segments(spatial_segments), num_segments(num_segments), offset(offset), scale_factors(scale_factors)	{
			// Calculate Project space scale
			Bounds project_bounds = get_spatial_segment_bounds(spatial_segments, num_segments);
			this->project_centroid = project_bounds.centre();
			
			// Set automatically if not specified (scale project bounds to fit pattern space)
			if (this->scale_factors == undefinedPoint) {
				this->scale_factors = project_bounds.magnitude() * (256.0/(2*pattern.resolution));

			}
			// Default offset to centre of project bounds so it is translated to be centered on origin of pattern space
			if (this->offset == undefinedPoint) {
				this->offset = this->project_centroid;
			}
		}
		
		// Initialise/Reset pattern state
		virtual void reset() override {		
			BasePatternMapping::reset();
			this->pattern.reset();
		};
		
		// Excute new frame of pattern and map results to LED array
		void newFrame(CRGB* leds)	override {
			BasePatternMapping::newFrame(leds);
			this->pattern.frameAction(this->frame_time);
			DPRINT("Scale Factors");
			DPRINTLN(this->scale_factors);
			// Loop through every LED (axis and axis position combination), determine spatial position and get value
			for (uint8_t axis_id=0; axis_id < this->num_segments; axis_id++) {
				SpatialStripSegment& axis = this->spatial_segments[axis_id];
				DPRINT("Axis Vector");
				DPRINTLN(axis.end_pos - axis.start_pos);
				// Loop through all positions on axis
				for (uint16_t axis_pos=0; axis_pos<axis.strip_segment.segment_len; axis_pos++) {
					// Get position from spatial axis
					Point pos = axis.getSpatialPosition(axis_pos);
					//DPRINT("Step");
					//DPRINTLN(axis.step);

					DPRINT("LED Pos");
					DPRINTLN(pos);
					// Get LED ID from strip segment
					uint16_t led_id = axis.strip_segment.getLEDId(axis_pos);
					// Translate spatial position to pattern coordinates
					Point pattern_pos = ((pos - this->offset)*256).hadamard_divide(this->scale_factors);
					DPRINT("Pattern Pos");
					DPRINTLN(pattern_pos);
					// Get value from pattern
					leds[led_id] = this->pattern.getLEDValue(pattern_pos);
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
		
	protected:
		SpatialPattern& pattern;
		SpatialStripSegment* spatial_segments;
		uint8_t num_segments;		// Number of configured strip segments to map pattern to
		Point offset;  			// Offset of Pattern space from Project space (in Project coordinates, before scaling applied)
		Point scale_factors; 	// Scaling vector for Project space to Pattern space transformation
		Point project_centroid; // Centre point of project coordinate bounds
};

// Allows for mapping a linear pattern to a vector in 3D space
// The middle of the linear pattern path will be aligned with the centre of the bounding box of the spatial strip segments, plus an optional offset
// The length of the linear pattern path will be equal to the distance through the bounding box in the direction of the desired vector, multipled by scale factor
class LinearToSpatialPatternMapping : public BasePatternMapping {
	public:
		// Constructor
		LinearToSpatialPatternMapping (
			LinearPattern& pattern,   		// LinearPattern object
			Point pattern_vector,						// Vector to map pattern to
			SpatialStripSegment* spatial_segments,		// Pointer to Array of SpatialStripSegment to map pattern to
			uint8_t num_segments,							// Number of SpatialStripSegments (length of spatial_segments)
			uint16_t frame_delay,  						// Delay between pattern frames (in ms)
			const char* name="LinearToSpatialPatternMapping", 	// Name to give this pattern configuration
			uint16_t duration=DEFAULT_DURATION,			
			uint16_t offset=0,			// Offset of pattern vector start position
			uint16_t scale=256,					// Scaling factor to apply to linear pattern vector length (fraction of 256),
			bool mirrored=false				// Whether linear pattern is mirrored around start position on vector
		): BasePatternMapping(frame_delay, duration, name), 	
		pattern(pattern), pattern_vector(pattern_vector), spatial_segments(spatial_segments), num_segments(num_segments), mirrored(mirrored)  {			
			// Get vector length 
			float vector_len = this->pattern_vector.norm();
			this->inverse_vector_len_scale = 256/vector_len;
			// Get bounding box of all Spatial Segments
			Bounds bounds = get_spatial_segment_bounds(spatial_segments, num_segments);
			Point bounds_size = bounds.magnitude();
			// Get full length of linear pattern vector within spatial bounding box
			uint16_t full_path_len = (abs(this->pattern_vector.x*bounds_size.x) + abs(this->pattern_vector.y*bounds_size.y) + abs(this->pattern_vector.z*bounds_size.z))/vector_len;
			
			// Get start position of pattern path (centre of bounds - (full_path_len/2 - offset)in direction of pattern_vector)
			this->path_start_pos = bounds.centre() - (this->pattern_vector * (full_path_len/2 - offset)/vector_len);
			// Apply scale to full vector length to get desired path length
			this->path_length = (scale * full_path_len)/256;
			this->path_end_pos = this->path_start_pos + (this->pattern_vector * this->path_length)/vector_len;
		}
		
		
		// Initialise/Reset pattern state
		virtual void reset() override {
			BasePatternMapping::reset();
			this->pattern.reset();
		};
		
		// Set palette of underlying pattern(s)
		virtual void setPalette(CRGBPalette16 new_palette)	{
			this->pattern.setPalette(new_palette);
		};
		
		// Reset palette of underlying pattern(s) to one it was initialised with
		virtual void resetPalette()	{
			this->pattern.resetPalette();
		};	
		
		// Excute new frame of pattern and map results to LED array
		void newFrame(CRGB* leds)	override {
			/*
			DPRINT("Start Pos");
			DPRINTLN(this->path_start_pos);
			DPRINT("End Pos");
			DPRINTLN(this->path_end_pos);
			DPRINT("Length");
			DPRINTLN(this->path_length);
			DPRINT("Vector");
			DPRINTLN(this->pattern_vector);
			*/
			
			BasePatternMapping::newFrame(leds);			
			this->pattern.frameAction(this->frame_time);
			// Loop through every LED (axis and axis position combination), determine spatial position and appropriate state from pattern
			for (uint8_t axis_id=0; axis_id < this->num_segments; axis_id++) {
				SpatialStripSegment& spatial_axis = this->spatial_segments[axis_id];
				// Loop through all positions on axis
				for (uint16_t axis_pos=0; axis_pos<spatial_axis.strip_segment.segment_len; axis_pos++) {
					// Get global LED ID from strip segment
					uint16_t led_id = spatial_axis.strip_segment.getLEDId(axis_pos);
					// Get spatial position of LED
					Point led_pos = spatial_axis.getSpatialPosition(axis_pos);
					//DPRINT("LED Pos");
					//DPRINTLN(led_pos);
					// If mirroring is not enabled, check if LED pos is in pattern_vector direction from start_pos
					if (!this->mirrored) {
						//Point rel_pos = led_pos - this->path_start_pos;
						// Get projected LED position on pattern path
						Point pos_on_path = led_pos.hadamard_product(this->pattern_vector).scale(this->inverse_vector_len_scale);
						
						
						/*
						DPRINT("Path Pos");
						DPRINTLN(pos_on_path);
						*/


						//((rel_pos.x*this->pattern_vector.x < 0) || (rel_pos.y*this->pattern_vector.y < 0) || (rel_pos.z*this->pattern_vector.z < 0))
						if (!between(pos_on_path.x, this->path_start_pos.x, this->path_end_pos.x) || 
							!between(pos_on_path.y, this->path_start_pos.y, this->path_end_pos.y) || 
							!between(pos_on_path.y, this->path_start_pos.z, this->path_end_pos.z)) {
							leds[led_id] = CRGB::Black;
							//DPRINTLN("Point is outside pattern path range");
							continue;
						}
					}
					// Get distance of LED from plane through pattern path start position
					uint16_t dist_from_start = led_pos.distance_to_plane(this->pattern_vector, this->path_start_pos);
					// Get pattern value at same proportional position along pattern axis
					// For now just round to nearest, could do interpolation between two
					uint16_t pattern_axis_pos = round((dist_from_start*(this->pattern.resolution-1))/this->path_length);
					leds[led_id] = this->pattern.getLEDValue(pattern_axis_pos);
				}
			}
		}
		
	protected:
		LinearPattern& pattern;
		Point pattern_vector;   // Vector of direction to apply linear pattern
		uint16_t inverse_vector_len_scale;		// Precomputed 1/length of pattern vector as fraction of 256
		SpatialStripSegment* spatial_segments;
		uint8_t num_segments;						// Number of configured strip segments to map pattern to
		uint16_t path_length;				// Length of path that linear pattern will travel through
		Point path_start_pos, path_end_pos;
		bool mirrored;
};

// Allows for multiple pattern mappings to be applied at the same time
// Can have multiple LinearPatternMapping or SpatialPatternMappings running concurrently on different parts of the same strip of LEDS
class MultiplePatternMapping : public BasePatternMapping {
	public:
		// Constructor
		MultiplePatternMapping(
			BasePatternMapping** mappings,	// Pointer to Array of pointers to other PatternMappings to apply
			uint8_t num_mappings,				// Number of Pattern Mappings (length of mappings)
			uint16_t frame_delay,  			// Delay between pattern frames (in ms)
			const char* name="MultiplePatternMapping", 			// Name to give this pattern configuration
			uint16_t duration=DEFAULT_DURATION
		): BasePatternMapping(frame_delay, duration, name), mappings(mappings), num_mappings(num_mappings)	{}

		// Initialise/Reset pattern state
		virtual void reset() override {	
			BasePatternMapping::reset();
			for (uint8_t i=0; i < this->num_mappings; i++) {
				this->mappings[i]->reset();
			}
		};
		
		// Excute new frame of all pattern mappings
		void newFrame(CRGB* leds)	override {
			BasePatternMapping::newFrame(leds);
			for (uint8_t i=0; i < this->num_mappings; i++) {				
				this->mappings[i]->newFrame(leds);
			}
		}
		
		// Set palette of underlying pattern(s)
		virtual void setPalette(CRGBPalette16 new_palette)	{
			for (uint8_t i=0; i < this->num_mappings; i++) {
				this->mappings[i]->setPalette(new_palette);
			}
		};
		
		// Reset palette of underlying pattern(s) to one it was initialised with
		virtual void resetPalette()	{
			for (uint8_t i=0; i < this->num_mappings; i++) {
				this->mappings[i]->resetPalette();
			}
		};
	protected:
		BasePatternMapping** mappings;
		uint8_t num_mappings;
};
#endif