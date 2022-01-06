#ifndef Axis_h
#define  Axis_h
#include "Point.h"
#include "utils.h"
/*
 * Class to define an StripSegment which corresponds to a sub-section of an LED Strip.  
 * Specify starting offset and lenth of segment. Allows extending over LED strip limits (wrap around from end back to start)
 */
class StripSegment {
	public:
		// Constructor
		StripSegment(
			unsigned int start_offset,  // Start offset of segment (relative to start of LED strip)
			unsigned int segment_len,   // Length of segment (number of LEDS)
			unsigned int strip_len,     // Full length of LED strip (to enable wrap-over)
			bool reverse=false          // Whether segment is reversed (LED strip ID decreases with increasing segment value)
			): start_offset(start_offset), segment_len(segment_len), strip_len(strip_len), reverse(reverse) {
		}
		// Get LED Strip ID from segment value
		unsigned int getLEDId(unsigned int segment_pos){
			int led_id;
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
			
			return (unsigned int) led_id; //constrain(led_id, start_offset, start_offset+len)
		}
		// Negation operator overloading to get reverse version of segment
		// New segment will cover the same set of LEDs, will have shifted start_offset and be in reverse direction
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
		
		unsigned int start_offset, segment_len, strip_len;
		bool reverse;
};


// Class to define spatial positioning of a strip segment for use with a SpatialPatternMapping
// Generally want to define axis positions such that the coordinate origin is at the physical centre of your project
//template<uint16_t t_segment_length>
class SpatialStripSegment {
	public:
		SpatialStripSegment(
			const StripSegment strip_segment, 	// LED Strip segment for axis	
			Point start_pos, 					// Start position of axis in 3D  (Position of first LED)
			Point end_pos						// End position of axis in 3D space (Position of last LED)
			): strip_segment(strip_segment), start_pos(start_pos), end_pos(end_pos)   {
				// Dynamically allocate array to store LED positions
				this->led_positions = new Point[strip_segment.segment_len];
				// Calculate coordinate positions of each LED in strip segment
				
				//float scale_factor = 256.0/(strip_segment.segment_len-1);
				for (uint16_t i=0; i < strip_segment.segment_len; i++) {
					led_positions[i] = start_pos + (end_pos-start_pos)*(((float) i)/(strip_segment.segment_len-1));//.scale(round(i*scale_factor));
				}
				
				//this->step = (end_pos-start_pos)/(strip_segment.segment_len-1);
			}
		
		// Get spatial position of an LED on the axis 
		Point getSpatialPosition(uint16_t axis_pos)	{
			/*
			DPRINT("Spatial axis start pos");
			DPRINTLN(this->start_pos);
			DPRINT("Spatial axis end pos");
			DPRINTLN(this->end_pos);
			DPRINT("Spatial axis step");
			DPRINTLN(this->step);
			*/
			//return this->start_pos + this->step*axis_pos;
			
			return this->led_positions[axis_pos];
		}
		
		// Negation operator overloading to get reverse version of axis
		// New axis will cover the same set of LEDs, new start_pos will be old end_pos, and direction reversed
		SpatialStripSegment operator-()	{
			// Reverse start and end position
			return SpatialStripSegment(-(this->strip_segment), this->end_pos, this->start_pos);
		}
		
		~SpatialStripSegment() {
			DPRINT("DECONSTRUCT");
			//delete [] this->led_positions;
		}
		
		
		StripSegment strip_segment;		// LED Strip segment for axis
		Point start_pos;	// Start position of axis in 3D space
		Point end_pos;		// End position of axis in 3D space
		
	protected:

		Point *led_positions; 	// Pre-calculated array of coordinate positions of each LED in strip segment
		Point step;			// Direction vector of axis with length equal to distance between LEDS on axis
};

#endif