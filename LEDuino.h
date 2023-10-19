/*
  Full implementations in header file due to issues with templated classes and linking...
*/
#ifndef LEDuino_h
#define  LEDuino_h
#include <FastLED.h>
#include "StripSegment.h"
#include "Pattern.h"
#include "PatternMapping.h"
#include "utils.h"
#include "pattern_examples.h"

// Controller object which applies pattern LED values to appropriate axes
class PatternController {
	public:
		//Constructor
		PatternController(
			CRGB* leds,											// Pointer to Array of CRGB LEDs which is registered with FastLED
			uint16_t num_leds,									// Number of LEDS (length of leds)
			BasePatternMapping** pattern_mappings,				// Pointer to Array of pointers to PatternMapping configurations to run
			uint8_t num_mappings,									// Number of pattern configurations (length of pattern_mappings)
			bool randomize=false									// Whether to randomize pattern order
			):  leds(leds), num_leds(num_leds), pattern_mappings(pattern_mappings), num_mappings(num_mappings), randomize(randomize) {
			// Initialise as max for immediate overflow to 0 at start
			this->current_mapping_id = num_mappings-1;
		}
		void initialise() {
			// Set initial pattern
			this->setNewPatternMapping();
		}
		
		// Run pattern newFrame() if ready, set new pattern if required
		void loop() {
			static unsigned long last_frame_time;
			unsigned long current_time;
			// Check if pattern config needs to be changed
			if (this->current_mapping->expired() && this->auto_change_pattern)	{
				this->setNewPatternMapping();
			}
			// New pattern frame
			if (this->current_mapping->frameReady())	{
				current_time = micros();
				DPRINT("Frame delay: ");
				DPRINTLN(current_time - last_frame_time);
				last_frame_time=current_time;
				// Run pattern frame logic
				
				long pre_frame_time = micros();
				this->current_mapping->newFrame(this->leds);
				long pre_show_time = micros();
				// Show LEDs
				FastLED.show();
				DPRINT("Frame Time: ");
				DPRINT(pre_show_time-pre_frame_time);
				DPRINT(" Show time: ");
				DPRINTLN(micros()-pre_show_time);
			}
		}
		// Set current active pattern mapper by array index
		void setPatternMapping(uint8_t mapping_id)   {
			this->current_mapping_id = mapping_id;
			this->current_mapping = this->pattern_mappings[mapping_id];
			DPRINT("Choosing new pattern: " );
			DPRINTLN(this->current_mapping->name);
			this->current_mapping->reset();
		}
		
		BasePatternMapping* current_mapping;		// Pointer to currently selected pattern
		uint8_t current_mapping_id;
		bool auto_change_pattern=true;				// Can be set to false to stop automatically changing pattern mapping configurations
	private:

		CRGB* leds;	
		const uint16_t num_leds;
		BasePatternMapping** pattern_mappings;
		const uint8_t num_mappings;
		const bool randomize;		
		long last_frame_time;
		
		// Set ID of new pattern configuration
		void setNewPatternMapping() {		
			uint8_t new_pattern_id;
			if (this->randomize)	{
				// Choose random pattern
				new_pattern_id = random(0, this->num_mappings);
			} else {
				// Choose next pattern
				new_pattern_id = (this->current_mapping_id + 1)%(this->num_mappings);
			}
			// Reset LED state
			// TODO: Add transition between patterns?
			FastLED.clear();  // clear all pixel data
			FastLED.show();
			setPatternMapping(new_pattern_id);
		}
};

#endif