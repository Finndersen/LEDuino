#ifndef PatternMapping_h
#define  PatternMapping_h
#include <FastLED.h>
#include <math.h>
#include "StripSegment.h"
#include "Pattern.h"
#include "Point.h"


// Base interface class for defining a mapping of a pattern to some kind of configuration of LEDS
// E.g. a linear segment (single axis) or 2D/3D spatial array of LEDs composed of multiple axes
class BasePatternMapper {
	public:
		// Initialise/Reset pattern state
		virtual void reset() const {};

		// Excute new frame of pattern and map results to LED array
		virtual void newFrame(CRGB* leds, uint16_t frame_time) const = 0;
		
		// Set palette of underlying pattern(s)
		virtual void setPalette(CRGBPalette16 new_palette) const = 0;
		
		// Reset palette of underlying pattern(s) to one it was initialised with
		virtual void resetPalette()	const = 0;

};

// Base class for Mappings that use a LinearPattern
class BaseLinearPatternMapper: public BasePatternMapper {
	public:
		BaseLinearPatternMapper(
			LinearPattern& pattern,   	// LinearPattern object
			CRGB* pixel_data,			// Pixel array for LinearPattern to mutate (length equal to num_pixels)
			uint16_t num_pixels		// Number of pixels for linear pattern to use (pattern resolution)
		): 
		pattern(pattern),
		pixel_data(pixel_data),
		num_pixels(num_pixels) {}

		// Initialise/Reset pattern state
		void reset() const override {
			fill_solid(this->pixel_data, this->num_pixels, CRGB::Black);
			this->pattern.reset();
		};

		// Set palette of underlying pattern(s)
		void setPalette(CRGBPalette16 new_palette) const override	{
			this->pattern.setPalette(new_palette);
		};
		
		// Reset palette of underlying pattern(s) to one it was initialised with
		void resetPalette()	const override {
			this->pattern.resetPalette();
		};
		
	protected:
		LinearPattern& pattern;
		CRGB* pixel_data;
		const uint16_t num_pixels;
		
};

// Handles the mapping of a LinearPattern to a collection of LED Strip Segments
// The pattern will be interpolated to the length of each strip segment
class LinearPatternMapper: public BaseLinearPatternMapper {
	public:
		// Constructor
		LinearPatternMapper(
			LinearPattern& pattern,   				// LinearPattern to map to segments
			CRGB* pixel_data,						// Pixel array for LinearPattern to mutate (length equal to num_pixels)
			uint16_t num_pixels,					// Number of pixels for linear pattern to use (pattern resolution)
			StripSegment* strip_segments,			// Array of StripSegments to map pattern to
			uint8_t num_segments					// Number of axes (length of strip_segments)
		): 
		BaseLinearPatternMapper(pattern, pixel_data, num_pixels), 
		strip_segments(strip_segments), 
		num_segments(num_segments) {}
		
		// Excute new frame of pattern and map results to LED array
		// This implementation involves calling pattern.getPixelValue() multiple times for the same pattern pixel index which is inefficient
		// Alternatively, it could be called once for every index and the results stored in an array which can be re-used
		// The two approaches are a trade-off between memory and CPU usage, but generally for linear patterns CPU is not a bottleneck,
		// and LinearStatePatterns have their own pixel array anyway and can be used if required
		void newFrame(CRGB* leds, uint16_t frame_time)	const override {
			// Run pattern logic
			this->pattern.frameAction(this->pixel_data, this->num_pixels, frame_time);			
			uint16_t pat_len = this->num_pixels;
			// Map pattern to all registered strip segments (will be scaled to each segment length)
			for (uint8_t seg_id=0; seg_id < this->num_segments; seg_id++) {
				StripSegment& strip_segment = this->strip_segments[seg_id];

				if (strip_segment.segment_len == pat_len) {
					// When segment length is equal to pattern pixel resolution, no need to downsample.
					interpolate_equal_length(leds, strip_segment);
				} else if (pat_len % strip_segment.segment_len == 0) {
					// Optimisation for when pattern length is an integer multiple of the segment length
					interpolate_integer_multiple_length(leds, strip_segment);
				} else {
					// General case of interpolating arbitrary length pattern data (resolution) to strip segment
					interpolate_arbitrary_length(leds, strip_segment);
				}			
			}
		}
		
