/*
  Full implementation in header file due to issues with templated classes and linking...
*/
#ifndef LEDController_h
#define  LEDController_h
#include <Arduino.h>
#include <FastLED.h>
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
		byte id;
		// Constructor
		LEDAxis(
			byte id,                    // unqiue ID of axis (used to identify when assigning LED values)
			unsigned int start_offset,  // Start offset of axis (relative to start of LED strip)
			unsigned int axis_len,      // Length of LED axis
			unsigned int strip_len,     // Full length of LED strip (to enable wrap-over)
			bool reverse=false          // Whether axis is reversed (LED strip ID decreases with increasing axis value)
			): id(id), start_offset(start_offset), axis_len(axis_len), strip_len(strip_len), reverse(reverse) {
		}
		// Get LED Strip ID from Axis value
		unsigned int get_led_id(unsigned int axis_value){
			int led_id;
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
		
	private:
		unsigned int start_offset, axis_len, strip_len;
		bool reverse;
};


#define DEFAULT_PATTERN_DURATION 10

// Abstract Base class for patterns. Override frameAction() to implement pattern logic
class BasePattern	{
	public:
		// Constructor
		BasePattern(	
			byte frame_delay,							// Delay between pattern frames (in ms)
			byte duration=DEFAULT_PATTERN_DURATION		// Duration of pattern (in s)
			): frame_delay(frame_delay), duration(duration) {
		} 
		unsigned long current_time;					// Current time in ms relative to pattern start time (refreshed each loop())
		byte frame_delay;							// Delay between pattern frames (in ms)
		const byte duration;						// Duration of pattern in seconds
		// Perform pattern intialisation
		virtual void init();
		
		// Called each time frame_delay expires
		// Takes byte value representing sound level from microphone to enable music-responsive patterns
		void newFrame(byte sound_level) {
			current_time = millis()-start_time;
			frameAction(sound_level);		
		}
		// Get value for LED at position 'i'. By default reads value from pattern_state, can override for custom behaviour
		virtual CRGB get_led_value(unsigned int i);
		// Get axis_len attribute
		virtual unsigned int get_axis_len();
		
	private:
		// Method called after every frame_delay. Contains main logic for pattern, generally populates contents of pattern_state array but can update state of pattern in other ways
		// Takes byte value representing sound level from microphone to enable music-responsive patterns
		virtual void frameAction(byte sound_level) {}
		
	protected: 	
		unsigned long start_time;					// Absolute time pattern was initialised (in ms)
};

// Base class for pattern with a specific axis length. Override frameAction() to implement pattern logic
template<unsigned int t_axis_len> 
class Pattern : public BasePattern	{
	public:
		// Constructor
		Pattern(	
			byte frame_delay,							// Delay between pattern frames (in ms)
			byte duration=DEFAULT_PATTERN_DURATION		// Duration of pattern (in s)
			): BasePattern(frame_delay, duration) {
		}
		CRGB pattern_state[t_axis_len];					// Contains LED values for pattern
		static const unsigned int axis_len = t_axis_len;	// Length of pattern axis (number of LEDS)
		
		// Perform pattern intialisation
		virtual void init() {		
			DPRINTLN("Initialising pattern");
			// Reset pattern state array to black
			for (byte i=0; i<t_axis_len; i++) {
				pattern_state[i] = CRGB::Black;
			}
			start_time = millis();
			current_time = 0;
		}
		
		virtual CRGB get_led_value(unsigned int i) {		
			return pattern_state[i];
		}

		virtual unsigned int get_axis_len() {
			return t_axis_len;
		}
};

// Class to define mapping of a pattern to set of axes
// Pattern AXIS_LEN must be equal to length of axes it is mapped to
class LEDPatternMapping {
	public:
		// Constructor
		LEDPatternMapping(
			BasePattern* pattern,   	// Pattern object
			byte axes,              			// Bytemask of axis IDs to apply pattern to
			byte duration,          			// Duration to run pattern/axes combination for (seconds)
			byte reverse_axes=0     			// Bytemask of axis IDs to apply pattern to in reverse order
			): pattern(pattern), axes(axes), duration(duration), reverse_axes(reverse_axes)	{
		}

