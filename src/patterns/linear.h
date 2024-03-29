#include <FastLED.h>
#include "Pattern.h"
#include "ColorPicker.h"

// BASIC PATTERNS

// Lights all LEDs up in one random color, then fades to the next random color.
// Can use a ConstantHuePicker to just have a constant solid color
// Adapted from WLED by Aircookie
class RandomColorFadePattern: public LinearPattern	{
	public:
		RandomColorFadePattern(
			uint8_t cycle_time=128, 	// Color cycle time in 16th of a second
			uint8_t fade_time=128,		// Time taken to fade to next colour as fraction of cycle_time
			const ColorPicker& color_picker=Basic_picker):		
		  LinearPattern(color_picker), 
		  cycle_time(cycle_time),
		  fadedur(uint16_t(fade_time*cycle_time) >> 8) {}
		
		void frameAction(CRGB* pixel_data, uint16_t num_pixels, uint32_t frame_time)	override {
			static uint32_t prev_change_time = 0;
			static uint16_t cycle_time_ms = this->cycle_time << 6;
			static uint8_t color, prev_color;
			uint16_t fade;
			uint32_t change_time = frame_time / cycle_time_ms;
			uint32_t rem = frame_time % cycle_time_ms;

			if (change_time != prev_change_time) //new color
			{
				prev_color = color;
				color = new_random_value8(prev_color);
				prev_change_time = change_time;
			}

			if (this->fadedur) {
				fade = (rem << 2) / (this->fadedur);
				if (fade > 255) fade = 255;
			} else {
				fade = 255;
			}
			fill_solid(pixel_data, num_pixels, blend(this->getColor(prev_color), this->getColor(color), fade));
		}
	protected:
		uint8_t cycle_time, fadedur;	// Cycle time and fade duration in 16th of a second
};

// SCROLLING & WAVE PATTERNS

// Pride2015
// Animated, ever-changing rainbows.
// by Mark Kriegsman. https://github.com/FastLED/FastLED/blob/master/examples/Pride2015/Pride2015.ino
// Recommend setting resolution equal to or close to number of leds in strip segment
class PridePattern: public LinearPattern	{
	public:
		PridePattern(uint8_t speed_factor=4):
			LinearPattern(), 
			speed_factor(speed_factor) {}
		
		void frameAction(CRGB* pixel_data, uint16_t num_pixels, uint32_t frame_time)	override {
			static uint32_t sPseudotime = 0;  // pseudo-time elapsed since pattern start
			static uint32_t sLastMillis = 0;  // actual time of last frame
			static uint16_t sHue16 = 0;       // Hue offset with 16-bit resolution
			// beatsin88 is used to get more granular low BPMs
			// Vary saturation slightly over time
			uint8_t sat8 = beatsin88( 87*this->speed_factor, 220, 250);
			// varies proportion of brightness which is determined by varying sine wave, vs constant
			uint8_t brightdepth = beatsin88( 341*this->speed_factor, 96, 224);
			// Vary brightness increment over time (measure of wavelength)
			uint16_t brightnessthetainc16 = beatsin88( 203*this->speed_factor, (25 * 256), (40 * 256));
			// varying time multiplyer, for varying rate of change of hue and brightness
			uint8_t msmultiplier = beatsin88(240*this->speed_factor, 40, 240);

			uint16_t hue16 = sHue16;//gHue * 256;
			// Vary hue increment over time (measure of rainbow colour gradient)
			uint16_t hueinc16 = beatsin88(113*this->speed_factor, 1, 3000);

			uint16_t deltams = frame_time - sLastMillis ;  // Time since last frame
			sLastMillis  = frame_time;
			sPseudotime += deltams * msmultiplier;
			// Increase hue offset by varying  amount
			sHue16 += deltams * beatsin88( 400, 5,9);
			// wave offset
			uint16_t brightnesstheta16 = sPseudotime;

			for (uint16_t i = 0 ; i < num_pixels; i++) {
				hue16 += hueinc16;
				uint8_t hue8 = hue16 / 256;

				brightnesstheta16  += brightnessthetainc16;
				uint16_t b16 = sin16( brightnesstheta16  ) + 32768;
				// Perform squared scaling to make sine wave sharper
				uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
				// Scale value from 0-65536 to 0-brightdepth
				uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
				// Scale to 0-255 (add constant amount)
				bri8 += (255 - brightdepth);

				CRGB newcolor = CHSV( hue8, sat8, bri8);
				
				nblend(pixel_data[i], newcolor, 64);
			}
		}
	protected:
		const uint8_t speed_factor;  // Factor to increase rate of change of pattern parameters
};