	protected:
		// Interpolate pattern pixel data to the provided strip segment, when pattern length (resolution) is equal to segment length
		void interpolate_equal_length(CRGB* leds, StripSegment& strip_segment) const {
			for (uint16_t led_seg_ind=0; led_seg_ind<strip_segment.segment_len; led_seg_ind++) 	{		
				// Get LED strip index for LED 
				uint16_t led_strip_ind = strip_segment.getLEDId(led_seg_ind);				
				// Can translate directly from virtual pixels to segment LED
				leds[led_strip_ind] = this->pixel_data[led_seg_ind];
			}
		};

		// Interpolate pattern pixel data to the provided strip segment, when pattern length (resolution) is an integer multiple of segment length
		void interpolate_integer_multiple_length(CRGB* leds, StripSegment& strip_segment) const {
			uint8_t scale_factor = this->num_pixels / strip_segment.segment_len;
			for (uint16_t led_seg_ind=0; led_seg_ind<strip_segment.segment_len; led_seg_ind++) 	{		
				// Get LED strip index for LED 
				uint16_t led_strip_ind = strip_segment.getLEDId(led_seg_ind);	
				// Sum RGB values over all pattern pixels for the segment LED, then divide by count to get average
				uint16_t r = 0, g = 0, b = 0;
				uint16_t start_pixel_id = scale_factor*led_seg_ind;
				for (uint16_t pixel_id = start_pixel_id; pixel_id < start_pixel_id + scale_factor; pixel_id++)	{
					CRGB led_val = this->pixel_data[pixel_id];
					r += led_val.red;
					g += led_val.green;
					b += led_val.blue;
				};
				
				leds[led_strip_ind] = CRGB(r/scale_factor, g/scale_factor, b/scale_factor);
			}
		};

