#ifndef PatternConfiguration_h
#define  PatternConfiguration_h
#include "PatternMapping.h"
#define DEFAULT_DURATION 15			// 15 second duration
#define DEFAULT_FRAME_DELAY 20		// 20 ms frame delay (50 FPS)


// Sets the duration, frame rate and optional name of a PatternMapping configuration
class PatternConfiguration {
    public: 
        PatternConfiguration(
            BasePatternMapping& pattern_mapping,
            uint16_t frame_delay=DEFAULT_FRAME_DELAY,  	// Delay between pattern frames (in ms)
            uint16_t duration=DEFAULT_DURATION,			// Duration in seconds
            const char* name=""
        ): 	
            name(name), 
            pattern_mapping(pattern_mapping),
			duration(duration*1000), 
			frame_delay(frame_delay)  {}

        // Initialise/Reset pattern state
		void reset() {		
			#ifdef LEDUINO_DEBUG
				Serial.print("Initialising pattern mapping configuration: ");
				Serial.println(this->name);
				Serial.flush()
			#endif
			this->start_time = millis();
			this->frame_time = 0;
			this->pattern_mapping.reset();
		};

        // Excute new frame of pattern and map results to LED array
		void newFrame(CRGB* leds) {
			this->frame_time = millis() - this->start_time;
			this->pattern_mapping.newFrame(leds, this->frame_time);
		}
		
		// Determine whether pattern has expired (exceeded duration)	
		bool expired()	{
			return this->frame_time >= this->duration;
		};
		
		// Return whether it is time to start a new frame (frame_delay has elapsed since previous frame time)
		bool frameReady()	{
			return (millis() - this->start_time - this->frame_time) >= this->frame_delay;
		};

		// Set palette of underlying pattern(s)
	  	void setPalette(CRGBPalette16 new_palette)	{
			this->pattern_mapping.setPalette(new_palette);
		};
		
		// Reset palette of underlying pattern(s) to one it was initialised with
		 void resetPalette()	{
			this->pattern_mapping.resetPalette();
		};

        const char* name;  // Name or description of pattern
    protected:
        BasePatternMapping& pattern_mapping;
    	uint16_t frame_time;			    // Time of the current frame since pattern started (in ms)
		uint32_t start_time;			    // Absolute time pattern was initialised (in ms)
        const uint16_t duration;  			// Duration of pattern mapping configuration (in ms)
		const uint16_t frame_delay;			// Delay between pattern frames (in ms)
};

#endif