//Moing sine wave with randomised speed, duration and rainbow colour offset, and changes direction
// Benefits from using higher resolution than segment length
class RandomRainbowsPattern: public LinearPattern  {
  public:
    RandomRainbowsPattern(): LinearPattern() {}
	
	void reset() override{
		LinearPattern::reset();
		this->pos = 0;
		this->randomize_state();
	}
	
	void randomize_state() {
		this->speed=random(1,6);
		this->scale_factor = random(1,4);
		if (random(2) == 1) {
			this->direction=!this->direction;
		}
		this->randomize_time = random(10, 200);
		this->colour_offset = random(0,255);
		this->dim = random(0, 7) == 6;
	}
	
	void frameAction(CRGB* pixel_data, uint16_t num_pixels, uint32_t frame_time)  override {
		if (randomize_time) {
			randomize_time--;
		} else {
			this->randomize_state();
		}
		if (direction) {
			this->pos = (this->pos + this->speed) % num_pixels;
		} else {
			this->pos = wrap_subtract(this->pos, this->speed, num_pixels);
		}
		for (uint16_t i = 0; i < num_pixels; i++) {
		  pixel_data[i] = this->get_pixel_value(num_pixels, i);
		}
		
	}
	 
	CRGB get_pixel_value(uint16_t num_pixels, uint16_t i)  {
		uint8_t virtual_pos = (255*(i + this->pos))/(num_pixels);
		uint8_t val = cubicwave8(uint16_t(virtual_pos*this->scale_factor) & 0xFF);
		return this->getColor((val+this->colour_offset) & 0xFF, this->dim ? val>>1 : val);
	}
	  
	protected:
		uint8_t speed;
		bool direction=false;
		uint8_t pos;    // Position from 0 to resolution
		uint8_t colour_offset;
		uint16_t randomize_time;	// How long until state is randomized again
		uint8_t scale_factor;
		bool dim;  //Whether to make pattern very dim (can look cool)
};

// Extends head to end of strip then retracts tail
class GrowThenShrinkPattern : public LinearPattern  {
	public:
		GrowThenShrinkPattern(const ColorPicker& color_picker=Basic_picker):
		LinearPattern(color_picker) {}
		
		void reset() override {
			LinearPattern::reset();
			this->head_pos = this->tail_pos = 0;
			this->reverse = false;
		}

		void frameAction(CRGB* pixel_data, uint16_t num_pixels, uint32_t frame_time) override {
			// Update head and tail positions
			if (this->reverse) 	{
				if (this->tail_pos > 0)	{
					// Extend along light strip
					this->tail_pos--;
				} else if (this->head_pos > 0) {
					// Retract tail
					this->head_pos--;
				} else {
					// Reverse
					this->reverse = false;
				}	
			} else {
				if (this->head_pos < num_pixels-1)	{
					// Extend along light strip
					this->head_pos++;
				} else if (this->tail_pos < this->head_pos) {
					// Retract tail
					this->tail_pos++;
				} else {
					// Reverse
					this->reverse = true;
				}	
			}

			// Set pixel data
			for (uint16_t i=0; i<num_pixels; i++) {
				if ((this->tail_pos <= i) && (i <= this->head_pos)) 	{
					pixel_data[i] = this->getColor((i*255)/num_pixels);
				} else {
					pixel_data[i] = CRGB::Black;
				}
			}
		}

	protected:
		uint16_t head_pos, tail_pos;
		bool reverse;
};


// DYNAMIC MOVEMENT & ACTIVE PATTERNS

