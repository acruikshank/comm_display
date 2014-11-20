// This is a demonstration on how to use an input device to trigger changes on your neo pixels.
// You should wire a momentary push button to connect from ground to a digital IO pin.  When you
// press the button it will change to a new pixel animation.  Note that you need to press the
// button once to start the first animation!

#include <Adafruit_NeoPixel.h>
#include <PS2Keyboard.h>
#include <SerialLCD.h>
#include <Wire.h>

#define PIXEL_PIN 6    // Digital IO pin connected to the NeoPixels.
#define KEYBOARD_DATA_PIN 8
#define KEYBOARD_IRQ_PIN 5
#define PIXEL_COUNT 288

PS2Keyboard keyboard;
int i=0;

// Parameter 1 = number of pixels in strip,  neopixel stick has 8
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream, correct for neopixel stick
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip), correct for neopixel stick
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

SerialLCD lcd(2,16,0x28,I2C);
char buffer[9];
int charPointer=0;

void setup() {
  // Initialize LCD module
  lcd.init();

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  keyboard.begin(KEYBOARD_DATA_PIN, KEYBOARD_IRQ_PIN);
  Serial.begin(9600);
  Serial.println("Keyboard Test:");
  
  lcd.clear();
  lcd.setContrast(40);
  
  for (int i=0; i<9; i++) buffer[i]=0;
}

void loop() {
  if (keyboard.available()) {
    char c = keyboard.read();
//    strip.clear();
//    strip.setPixelColor(c, Wheel(c));
//    strip.show();
    if (c == PS2_BACKSPACE) {
      if (charPointer > 0)
        buffer[--charPointer] = 0;
    } else {
      if (charPointer < 8)
        buffer[charPointer++] = c;
    }
    lcd.clear();
    lcd.home();
    lcd.print(buffer);
    
    bitLights(buffer);
  }
//  i = (i +1)% 288;
//  delay(20);
}

void bitLights(char *buffer) {
  strip.clear();
  for (int i=0; i<8; i++) {
    for (int j=0; j<8; j++) {
      int pos = i*8+j;
      strip.setPixelColor( pos, (buffer[i] >> j) & 0x1 ? Wheel(pos+charPointer*6) : strip.Color(0,0,0) );
    }
  }
  strip.show();
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}