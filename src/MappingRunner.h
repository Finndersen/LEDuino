#ifndef PatternConfiguration_h
#define  PatternConfiguration_h
#include "PatternMapping.h"
// Can override these defaults by setting before including LEDuino
#ifndef LEDUINO_DEFAULT_DURATION
	#define LEDUINO_DEFAULT_DURATION 15			// 15 second duration
#endif
#ifndef LEDUINO_DEFAULT_FRAME_DELAY	
	#define LEDUINO_DEFAULT_FRAME_DELAY 20		// 20 ms frame delay (50 FPS)
#endif

// Manages the duration and frame rate of a PatternMapper configuration
// Can be assigned a name for identification
class MappingRunner {
    public: 
        MappingRunner(
            BasePatternMapper& pattern_mapper,
            uint16_t frame_delay=LEDUINO_DEFAULT_FRAME_DELAY,  	// Delay between pattern frames (in ms)
            uint16_t duration=LEDUINO_DEFAULT_DURATION,			// Duration in seconds
            const char* name=""
        ): 	
            name(name), 
            pattern_mapper(pattern_mapper),
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
			this->pattern_mapper.reset();
		};

        // Excute new frame of pattern and map results to LED array
		void newFrame(CRGB* leds) {
			this->frame_time = millis() - this->start_time;
			this->pattern_mapper.newFrame(leds, this->frame_time);
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
			this->pattern_mapper.setPalette(new_palette);
		};
		
		// Reset palette of underlying pattern(s) to one it was initialised with
		 void resetPalette()	{
			this->pattern_mapper.resetPalette();
		};

        const char* name;  // Name or description of pattern
    protected:
        BasePatternMapper& pattern_mapper;
    	uint16_t frame_time;			    // Time of the current frame since pattern started (in ms)
		uint32_t start_time;			    // Absolute time pattern was initialised (in ms)
        const uint16_t duration;  			// Duration of pattern mapping configuration (in ms)
		const uint16_t frame_delay;			// Delay between pattern frames (in ms)
};

#endif