		// Interpolate pattern pixel data to the provided strip segment, for an arbitrary pattern length (resolution)
		void interpolate_arbitrary_length(CRGB* leds, StripSegment& strip_segment) const	{
			uint16_t seg_len = strip_segment.segment_len;
			uint16_t pat_len = this->num_pixels;
			for (uint16_t led_seg_ind=0; led_seg_ind<strip_segment.segment_len; led_seg_ind++) 	{		
				// Get LED strip index for LED 
				uint16_t led_strip_ind = strip_segment.getLEDId(led_seg_ind);
				// Perform downsampling of pattern data of any resolution to strip segment. 
				// Basically trying to scale pattern of length pat_len onto LED segment of length seg_len
				// The value of each actual LED will be derived from a weighted average of values from a range of virtual pattern pixels
				// To represent weights using integers (for efficiency), each pattern pixel will be weighted as a fraction of segment length seg_len (for convenience),
				// Therefore a weight value of 'seg_len' corresponds to weighting of 1 for that pattern pixel
				// The sum of weights of all pattern pixels for a strip segment LED is equal to pat_len (pattern length)
				// Get index of first pattern virtual pixel to downsample for current LED
				uint16_t start_index = (led_seg_ind*pat_len)/seg_len;
				// Weighting to use on first pattern pixel 
				uint16_t first_weight = seg_len - (led_seg_ind*pat_len - start_index*seg_len);
				uint16_t remaining_weight = pat_len;
				// Cumulative RGB colour values 
				uint16_t r = 0, g = 0, b = 0;
				// Add weighted values to colour components from pattern state
				uint16_t pat_ind = start_index;
				do {
					CRGB led_val = this->pixel_data[pat_ind];
					uint16_t weight;
					if (pat_ind == start_index) {
						weight = first_weight;
					} else if (remaining_weight > seg_len) {
						weight = seg_len;
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
				leds[led_strip_ind] = CRGB(r/pat_len, g/pat_len, b/pat_len);
			}
		};

		StripSegment* strip_segments;
		const uint8_t num_segments;				// Number of configured strip segments to map pattern to

};


// Class for handling the mapping of a 3DPattern to set of segments with spatial positioning
// The SpatialPattern has its own coordinate system (bounds of +/- resolution on each axis),
// and there is also the physical project coordinate system (the spatial positions of LEDS as defined in SpatialStripSegments)
// The 'scale' and 'offset' vectors are used to map the pattern coordinate system to project space
// If not specified, scale is calcualted automatically based on bounds of SpatialStripSegment, and offset is equal to project centroid
class SpatialPatternMapper: public BasePatternMapper {
	public:
		// Constructor
		SpatialPatternMapper(
			SpatialPattern& pattern,   					// Reference to SpatialPattern object
			SpatialStripSegment spatial_segments[],		// Array of SpatialStripSegments to map pattern to
			uint8_t num_segments,						// Number of SpatialStripSegments (length of spatial_segments)
			Point offset=undefinedPoint,				// Translational offset to apply to Project coordinate system before scaling
			Point scale_factors=undefinedPoint			// Scaling factors to apply to Project coordinate system to map to Pattern coordinates 
		): 
		pattern(pattern), 
		spatial_segments(spatial_segments), 
		num_segments(num_segments), 
		offset(offset), 
		scale_factors(scale_factors)	{
			// Calculate Project space scale
			Bounds project_bounds = get_spatial_segment_bounds(spatial_segments, num_segments);
			this->project_centroid = project_bounds.centre();
			
			// Set automatically if not specified (scale project bounds to fit pattern space)
			if (this->scale_factors == undefinedPoint) {
				this->scale_factors = (2.0*pattern.resolution)/project_bounds.magnitude();

			}
			// Default offset to centre of project bounds so it is translated to be centered on origin of pattern space
			if (this->offset == undefinedPoint) {
				this->offset = this->project_centroid;
			}
		}

		// Initialise/Reset pattern state
		void reset() const override {		
			BasePatternMapper::reset();
			this->pattern.reset();
		};
		
		// Excute new frame of pattern and map results to LED array
		void newFrame(CRGB* leds, uint16_t frame_time) const override {
			// Run pattern frame logic
			this->pattern.frameAction(frame_time);
			// Loop through every LED (segment and segment index combination), determine spatial position and get value
			for (uint8_t segment_id=0; segment_id < this->num_segments; segment_id++) {
				SpatialStripSegment& spatial_segment = this->spatial_segments[segment_id];
				// Loop through all positions on axis
				for (uint16_t segment_pos=0; segment_pos < spatial_segment.strip_segment.segment_len; segment_pos++) {
					// Get position from spatial axis
					Point pos = spatial_segment.getSpatialPosition(segment_pos);
					// Get LED ID from strip segment
					uint16_t led_id = spatial_segment.strip_segment.getLEDId(segment_pos);
					// Translate spatial position to pattern coordinates
					Point pattern_pos = ((pos - this->offset)).hadamard_product(this->scale_factors);
					// Get and assign LED value from pattern
					leds[led_id] = this->pattern.getPixelValue(pattern_pos);
				}
			}
		}
		
		// Set palette of underlying pattern(s)
		void setPalette(CRGBPalette16 new_palette) const override 	{
			this->pattern.setPalette(new_palette);
		};
		
		// Reset palette of underlying pattern(s) to one it was initialised with
		void resetPalette()	const override {
			this->pattern.resetPalette();
		};		
		
	protected:
		SpatialPattern& pattern;
		SpatialStripSegment* spatial_segments;	// Array of points to SpatialStripSegments to map pattern to
		const uint8_t num_segments;		// Number of configured strip segments to map pattern to
		Point offset;  					// Offset of Pattern space from Project space (in Project coordinates, before scaling applied)
		Point scale_factors; 			// Scaling vector for Project space to Pattern space transformation
		Point project_centroid; 		// Centre point of project coordinate bounds
};

// Allows for mapping a linear pattern to a vector (linear path/direction) in 3D space
// The pattern pixel applied to each LED is determined by the LED's distance from the perpendicular plane at the start of the vector path
// By default, the linear pattern path will start on the edge of the projects bounds and move through it in the direction of the vector and end at the bounds on the other side
// The start position and length of the path can be adjusted using the offset and scale parameters
// Since the pattern pixel for an LED is determine by its distance from the start position, be default the effect will be mirrored about the start of the vector path
// If start position is outside the bounds of the LEDs, then this will not make any difference. Otherwise, this can be disabled by setting mirrored=false (with an extra performance cost)
class LinearToSpatialPatternMapper : public BaseLinearPatternMapper {
	public:
		// Constructor
		LinearToSpatialPatternMapper (
			LinearPattern& pattern,   					// LinearPattern object
			CRGB* pixel_data,							// Pixel array for LinearPattern to mutate (length equal to pattern resolution)
			uint16_t num_pixels,						// Number of pixels for linear pattern to use (pattern resolution)
			Point pattern_vector,						// Direction vector to map pattern to
			SpatialStripSegment spatial_segments[],		// Array of SpatialStripSegments to map pattern to
			uint8_t num_segments,						// Number of SpatialStripSegments (length of spatial_segments)
			int16_t offset=0,							// Offset of pattern vector start position
			float scale=1,								// Scaling factor to apply to linear pattern vector length
			bool mirrored=true							// Whether linear pattern is mirrored around start position on vector
		): 	
		BaseLinearPatternMapper(pattern, pixel_data, num_pixels), 
		pattern_vector(pattern_vector), 
		spatial_segments(spatial_segments), 
		num_segments(num_segments), 
		mirrored(mirrored)  {			
			// Get vector length 
			float vector_len = this->pattern_vector.norm();

			// Get bounding box of all Spatial Segments
			Bounds bounds = get_spatial_segment_bounds(spatial_segments, num_segments);
			Point bounds_size = bounds.magnitude();
			// Get full length of linear pattern vector within spatial bounding box
			uint16_t unscaled_path_len = (abs(this->pattern_vector.x*bounds_size.x) + abs(this->pattern_vector.y*bounds_size.y) + abs(this->pattern_vector.z*bounds_size.z))/vector_len;
			
			// Get start position of pattern path (centre of bounds - (unscaled_path_len/2 - offset) in direction of pattern_vector)
			this->path_start_pos = bounds.centre() - (((unscaled_path_len/2 - offset) * this->pattern_vector)/vector_len);
			// Apply scale to full vector length to get desired path length
			this->path_length = scale * unscaled_path_len;
			this->path_end_pos = this->path_start_pos + (this->path_length * this->pattern_vector)/vector_len;
			
			// Pre-Calculate coefficent D of plane equation (for calculating distance from plane)
			this->plane_eq_D = pattern_vector.x*this->path_start_pos.x + pattern_vector.y*this->path_start_pos.y + pattern_vector.z*this->path_start_pos.z;
			// Pre-calculate inverse of pattern vector norm
			this->inv_pattern_vect_norm = 1/vector_len;
			// Pre-calculate pattern resolution / length constant
			this->res_per_len = ((float) this->num_pixels-1.0)/this->path_length;
		}
		
		// Excute new frame of pattern and map results to LED array
		void newFrame(CRGB* leds, uint16_t frame_time) const override {
			// Run pattern logic
			this->pattern.frameAction(this->pixel_data, this->num_pixels, frame_time);
			// Loop through every LED (axis and axis position combination), determine spatial position and appropriate state from pattern
			for (uint8_t segment_id=0; segment_id < this->num_segments; segment_id++) {
				SpatialStripSegment& spatial_axis = this->spatial_segments[segment_id];
				// Loop through all positions on segment
				for (uint16_t segment_pos=0; segment_pos<spatial_axis.strip_segment.segment_len; segment_pos++) {
					// Get global LED ID from strip segment
					uint16_t led_id = spatial_axis.strip_segment.getLEDId(segment_pos);
					// Get spatial position of LED
					Point led_pos = spatial_axis.getSpatialPosition(segment_pos);
					// If mirroring is not enabled, need check if LED pos is in pattern_vector direction from start_pos
					if (!this->mirrored) {
						// Get projected LED position on pattern path
						Point pos_on_path = led_pos.hadamard_product(this->pattern_vector) * this->inv_pattern_vect_norm;
						if (!between(pos_on_path.x, this->path_start_pos.x, this->path_end_pos.x) || 
							!between(pos_on_path.y, this->path_start_pos.y, this->path_end_pos.y) || 
							!between(pos_on_path.y, this->path_start_pos.z, this->path_end_pos.z)) {
							leds[led_id] = CRGB::Black;
							continue;
						}
					}
					// Get distance of LED from plane through pattern path start position (use pre-calculated constants instead of Point.distance_to_plane() for efficiency)
					uint16_t dist_from_start = abs(this->pattern_vector.x*led_pos.x + this->pattern_vector.y*led_pos.y + this->pattern_vector.z*led_pos.z - this->plane_eq_D) * this->inv_pattern_vect_norm;
					if (dist_from_start > this->path_length) {
						// All LEDS beyond the end of the path should be set to black
						leds[led_id] = CRGB::Black;
					} else {
						// Get pattern value at same proportional position along pattern axis
						// For now just round to nearest, could do interpolation between two
						uint16_t pattern_axis_pos = round(dist_from_start*this->res_per_len);
						leds[led_id] = this->pixel_data[pattern_axis_pos];
					}
				}
			}
		}
		
	protected:
		const Point pattern_vector;   			// Vector of direction to apply linear pattern
		SpatialStripSegment* spatial_segments;
		const uint8_t num_segments;				// Number of configured strip segments to map pattern to
		const bool mirrored;

		Point path_start_pos, path_end_pos;
		uint16_t path_length;				// Length of path that linear pattern will travel through
		float plane_eq_D, inv_pattern_vect_norm, res_per_len;  // Pre-calculated constants for plane distance calculation
};

// Allows for multiple pattern mappings to be applied at the same time
// Can have multiple LinearPatternMapper or SpatialPatternMappings running concurrently on different parts of the same strip of LEDS
// Will run the new frame logic of all included patterns, so could be CPU intensive and cause lag
// Need to specify a global frame_delay which will override that of the included PatternMappings
class MultiplePatternMapper : public BasePatternMapper {
	public:
		// Constructor
		MultiplePatternMapper(
			BasePatternMapper** mappings,				// Array of pointers to other PatternMappings to apply
			uint8_t num_mappings						// Number of Pattern Mappings (length of mappings)
		): 
		mappings(mappings), 
		num_mappings(num_mappings)	{}

		// Initialise/Reset pattern state
		void reset() const override {	
			for (uint8_t i=0; i < this->num_mappings; i++) {
				this->mappings[i]->reset();
			}
		};
		
		// Excute new frame of all pattern mappings
		void newFrame(CRGB* leds, uint16_t frame_time) const override {
			for (uint8_t i=0; i < this->num_mappings; i++) {				
				this->mappings[i]->newFrame(leds, frame_time);
			}
		}
		
		// Set palette of underlying pattern(s)
		void setPalette(CRGBPalette16 new_palette) const override {
			for (uint8_t i=0; i < this->num_mappings; i++) {
				this->mappings[i]->setPalette(new_palette);
			}
		};
		
		// Reset palette of underlying pattern(s) to one it was initialised with
		void resetPalette() const	override {
			for (uint8_t i=0; i < this->num_mappings; i++) {
				this->mappings[i]->resetPalette();
			}
		};
	protected:
		BasePatternMapper** mappings;
		const uint8_t num_mappings;
};
#endif