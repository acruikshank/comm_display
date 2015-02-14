// This is a demonstration on how to use an input device to trigger changes on your neo pixels.
// You should wire a momentary push button to connect from ground to a digital IO pin.  When you
// press the button it will change to a new pixel animation.  Note that you need to press the
// button once to start the first animation!

#include <PS2Keyboard.h>
#include <SerialLCD.h>
#include <Wire.h>
#include <OctoWS2811.h>



#define KEYBOARD_DATA_PIN 0
#define KEYBOARD_IRQ_PIN 1
#define CENTER_PIXEL_COUNT 600
#define LEFT_PIXEL_COUNT 72
#define RIGHT_PIXEL_COUNT 72
#define BUFFER_SIZE 8

const int ledsPerStrip = 72;
const int WORD_LENGTH = BUFFER_SIZE * 8;
const int LINE_LENGTH = ledsPerStrip * 4;

enum states { CONTROL_OPEN, LEFT_CONTROL, LEFT_SENDING, RIGHT_CONTROL, RIGHT_SENDING };
enum bufferPos { BUFFER_LEFT, BUFFER_RIGHT };

DMAMEM int displayMemory[ledsPerStrip*6];
int drawingMemory[ledsPerStrip*6];

const int config = WS2811_GRB | WS2811_800kHz;

OctoWS2811 leds(ledsPerStrip, displayMemory, drawingMemory, config);


PS2Keyboard keyboard;
int i=0;

SerialLCD lcd(2,16,0x28,I2C);
char buffer[BUFFER_SIZE + 1];
int charPointer = 0;
int sendShift = 0;
int transmitShift = 0;
int receiveShift = 0;
int state = CONTROL_OPEN;
int rainbowColors[180];
int activeTransmitLed = 0;

boolean ledState = 0;
void setup() {
  // Initialize LCD module
  lcd.init();
  for (int i=0; i<180; i++) {
    int hue = i * 2;
    int saturation = 100;
    int lightness = 50;
    // pre-compute the 180 rainbow colors
    rainbowColors[i] = makeColor(hue, saturation, lightness);
  }


  leds.begin();
  clearLeds();
  leds.show();

  keyboard.begin(KEYBOARD_DATA_PIN, KEYBOARD_IRQ_PIN);
  Serial.begin(9600);
  Serial.println("Keyboard Test:");
  
  lcd.clear();
  lcd.setContrast(40);
  
  for (int i=0; i<=BUFFER_SIZE; i++) buffer[i]=0;
}

void loop() {
  switch (state) {
    case CONTROL_OPEN:
    case RIGHT_CONTROL: control(true); break;
    case LEFT_CONTROL: control(false); break;
    case RIGHT_SENDING: rightSending(); break;
    case LEFT_SENDING: leftSending(); break;
  }
//  delay(6);
}

//writes zeroes to the entire strip buffer
void clearLeds() {
  for (i=0; i < ledsPerStrip * 3; i++) {
    leds.setPixel(i, 0);
  }
}


void control(boolean rightSide) {
  if (keyboard.available()) {
    char c = keyboard.read();

    if (c == PS2_BACKSPACE) {
      if (charPointer > 0)
        buffer[--charPointer] = 0;
    } else if (c == PS2_ENTER) {
      if (rightSide) {
        sendShift = 0;
        transmitShift = WORD_LENGTH;
        state = RIGHT_SENDING;
      } else {
        sendShift = WORD_LENGTH;
        transmitShift = LINE_LENGTH - WORD_LENGTH;
        state = LEFT_SENDING;
      }
    } else {
      if (charPointer < BUFFER_SIZE)
        buffer[charPointer++] = c;
    }

    lcd.clear();
    
    lcd.home();
    lcd.print(buffer);
    
    bitLights(buffer);
  }
}

void rightSending() {
  bitLights(buffer);
  transmitShift += 2;
  if (transmitShift > LINE_LENGTH - WORD_LENGTH) {
    transmitShift = WORD_LENGTH;
    sendShift++;
  }
  
  if (sendShift > WORD_LENGTH) {
    sendShift = WORD_LENGTH;
    state = LEFT_CONTROL;
    bitLights(buffer);
  }
}

void leftSending() {
  bitLights(buffer);
  transmitShift -= 2;
  if (transmitShift < WORD_LENGTH) {
    transmitShift = LINE_LENGTH - WORD_LENGTH - 1;
    sendShift--;
    if (sendShift < 0) {
      sendShift = 0;
      state = RIGHT_CONTROL;
      bitLights(buffer);
    }
  }
  
}

void transmitBit() {
  if((*buffer & ( 1 << sendShift)) >> sendShift) {
    for (int i=64; i < 223; i++) {
      leds.setPixel( i, rainbowColors[sendShift % 180]);
      leds.setPixel( i-1, 0);
      leds.show();
    }
  }
}
  
  

void bitLights(char *buffer) {
  boolean transmit = 0;
  for (int i=0; i<LINE_LENGTH; i++) leds.setPixel(i,0);
  
  if (state == RIGHT_SENDING || state == LEFT_SENDING) {
    leds.setPixel( transmitShift, rainbowColors[transmitShift % 180] );
    leds.setPixel( transmitShift-1, rainbowColors[transmitShift % 180] );
  }
    
  for (int i=0; i<8; i++) {
    for (int j=0; j<8; j++) {
      
      int pos = sendShift + i*8+j;
      if (pos >= WORD_LENGTH)
        pos += (LINE_LENGTH - 2*WORD_LENGTH);
      
      ledState = (buffer[i] >> j) & 0x1;
      leds.setPixel( pos, ledState * dim(rainbowColors[pos % 180])) ;
    }
  }
  leds.show();
}

unsigned int dim(unsigned int color) {
  
  return (color >> 2) & 4144959;
}
