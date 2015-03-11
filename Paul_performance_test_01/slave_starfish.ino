#include <EEPROM.h>
#include "SPI.h"
#include <digitalWriteFast.h>

/*****************************************************************************
Caroushell-style slave processor for Starfish-style LED strands

based on Electrical Test for CarouShell tower LEDs, slave processors

Connect pixel strands to the SPI pins: 11 and 13
Connect RS-485 adapter to RX and TX

Interprets single-byte command codes as follows:

R    shifts in red pixels
G    shifts in green pixels
B    shifts in blue pixels
W    shifts in white pixels
O    shifts in black pixels
X    rainbow
Y    rainbow

Each shift-in completes before additional commands are accepted.

*****************************************************************************/

#define  DATA_PIN    11
#define  CLOCK_PIN   13
#define  DEBUG1_PIN  4
#define  DEBUG2_PIN  7

#define  LEDS_PER_STRAND  54    // three Starfish strands daisy-chained
//#define  LEDS_PER_STRAND  108    // 6 Starfish strands daisy-chained
//#define  LEDS_PER_STRAND  162    // 9 Starfish strands daisy-chained

uint8_t *live_strand;
uint8_t *backing_strand;

uint8_t my_address_bit = 0x01;

#define  TICK_LENGTH  10         // milliseconds per tick in a local sequence
uint8_t current_sequence = 0;    // 0 means no sequence is active

