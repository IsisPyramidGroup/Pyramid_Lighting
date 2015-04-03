#include <EEPROM.h>
#include "SPI.h"

/*****************************************************************************
Electrical Test for the upper controllers (0-7) of the Isis Pyramid 1.1,
exclusive of the RS-485 serial port.

This is just strandtest hardwired for Caroushell strips, sped up so the
wait isn't so long.

Connect pixel strands to the SPI pins: 11 (DATA) and 13 (CLOCK)

*****************************************************************************/

#define  MAX_LEDS_PER_STRAND  78  // LEDs
#define	 LEDS_PER_WS2801_STRAND		50
#define	 LEDS_PER_SM16716_STRAND	78

#define  WIPE_WAIT        20  // ms

#define  DATA_PIN    11
#define  CLOCK_PIN   13
#define  JUMPER_IN    8
#define  JUMPER_OUT   9

#define	WS2801	0
#define SM16716 1
uint8_t strand_type = WS2801;

uint8_t *live_strand;

uint8_t *alloc_strand_buffer() {
  return (uint8_t *)calloc(MAX_LEDS_PER_STRAND,3);
}

void start_strand(void) {
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  digitalWrite(DATA_PIN, LOW);
  digitalWrite(CLOCK_PIN, LOW);
  
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV16); // 1 MHz max, else flicker supposedly
  SPI.end();
}

void update_strand(uint8_t *pixels) {
  switch (strand_type) {
	case WS2801:	update_WS2801_strand(pixels);
			return;
					
	case SM16716:	update_SM16716_strand(pixels);
			return;
					
	default:        return;
  }
}

void update_WS2801_strand(uint8_t *pixels) {
  uint8_t i;
  
  SPI.begin();
  for (i=0; i < LEDS_PER_WS2801_STRAND*3; i++) {
    SPDR = pixels[i];
    while (!(SPSR & (1<<SPIF)));
  }
  SPI.end();
  delay(1);   // Data is latched by holding clock pin low for 1ms
}

void update_SM16716_strand(uint8_t *pixels) {
  uint8_t i;
  
  // 50-bit block of zeros before each update of pixels
  SPI.begin();
  SPDR = 0;
  while (!(SPSR & (1<<SPIF)));
  SPDR = 0;
  while (!(SPSR & (1<<SPIF)));
  SPDR = 0;
  while (!(SPSR & (1<<SPIF)));
  SPDR = 0;
  while (!(SPSR & (1<<SPIF)));
  SPDR = 0;
  while (!(SPSR & (1<<SPIF)));
  SPDR = 0;
  while (!(SPSR & (1<<SPIF)));

  SPI.end();
  
  digitalWrite(DATA_PIN, 0);
  digitalWrite(CLOCK_PIN, HIGH); digitalWrite(CLOCK_PIN, LOW);
  digitalWrite(CLOCK_PIN, HIGH); digitalWrite(CLOCK_PIN, LOW);
  
  for (i=0; i < LEDS_PER_SM16716_STRAND*3; i+=3) {

    // start bit
    digitalWrite(DATA_PIN, 1);
    digitalWrite(CLOCK_PIN, HIGH); digitalWrite(CLOCK_PIN, LOW);
  
    SPI.begin();  
    // send the pixel values
    SPDR = pixels[i];
    while (!(SPSR & (1<<SPIF)));
    SPDR = pixels[i+1];
    while (!(SPSR & (1<<SPIF)));
    SPDR = pixels[i+2];
    while (!(SPSR & (1<<SPIF)));
    SPI.end();
  }
  
  // send an extra blank pixel
    // start bit
    digitalWrite(DATA_PIN, 1);
    digitalWrite(CLOCK_PIN, HIGH); digitalWrite(CLOCK_PIN, LOW);
  SPI.begin();
  SPDR = 0;
  while (!(SPSR & (1<<SPIF)));
  SPDR = 0;
  while (!(SPSR & (1<<SPIF)));
  SPDR = 0;
  while (!(SPSR & (1<<SPIF)));
  SPI.end();

  
  delay(1);  // do we need to delay here?
  
//#endif
}

// Set pixel color from separate 8-bit R, G, B components:
void setPixelRGB(uint8_t *strand, uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
  uint8_t *p = strand + (n*3);
  
  *p++ = r;
  *p++ = g;
  *p   = b;
}

// Set pixel color from 'packed' 32-bit RGB value:
void setPixelColor(uint8_t *strand, uint16_t n, uint32_t c) {
  uint8_t *p = strand + (n*3);
  *p++ = c >> 16;   // Red
  *p++ = c >>  8;   // Green
  *p   = c;         // Blue
}

void setup() {
  
  pinMode(JUMPER_IN, INPUT);
  digitalWrite(JUMPER_IN, HIGH);
  pinMode(JUMPER_OUT, OUTPUT);
  
  live_strand = alloc_strand_buffer();
  start_strand();         // initialize
  update_strand(live_strand);   // send data to the strand
    
  Serial.begin(57600);    // set up the serial port for debug
}


void loop() {
  uint8_t in1, in2;
  
  switch(strand_type) {
    case WS2801:  Serial.println("WS2801 Caroushell strand test");
                  break;
                  
    case SM16716: Serial.println("SM16716 Starfish strand test");
                  break;
                  
    default:      Serial.println("Bogus!");
                  break;
                  
  }

  Serial.println("Red wipe");
  colorWipe(Color(255,   0,   0), WIPE_WAIT);
  Serial.println("Green wipe");
  colorWipe(Color(  0, 255,   0), WIPE_WAIT);
  Serial.println("Blue wipe");
  colorWipe(Color(  0,   0, 255), WIPE_WAIT);
  Serial.println("White wipe");
  colorWipe(Color(255, 255, 255), WIPE_WAIT);
  Serial.println("Waiting 2 seconds at max brightness");
  delay(2000);  // let it cook at maximum brightness for a bit
  Serial.println("Black wipe");
  colorWipe(Color(  0,   0,   0), WIPE_WAIT);


    
}

// fill the dots one after the other with said color
// good for testing purposes
void colorWipe(uint32_t c, uint8_t wait) {
  int i;
    
  for (i=0; i < LEDS_PER_WS2801_STRAND; i++) {
      setPixelColor(live_strand, i, c);
      update_strand(live_strand);
      delay(wait);
  }
}

/* Helper functions */

// Create a 24 bit color value from R,G,B
uint32_t Color(byte r, byte g, byte b)
{
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}