// Simple moving pulse of light along axis. Pulse has a bright head with a tapering tail
class MovingPulsePattern: public LinearPattern   {
  public:
    MovingPulsePattern(
		uint8_t pulse_len=3, 	// Length of pulse 
		const ColorPicker& color_picker=Basic_picker):
      LinearPattern(color_picker), 
	  head_pos(0), 
	  pulse_len(pulse_len), 	
	  tail_interpolator(Interpolator(0, 255, pulse_len + 1, 0))  {}

    // Update pulse position (on virtual axis)
    void frameAction(CRGB* pixel_data, uint16_t num_pixels, uint32_t frame_time)  override {
      this->head_pos = (this->head_pos + 1) % num_pixels;
	  for (uint16_t i=0; i<num_pixels; i++) {
		  pixel_data[i] = this->get_pixel_value(num_pixels, i);
	  }
    }

    // Construct pulse from head position
	// i is from 0 -> resolution
    CRGB get_pixel_value(uint16_t num_pixels, uint16_t i) {
		// Figure out distance behind pulse head to get brightness
		int distance_behind_head = this->head_pos - i;
		// Case of when position is in front of pulse head (after head has looped around to start), so distance_behind_head is negative
		if (distance_behind_head < 0)	{
			distance_behind_head = num_pixels + distance_behind_head;
		}
		// If not within pulse width, return black
		if (distance_behind_head > this->pulse_len)	{
			return CRGB::Black;
		}
		// Use interpolator to get brightness
		uint8_t lum = tail_interpolator.get_value(distance_behind_head);
		uint8_t hue = (i*255) / num_pixels; // Change colour along axis
		return this->getColor(hue, lum);
	}

  private:
	
    uint16_t head_pos;    				// Position of head of pulse
    uint8_t pulse_len;        			// Length of pulse 
	Interpolator tail_interpolator;  	// Linear Interpolator for pulse tail brightness
	
};

//https://gist.github.com/kriegsman/626dca2f9d2189bd82ca
// *Flashing* rainbow lights that zoom back and forth to a beat.
class DiscoStrobePattern : public LinearPattern  {
  public:
    DiscoStrobePattern(
		const ColorPicker& color_picker=HalloweenColors_picker):
      LinearPattern(color_picker) {}
	
