#ifndef palettes_h
#define palettes_h
#include <FastLED.h>

const TProgmemRGBPalette16 White_p FL_PROGMEM =
{
	CRGB::Grey, CRGB::Grey, CRGB::Grey, CRGB::Grey, 
	CRGB::Grey, CRGB::Grey, CRGB::Grey, CRGB::Grey, 
	CRGB::Grey, CRGB::Grey, CRGB::Grey, CRGB::Grey, 
	CRGB::Grey, CRGB::Grey, CRGB::Grey, CRGB::Grey 
};

// A pure "fairy light" palette with some brightness variations
#define HALFFAIRY ((CRGB::FairyLight & 0xFEFEFE) / 2)
#define QUARTERFAIRY ((CRGB::FairyLight & 0xFCFCFC) / 4)
const TProgmemRGBPalette16 FairyLight_p FL_PROGMEM =
{  CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight, 
   HALFFAIRY,        HALFFAIRY,        CRGB::FairyLight, CRGB::FairyLight, 
   QUARTERFAIRY,     QUARTERFAIRY,     CRGB::FairyLight, CRGB::FairyLight, 
   CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight };

// A palette of soft snowflakes with the occasional bright one
const TProgmemRGBPalette16 Snow_p FL_PROGMEM =
{  0x304048, 0x304048, 0x304048, 0x304048,
   0x304048, 0x304048, 0x304048, 0x304048,
   0x304048, 0x304048, 0x304048, 0x304048,
   0x304048, 0x304048, 0x304048, 0xE0F0FF };
   
// A palette reminiscent of large 'old-school' C9-size tree lights
// in the five classic colors: red, orange, green, blue, and white.
#define C9_Red    0xB80400
#define C9_Orange 0x902C02
#define C9_Green  0x046002
#define C9_Blue   0x070758
#define C9_White  0x606820
const TProgmemRGBPalette16 RetroC9_p FL_PROGMEM =
{  C9_Red,    C9_Orange, C9_Red,    C9_Orange,
   C9_Orange, C9_Red,    C9_Orange, C9_Red,
   C9_Green,  C9_Green,  C9_Green,  C9_Green,
   C9_Blue,   C9_Blue,   C9_Blue,
   C9_White
};

// define some shorthands for the Halloween colors
#define PURP 0x6611FF
#define ORAN 0xFF6600
#define GREN 0x00FF11
#define WHIT 0xCCCCCC

// set up a new 16-color palette with the Halloween colors
const TProgmemRGBPalette16 HalloweenColors_p FL_PROGMEM =
{ 
  PURP, PURP, PURP, PURP,
  ORAN, ORAN, ORAN, ORAN,
  PURP, PURP, PURP, PURP,
  GREN, GREN, GREN, WHIT
};


#endif