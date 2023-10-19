#include <FastLED.h>
#include "Pattern.h"

// Simple moving pulse of light along axis. Pulse has a bright head with a tapering tail
// LinearStatePattern is used for performance when mapped to many segments (only need to calculate LED values once per frame)
template<uint16_t t_resolution> 
class MovingPulse: public LinearStatePattern<t_resolution>   {
  public:
    MovingPulse(
		uint8_t pulse_len=3, 	// Length of pulse 
		uint8_t speed=4, 		// Number of virtual pixels to move each step
		CRGBPalette16 colour_palette = RainbowColors_p):
      LinearStatePattern<t_resolution>(colour_palette), 
	  head_pos(0), 
	  pulse_len(pulse_len), 	
	  speed(speed), 
	  tail_interpolator(Interpolator(0, 255, pulse_len + 1, 0))  {}

    // Update pulse position (on virtual axis)
    void frameAction(uint32_t frame_time)  override {
      this->head_pos = (this->head_pos + this->speed) % this->resolution;
	  for (uint16_t i=0; i<this->resolution; i++) {
		  this->pattern_state[i] = this->get_pos_value(i);
	  }
    }

    // Construct pulse from head position
	// i is from 0 -> resolution
    CRGB get_pos_value(uint16_t i) {
		// Figure out distance behind pulse head to get brightness
		int distance_behind_head = this->head_pos - i;
		// Case of when position is in front of pulse head so distance_behind_head is negative
		if (distance_behind_head < 0)	{
			distance_behind_head = this->resolution + distance_behind_head;
		}
		// If not within pulse width, return black
		if (distance_behind_head > this->pulse_len)	{
			return CRGB::Black;
		}
		// Use interpolator to get brightness
		uint8_t lum = tail_interpolator.get_value(distance_behind_head);
		uint8_t hue = (i*255) / this->resolution; // Change colour along axis
		return this->colorFromPalette(hue, lum);
	}

  private:
	
    uint16_t head_pos;    				// Position of head of pulse
    uint8_t pulse_len;        			// Length of pulse 
	uint8_t speed; 						// Number of virtual pixels to move each step
	Interpolator tail_interpolator;  	// Linear Interpolator for pulse tail brightness
	
};

//Moing sine wave with randomised speed, duration and rainbow colour offset, and changes direction
template<uint16_t t_resolution> 
class RandomRainbows: public LinearStatePattern<t_resolution>  {
  public:
    RandomRainbows(): LinearStatePattern<t_resolution>(RainbowColors_p) {}
	
	void reset() override{
		LinearStatePattern<t_resolution>::reset();
		this->pos = 0;
		this->randomize_state();
	}
	
	void randomize_state() {
		this->speed=random(1,12);
		this->scale_factor = random(1,3);
		this->direction=!this->direction;
		this->randomize_time = random(10, 200);
		this->colour_offset = random(0,255);
		this->dim = random(0, 7) == 6;
	}
	
	void frameAction(uint32_t frame_time)  override {
		if (randomize_time) {
			randomize_time--;
		} else {
			this->randomize_state();
		}
		if (direction) {
			this->pos = (this->pos + this->speed) % this->resolution;
		} else {
			this->pos = wrap_subtract(this->pos, this->speed, this->resolution);
		}
		for (uint16_t i = 0; i < this->resolution; i++) {
		  this->pattern_state[i] = this->get_pos_value(i);
		}
		
	}
	 