	void frameAction(CRGB* pixel_data, uint16_t num_pixels, uint32_t frame_time)	override {
		// First, we black out all the LEDs
		fill_solid(pixel_data, num_pixels, CRGB::Black);
		
		// To achive the strobe effect, we actually only draw lit pixels
		// every Nth frame (e.g. every 4th frame).  
		// sStrobePhase is a counter that runs from zero to kStrobeCycleLength-1,
		// and then resets to zero.  
		const uint8_t kStrobeCycleLength = 4; // light every Nth frame
		static uint8_t sStrobePhase = 0;
		sStrobePhase = sStrobePhase + 1;
		if( sStrobePhase >= kStrobeCycleLength ) { 
			sStrobePhase = 0; 
		}

		// We only draw lit pixels when we're in strobe phase zero; 
		// in all the other phases we leave the LEDs all black.
		if( sStrobePhase == 0 ) {
			// The dash spacing cycles from 4 to 9 and back, 8x/min (about every 7.5 sec)
			uint8_t dashperiod= beatsin8( 8/*cycles per minute*/, 4,10);
			// The width of the dashes is a fraction of the dashperiod, with a minimum of one pixel
			uint8_t dashwidth = (dashperiod / 4) + 1;

			// The distance that the dashes move each cycles varies 
			// between 1 pixel/cycle and half-the-dashperiod/cycle.
			// At the maximum speed, it's impossible to visually distinguish
			// whether the dashes are moving left or right, and the code takes
			// advantage of that moment to reverse the direction of the dashes.
			// So it looks like they're speeding up faster and faster to the
			// right, and then they start slowing down, but as they do it becomes
			// visible that they're no longer moving right; they've been 
			// moving left.  Easier to see than t o explain.
			//
			// The dashes zoom back and forth at a speed that 'goes well' with
			// most dance music, a little faster than 120 Beats Per Minute.  You
			// can adjust this for faster or slower 'zooming' back and forth.
			int8_t  dashmotionspeed = beatsin8( (bpm /2), 1,dashperiod);
			// This is where we reverse the direction under cover of high speed
			// visual aliasing.
			if( dashmotionspeed >= (dashperiod/2)) { 
				dashmotionspeed = 0 - (dashperiod - dashmotionspeed );
			}

			// The hueShift controls how much the hue of each dash varies from 
			// the adjacent dash.  If hueShift is zero, all the dashes are the 
			// same color. If hueShift is 128, alterating dashes will be two
			// different colors.  And if hueShift is range of 10..40, the
			// dashes will make rainbows.
			// Initially, I just had hueShift cycle from 0..130 using beatsin8.
			// It looked great with very low values, and with high values, but
			// a bit 'busy' in the middle, which I didnt like.
			//   uint8_t hueShift = beatsin8(2,0,130);
			//
			// So instead I layered in a bunch of 'cubic easings'
			// (see http://easings.net/#easeInOutCubic )
			// so that the resultant wave cycle spends a great deal of time
			// "at the bottom" (solid color dashes), and at the top ("two
			// color stripes"), and makes quick transitions between them.
			uint8_t cycle = beat8(2); // two cycles per minute
			uint8_t easedcycle = ease8InOutCubic( ease8InOutCubic( cycle));
			uint8_t wavecycle = cubicwave8( easedcycle);
			uint8_t hueShift = scale8( wavecycle,130);


			// Each frame of the animation can be repeated multiple times.
			// This slows down the apparent motion, and gives a more static
			// strobe effect.  After experimentation, I set the default to 1.
			uint8_t strobesPerPosition = 1; // try 1..4


			// Now that all the parameters for this frame are calculated,
			// we call the 'worker' function that does the next part of the work.
			this->discoWorker(pixel_data, num_pixels, dashperiod, dashwidth, dashmotionspeed, strobesPerPosition, hueShift);
		}

	}
	protected:
		// discoWorker updates the positions of the dashes, and calls the draw function
		void discoWorker( 
			CRGB* pixel_data, uint16_t num_pixels,
			uint8_t dashperiod, uint8_t dashwidth, int8_t  dashmotionspeed,
			uint8_t stroberepeats,
			uint8_t huedelta)
		 {
		  static uint8_t sRepeatCounter = 0;
		  static int8_t sStartPosition = 0;
		  static uint8_t sStartHue = 0;

		  // Always keep the hue shifting a little
		  sStartHue += 1;

		  // Increment the strobe repeat counter, and
		  // move the dash starting position when needed.
		  sRepeatCounter = sRepeatCounter + 1;
		  if( sRepeatCounter>= stroberepeats) {
			sRepeatCounter = 0;
			
			sStartPosition = sStartPosition + dashmotionspeed;
			
			// These adjustments take care of making sure that the
			// starting hue is adjusted to keep the apparent color of 
			// each dash the same, even when the state position wraps around.
			if( sStartPosition >= dashperiod ) {
			  while( sStartPosition >= dashperiod) { sStartPosition -= dashperiod; }
			  sStartHue  -= huedelta;
			} else if( sStartPosition < 0) {
			  while( sStartPosition < 0) { sStartPosition += dashperiod; }
			  sStartHue  += huedelta;
			}
		  }

		  // draw dashes with full brightness (value), and somewhat
		  // desaturated (whitened) so that the LEDs actually throw more light.
		  const uint8_t kSaturation = 208;
		  const uint8_t kValue = 255;

		  // call the function that actually just draws the dashes now
		  this->drawRainbowDashes(pixel_data, num_pixels,
		   						sStartPosition, dashperiod, dashwidth, 
							 	sStartHue, huedelta, kSaturation, kValue);
		}
		// drawRainbowDashes - draw rainbow-colored 'dashes' of light along the led strip:
		//   starting from 'startpos', up to and including 'lastpos'
		//   with a given 'period' and 'width'
		//   starting from a given hue, which changes for each successive dash by a 'huedelta'
		//   at a given saturation and value.
		//
		//   period = 5, width = 2 would be  _ _ _ X X _ _ _ Y Y _ _ _ Z Z _ _ _ A A _ _ _ 
		//                                   \-------/       \-/
		//                                   period 5      width 2
		//
		void drawRainbowDashes( 
			CRGB* pixel_data, uint16_t num_pixels,
		  uint8_t startpos, uint8_t period, uint8_t width, 
		  uint8_t huestart, uint8_t huedelta, uint8_t saturation, uint8_t value)
		{
		  uint8_t hue = huestart;
		  for( uint16_t i = startpos; i <= num_pixels-1; i += period) {
			// Switched from HSV color wheel to color palette
			// Was: CRGB color = CHSV( hue, saturation, value); 
			CRGB color = this->getColor(hue, value);
			
			// draw one dash
			uint16_t pos = i;
			for( uint8_t w = 0; w < width; w++) {
			  pixel_data[pos] = color;
			  pos++;
			  if( pos >= num_pixels) {
				break;
			  }
			}
			
			hue += huedelta;
		  }
		}
		uint8_t bpm=61;
};

