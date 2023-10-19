#ifndef STRIPSEGMENT_H
#define  STRIPSEGMENT_H
#include "Point.h"
#include "utils.h"
// #include "Array.h"
/*
 * Class to define an StripSegment which corresponds to a sub-section of an LED Strip.  
 * Specify starting offset and lenth of segment. Allows extending over LED strip limits (wrap around from end back to start)
 */
class StripSegment {
	public:
		// Constructor
		StripSegment(
			uint16_t start_offset,	// Start offset of segment (relative to start of LED strip)
			uint16_t segment_len,   // Length of segment (number of LEDS)
			uint16_t strip_len,     // Full length of LED strip (to enable wrap-over)
			bool reverse=false      // Whether segment is reversed (LED strip ID decreases with increasing segment index)
			): 
			start_offset(start_offset), 
			segment_len(segment_len), 
			strip_len(strip_len), 
			reverse(reverse) {}

		// Get LED Strip ID from segment position
		uint16_t getLEDId(uint16_t segment_pos) const {
			uint16_t led_id;
			// Limit value to maximum length
			if (segment_pos > this->segment_len)	{
				segment_pos = this->segment_len;
			}
			if (this->reverse)  {
				// Detect wrap around from 0 back to end of LED strip
				led_id = wrap_subtract(this->start_offset, segment_pos+1, this->strip_len-1);
			} else {
				// Modulo with strip len to enable wrap-around
				led_id = (this->start_offset + segment_pos)%this->strip_len;
			}
			
			return  led_id;
		}

		// Negation operator overloading to get reverse version of segment
		// New segment will cover the same set of LEDs, but have shifted start_offset and be in reverse direction
		StripSegment operator-()	{
			int reverse_start_offset;
			if (this->reverse)	{
				reverse_start_offset = this->start_offset - this->segment_len;
				// Handle wrap-around below 0
				if (reverse_start_offset < 0)	{
					reverse_start_offset = this->segment_len + reverse_start_offset;
				}
				return StripSegment(reverse_start_offset, this->segment_len, this->strip_len, false);
				
			} else {
				reverse_start_offset = (this->start_offset + this->segment_len)%this->strip_len;
				return StripSegment(reverse_start_offset, this->segment_len, this->strip_len, true);
			}
		}
		
		const uint16_t start_offset, segment_len, strip_len;
		const bool reverse;
};

// Base interface class for SpatialStripSegment, used for typing
class SpatialStripSegmentInterface 	{
	public:
		SpatialStripSegmentInterface(
			const StripSegment& strip_segment 		// LED Strip segment
		): strip_segment(strip_segment) {}

		// Get spatial bounding area covered by this spatial segment
		virtual Bounds get_bounds() = 0;
		// Get spatial position of an LED on the segment 
		virtual Point getSpatialPosition(uint16_t segment_pos)	= 0;

		const StripSegment& strip_segment;		// LED Strip segment for axis

};

// Class to define spatial positioning of a strip segment for use with a SpatialPatternMapping
// Provide a StripSegment along with an array of Points which define the positions of each LED in the segment
// If the segment is straight and LEDs are evenly spaced, can initialise with the start and end positions of the segment 
// and the coordinates for each LED will be automatically calculated.
// Generally want to define axis positions such that the coordinate origin is at the physical centre of your project
// template<size_t t_segment_length>
class SpatialStripSegment : public SpatialStripSegmentInterface {
	public:
		// Construct with pre-defined array of LED positions
		SpatialStripSegment(
			const StripSegment& strip_segment, 		// LED Strip segment
			Point* led_positions 					// Array of coordinates of segment LEDs (same length as segment)
		): SpatialStripSegmentInterface(strip_segment), led_positions(led_positions) {}

		// If the segment is straight and LEDs are evenly spaced, can initialise with the start and end positions 
		// of the segment and the coordinates for each LED will be automatically calculated
		SpatialStripSegment(
			const StripSegment& strip_segment, 	// LED Strip segment
			Point* led_positions, 				// Empty pre-allocated array (same length as segment) to fill with calculated values 
			Point start_pos, 					// Start position of straight segment in 3D  (Position of first LED)
			Point end_pos						// End position of straight segment in 3D space (Position of last LED)
			): SpatialStripSegment(strip_segment, led_positions)  {				
				// Pre-Calculate coordinate positions of each LED in strip segment
				for (uint16_t i=0; i < strip_segment.segment_len; i++) {
					led_positions[i] = start_pos + (end_pos-start_pos)*(((float) i)/(strip_segment.segment_len-1));
				}
			}
		
		// Get spatial bounding area covered by this spatial segment
		virtual Bounds get_bounds() {
			return get_bounds_of_points(this->led_positions, this->strip_segment.segment_len);
		};

		// Get spatial position of an LED on the segment 
		Point getSpatialPosition(uint16_t segment_pos)	{
			// Constrain to max position
			if (segment_pos > this->strip_segment.segment_len) {
				segment_pos = this->strip_segment.segment_len;
			}
			return this->led_positions[segment_pos];
		}
		
		// Negation operator overloading to get reverse version of axis
		// New axis will cover the same set of LEDs, new start_pos will be old end_pos, and direction reversed
		// SpatialStripSegment operator-()	{
		// 	// Reverse start and end position
		// 	return SpatialStripSegment(-(this->strip_segment), this->end_pos, this->start_pos);
		// }
		
		// ~SpatialStripSegment() {
		// 	DPRINT("DECONSTRUCT");
		// 	//delete [] this->led_positions;
		// }
	protected:
		Point* led_positions; 	// Array of coordinate positions of each LED in strip segment
};

#endif