	CRGB get_pos_value(uint16_t i)  {
		uint8_t virtual_pos = (255*(i + this->pos))/(this->resolution);
		uint8_t val = cubicwave8((virtual_pos*this->scale_factor)%255);
		return this->colorFromPalette((val+this->colour_offset)%255, this->dim ? val>>2 : val);
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

// Pride2015
// Animated, ever-changing rainbows.
// by Mark Kriegsman. https://github.com/FastLED/FastLED/blob/master/examples/Pride2015/Pride2015.ino
// Recommend setting resolution equal to or close to number of leds in strip segment
template<uint16_t t_resolution> 
class PridePattern: public LinearStatePattern<t_resolution>	{
	public:
		PridePattern(uint8_t speed_factor=4):
		  LinearStatePattern<t_resolution>(RainbowColors_p), 
		  speed_factor(speed_factor) {}
		
		void frameAction(uint32_t frame_time)	override {
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

			for( uint16_t i = 0 ; i < this->resolution; i++) {
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
				
				nblend( this->pattern_state[i], newcolor, 64);
			}
		}
	protected:
		const uint8_t speed_factor;  // Factor to increase rate of change of pattern parameters
};


// Pacifica
//  Gentle, blue-green ocean waves.
//  December 2019, Mark Kriegsman and Mary Corey March.
// https://github.com/FastLED/FastLED/blob/master/examples/Pacifica/Pacifica.ino
CRGBPalette16 pacifica_palette_1 = 
    { 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117, 
      0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x14554B, 0x28AA50 };
CRGBPalette16 pacifica_palette_2 = 
    { 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117, 
      0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x0C5F52, 0x19BE5F };
CRGBPalette16 pacifica_palette_3 = 
    { 0x000208, 0x00030E, 0x000514, 0x00061A, 0x000820, 0x000927, 0x000B2D, 0x000C33, 
      0x000E39, 0x001040, 0x001450, 0x001860, 0x001C70, 0x002080, 0x1040BF, 0x2060FF };
	  
template<uint16_t t_resolution> 
class PacificaPattern: public LinearStatePattern<t_resolution>	{
	public:
		PacificaPattern():
		  LinearStatePattern<t_resolution>(White_p) {}
		
		void frameAction(uint32_t frame_time)	override {
			// Increment the four "color index start" counters, one for each wave layer.
			// Each is incremented at a different speed, and the speeds vary over time.
			static uint16_t sCIStart1, sCIStart2, sCIStart3, sCIStart4;
			static uint32_t sLastms = 0;
			uint32_t deltams = frame_time - sLastms;
			sLastms = frame_time;
			uint16_t speedfactor1 = beatsin16(3, 179, 269);
			uint16_t speedfactor2 = beatsin16(4, 179, 269);
			uint32_t deltams1 = (deltams * speedfactor1) / 256;
			uint32_t deltams2 = (deltams * speedfactor2) / 256;
			uint32_t deltams21 = (deltams1 + deltams2) / 2;
			sCIStart1 += (deltams1 * beatsin88(1011,10,13));
			sCIStart2 -= (deltams21 * beatsin88(777,8,11));
			sCIStart3 -= (deltams1 * beatsin88(501,5,7));
			sCIStart4 -= (deltams2 * beatsin88(257,4,6));

			// Clear out the LED array to a dim background blue-green
			fill_solid(this->pattern_state, this->resolution, CRGB( 2, 6, 10));

			// Render each of four layers, with different scales and speeds, that vary over time
			this->pacifica_one_layer( pacifica_palette_1, sCIStart1, beatsin16( 3, 11 * 256, 14 * 256), beatsin8( 10, 70, 130), 0-beat16( 301) );
			this->pacifica_one_layer( pacifica_palette_2, sCIStart2, beatsin16( 4,  6 * 256,  9 * 256), beatsin8( 17, 40,  80), beat16( 401) );
			this->pacifica_one_layer( pacifica_palette_3, sCIStart3, 6 * 256, beatsin8( 9, 10,38), 0-beat16(503));
			this->pacifica_one_layer( pacifica_palette_3, sCIStart4, 5 * 256, beatsin8( 8, 10,28), beat16(601));

			// Add brighter 'whitecaps' where the waves lines up more
			this->pacifica_add_whitecaps();

			// Deepen the blues and greens a bit
			this->pacifica_deepen_colors();
		}
	protected:
		// Add one layer of waves into the led array
		void pacifica_one_layer( CRGBPalette16& p, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff)
		{
			uint16_t ci = cistart;
			uint16_t waveangle = ioff;
			uint16_t wavescale_half = (wavescale / 2) + 20;
			for( uint16_t i = 0; i < this->resolution; i++) {
				waveangle += 250;
				uint16_t s16 = sin16( waveangle ) + 32768;
				uint16_t cs = scale16( s16 , wavescale_half ) + wavescale_half;
				ci += cs;
				uint16_t sindex16 = sin16( ci) + 32768;
				uint8_t sindex8 = scale16( sindex16, 240);
				CRGB c = ColorFromPalette( p, sindex8, bri, LINEARBLEND);
				this->pattern_state[i] += c;
			}
		}
		// Add extra 'white' to areas where the four layers of light have lined up brightly
		void pacifica_add_whitecaps()
		{
			uint8_t basethreshold = beatsin8( 9, 55, 65);
			uint8_t wave = beat8( 7 );

			for( uint16_t i = 0; i < this->resolution; i++) {
				uint8_t threshold = scale8( sin8( wave), 20) + basethreshold;
				wave += 7;
				uint8_t l = this->pattern_state[i].getAverageLight();
				if( l > threshold) {
					uint8_t overage = l - threshold;
					uint8_t overage2 = qadd8( overage, overage);
					this->pattern_state[i] += CRGB( overage, overage2, qadd8( overage2, overage2));
				}
			}
		}
		// Deepen the blues and greens
		void pacifica_deepen_colors()
		{
			for( uint16_t i = 0; i < this->resolution; i++) {
				this->pattern_state[i].blue = scale8( this->pattern_state[i].blue,  145); 
				this->pattern_state[i].green= scale8( this->pattern_state[i].green, 200); 
				this->pattern_state[i] |= CRGB( 2, 5, 7);
			}
		}
};

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
template<uint16_t t_resolution> 
class Twinkle : public LinearStatePattern<t_resolution>   {
  public:
    Twinkle(uint8_t twinkle_speed = 6, CRGBPalette16 colour_palette = FairyLight_p, uint8_t twinkle_density = 4, CRGB bg = CRGB::Black):
      LinearStatePattern<t_resolution>(colour_palette), bg(bg), bg_brightness(bg.getAverageLight()), twinkle_speed(twinkle_speed), twinkle_density(twinkle_density)  {}

    void frameAction(uint32_t frame_time) override {
		// "this->PRNG16" is the pseudorandom number generator
		this->PRNG16 = 11337;
		this->clock32 = frame_time;
		// Set pattern state
		for (uint16_t i=0; i<this->resolution; i++) {
		  this->pattern_state[i] = this->get_pos_value(i);
		}
    }

    CRGB get_pos_value(uint16_t i)  {
      CRGB pixel;
      this->PRNG16 = (uint16_t)(this->PRNG16 * 2053) + 1384; // next 'random' number
      uint16_t myclockoffset16 = this->PRNG16; // use that number as clock offset
      this->PRNG16 = (uint16_t)(this->PRNG16 * 2053) + 1384; // next 'random' number
      // use that number as clock speed adjustment factor (in 8ths, from 8/8ths to 23/8ths)
      uint8_t myspeedmultiplierQ5_3 =  ((((this->PRNG16 & 0xFF) >> 4) + (this->PRNG16 & 0x0F)) & 0x0F) + 0x08;
      uint32_t myclock30 = (uint32_t)((this->clock32 * myspeedmultiplierQ5_3) >> 3) + myclockoffset16;
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
        c = this->colorFromPalette(hue, bright, NOBLEND);
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
    uint16_t PRNG16, clock32;
};

// Extends head to end of strip then retracts tail
class GrowThenShrink : public LinearPattern  {
	public:
		GrowThenShrink(uint16_t resolution, CRGBPalette16 colour_palette = RainbowColors_p):
		LinearPattern(resolution, colour_palette) {}
		
		void reset() override {
			LinearPattern::reset();
			this->head_pos = this->tail_pos = 0;
			this->reverse = false;
		}

		void frameAction(uint32_t frame_time) override {
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
				if (this->head_pos < this->resolution-1)	{
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

		}

		CRGB getPixelValue(uint16_t i)  const override  {
			if ((this->tail_pos <= i) && (i <= this->head_pos)) 	{
				return this->colorFromPalette((i*255)/this->resolution);
			} else {
				return CRGB::Black;
			}
		}
	protected:
		uint16_t head_pos, tail_pos;
		bool reverse;
};
		
template<uint16_t t_resolution> 
class SparkleFill : public LinearStatePattern<t_resolution>  {
  public:
    SparkleFill(CRGBPalette16 colour_palette=RainbowColors_p):
      LinearStatePattern<t_resolution>(colour_palette) {}
	  
	void reset() {
		LinearStatePattern<t_resolution>::reset();
		this->fill = true;
		this->remaining = this->resolution;
	}
	
	void frameAction(uint32_t frame_time)	override {
		for (uint16_t i=0; i < this->resolution; i++) 	{
			uint8_t brightness = this->pattern_state[i].getAverageLight();
			// Probabilty to fill/un-fill is proportional to amount remaining
			if (random(0,this->remaining) == 0) {
				// Fill with random palette value, or unfill
				if (brightness) 	{
					// Brighten existing pixel if filling
					if (this->fill)	{
						if (brightness < 200)	{
							this->pattern_state[i]*=2;
						}
					} else {
						// reduce brightness or set to black if un-filling
						if (brightness > 32)	{
							this->pattern_state[i]/= 2;
						} else {
							this->pattern_state[i] = CRGB::Black;
							this->remaining--;
						}
					}
				} else if (this->fill) {
					// Fill with new colour
					this->pattern_state[i] = this->colorFromPalette(random(0,256), 32);
					this->remaining--;
				}
				
				if (this->remaining == 0)	{
					this->remaining = this->resolution;
					this->fill = !this->fill;
				}
			}
		}
		
	}
	
	protected:
		bool fill=true;   // Whether pattern is in fill mode (True) or un-fill
		uint16_t remaining=t_resolution;	// Number of remaining pixels to fill/un-fill
	
};

//https://gist.github.com/kriegsman/626dca2f9d2189bd82ca
// *Flashing* rainbow lights that zoom back and forth to a beat.
template<uint16_t t_resolution> 
class DiscoStrobe : public LinearStatePattern<t_resolution>  {
  public:
    DiscoStrobe(CRGBPalette16 colour_palette=HalloweenColors_p):
      LinearStatePattern<t_resolution>(colour_palette) {}
	
	void frameAction(uint32_t frame_time)	override {
		// First, we black out all the LEDs
		fill_solid(this->pattern_state, this->resolution, CRGB::Black);
		
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
			this->discoWorker( dashperiod, dashwidth, dashmotionspeed, strobesPerPosition, hueShift);
		}

	}
	protected:
		// discoWorker updates the positions of the dashes, and calls the draw function
		void discoWorker( 
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
		  this->drawRainbowDashes( sStartPosition, 
							 dashperiod, dashwidth, 
							 sStartHue, huedelta, 
							 kSaturation, kValue);
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
		  uint8_t startpos, uint8_t period, uint8_t width, 
		  uint8_t huestart, uint8_t huedelta, uint8_t saturation, uint8_t value)
		{
		  uint8_t hue = huestart;
		  for( uint16_t i = startpos; i <= this->resolution-1; i += period) {
			// Switched from HSV color wheel to color palette
			// Was: CRGB color = CHSV( hue, saturation, value); 
			CRGB color = this->colorFromPalette(hue, value, NOBLEND);
			
			// draw one dash
			uint16_t pos = i;
			for( uint8_t w = 0; w < width; w++) {
			  this->pattern_state[ pos ] = color;
			  pos++;
			  if( pos >= this->resolution) {
				break;
			  }
			}
			
			hue += huedelta;
		  }
		}
		uint8_t bpm=61;
};

//https://github.com/FastLED/FastLED/blob/master/examples/Fire2012WithPalette/Fire2012WithPalette.ino
template<uint16_t t_resolution> 
class FirePattern: public LinearPattern   {
  public:
		FirePattern(
		uint8_t cooling=60,    // Less cooling = taller flames.  More cooling = shorter flames. Default 60, suggested range 20-100 
		uint8_t sparking=100): // Higher chance = more roaring fire.  Lower chance = more flickery fire. Default 100, suggested range 50-200.
		LinearPattern(t_resolution, CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::Yellow, CRGB::White)),
		cooling(cooling),  	
		sparking(sparking)  
		{

		};
		
	void reset()	override {
		LinearPattern::reset();
		// Reset heat array to 0
		for (uint8_t i=0; i<this->resolution; i++) {
			this->heat[i] = 0;
		}	
	}
		
	void frameAction(uint32_t frame_time) override {
		random16_add_entropy(random());
		// Step 1.  Cool down every cell a little
		for(uint8_t i = 0; i < this->resolution; i++) {
		  this->heat[i] = qsub8( this->heat[i],  random8(0, ((this->cooling * 10) / this->resolution) + 2));
		}
	  
		// Step 2.  Heat from each cell drifts 'up' and diffuses a little
		for(uint8_t k= this->resolution - 1; k >= 2; k--) {
		  this->heat[k] = (this->heat[k - 1] + 2*this->heat[k - 2]) / 3;
		}
		
		// Step 3.  Randomly ignite new 'sparks' of heat near the bottom
		if(random8() < this->sparking ) {
		  uint8_t y = random8(this->resolution/5 + 1);
		  this->heat[y] = qadd8( this->heat[y], random8(160,220) );
		}
	}
	
	CRGB getPixelValue(uint16_t i) const override {
		// Get heat value, Scale from 0-255 down to 0-240, select colour from palette
		uint8_t colorindex = scale8(this->heat[i], 240);
		// Constrain base heat (so base of fire doesnt look too bright
		if (i < (this->resolution/10) + 1)	{
			colorindex = constrain(colorindex, 40, 120);
		}
		return this->colorFromPalette(colorindex);
	}
	
	protected:
		uint8_t heat[t_resolution]; // Array to store heat values
		uint8_t cooling, sparking;
	
};

class GrowingSphere: public SpatialPattern	{
	public:
		GrowingSphere(
			uint8_t speed=1,
			CRGBPalette16 colour_palette=RainbowColors_p
		) : SpatialPattern(colour_palette), 
		speed(speed) {}
		
		void reset()	override {
			SpatialPattern::reset();
			this->radius = 0;
			this->growing = true;
		}
		
		void frameAction(uint32_t frame_time) override {
			if (this->growing) {
				this->radius += this->speed;
				if (this->radius >= this->resolution)	{
					this->growing = false;
				}
			} else {
				if (this->radius <= this->speed) {
					this->growing = true;
				} else {
					this->radius -= this->speed;
				}
			}
		};
		
		CRGB getPixelValue(Point point) const override { 
			float point_distance = point.norm();
			if (point_distance > this->radius) 	{
				return CRGB::Black;
			} else {
				return this->colorFromPalette((255*point_distance)/this->resolution);
			}
		}
	private:
		const uint8_t speed; 		// Speed at which sphere grows and shrinks
		uint16_t radius;   	// Current radius of sphere
		bool growing;		// Whether sphere is growing or shrinking
};