// Pulse which jumps to random position on segment and flashes
class SkippingSpikePattern: public LinearPattern  {
  public:
    SkippingSpikePattern(
      uint8_t max_pulse_width,
      uint8_t pulse_speed=1,
	  const ColorPicker& color_picker=RainbowColors_picker):
      LinearPattern(color_picker),
	    max_pulse_width(max_pulse_width),
	    pulse_speed(pulse_speed) {}
	 
    void reset() override {
      LinearPattern::reset();
      this->pulse_pos = this->max_pulse_width;
      this->ramp = 0;
      this->ramp_up= true;
    }

    void frameAction(CRGB* pixel_data, uint16_t num_pixels, uint32_t frame_time)  override {
        if (this->ramp_up) {	// Pulse expanding		
          if (this->max_pulse_width-this->ramp <= this->pulse_speed) {  // Reached top of pulse
            this->ramp_up = false;
          } else {
            this->ramp += this->pulse_speed;  // Increase pulse brightness
          }
        } else {  // Pulse contracting
          if (this->ramp <= this->pulse_speed) {  // End of pulse
            // Move pulse position
            this->pulse_pos = (this->max_pulse_width/4) + random(0, num_pixels - this->max_pulse_width/4);
            /*
            if ((this->resolution - pulse_pos) <= pulse_offset) {
              // Loop over position back to start
              this->pulse_pos = pulse_offset/2;
            } else {
              this->pulse_pos += pulse_offset; 
            }
            */
            this->ramp_up = true;	 // Begin next pulse
          } else {
            this->ramp -= this->pulse_speed;  
          }
        }

		// Fill pixel array
		for (uint16_t i=0; i < num_pixels; i++) 	{
			// Get distance of pixel from pulse_pos
			uint8_t diff = i >= this->pulse_pos ? i - this->pulse_pos : this->pulse_pos - i;
			if (diff > this->ramp) {
				pixel_data[i] = CRGB::Black;
			} else {
				uint8_t lum = (255 - (diff*255)/this->ramp);
				pixel_data[i] = this->getColor(255-lum, lum);
			}
		}
    }

	protected:
		const uint8_t max_pulse_width, pulse_speed;
		uint16_t pulse_pos; //Position of current pulse
		uint8_t ramp;
		bool ramp_up;
};


// OTHER PATTERNS

// This function takes a pixel, and if its in the 'fading down'
// part of the cycle, it adjusts the color a little bit like the
// way that incandescent bulbs fade toward 'red' as they dim.
void coolLikeIncandescent( CRGB& c, uint8_t phase)
{
  if ( phase < 128) return;

  uint8_t cooling = (phase - 128) >> 4;
  c.g = qsub8( c.g, cooling);
  c.b = qsub8( c.b, cooling * 2);
}

