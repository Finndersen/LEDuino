#include <FastLED.h>
#include "Pattern.h"

class GrowingSpherePattern: public SpatialPattern	{
	public:
		GrowingSpherePattern(
			uint8_t speed=1,
			const ColorPicker& color_picker=RainbowColors_picker
		) : SpatialPattern(color_picker), 
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
				return this->getColor((255*point_distance)/this->resolution);
			}
		}
	private:
		const uint8_t speed; 		// Speed at which sphere grows and shrinks
		uint16_t radius;   	// Current radius of sphere
		bool growing;		// Whether sphere is growing or shrinking
};