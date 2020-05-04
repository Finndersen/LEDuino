#ifndef utils_h
#define  utils_h
#ifdef DEBUG    //Macros are usually in all capital letters.
  #define DPRINT(...)    Serial.print(__VA_ARGS__); Serial.flush(); //DPRINT is a macro, debug print
  #define DPRINTLN(...)  Serial.println(__VA_ARGS__); Serial.flush();   //DPRINTLN is a macro, debug print with new line
#else
  #define DPRINT(...)     //now defines a blank line
  #define DPRINTLN(...)   //now defines a blank line
#endif

// Simple linear interpolation class
class Interpolator	{
	public:
		Interpolator(float x1, float y1, float x2, float y2) : x1(x1), y1(y1), gradient((y2-y1)/(x2-x1))	{

		}
		// Interpolate y value for x input
		float get_value(float x)	{
			return y1 + (x-x1)*gradient;
		}
		
	protected:
		float x1, y1, gradient;
};

// Subtraction with support for wrapping around back to max_value
unsigned int wrap_subtract(unsigned int value, unsigned int subtract, unsigned int max_value)	{
	if (subtract <= value) 	{
		return value - subtract;
	} else {
		return max_value - (subtract - value - 1);
	}
}

#endif