		BasePattern* pattern;
		byte axes;
		byte duration;
		byte reverse_axes;

};


// Controller object which applies pattern LED values to appropriate axes
class LEDController {
	public:
		//Constructor
		LEDController(
			LEDAxis* led_axes,									// Pointer to Array of LEDAxis to register with controller
			byte num_axes,										// Number of axes (length of led_axes)
			CRGB* leds,											// Pointer to Array of CRGB LEDs which is registered with FastLED
			unsigned int num_leds,								// Number of LEDS (length of leds)
			LEDPatternMapping* pattern_mappings,				// Pointer to Array of pattern to axes mapping configurations to run
			byte num_patterns,									// Number of pattern configurations (length of pattern_mappings)
			bool randomize=true									// Whether to randomize pattern order
			): led_axes(led_axes), num_axes(num_axes), leds(leds), num_leds(num_leds), pattern_mappings(pattern_mappings), num_patterns(num_patterns), randomize(randomize) {
			// Initialise as max for immediate overflow to 0 at start
			current_pattern_id = num_patterns;
			// Set initial pattern
			setNewPattern();
		}
		// Run pattern newFrame() if ready, set new pattern if required
		void loop(byte sound_level=0) {
			unsigned long current_time = millis();
			//bool new_frame;
			CRGB led_val;
			// Check if pattern config needs to be changed
			if (current_pattern->pattern->current_time > (current_pattern->duration*1000))	{
				setNewPattern();
			}
			// New pattern frame
			if ((current_time - last_frame_time) >= current_pattern->pattern->frame_delay)	{
				// Run pattern frame logic
				DPRINTLN("New Frame");
				current_pattern->pattern->newFrame(sound_level);
				// Get pattern LED values and apply to axes
				unsigned int axis_len = current_pattern->pattern->get_axis_len();
				for (unsigned int i=0; i<axis_len; i++) {
					led_val = current_pattern->pattern->get_led_value(i);
					// Apply value to axis in normal direction
					if (current_pattern->axes) {
						setLED(current_pattern->axes, i, led_val);
					}
					// Apply value to axis in reverse direction
					if (current_pattern->reverse_axes) {
						setLED(current_pattern->reverse_axes, axis_len-i, led_val);
					}
					// Show LEDs
					FastLED.show();
				}
				last_frame_time = current_time;
			}
		}
		// Set value of LED on specified axes
		void setLED(
			byte axes_mask,			// Bitmask to select axes to set LED value on
			unsigned int axis_pos,	// Axis LED position
			CRGB value				// LED value to set
			) {
			for (byte i=0; i<num_axes; i++) {
				LEDAxis* axis = &led_axes[i];
				if (axis->id & axes_mask)	{
					leds[axis->get_led_id(axis_pos)] = value;
				}
			}
		}
	private:
		LEDAxis* led_axes;
		byte num_axes;
		CRGB* leds;	
		unsigned int num_leds;
		LEDPatternMapping* pattern_mappings;
		byte num_patterns;
		bool randomize;
		byte current_pattern_id;
		LEDPatternMapping* current_pattern;		// Pointer to currently selected pattern
		unsigned long last_frame_time;
		
		// Set ID of new pattern configuration
		void setNewPattern() {		
			if (randomize)	{
				// Choose random pattern
				current_pattern_id = random(0, num_patterns);
			} else {
				current_pattern_id = (current_pattern_id + 1)%num_patterns;
			}
			DPRINT("Choosing new pattern ID: " );
			DPRINTLN(current_pattern_id);
			current_pattern = &pattern_mappings[current_pattern_id];
			current_pattern->pattern->init();
		}
};



#endif