// Adapted from pattern by Mark Kriegsman
// https://gist.github.com/kriegsman/756ea6dcae8e30845b5a
//  The idea behind this (new) implementation is that there's one
//  basic, repeating pattern that each pixel follows like a waveform:
//  The brightness rises from 0..255 and then falls back down to 0.
//  The brightness at any given point in time can be determined as
//  as a function of time, for example:
//    brightness = sine( time ); // a sine wave of brightness over time
//
//  So the way this implementation works is that every pixel follows
//  the exact same wave function over time.  In this particular case,
//  I chose a sawtooth triangle wave (triwave8) rather than a sine wave,
//  but the idea is the same: brightness = triwave8( time ).
// 	Works well when resolution is equal to segment length
class TwinklePattern : public LinearPattern   {
  public:
    TwinklePattern(
		uint8_t twinkle_speed = 6, 
		uint8_t twinkle_density = 4, 
		const ColorPicker& color_picker = FairyLight_picker, 
		CRGB bg = CRGB::Black):
      LinearPattern(color_picker), 
	  bg(bg), 
	  bg_brightness(bg.getAverageLight()), 
	  twinkle_speed(twinkle_speed), 
	  twinkle_density(twinkle_density)  {}

    void frameAction(CRGB* pixel_data, uint16_t num_pixels, uint32_t frame_time) override {
		// "this->PRNG16" is the pseudorandom number generator
		this->PRNG16 = 11337;
		// Set pixel data
		for (uint16_t i=0; i<num_pixels; i++) {
		  pixel_data[i] = this->get_pixel_value(frame_time, i);
		}
    }

    CRGB get_pixel_value(uint16_t frame_time, uint16_t i)  {
      CRGB pixel;
      this->PRNG16 = (uint16_t)(this->PRNG16 * 2053) + 1384; // next 'random' number
      uint16_t myclockoffset16 = this->PRNG16; // use that number as clock offset
      this->PRNG16 = (uint16_t)(this->PRNG16 * 2053) + 1384; // next 'random' number
      // use that number as clock speed adjustment factor (in 8ths, from 8/8ths to 23/8ths)
      uint8_t myspeedmultiplierQ5_3 =  ((((this->PRNG16 & 0xFF) >> 4) + (this->PRNG16 & 0x0F)) & 0x0F) + 0x08;
      uint32_t myclock30 = (uint32_t)((frame_time * myspeedmultiplierQ5_3) >> 3) + myclockoffset16;
      uint8_t  myunique8 = this->PRNG16 >> 8; // get 'salt' value for this pixel

      // We now have the adjusted 'clock' for this pixel, now we call
      // the function that computes what color the pixel should be based
      // on the "brightness = f( time )" idea.
      CRGB c = computeOneTwinkle( myclock30, myunique8);
      uint8_t cbright = c.getAverageLight();
      int16_t deltabright = cbright - bg_brightness;
      if ( deltabright >= 32 || (!bg)) {
        // If the new pixel is significantly brighter than the background color,
        // use the new color.
        pixel = c;
      } else if ( deltabright > 0 ) {
        // If the new pixel is just slightly brighter than the background color,
        // mix a blend of the new color and the background color
        pixel = blend( bg, c, deltabright * 8);
      } else {
        // if the new pixel is not at all brighter than the background color,
        // just use the background color.
        pixel = bg;
      }
      return pixel;
    }
  protected:
    CRGB computeOneTwinkle( uint32_t ms, uint8_t salt) {
      uint16_t ticks = ms >> (8 - twinkle_speed);
      uint8_t fastcycle8 = ticks;
      uint16_t slowcycle16 = (ticks >> 8) + salt;
      slowcycle16 += sin8( slowcycle16);
      slowcycle16 =  (slowcycle16 * 2053) + 1384;
      uint8_t slowcycle8 = (slowcycle16 & 0xFF) + (slowcycle16 >> 8);

      uint8_t bright = 0;
      if ( ((slowcycle8 & 0x0E) / 2) < twinkle_density) {
        bright = attackDecayWave8( fastcycle8);
      }

      uint8_t hue = slowcycle8 - salt;
      CRGB c;
      if ( bright > 0) {
        c = this->getColor(hue, bright);
        coolLikeIncandescent( c, fastcycle8);
      } else {
        c = CRGB::Black;
      }
      return c;
    }

    // Background colour
	CRGB bg;
    uint8_t bg_brightness;

    uint8_t twinkle_speed;     // 0-8
    uint8_t twinkle_density;   // 0-8
    uint16_t PRNG16;
};


