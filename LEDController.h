/*
  Full implementation in header file due to issues with templated classes and linking...
*/
#ifndef LEDController_h
#define  LEDController_h
#include <Arduino.h>
#include <FastLED.h>
#include <cmath>
//#include <array>
//#include <vector>
#ifdef DEBUG    //Macros are usually in all capital letters.
  #define DPRINT(...)    Serial.print(__VA_ARGS__); Serial.flush(); //DPRINT is a macro, debug print
  #define DPRINTLN(...)  Serial.println(__VA_ARGS__); Serial.flush();   //DPRINTLN is a macro, debug print with new line
#else
  #define DPRINT(...)     //now defines a blank line
  #define DPRINTLN(...)   //now defines a blank line
#endif

/*
 * Class to define an Axis which corresponds to a sub-section of an LED Strip.  
 * Specify starting offset and lenth of axis. Allows extending over LED strip limits (wrap around from end back to start)
 */
class LEDAxis {
	public:
		// Constructor
		LEDAxis(
			unsigned int start_offset,  // Start offset of axis (relative to start of LED strip)
			unsigned int axis_len,      // Length of LED axis
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
			int new_start_offset;
			if (reverse)	{
				new_start_offset = start_offset - axis_len;
				// Handle wrap-around below 0
				if (new_start_offset < 0)	{
					new_start_offset = axis_len + new_start_offset;
				}
				return LEDAxis(new_start_offset, axis_len, strip_len, false);
				
			} else {
				new_start_offset = (start_offset + axis_len)%strip_len;
				return LEDAxis(new_start_offset, axis_len, strip_len, true);
			}
		}
		
		unsigned int start_offset, axis_len, strip_len;
		bool reverse;
};


#define DEFAULT_PATTERN_DURATION 10

// Abstract Base class for patterns. Override frameAction() to implement pattern logic
class BasePattern	{
	public:
		// Constructor
		BasePattern(	
			unsigned int frame_delay,					// Delay between pattern frames (in ms)
			byte duration=DEFAULT_PATTERN_DURATION		// Duration of pattern (in s)
			): frame_delay(frame_delay), duration(duration) {} 
			
		unsigned long frame_time;					// Time of current frame in ms relative to pattern start time
		unsigned int frame_delay;							// Delay between pattern frames (in ms)
		const byte duration;						// Duration of pattern in seconds
		// Perform pattern intialisation
		virtual void init() {		
			DPRINTLN("Initialising pattern");

			start_time = millis();
			frame_time = 0;
		}
		// Determine whether pattern has expired (exceeded duration)	
		bool expired()	{
			return frame_time >= (duration*1000);
		}
		// Whether new frame is ready (frame_delay has elapsed)
		bool frameReady()	{
			return (millis() - start_time - frame_time) >= frame_delay;
		}
		
		// Called each time frame_delay expires
		// Takes byte value representing sound level from microphone to enable music-responsive patterns
		void newFrame(byte sound_level) {
			frame_time = millis()-start_time;
			frameAction(sound_level);		
		}
		
	protected:
		// Method called after every frame_delay. Contains main logic for pattern, generally populates contents of pattern_state array but can update state of pattern in other ways
		// Takes byte value representing sound level from microphone to enable music-responsive patterns
		virtual void frameAction(byte sound_level) {}
		unsigned long start_time;					// Absolute time pattern was initialised (in ms)
		
};

// Base class for patterns defined on a single linear axis
// Converts an axis positio into an LED value
class LinearPattern: public BasePattern	{
	public:
		LinearPattern(	
			unsigned int axis_len,
			unsigned int frame_delay,					// Delay between pattern frames (in ms)
			byte duration=DEFAULT_PATTERN_DURATION		// Duration of pattern (in s)
			): BasePattern(frame_delay, duration), axis_len(axis_len)  {}
		
		unsigned int axis_len;
		
		// Get value for LED at position 'i' along axis 
		virtual CRGB get_led_value(unsigned int i);
};

// Base class for linear pattern which uses an array of length t_axis_len to store state 
// Used for more complex patterns that need to use detailed state from previous frame
template<unsigned int t_axis_len> 
class LinearStatePattern : public LinearPattern	{
	public:
		// Constructor
		LinearStatePattern(	
			unsigned int frame_delay,							// Delay between pattern frames (in ms)
			byte duration=DEFAULT_PATTERN_DURATION		// Duration of pattern (in s)
			): LinearPattern(t_axis_len, frame_delay, duration) {
		}
		CRGB pattern_state[t_axis_len];					// Contains LED values for pattern
		
		virtual void init()	{
			LinearPattern::init();
			// Reset pattern state array to black
			for (byte i=0; i<axis_len; i++) {
				pattern_state[i] = CRGB::Black;
			}	
		}
		
		virtual CRGB get_led_value(unsigned int i) {
			//Read value from pattern_state
			return pattern_state[i];
		}
};

// Structure to represent a cartesian coordinate or vector 
struct Point {
	float x=0.0, y=0.0, z=0.0;
	
	//Initialise explicitly
	Point(float x, float y, float z): x(x), y(y), z(z)	{};
	// Initialise from array
	Point(float* arr): x(arr[0]), y(arr[1]), z(arr[2])	{};
	
	// Vector Addition and subtraction
	Point& operator+=(const Point &RHS) { x += RHS.x; y += RHS.y; z += RHS.z; return *this; };
	Point& operator-=(const Point &RHS) { x -= RHS.x; y -= RHS.y; z -= RHS.z; return *this; };
	
	Point operator+(const Point &RHS) { return Point(*this) += RHS; };
	Point operator-(const Point &RHS) { return Point(*this) -= RHS; };
	
