#include <LEDController.h>
#include <FastLED.h>

// Simple moving pulse of light along axis
// Includes sub-pixel interpolation for smoother motion 
// So instead of jumping one pixel at a time, can effectively move a fraction of a pixel with each step
class MovingPulse: public LinearPattern  {
  public:
    MovingPulse(unsigned int axis_len, unsigned int frame_delay, byte pulse_len=3, byte smooth_factor=2, CRGBPalette16 colour_palette = White_p):
      LinearPattern(axis_len, frame_delay, colour_palette), 
	  head_pos(0), pulse_len(pulse_len), smooth_factor(smooth_factor), tail_interpolator(Interpolator(0, 255, (pulse_len*smooth_factor) + 1, 0))  {}

    // Update pulse position (on higher-resolution virtual axis)
	// Bounds are from 0 -> axis_len*smooth_factor
    void frameAction()  override {
      this->head_pos = (this->head_pos + 1) % (this->axis_len*this->smooth_factor);
    }

    // Construct pulse from head position
	// i is from 0 -> axis_len
    CRGB getLEDValue(unsigned int i) {
		// Start with black
		CRGB val = CRGB::Black;
		// Get average value of all virtual sub-pixels that make up physical pixel
		for (unsigned int vp=i*this->smooth_factor; vp < (i+1)*this->smooth_factor; vp++)	{
			val += this->getVirtualLEDValue(vp)/this->smooth_factor;
		}
		return val;
      
    }

  private:
	// Get value for pixel as position i on virtual high-resolution axis
	// i is from 0 -> axis_len*smooth_factor
	CRGB getVirtualLEDValue(unsigned int i)	{
		// Figure out distance behind pulse head to get brightness
		int distance_behind_head = this->head_pos - i;
		// Case of when position is in front of pulse head so distance_behind_head is negative
		if (distance_behind_head < 0)	{
			distance_behind_head = (this->axis_len*this->smooth_factor) + distance_behind_head;
		}
		// If not within pulse width, return black
		if (distance_behind_head > this->pulse_len*this->smooth_factor)	{
			return CRGB::Black;
		}
		// Use interpolator to get brightness
		byte lum = tail_interpolator.get_value(distance_behind_head);
		byte hue = (i*255) / (this->axis_len*this->smooth_factor);
		return this->colorFromPalette(hue, lum);
	}
	
	
    unsigned int head_pos;    			// Position of head of pulse
    byte pulse_len;        				// Length of pulse (in actual pixels). Virtual pulse width is pulse_len*smooth_factor
	byte smooth_factor;					// Sub-pixel factor for smoother movement (higher values will require faster LED update for same pulse 'speed')
    Interpolator tail_interpolator;  	// Interpolator for pulse tail brightness
};


// Direction vectors
Point v_x(1, 0, 0);
Point v_y(0, 1, 0);
Point v_z(0, 0, 1);


// Plane that moves in given direction
class SweepingPlane : public SpatialPattern  {
  public:
    SweepingPlane(
      Point bounds,               // Point vector defining maximum magnitude of pattern space in x, y and z directions
      unsigned int frame_delay,   // Delay between pattern frames (in ms)
      Point direction,            // Vector defining direction of plane (normal to plane)
      byte thickness = 3,         // Thickness of plane
      CRGBPalette16 colour_palette = White_p,  // Colour palette to use for pattern (default to white)
      byte duration = DEFAULT_PATTERN_DURATION // Duration of pattern (in s)
    ): SpatialPattern(bounds, frame_delay, colour_palette, duration), pos(-bounds), direction(direction), thickness(thickness), brightness_interpolator(Interpolator(0, 255, thickness, 0)) {

    };

    void frameAction() override {
      // Update position
      pos = pos + direction;
      // Detect exceeding bounds
      if ((pos.x > bounds.x) || (pos.y > bounds.y) || (pos.z > bounds.z)) {
        pos = -bounds;

      }
    }

    // Brightness of point depends on distance to light ball
    CRGB getLEDValue(Point point) {
      // Get distance from point to plane
      float distance = point.distance_to_plane(direction, pos);
      byte lum = 0;
      if (distance > thickness) {
        lum = 0;
      } else  {
        lum = brightness_interpolator.get_value(distance);
      }
      return CRGB(lum, lum, lum);
    }

  protected:
    Point pos;          // Position of point on plane
    Point direction;    // Direction vector normal to plane
    byte thickness;     // Thickness of plane
    Interpolator brightness_interpolator;
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
class Twinkle : public LinearPattern  {
  public:
    Twinkle(unsigned int axis_len, unsigned int frame_delay, CRGBPalette16 colour_palette = FairyLight_p, CRGB bg = CRGB::Black, byte twinkle_speed = 6, byte twinkle_density = 4):
      LinearPattern(axis_len, frame_delay, colour_palette), bg(bg), bg_brightness(bg.getAverageLight()), twinkle_speed(twinkle_speed), twinkle_density(twinkle_density)  {}

