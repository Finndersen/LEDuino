/*
  Full implementations in header file due to issues with templated classes and linking...
*/
#ifndef LEDController_h
#define  LEDController_h
#include <FastLED.h>
#include "Axis.h"
#include "Pattern.h"
#include "PatternMapping.h"
#include "utils.h"

// Controller object which applies pattern LED values to appropriate axes
class LEDController {
	public:
		//Constructor
		LEDController(
			CRGB* leds,											// Pointer to Array of CRGB LEDs which is registered with FastLED
			unsigned int num_leds,								// Number of LEDS (length of leds)
			BasePatternMapping** pattern_mappings,				// Pointer to Array of points to PatternMapping configurations to run
			byte num_patterns,									// Number of pattern configurations (length of pattern_mappings)
			bool randomize=true									// Whether to randomize pattern order
			):  leds(leds), num_leds(num_leds), pattern_mappings(pattern_mappings), num_patterns(num_patterns), randomize(randomize) {
			// Initialise as max for immediate overflow to 0 at start
			this->current_mapping_id = num_patterns-1;
			// Set initial pattern
			this->setNewPatternMapping();
		}
		// Run pattern newFrame() if ready, set new pattern if required
		void loop(byte sound_level=0) {
			//unsigned long frame_time = millis();
			// Check if pattern config needs to be changed
			if (this->current_mapping->expired())	{
				this->setNewPatternMapping();
			}
			// New pattern frame
			if (this->current_mapping->frameReady())	{
				// Run pattern frame logic
				
				long pre_frame_time = millis();
				this->current_mapping->newFrame(leds, sound_level);
				long pre_show_time = millis();
				// Show LEDs
				FastLED.show();
				//DPRINT("Frame Time: ");
				//DPRINT(pre_show_time-pre_frame_time);
				//DPRINT(" Show time: ");
				//DPRINTLN(millis()-pre_show_time);
			}
		}

	private:

		CRGB* leds;	
		unsigned int num_leds;
		BasePatternMapping** pattern_mappings;
		byte num_patterns;
		bool randomize;
		byte current_mapping_id;
		BasePatternMapping* current_mapping;		// Pointer to currently selected pattern
		long last_frame_time;
		
		// Set ID of new pattern configuration
		void setNewPatternMapping() {		
			if (this->randomize)	{
				// Choose random pattern
				this->current_mapping_id = random(0, this->num_patterns);
			} else {
				// Choose next pattern
				this->current_mapping_id = (this->current_mapping_id + 1)%(this->num_patterns);
			}
			DPRINT("Choosing new pattern ID: " );
			DPRINTLN(this->current_mapping_id);
			this->current_mapping = this->pattern_mappings[current_mapping_id];
			this->current_mapping->reset();
		}
};

#endif