	// Scalar addition and subtraction
	Point& operator+=(const double &RHS) { x += RHS; y += RHS; z += RHS; return *this; };
	Point& operator-=(const double &RHS) { x -= RHS; y -= RHS; z -= RHS; return *this; };

	Point operator+(const double &RHS) { return Point(*this) += RHS; };
	Point operator-(const double &RHS) { return Point(*this) -= RHS; };
	
	// Scalar product and division
	Point& operator*=(const double &RHS) { x *= RHS; y *= RHS; z *= RHS; return *this; };
	Point& operator/=(const double &RHS) { x /= RHS; y /= RHS; z /= RHS; return *this; };

	Point operator*(const double &RHS) { return Point(*this) *= RHS; };
	Point operator/(const double &RHS) { return Point(*this) /= RHS; };
	
	// Euclidean norm
	double norm() { 
		return std::sqrt(std::pow(x, 2) + std::pow(y, 2) + std::pow(z, 2)); 
	};
	
	// Distance to other point
	double distance(const Point& other)	{
		return std::sqrt(std::pow(other.x-x, 2) + std::pow(other.y-y, 2) + std::pow(other.z-z, 2)); 
	}
};

// Pattern defined in 3D space. Converts a 3D coordinate into an LED value
// The pattern occupies a 3D space constrained by attribute 'bounds'. 
// 
class SpatialPattern : public BasePattern {
	public:
		SpatialPattern(	
			Point bounds,							// Point vector defining maximum magnitude of pattern space in x, y and z directions
			unsigned int frame_delay,				// Delay between pattern frames (in ms)
			byte duration=DEFAULT_PATTERN_DURATION	// Duration of pattern (in s)
			): BasePattern(frame_delay, duration), bounds(bounds) {}
		
		Point& bounds;
		
		// Get value for LED at point coordinate. Point will be within range (+/-bounds.x, +/-bounds.y, +/-bounds.z)
		virtual CRGB get_led_value(Point point);

};

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

// Class to define spatial positioning of an axis for use with a SpatialPatternMapping
// Generally want to define axis positions such that the coordinate origin is at the physical centre of your 
class SpatialAxis : public LEDAxis {
	public:
		// Constructor for initialising spatialaxis from scratch
		SpatialAxis(
			unsigned int start_offset,  // Start offset of axis (relative to start of LED strip)
			unsigned int axis_len,      // Length of LED axis
			unsigned int strip_len,     // Full length of LED strip (to enable wrap-over)			
			Point start_pos, 	// Start position of axis in 3D space
			Point direction,		// Vector representing direction of axis in 3D space
			bool reverse=false          // Whether axis is reversed (LED strip ID decreases with increasing axis value)
		): LEDAxis(start_offset, axis_len, strip_len, reverse), start_pos(start_pos), direction(direction/direction.norm())   {}
		
		// Constructor for initialising from existing LEDAxis
		SpatialAxis(
			const LEDAxis existing_axis, 	// Pre-defined LED Axis object
			Point start_pos, 		// Start position of axis in 3D space
			Point direction			// Vector representing direction of axis in 3D space
		): LEDAxis(existing_axis.start_offset, existing_axis.axis_len, existing_axis.strip_len, existing_axis.reverse), start_pos(start_pos), direction(direction/direction.norm())  {}
		
		// Get spatial position of an LED on the axis 
		Point get_spatial_position(unsigned int axis_pos)	{
			return start_pos + direction*axis_pos;
		}
		
	protected:
		Point start_pos;
		Point direction;
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

// Controller object which applies pattern LED values to appropriate axes
class LEDController {
	public:
		//Constructor
		LEDController(
			CRGB* leds,											// Pointer to Array of CRGB LEDs which is registered with FastLED
			unsigned int num_leds,								// Number of LEDS (length of leds)
			BasePatternMapping* pattern_mappings,				// Pointer to Array of pattern to axes mapping configurations to run
			byte num_patterns,									// Number of pattern configurations (length of pattern_mappings)
			bool randomize=true									// Whether to randomize pattern order
			):  leds(leds), num_leds(num_leds), pattern_mappings(pattern_mappings), num_patterns(num_patterns), randomize(randomize) {
			// Initialise as max for immediate overflow to 0 at start
			current_mapping_id = num_patterns;
			// Set initial pattern
			setNewPatternMapping();
		}
		// Run pattern newFrame() if ready, set new pattern if required
		void loop(byte sound_level=0) {
			//unsigned long frame_time = millis();
			// Check if pattern config needs to be changed
			if (current_mapping->expired())	{
				setNewPatternMapping();
			}
			// New pattern frame
			if (current_mapping->frameReady())	{
				// Run pattern frame logic
				DPRINTLN("New Frame");
				current_mapping->newFrame(leds, sound_level);
				// Show LEDs
				FastLED.show();
			}
		}

	private:

		CRGB* leds;	
		unsigned int num_leds;
		BasePatternMapping* pattern_mappings;
		byte num_patterns;
		bool randomize;
		byte current_mapping_id;
		BasePatternMapping* current_mapping;		// Pointer to currently selected pattern
		
		// Set ID of new pattern configuration
		void setNewPatternMapping() {		
			if (randomize)	{
				// Choose random pattern
				current_mapping_id = random(0, num_patterns);
			} else {
				current_mapping_id = (current_mapping_id + 1)%num_patterns;
			}
			DPRINT("Choosing new pattern ID: " );
			DPRINTLN(current_mapping_id);
			current_mapping = &pattern_mappings[current_mapping_id];
			current_mapping->init();
		}
};



#endif