    void frameAction() override {
      // "PRNG16" is the pseudorandom number generator
      PRNG16 = 11337;
      clock32 = millis();
    }

    CRGB getLEDValue(unsigned int i) {
      CRGB pixel;
      PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
      uint16_t myclockoffset16 = PRNG16; // use that number as clock offset
      PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
      // use that number as clock speed adjustment factor (in 8ths, from 8/8ths to 23/8ths)
      uint8_t myspeedmultiplierQ5_3 =  ((((PRNG16 & 0xFF) >> 4) + (PRNG16 & 0x0F)) & 0x0F) + 0x08;
      uint32_t myclock30 = (uint32_t)((clock32 * myspeedmultiplierQ5_3) >> 3) + myclockoffset16;
      uint8_t  myunique8 = PRNG16 >> 8; // get 'salt' value for this pixel

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
    byte bg_brightness;

    byte twinkle_speed;     // 0-8
    byte twinkle_density;   // 0-8
    uint16_t PRNG16, clock32;
};

// Extends to end of strip then end follows
class GrowThenShrink : public LinearPattern  {
	public:
		GrowThenShrink(unsigned int axis_len, unsigned int frame_delay, CRGBPalette16 colour_palette = RainbowColors_p):
		LinearPattern(axis_len, frame_delay, colour_palette) {}
		
		void reset() override {
			LinearPattern::reset();
			this->head_pos = this->tail_pos = 0;
			this->reverse = false;
		}

		void frameAction() override {
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
				if (this->head_pos < this->axis_len-1)	{
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

		CRGB getLEDValue(unsigned int i) {
			if ((this->tail_pos <= i) && (i <= this->head_pos)) 	{
				return this->colorFromPalette((i*255)/this->axis_len);
			} else {
				return CRGB::Black;
			}
		}
	protected:
		unsigned int head_pos, tail_pos;
		bool reverse;
};
		
template<unsigned int t_axis_len> 
class RandomFill : public LinearStatePattern<t_axis_len>  {
  public:
    RandomFill(byte frame_delay, CRGBPalette16 colour_palette=RainbowColors_p, byte duration=DEFAULT_PATTERN_DURATION):
      LinearStatePattern<t_axis_len>(frame_delay, colour_palette, duration) {}
	  
	void reset() {
		LinearStatePattern<t_axis_len>::reset();
		this->fill = true;
		this->remaining = this->axis_len;
	}
	
	void frameAction()	override {
		for (unsigned int i=0; i < this->axis_len; i++) 	{
			byte brightness = this->pattern_state[i].getAverageLight();
			// Check whether pixel is candidate to be filled/unfilled
			//if (is_filled != this->fill) 	{
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
						this->remaining = this->axis_len;
						this->fill = !this->fill;
					}
				}
				
			//}
		}
		
	}
	
	protected:
		bool fill=true;   // Whether pattern is in fill mode (True) or un-fill
		unsigned int remaining=t_axis_len;	// Number of remaining pixels to fill/un-fill
	
};

//https://gist.github.com/kriegsman/626dca2f9d2189bd82ca
// *Flashing* rainbow lights that zoom back and forth to a beat.
template<unsigned int t_axis_len> 
class DiscoStrobe : public LinearStatePattern<t_axis_len>  {
  public:
    DiscoStrobe(byte frame_delay, CRGBPalette16 colour_palette=HalloweenColors_p):
      LinearStatePattern<t_axis_len>(frame_delay, colour_palette) {}
	
	void frameAction()	override {
		// First, we black out all the LEDs
		fill_solid(this->pattern_state, this->axis_len, CRGB::Black);
		
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
		  for( uint16_t i = startpos; i <= this->axis_len-1; i += period) {
			// Switched from HSV color wheel to color palette
			// Was: CRGB color = CHSV( hue, saturation, value); 
			CRGB color = this->colorFromPalette(hue, value, NOBLEND);
			
			// draw one dash
			uint16_t pos = i;
			for( uint8_t w = 0; w < width; w++) {
			  this->pattern_state[ pos ] = color;
			  pos++;
			  if( pos >= this->axis_len) {
				break;
			  }
			}
			
			hue += huedelta;
		  }
		}
		byte bpm=61;
};

//https://github.com/FastLED/FastLED/blob/master/examples/Fire2012WithPalette/Fire2012WithPalette.ino
// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 55, suggested range 20-100 
#define COOLING  65

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 100
template<unsigned int t_flame_height> 
class SpatialFire : public SpatialPattern	{
	public:
		SpatialFire(
		  Point bounds,               // Point vector defining maximum magnitude of pattern space in x, y and z directions
		  unsigned int frame_delay,   // Delay between pattern frames (in ms)
		  CRGBPalette16 colour_palette = HeatColors_p,  // Colour palette to use for pattern (default to white)
		  byte duration = DEFAULT_PATTERN_DURATION // Duration of pattern (in s)
		): SpatialPattern(bounds, frame_delay, colour_palette, duration) {

		};
		
