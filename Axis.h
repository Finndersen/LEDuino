#ifndef Axis_h
#define  Axis_h
#include "Point.h"
/*
 * Class to define an Axis which corresponds to a sub-section of an LED Strip.  
 * Specify starting offset and lenth of axis. Allows extending over LED strip limits (wrap around from end back to start)
 */
class LEDAxis {
	public:
		// Constructor
		LEDAxis(
			unsigned int start_offset,  // Start offset of axis (relative to start of LED strip)
			unsigned int axis_len,      // Length of LED axis (number of LEDS)
			unsigned int strip_len,     // Full length of LED strip (to enable wrap-over)
			bool reverse=false          // Whether axis is reversed (LED strip ID decreases with increasing axis value)
			): start_offset(start_offset), axis_len(axis_len), strip_len(strip_len), reverse(reverse) {
		}
		// Get LED Strip ID from Axis value
		unsigned int get_led_id(unsigned int axis_value){
			int led_id;
			// Limit value to maximum length
			if (axis_value > axis_len)	{
				axis_value = axis_len;
			}
			if (reverse) {
				led_id = start_offset - axis_value;
				// Detect wrap around from 0 back to end of LED strip
				if (led_id < 0)	{
					led_id = strip_len + led_id;
				}

			} else {
				// Modulo with strip len to enable wrap-around
				led_id = (start_offset + axis_value)%strip_len;
			}
			
			return (unsigned int) led_id; //constrain(led_id, start_offset, start_offset+len)
		}
		// Negation operator overloading to get reverse version of axis
		// New axis will cover the same set of LEDs, will have shifted start_offset and be in reverse direction
		LEDAxis operator-()	{
			int reverse_start_offset;
			if (reverse)	{
				reverse_start_offset = start_offset - axis_len;
				// Handle wrap-around below 0
				if (reverse_start_offset < 0)	{
					reverse_start_offset = axis_len + reverse_start_offset;
				}
				return LEDAxis(reverse_start_offset, axis_len, strip_len, false);
				
			} else {
				reverse_start_offset = (start_offset + axis_len)%strip_len;
				return LEDAxis(reverse_start_offset, axis_len, strip_len, true);
			}
		}
		
		unsigned int start_offset, axis_len, strip_len;
		bool reverse;
};


// Class to define spatial positioning of an axis for use with a SpatialPatternMapping
// Generally want to define axis positions such that the coordinate origin is at the physical centre of your 
class SpatialAxis : public LEDAxis {
	public:
		// Constructor for initialising spatialaxis from scratch
		SpatialAxis(
			unsigned int start_offset,  // Start offset of axis (relative to start of LED strip)
			unsigned int axis_len,      // Length of LED axis (number of LEDS)
			unsigned int strip_len,     // Full length of LED strip (to enable wrap-over)			
			Point start_pos, 			// Start position of axis in 3D space
			Point end_pos,				// End position of axis in 3D space
			bool reverse=false          // Whether axis is reversed (LED strip ID decreases with increasing axis value)
		): LEDAxis(start_offset, axis_len, strip_len, reverse), start_pos(start_pos), end_pos(end_pos)   {
			// Calculate direction vector of axis with length equal to distance between LEDs 
			step = (end_pos-start_pos)/(axis_len-1);
		}
		
		// Constructor for initialising from existing LEDAxis
		SpatialAxis(
			const LEDAxis& existing_axis, 	// Pre-defined LED Axis object
			Point start_pos, 		// Start position of axis in 3D space
			Point end_pos				// End position of axis in 3D space
		): SpatialAxis(existing_axis.start_offset, existing_axis.axis_len, existing_axis.strip_len, start_pos, end_pos, existing_axis.reverse)  {}
		
		// Get spatial position of an LED on the axis 
		Point get_spatial_position(unsigned int axis_pos)	{
			return start_pos + step*axis_pos;
		}
		
		// Negation operator overloading to get reverse version of axis
		// New axis will cover the same set of LEDs, new start_pos will be old end_pos, and direction reversed
		SpatialAxis operator-()	{
			// Get representation of current underlying LEDAxis config
			LEDAxis current_axis(start_offset, axis_len, strip_len, reverse);
			// Reverse start and end position
			return SpatialAxis(-current_axis, end_pos, start_pos);
			
		}
		
	protected:
		Point start_pos;	// Start position of axis in 3D space
		Point end_pos;		// End position of axis in 3D space
		Point step;			// Direction vector of axis with length equal to distance between LEDS on axis
};

#endif