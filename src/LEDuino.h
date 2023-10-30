/*
  Full implementations in header file due to issues with templated classes and linking...
*/
#ifndef LEDuino_h
#define  LEDuino_h
#include <FastLED.h>
#include "utils.h"
#include "Point.h"
#include "StripSegment.h"
#include "Pattern.h"
#include "PatternMapping.h"
#include "MappingRunner.h"

#include "patterns/linear.h"
#include "patterns/spatial.h"

// Controller object which manages a collection of MappingRunners
// Chooses which mapping to run, and handles running it at the desired framerate
class LEDuinoController {
	public:
		//Constructor
		LEDuinoController(
			CRGB* leds,									// Pointer to Array of CRGB LEDs which is registered with FastLED
			uint16_t num_leds,							// Number of LEDS (length of leds)
			MappingRunner* mapping_runners,				// Array of PatternConfigurations to run
			uint8_t num_mappings,						// Number of pattern configurations (length of mapping_runners)
			bool randomize=false						// Whether to randomize pattern order
			):  
			leds(leds), 
			num_leds(num_leds), 
			mapping_runners(mapping_runners), 
			num_mappings(num_mappings), 
			randomize(randomize), 
			current_runner_id(num_mappings-1) {}

		void initialise() {
			// Set initial pattern
			this->setNewPatternMapping();
		}
		
		// Run pattern newFrame() if ready, set new pattern if required
		void loop() {
			// Check if pattern config needs to be changed
			if (this->current_runner->expired() && this->auto_change_pattern)	{
				this->setNewPatternMapping();
			}
			// New pattern frame
			if (this->current_runner->frameReady())	{		
				#ifdef LEDUINO_DEBUG		
					long pre_frame_time = micros();
				#endif
				// Run pattern frame logic
				this->current_runner->newFrame(this->leds);

				#ifdef LEDUINO_DEBUG
					long pre_show_time = micros();
				#endif
				// Show LEDs
				FastLED.show();

				// Print frame logic execution time and FastLED.show() time if DEBUG is enabled
				#ifdef LEDUINO_DEBUG
					Serial.print("Frame Time: ");
					Serial.print(pre_show_time-pre_frame_time);
					Serial.print(" Show time: ");
					Serial.println(micros()-pre_show_time);
					Serial.flush();
				#endif
			}
		}
		// Set current active pattern mapper by array index
		void setPatternMapping(uint8_t runner_id)   {
			runner_id = limit(runner_id, this->num_mappings-1);
			this->current_runner_id = runner_id;
			this->current_runner = &(this->mapping_runners[runner_id]);
			#ifdef LEDUINO_DEBUG
				Serial.print("Choosing new pattern: " );
				Serial.println(this->current_runner->name);
				Serial.flush();
			#endif
			this->current_runner->reset();
			// Reset LED state
			FastLED.clear();
			FastLED.show();
		}
		
		MappingRunner* current_runner;		// Currently selected mapping runner
		bool auto_change_pattern=true;		// Can be set to false to stop automatically changing pattern mapping configurations
	private:

		CRGB* leds;	
		const uint16_t num_leds;
		MappingRunner* mapping_runners;
		const uint8_t num_mappings;
		const bool randomize;
		long last_frame_time;
		uint8_t current_runner_id;
		
		// Set ID of new pattern configuration
		void setNewPatternMapping() {		
			uint8_t new_pattern_id;
			if (this->randomize)	{
				// Choose random pattern
				new_pattern_id = random(0, this->num_mappings);
			} else {
				// Choose next pattern
				new_pattern_id = (this->current_runner_id + 1)%(this->num_mappings);
			}

			// TODO: Add transition between patterns?
			setPatternMapping(new_pattern_id);
		}
};

#endif