	void frameAction() override {
		random16_add_entropy( random());
		// Step 1.  Cool down every cell a little
		for(byte i = 0; i < t_flame_height; i++) {
		  this->heat[i] = qsub8( this->heat[i],  random8(0, ((COOLING * 10) / t_flame_height) + 2));
		}
	  
		// Step 2.  Heat from each cell drifts 'up' and diffuses a little
		for( byte k= t_flame_height - 1; k >= 2; k--) {
		  this->heat[k] = (this->heat[k - 1] + this->heat[k - 2] + this->heat[k - 2] ) / 3;
		}
		
		// Step 3.  Randomly ignite new 'sparks' of heat near the bottom
		if( random8() < SPARKING ) {
		  byte y = random8(7);
		  this->heat[y] = qadd8( this->heat[y], random8(160,255) );
		}
	}
	
	CRGB getLEDValue(Point point)	{
		// Get heat value by y-coordinate (-bounds.z -> bounds.z to 0 -> buonds.z*2)
		// Scale the heat value from 0-255 down to 0-240
		// for best results with color palettes.
		byte heat_index = (int) (this->bounds.z + point.z+0.5);
		byte colorindex = scale8(this->heat[heat_index], 240);
		// Constrain base heat
		if (heat_index == 0)	{
			colorindex = constrain(colorindex, 40, 120);
		}
		CRGB color = this->colorFromPalette(colorindex);
		return color;
	}
	
	protected:
		byte heat[t_flame_height];
	
};


//class BouncingBall : public SpatialPattern  {
//  public:
//    BouncingBall(
//      Point bounds,               // Point vector defining maximum magnitude of pattern space in x, y and z directions
//      unsigned int frame_delay,   // Delay between pattern frames (in ms)
//      byte ball_size=6,           // Size of light ball
//      CRGBPalette16 colour_palette=White_p,    // Colour palette to use for pattern (default to white)
//      byte duration=DEFAULT_PATTERN_DURATION  // Duration of pattern (in s)
//      ): SpatialPattern(bounds, frame_delay, colour_palette, duration), ball_pos(Point()) {
//        // Randomize initial velocity proportional to bounds
//        float bounds_size = bounds.norm()/30;
//        velocity = Point(bounds_size*(random(0,200)/100.0 - 1.0), bounds_size*(random(0,200)/100.0 - 1.0), bounds_size*(random(0,200)/100.0 - 1.0));
//
//    };
//
//    void frameAction() override {
//      DPRINT(ball_pos.x);
//      DPRINT(ball_pos.y);
//      DPRINT(ball_pos.z);
//      DPRINT(velocity.x);
//      DPRINT(velocity.y);
//      DPRINT(velocity.z);
//      // Update position
//      ball_pos = ball_pos + velocity;
//      // Detect bounces
//      // Bounce off positive X face
//      if ((ball_pos.x >= bounds.x) && (velocity.x > 0)) {
//        velocity.x = -velocity.x;
//      }
//      // Bounce off negative X face
//      if ((ball_pos.x <= -bounds.x) && (velocity.x < 0)) {
//        velocity.x = -velocity.x;
//      }
//
//      // Bounce off positive Y face
//      if ((ball_pos.y >= bounds.y) && (velocity.y > 0)) {
//        velocity.y = -velocity.y;
//      }
//      // Bounce off negative Y face
//      if ((ball_pos.y <= -bounds.y) && (velocity.y < 0)) {
//        velocity.y = -velocity.y;
//      }
//
//      // Bounce off positive Z face
//      if ((ball_pos.z >= bounds.z) && (velocity.z > 0)) {
//        velocity.z = -velocity.z;
//      }
//      // Bounce off negative Z face
//      if ((ball_pos.z <= -bounds.z) && (velocity.z < 0)) {
//        velocity.z = -velocity.z;
//      }
//    }
//
//    // Brightness of point depends on distance to light ball
//    CRGB get_led_value(Point point) {
//      byte lum = 100 - point.distance(ball_pos)*10;//std::pow(point.distance(ball_pos),2);
//      return CRGB(lum, lum, lum);
//    }
//
//  protected:
//    Point ball_pos;
//    Point velocity;
//};