class SparkleFillPattern : public LinearPattern {
  public:
    SparkleFillPattern(const ColorPicker& color_picker=Basic_picker):
      LinearPattern(color_picker) {}
	  
	void reset() {
		LinearPattern::reset();
		this->fill = true;
		this->pixels_changed = 0;
	}
	
	void frameAction(CRGB* pixel_data, uint16_t num_pixels, uint32_t frame_time)	override {
		uint16_t remaining = num_pixels - this->pixels_changed;
		for (uint16_t i=0; i < num_pixels; i++) 	{
			uint8_t brightness = pixel_data[i].getAverageLight();
			// Probabilty to fill/un-fill the pixel is inversely proportional to amount remaining
			if (random(0,remaining) == 0) {
				// Fill with random palette value, or unfill
				if (brightness) 	{
					// Brighten existing pixel if filling
					if (this->fill)	{
						if (brightness < 200)	{
							pixel_data[i]*=2;
						} 
					} else {
						// reduce brightness or set to black if un-filling
						if (brightness > 32)	{
							pixel_data[i]/= 2;
						} else {
							pixel_data[i] = CRGB::Black;
							this->pixels_changed++;
						}
					}
				} else if (this->fill) {
					// Fill with new colour
					pixel_data[i] = this->getColor(random(0,256), 32);
					this->pixels_changed++;
				}
				
				if (this->pixels_changed == num_pixels)	{
					this->pixels_changed = 0;
					this->fill = !this->fill;
				}
			}
		}
		
	}
	
	protected:
		bool fill=true;   // Whether pattern is in fill mode (True) or un-fill
		uint16_t pixels_changed=0;	// Number of remaining pixels to fill/un-fill
	
};

//https://github.com/FastLED/FastLED/blob/master/examples/Fire2012WithPalette/Fire2012WithPalette.ino
template<uint16_t t_resolution> 
class FirePattern: public LinearPattern   {
  public:
		FirePattern(
		uint8_t cooling=60,    	// Less cooling = taller flames.  More cooling = shorter flames. Default 60, suggested range 20-100 
		uint8_t sparking=100,	// Higher chance = more roaring fire.  Lower chance = more flickery fire. Default 100, suggested range 50-200.
		const ColorPicker& color_picker=HeatColors_picker): 
		LinearPattern(color_picker),
		cooling(cooling),  	
		sparking(sparking)  
		{};
		
	void reset()	override {
		LinearPattern::reset();
		// Reset heat array to 0
		for (uint8_t i=0; i<t_resolution; i++) {
			this->heat[i] = 0;
		}	
	}
		
	void frameAction(CRGB* pixel_data, uint16_t num_pixels, uint32_t frame_time) override {
		random16_add_entropy(random());
		// Step 1.  Cool down every cell a little
		for(uint8_t i = 0; i < num_pixels; i++) {
		  this->heat[i] = qsub8( this->heat[i],  random8(0, ((this->cooling * 10) / num_pixels) + 2));
		}
	  
		// Step 2.  Heat from each cell drifts 'up' and diffuses a little
		for(uint8_t k= num_pixels - 1; k >= 2; k--) {
		  this->heat[k] = (this->heat[k - 1] + 2*this->heat[k - 2]) / 3;
		}
		
		// Step 3.  Randomly ignite new 'sparks' of heat near the bottom
		if(random8() < this->sparking ) {
		  uint8_t y = random8(num_pixels/5 + 1);
		  this->heat[y] = qadd8( this->heat[y], random8(160,220) );
		}

		// Fill pixel array
		for (uint16_t i=0; i < num_pixels; i++) 	{
			// Get heat value, Scale from 0-255 down to 0-240, select colour from palette
			uint8_t colorindex = scale8(this->heat[i], 240);
			// Constrain base heat (so base of fire doesnt look too bright
			if (i < (num_pixels/10) + 1)	{
				colorindex = constrain(colorindex, 40, 120);
			}
			pixel_data[i] = this->getColor(colorindex);
		}
	}
	   
	protected:
		uint8_t heat[t_resolution]; 		// Array to store heat values
		const uint8_t cooling, sparking;
	
};

