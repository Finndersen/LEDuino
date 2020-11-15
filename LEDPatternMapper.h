/*
  Full implementations in header file due to issues with templated classes and linking...
*/
#ifndef PatternController_h
#define  PatternController_h
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
			byte num_patterns,									// Number of pattern configurations (length of pattern_mappings)
			bool randomize=true									// Whether to randomize pattern order
			):  leds(leds), num_leds(num_leds), pattern_mappings(pattern_mappings), num_patterns(num_patterns), randomize(randomize) {
			// Initialise as max for immediate overflow to 0 at start
			this->current_mapping_id = num_patterns-1;
		}
		void initialise() {
			// Set initial pattern
			this->setNewPatternMapping();
		}
		
		// Run pattern newFrame() if ready, set new pattern if required
		void loop() {
			//static unsigned long last_frame_time;
			//unsigned long current_time;
			// Check if pattern config needs to be changed
			if (this->current_mapping->expired() && this->auto_change_pattern)	{
				this->setNewPatternMapping();
			}
			// New pattern frame
			if (this->current_mapping->frameReady())	{
				//current_time = millis();
				//DPRINT("Frame delay: ");
				//DPRINTLN(current_time - last_frame_time);
				//last_frame_time=current_time;
				// Run pattern frame logic
				
				//long pre_frame_time = millis();
				this->current_mapping->newFrame(this->leds);
				//long pre_show_time = millis();
				// Show LEDs
				FastLED.show();
				//DPRINT("Frame Time: ");
				//DPRINT(pre_show_time-pre_frame_time);
				//DPRINT(" Show time: ");
				//DPRINTLN(millis()-pre_show_time);
			}
		}
		// Set current active pattern mapper by array index
		void setPatternMapping(byte mapping_id)   {
			this->current_mapping_id = mapping_id;
			this->current_mapping = this->pattern_mappings[mapping_id];
			DPRINT("Choosing new pattern: " );
			DPRINTLN(this->current_mapping->name);
			this->current_mapping->reset();
		}
		
		BasePatternMapping* current_mapping;		// Pointer to currently selected pattern
		byte current_mapping_id;
		bool auto_change_pattern=true;
	private:

		CRGB* leds;	
		uint16_t num_leds;
		BasePatternMapping** pattern_mappings;
		byte num_patterns;
		bool randomize;		
		long last_frame_time;
		
		// Set ID of new pattern configuration
		void setNewPatternMapping() {		
			byte new_pattern_id;
			if (this->randomize)	{
				// Choose random pattern
				new_pattern_id = random(0, this->num_patterns);
			} else {
				// Choose next pattern
				new_pattern_id = (this->current_mapping_id + 1)%(this->num_patterns);
			}
			setPatternMapping(new_pattern_id);
		}
};

#endif