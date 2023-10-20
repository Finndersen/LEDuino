#ifndef utils_h
#define  utils_h

#define sgn(x) ((x > 0) - (x < 0))
#define same_sign(x, y) ((x<0) == (y<0))

// Check whether a is between x and y (inclusive)
#define between(a, x, y) (y >= x ? (a >= x && a <= y) : (a >= y && a <= x))

// Limit maximum value
#define limit(x, max) (x > max ? max : x)

// Used for interpolating values on a linear gradient determined by two provided points
class Interpolator	{
	public:
		Interpolator(float x1, float y1, float x2, float y2) : 
    x1(x1), 
    y1(y1), 
    gradient((y2-y1)/(x2-x1))	{

		}
		// Interpolate y value for x input
		float get_value(float x)	{
			return y1 + (x-x1)*gradient;
		}
		
	protected:
		const float x1, y1, gradient;
};

// Subtraction with support for wrapping around back to max_value
uint16_t wrap_subtract(uint16_t value, uint16_t subtract, uint16_t max_value)	{
	if (subtract <= value) 	{
		return value - subtract;
	} else {
		return max_value - (subtract - value-1);
	}
}

/*
This function is like 'triwave8', which produces a 
 symmetrical up-and-down triangle sawtooth waveform, except that this
 function produces a triangle wave with a faster attack and a slower decay:

     / \ 
    /    \ 
   /       \ 
  /          \ 
*/

uint8_t attackDecayWave8( uint8_t i)
{
  if( i < 86) {
    return i * 3;
  } else {
    i -= 86;
    return 255 - (i + (i/2));
  }
}

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}

#endif