uint8_t *alloc_strand_buffer() {
  return (uint8_t *)calloc(LEDS_PER_STRAND,3);
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
  int i;
  
  digitalWrite(DEBUG1_PIN, HIGH);
  
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

  SPCR &= ~_BV(SPE);  // the useful part of SPI.end();
  
  digitalWriteFast2(DATA_PIN, 0);
  digitalWriteFast2(CLOCK_PIN, HIGH); digitalWriteFast2(CLOCK_PIN, LOW);
  digitalWriteFast2(CLOCK_PIN, HIGH); digitalWriteFast2(CLOCK_PIN, LOW);
  
  for (i=0; i < LEDS_PER_STRAND*3; i+=3) {

    // start bit
    digitalWriteFast2(DATA_PIN, 1);
    digitalWriteFast2(CLOCK_PIN, HIGH); digitalWriteFast2(CLOCK_PIN, LOW);
  
    SPCR |= _BV(SPE);  // the useful part of SPI.begin();  
    // send the pixel values
    SPDR = pixels[i];
    while (!(SPSR & (1<<SPIF)));
    SPDR = pixels[i+1];
    while (!(SPSR & (1<<SPIF)));
    SPDR = pixels[i+2];
    while (!(SPSR & (1<<SPIF)));
    SPCR &= ~_BV(SPE);  // the useful part of SPI.end();
  }
  
  // send an extra blank pixel
    // start bit
  digitalWriteFast2(DATA_PIN, 1);
  digitalWriteFast2(CLOCK_PIN, HIGH); digitalWriteFast2(CLOCK_PIN, LOW);
  SPCR |= _BV(SPE);  // the useful part of SPI.begin();  
  SPDR = 0;
  while (!(SPSR & (1<<SPIF)));
  SPDR = 0;
  while (!(SPSR & (1<<SPIF)));
  SPDR = 0;
  while (!(SPSR & (1<<SPIF)));
  SPI.end();

  
  //delay(1);  // do we need to delay here?
  
  digitalWrite(DEBUG1_PIN, LOW);

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

#define BUFFER_LEN  30
#define MIN_PACKET  3     // anything shorter than this is invalid

#define  CMD_FIELD_COMMAND 0
#define  CMD_FIELD_ADDMAP  1
#define  CMD_FIELD_DATA    2

#define  CMD_TEST  'A'
#define  CMD_FILL  'F'
#define  CMD_SHIFT_UP  'U'
#define  CMD_SHIFT_DOWN  'D'
#define  CMD_ROTATE  'R'
#define  CMD_RANDOMIZE  'X'
#define  CMD_LOADONE  'L'
#define  CMD_SEQUENCE  'S'

// States of the receive state machine
#define	IDLE	0			// we're waiting for an initial flag
#define	RECV	1			// putting bytes into the buffer
#define	STUF	2			// next byte is a stuffed byte

// Special codes for byte stuffing. Per KISS or SLIP protocol standards.
#define	FEND	0xC0
#define FESC	0xDB
#define TFEND	0xDC
#define	TFESC	0xDD

byte state = IDLE;
byte buffer[BUFFER_LEN];
byte count = 0;

void add_to_buffer(byte in) {  
  buffer[count] = in;
  count++;
  if (count >= BUFFER_LEN)              // overflow!
    state = IDLE;
}


void handle_serial_byte(byte bytein) {
  
  switch (state) {
    case	IDLE:
      if (bytein == FEND) {
        state = RECV;
	count = 0;
      }
      break;

    case 	RECV:
      if ((bytein == FEND) && (count == 0)) {
      } else if (bytein == FEND) {         // possible end of packet
        handle_packet();
        state = IDLE;
      } else if (bytein == FESC)      // byte de-stuffing to happen next
        state = STUF;
      else                          // ordinary data byte
        add_to_buffer(bytein);

      break;

    case	STUF:
      if (bytein == TFESC) {
        add_to_buffer(FESC);
        state = RECV;
      } else if (bytein == TFEND) {
        add_to_buffer(FEND);
        state = RECV;
      } else                          // byte stuffing error
        state = IDLE;
      break;

    default:  // "can't happen"
      state = IDLE;
      break;
  }
}

void handle_packet() {
  uint8_t command;
  uint8_t n, r, g, b;
  uint8_t p1, p2, p3;
  int i;
  
  if (count < MIN_PACKET)
    return;
        
  if ((buffer[CMD_FIELD_ADDMAP] & my_address_bit) == 0)
    return;
    
  switch(buffer[CMD_FIELD_COMMAND]) {
    case CMD_TEST:
  
      if (count != 3)
        return;
      command = buffer[CMD_FIELD_DATA];

      if (command == 'r')
        colorWipe(Color(255,   0,   0), LEDS_PER_STRAND);
      else if (command == 'g')
        colorWipe(Color(  0, 255,   0), LEDS_PER_STRAND);
      else if (command == 'b')
        colorWipe(Color(  0,   0, 255), LEDS_PER_STRAND);
      else if (command == 'w')
        colorWipe(Color(255, 255, 255), LEDS_PER_STRAND);
      else if (command == 'o')
        colorWipe(Color(  0,   0,   0), LEDS_PER_STRAND);
      else if (command == 'x')
        rainbow(20);
      else if (command == 'y')
        rainbowCycle(20);
      else if (command == 'f')
        fastblink(0);
      else if (command == 's')
        fastblink(20);
      
      break;
      
    case CMD_FILL:
      if (count != 5)
        return;
      r = buffer[CMD_FIELD_DATA + 0];
      g = buffer[CMD_FIELD_DATA + 1];
      b = buffer[CMD_FIELD_DATA + 2];
      for (i=0; i < LEDS_PER_STRAND; i++)
        setPixelRGB(live_strand, i, r, g, b);
      update_strand(live_strand);
      break;
      
    case CMD_SHIFT_UP:      // shift colors up, filling with a constant
      if (count != 6)
        return;
      n = buffer[CMD_FIELD_DATA + 0];
      r = buffer[CMD_FIELD_DATA + 1];
      g = buffer[CMD_FIELD_DATA + 2];
      b = buffer[CMD_FIELD_DATA + 3];
      memmove(live_strand+n*3, live_strand, (LEDS_PER_STRAND-n)*3);
      for (i=0; i < n; i++)
        setPixelRGB(live_strand, i, r, g, b);
      update_strand(live_strand);
      break;

    case CMD_SHIFT_DOWN:    // shift colors down, filling with a constant    
      if (count != 6)
        return;
      n = buffer[CMD_FIELD_DATA + 0];
      r = buffer[CMD_FIELD_DATA + 1];
      g = buffer[CMD_FIELD_DATA + 2];
      b = buffer[CMD_FIELD_DATA + 3];
      memmove(live_strand, live_strand+n*3, (LEDS_PER_STRAND-n)*3);
      for (i=LEDS_PER_STRAND-n; i < LEDS_PER_STRAND; i++)
        setPixelRGB(live_strand, i, r, g, b);
      update_strand(live_strand);
      break;
      
    case CMD_ROTATE:        // rotate colors, wrapping around
      if (count != 3)
        return;
      n = buffer[CMD_FIELD_DATA + 0];
      rotate_by_n(n);
      update_strand(live_strand);
      break;
    
    case CMD_RANDOMIZE:      // fill with random colors from the wheel
      if (count != 3)
        return;
      for (i=0; i < LEDS_PER_STRAND; i++)
        setPixelColor(live_strand, i, Wheel(random(256)));
      update_strand(live_strand);
      break;
    
    case CMD_LOADONE:        // load a single pixel with a given color
      if (count != 6)
        return;
      n = buffer[CMD_FIELD_DATA + 0];
      r = buffer[CMD_FIELD_DATA + 1];
      g = buffer[CMD_FIELD_DATA + 2];
      b = buffer[CMD_FIELD_DATA + 3];
      setPixelRGB(live_strand, n, r, g, b);
      update_strand(live_strand);
      break;
      
    case CMD_SEQUENCE:      // begin a preprogrammed sequence animation
      if (count != 6) {
        current_sequence = 0;
        return;
      }
      n = buffer[CMD_FIELD_DATA + 0];  // sequence index; 0 means stop
      p1 = buffer[CMD_FIELD_DATA + 1];  // sequence-specific parameters
      p2 = buffer[CMD_FIELD_DATA + 2];
      p3 = buffer[CMD_FIELD_DATA + 3];
      initialize_sequence(n, p1, p2, p3);
      break;
  
    default:
      break;
  }
}

void setup() {
  randomSeed(analogRead(0));
  
  pinMode(DEBUG1_PIN, OUTPUT);
  pinMode(DEBUG2_PIN, OUTPUT);
  digitalWrite(DEBUG1_PIN, LOW);
  digitalWrite(DEBUG2_PIN, LOW);
  
  live_strand = alloc_strand_buffer();
  backing_strand = alloc_strand_buffer();
  start_strand();         // initialize
  update_strand(live_strand);   // send data to the strand
  
  my_address_bit = EEPROM.read(0);    // get my unique address from EEPROM
  if (my_address_bit == 0xFF)         // default value in EEPROM!
    my_address_bit = 0x01;            // use a reasonable default instead
  
  Serial.begin(9600);    // set up the serial port for network communications
}


void loop() {
  
  static unsigned long last_tick_millis = 0;
  unsigned long t;
  
  if (Serial.available() > 0) {
    handle_serial_byte(Serial.read());
  }
  
  if (current_sequence != 0) {
    t = millis();
    if (t - last_tick_millis >= TICK_LENGTH) {
      last_tick_millis = t;
      sequence_tick();
    }
  }
    
}

void fastblink(uint8_t wait) {
  int count;
  int i;
  
  for (count = 0; count < 100; count++) {
    for (i = 0; i < LEDS_PER_STRAND; i++) {
      setPixelRGB(live_strand, i, 255,255,255);
    }
    update_strand(live_strand);
    if (wait)
      delay(wait);
    for (i = 0; i < LEDS_PER_STRAND; i++) {
      setPixelRGB(live_strand, i, 0,0,0);
    }
    update_strand(live_strand);
    if (wait)
      delay(wait);
  }
}

void rainbow(uint8_t wait) {
  int i, j;
   
  for (j=0; j < 256; j++) {     // 3 cycles of all 256 colors in the wheel
    for (i=0; i < LEDS_PER_STRAND; i++) {
      setPixelColor(live_strand, i, Wheel( (i + j) % 255));
    }
    update_strand(live_strand);
    delay(wait);
  }
}

// Slightly different, this one makes the rainbow wheel equally distributed 
// along the chain
void rainbowCycle(uint8_t wait) {
  int i, j;
  
  for (j=0; j < 256 * 2; j++) {     // 2 cycles of all 25 colors in the wheel
    for (i=0; i < LEDS_PER_STRAND; i++) {
      // tricky math! we use each pixel as a fraction of the full 96-color wheel
      // (thats the i / strip.numPixels() part)
      // Then add in j which makes the colors go around per pixel
      // the % 96 is to make the wheel cycle around
      setPixelColor(live_strand, i, Wheel( ((i * 256 / LEDS_PER_STRAND) + j) % 256) );
    }
    update_strand(live_strand);
    delay(wait);
  }
}

// fill the dots one after the other with said color
// good for testing purposes
void colorWipe(uint32_t c, uint8_t wait) {
  int i;
  
  for (i=0; i < LEDS_PER_STRAND; i++) {
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

//Input a value 0 to 255 to get a color value.
//The colours are a transition r - g -b - back to r
uint32_t Wheel(byte WheelPos)
{
  if (WheelPos < 85) {
   return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
   WheelPos -= 85;
   return Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170; 
   return Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

void rotate_by_n(uint8_t n) {
  uint8_t i, r, g, b;

  for (i=0; i < n; i++) {
    r = live_strand[LEDS_PER_STRAND*3-3];
    g = live_strand[LEDS_PER_STRAND*3-2];
    b = live_strand[LEDS_PER_STRAND*3-1];
    memmove(live_strand+3, live_strand, (LEDS_PER_STRAND-1)*3);
    live_strand[0] = r;
    live_strand[1] = g;
    live_strand[2] = b;
  }
}

#define  SEQ_ROTATE  1
#define  SEQ_SPARKLE  2
#define  SEQ_RAINBOW  3

unsigned long seq_prev_t;
uint8_t seq_ticks;
uint8_t seq_rate;
uint8_t seq_step;
uint8_t seq_pos;
uint8_t seq_save_r, seq_save_g, seq_save_b;

void initialize_sequence(uint8_t sequence, uint8_t p1, uint8_t p2, uint8_t p3) {
  if (current_sequence != 0) {
    // clean up the old sequence if necessary
  }
  
  current_sequence = sequence;
  switch(sequence) {
    case  SEQ_ROTATE:
      seq_ticks = 0;
      seq_rate = p1;
      seq_step = p2;
      break;
      
    case SEQ_SPARKLE:
      seq_ticks = 0;
      seq_rate = p1;
      break;
    
    case SEQ_RAINBOW:
      seq_ticks = 0;
      seq_rate = p1;
      seq_pos = 0;
      break;
    
    default:
      current_sequence = 0;    // invalid, treat as no sequence
      break;
  }
}

void sequence_tick(void) {
  int i;
  
  switch(current_sequence) {
    case  SEQ_ROTATE:
      seq_ticks++;
      if (seq_ticks >= seq_rate) {
        rotate_by_n(seq_step);
        update_strand(live_strand);
        seq_ticks = 0;
      }
      break;
    
    case SEQ_SPARKLE:
      seq_ticks++;
      if (seq_ticks == seq_rate) {
        seq_pos = random(LEDS_PER_STRAND);    // which LED will sparkle this time
        seq_save_r = live_strand[seq_pos * 3];
        seq_save_g = live_strand[seq_pos * 3 + 1];
        seq_save_b = live_strand[seq_pos * 3 + 2];
        live_strand[seq_pos * 3    ] = 255;           // override with white
        live_strand[seq_pos * 3 + 1] = 255;
        live_strand[seq_pos * 3 + 2] = 255;
        update_strand(live_strand);
      } else if (seq_ticks >= seq_rate + 1) {
        live_strand[seq_pos * 3    ] = seq_save_r;    // put it back
        live_strand[seq_pos * 3 + 1] = seq_save_g;
        live_strand[seq_pos * 3 + 2] = seq_save_b;
        update_strand(live_strand);
        seq_ticks = 0;
      }
      break;
    
    case SEQ_RAINBOW:
      seq_ticks++;
      if (seq_ticks >= seq_rate) {
        seq_pos++;    // wraps
        for (i=0; i < LEDS_PER_STRAND; i++) {
          // tricky math! we use each pixel as a fraction of the full 96-color wheel
          // (thats the i / strip.numPixels() part)
          // Then add in j which makes the colors go around per pixel
          // the % 96 is to make the wheel cycle around
          setPixelColor(live_strand, i, Wheel( (((int)i * 256 / LEDS_PER_STRAND) + (int)seq_pos) % 256) );
        }
        update_strand(live_strand);
        seq_ticks = 0;
      }
      break;
        
    
    default:
      current_sequence = 0;    // invalid, treat as no sequence
      break;
  }
}

