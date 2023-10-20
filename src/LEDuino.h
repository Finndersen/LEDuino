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
#include "PatternConfiguration.h"

#include "pattern_examples.h"

// Controller object which applies pattern LED values to appropriate axes
class PatternController {
	public:
		//Constructor
		PatternController(
			CRGB* leds,											// Pointer to Array of CRGB LEDs which is registered with FastLED
			uint16_t num_leds,									// Number of LEDS (length of leds)
			PatternConfiguration* pattern_configs,				// Array of PatternConfigurations to run
			uint8_t num_configs,								// Number of pattern configurations (length of pattern_configs)
			bool randomize=false								// Whether to randomize pattern order
			):  
			leds(leds), 
			num_leds(num_leds), 
			pattern_configs(pattern_configs), 
			num_configs(num_configs), 
			randomize(randomize), 
			current_config_id(num_configs-1) {}

		void initialise() {
			// Set initial pattern
			this->setNewPatternMapping();
		}
		
		// Run pattern newFrame() if ready, set new pattern if required
		void loop() {
			// Check if pattern config needs to be changed
			if (this->current_config->expired() && this->auto_change_pattern)	{
				this->setNewPatternMapping();
			}
			// New pattern frame
			if (this->current_config->frameReady())	{		
				#ifdef LEDUINO_DEBUG		
					long pre_frame_time = micros();
				#endif
				// Run pattern frame logic
				this->current_config->newFrame(this->leds);

				#ifdef LEDUINO_DEBUG
					long pre_show_time = micros();
				#endif
				// Show LEDs
				FastLED.show();

				// Print frame logic execution time and FastLED.show() time if DEBUG is enabled
				#ifdef LEDUINO_DEBUG
					Serial.print("Frame Time: ");
					Serial.print(frame_process_time);
					Serial.print(" Show time: ");
					Serial.println(micros()-pre_show_time);
					Serial.flush()
				#endif
			}
		}
		// Set current active pattern mapper by array index
		void setPatternMapping(uint8_t config_id)   {
			config_id = limit(config_id, this->num_configs-1);
			this->current_config_id = config_id;
			this->current_config = &(this->pattern_configs[config_id]);
			#ifdef LEDUINO_DEBUG
				Serial.print("Choosing new pattern: " );
				Serial.println(this->current_config->name);
				Serial.flush()
			#endif
			this->current_config->reset();
			// Reset LED state
			FastLED.clear();
			FastLED.show();
		}
		
		PatternConfiguration* current_config;		// Currently selected config
		bool auto_change_pattern=true;				// Can be set to false to stop automatically changing pattern mapping configurations
	private:

		CRGB* leds;	
		const uint16_t num_leds;
		PatternConfiguration* pattern_configs;
		const uint8_t num_configs;
		const bool randomize;
		long last_frame_time;
		uint8_t current_config_id;
		
		// Set ID of new pattern configuration
		void setNewPatternMapping() {		
			uint8_t new_pattern_id;
			if (this->randomize)	{
				// Choose random pattern
				new_pattern_id = random(0, this->num_configs);
			} else {
				// Choose next pattern
				new_pattern_id = (this->current_config_id + 1)%(this->num_configs);
			}

			// TODO: Add transition between patterns?
			setPatternMapping(new_pattern_id);
		}
};

#endif