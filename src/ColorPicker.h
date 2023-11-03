#ifndef colorpicker_h
#define colorpicker_h
#include <FastLED.h>


// Base class for picking a color for use by a pattern
// Allows for subclasses which wrap and provides an interface to the various types of palettes used in FastLED
// Can be subclassed to make custom palette implementations (doesnt have to involve using FastLEDs palette utilities)  
class ColorPicker {
  public:

    // Basic placeholder implementation just gets colour from provided hue and brightness
		virtual CRGB getColor(uint8_t hue, uint8_t brightness=255, uint8_t saturation=255) const {
      return CHSV(hue, saturation, brightness);
    };

};


// Templated Colour Picker class for using FastLED 16-entry RGB palette types (CRGBPalette16 and TProgmemRGBPalette16)
template <typename T>
class PaletteColorPicker: public ColorPicker {
  public: 
    PaletteColorPicker(
      const T& colour_palette,
      TBlendType blendType=LINEARBLEND    // Set the blend type to use when choosing palette colour
    ): _palette(colour_palette),
      blendType(blendType) {}

  		// Select colour from palette
		CRGB getColor(uint8_t hue, uint8_t brightness=255, uint8_t saturation=255) const override {
      return ColorFromPalette(this->_palette, hue, brightness, this->blendType);
    }

  protected:
    const T& _palette;
    TBlendType blendType;
};

// Allows using a FastLED CRGBPalette16 defined in RAM
typedef PaletteColorPicker<CRGBPalette16> RGBPalettePicker;

// Allows using a FastLED TProgmemPalette16 type static palette which has its data stored in flash,
// such as the FastLED 'preset' palettes: RainbowColors_p, RainbowStripeColors_p, OceanColors_p, etc
typedef PaletteColorPicker<TProgmemRGBPalette16> ProgmemRGBPalettePicker;

// Allows using a FastLED static gradient palette, declared using DEFINE_GRADIENT_PALETTE macro and stored in PROGMEM (Flash)
// Creates a CRGBPalette16 instance in memory when initialised
class GradientPalettePicker: public ColorPicker {
  public:
    GradientPalettePicker(
      const TProgmemRGBGradientPalettePtr colour_palette  // Equivalent to TProgmemRGBGradientPalette_bytes
    ): _palette(colour_palette) {}

  		// Select colour from palette
		CRGB getColor(uint8_t hue, uint8_t brightness=255, uint8_t saturation=255) const override {
      return ColorFromPalette(this->_palette, hue, brightness, LINEARBLEND);
    }

  protected:
    const CRGBPalette16 _palette;
};


// Use when pattern does not require a palette
ColorPicker Basic_picker;

// LEDuino versions of some FastLED preset palettes
ProgmemRGBPalettePicker RainbowColors_picker(RainbowColors_p);
ProgmemRGBPalettePicker HeatColors_picker(HeatColors_p);


// Additional Example Palettes

// A pure "fairy light" palette with some brightness variations
#define HALFFAIRY ((CRGB::FairyLight & 0xFEFEFE) / 2)
#define QUARTERFAIRY ((CRGB::FairyLight & 0xFCFCFC) / 4)
const TProgmemRGBPalette16 FairyLight_p FL_PROGMEM =
{  CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight, 
   HALFFAIRY,        HALFFAIRY,        CRGB::FairyLight, CRGB::FairyLight, 
   QUARTERFAIRY,     QUARTERFAIRY,     CRGB::FairyLight, CRGB::FairyLight, 
   CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight 
};
ProgmemRGBPalettePicker FairyLight_picker(FairyLight_p, NOBLEND);

   
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
ProgmemRGBPalettePicker RetroC9Colors_picker(RetroC9_p);

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
ProgmemRGBPalettePicker HalloweenColors_picker(HalloweenColors_p, NOBLEND);

// Example Gradient palette
// DEFINE_GRADIENT_PALETTE(xmas_p) {
//   0, 255, 0, 0, // red
//   64, 0, 255, 0, // green
//   128, 0, 0, 255, // blue
//   192, 255, 255, 0, // yellow
//   255, 235, 40, 200 // purple
// };

// GradientPalettePicker XMasColors_picker(xmas_p);

#endif