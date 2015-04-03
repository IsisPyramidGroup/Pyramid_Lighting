#include <EEPROM.h>
#include <digitalWriteFast.h>
#include <SPI.h>

/***************************************************************************
Isis Pyramid 1.1 slave processor (single load for both types of strands)

For Diavolino (Arduino clone).
Program with board = Duemilanove, processor = ATmega328

Connect pixel strands to the SPI pins: 11 and 13
Connect RS-485 adapter to RX (TX not used)

Entity-based addressing:
Most visual operations are based on logical "entities", which are runs of pixels
that look like a single thing visually, but may not correspond to a physical strand
of LED pixels. Entities on the pyramid are the eight diagonals (two at each corner of
the pyramid, one on either side of the supporting beam) and the four bottom edges.
Each entity may be divided into multiple segments, each of which is a contiguous
run of pixels on a single LED strand, and thus controlled by a single slave controller.
As it happens, in the pyramid every entity has two active segments. It's also possible
for segments of an entity to be unimplemented. In the pyramid, the doorway in the
front face interrupts the front edge, so there are no physical pixels corresponding
to that part of the front edge entity.

A slave controller can handle up to MAX_SEGMENTS segments of entities. In the
pyramid, there are 8 controllers (0 - 7) that handle a single segment (the upper part
of a diagonal) and 8 other controllers (8- 15) that handle two segments (the lower part
of a diagonal and half of a bottom edge).

The master will mostly send out commands to entities. Each slave will compare
these commands to its own table of segments, and ignore commands that don't apply
to any of its segments. For commands that do apply, the slave will generally
update local data structures, including a RAM buffer for the RGB+D contents of the
entire entity. No actual LED pixels are updated directly based on entity commands.

All entity command packets have an effective time, a repeat count, and a repeat interval.
They can be sent for immediate one-shot effect by setting the effective time to 0
and the repeat count to 1; such packets are not stored. If the effective time is
in the future, or if the repeat count is greater than 1, the entire packet is stored
on the deferred queue. The deferred queue is scanned every tick, and any packet
that has a past or present effective time is executed, its repeat count decremented,
and the effective time updated according to the repeat interval.

Every tick (10ms), each configured segment is copied from the entity buffer to
the physical strand buffer, according to the mapping defined by the segment table.
Then, the physical strand is updated from the strand buffer.

While the physical strand is being updated, additional effects called "dynamics"
may be computed and applied to the RGB values being sent out. These computations are
done in a pipeline with the actual shifting-out of data, so they don't cost much CPU
time. Dynamics computations are strictly local to each pixel. Each pixel has an extra
"D" byte (besides the bytes for red, green, and blue) to control the dynamic effect.

****************************************************************************/

// Arduino pin assignments
#define  DATA_PIN    11
#define  CLOCK_PIN   13
#define  DEBUG1_PIN  4
#define  DEBUG2_PIN  7

// Packet command codes for slave commands
#define  CMD_S_RESET_CLOCK  0x00
#define  CMD_S_DYN_BLINK    0x01
#define  CMD_S_DYN_THROB    0x02
#define  CMD_S_DYN_SPARKLE  0x03


// Packet command codes for entity commands
#define  CMD_E_FILL_RGB    0x40
#define  CMD_E_FILL_D      0x41
#define  CMD_E_SHIFT_UP    0x42
#define  CMD_E_SHIFT_DOWN  0x43
#define  CMD_E_ROTATE      0x44
#define  CMD_E_RANDOMIZE   0x45
#define  CMD_E_LOADONE     0x46
#define  CMD_E_RAINBOW     0x47


// Offsets into the packet format for parsing all packets.
#define  PKT_COMMAND_OFFSET          0
#define  PKT_ADDRESS_OFFSET          1    // uint16_t

// Offsets into the packet format for parsing slave packets.
#define  PKT_S_DATA_OFFSET            3    // any number of packet-specific arguments

// Offsets into the packet format for parsing entity packets.
// All entity packets share a common header.
#define  PKT_REPEAT_COUNT_OFFSET     3
#define  PKT_EFFECTIVE_TIME_OFFSET   4    // uint16_t
#define  PKT_REPEAT_INTERVAL_OFFSET  6    // uint16_t
#define  PKT_E_DATA_OFFSET           8    // any number of packet-specific arguments

#define  PACKET_MAX                 15    // number of bytes in longest valid packet
                                          // applies to slave or entity packets

// Deferred execution queue. For any packet that comes in with entity addressing
// (for one of our entities) that is not for one-shot immediate execution, we'll
// stick it into one of these QUEUE_MAX buffers. Then every tick we'll look for
// any buffers that are due to be processed.
// (No, this is not really implemented as a queue, but that's how I think of it.)
#define  QUEUE_MAX  10
uint8_t deferred_queue[QUEUE_MAX][PACKET_MAX];


// Offsets into the strand buffer for each component of each pixel
#define  COMPONENT_0       0    // Red component
#define  COMPONENT_1       1    // Green component
#define  COMPONENT_2       2    // Blue component
#define  PIXEL_PARAMETERS  3    // D
#define  BYTES_PER_PIXEL   4    // Note: this can't be changed without other code changes.

// Two different LED strand types are supported. (One at a time!)
#define  STRAND_CAROUSHELL  0   // based on WS2801 chip in each pixel
#define  STRAND_STARFISH    1   // based on SM16716 chip in each pixel

#define  TICK_LENGTH    10      // milliseconds per tick
uint16_t current_tick = 0;      // counter of ticks processed since powerup or origin reset

// Time-based commands are processed relative to this time origin,
// which can be reset by command.
unsigned long time_origin = 0;

// Slave configuration info read from EEPROM
uint8_t slave_address;
uint16_t config_version;
uint8_t strand_type;
uint8_t pixels_in_strand;
uint8_t num_segments;
#define  MAX_SEGMENTS  4
struct segment {
  uint8_t    address;
  uint8_t    entity_index;      // index into local entity table
  uint8_t    pixels_in_entity;
  uint8_t    first_entity_index;
  uint8_t    pixels_in_segment;
  uint8_t    first_strand_index;
  uint8_t    reverse_index;
 };
struct segment segments[MAX_SEGMENTS];

// Slave configuration derived from EEPROM fields
uint16_t slave_address_bitmap;
uint16_t entity_address_bitmap;


// Entity table
#define  MAX_ENTITIES  4
uint8_t  num_entities;
struct entity {
  uint8_t  address;
  uint8_t  pixel_count;
  uint8_t  *buffer;
};
struct entity entities[MAX_ENTITIES];

// This is the local RAM buffer for the contents of the physical LED strand
uint8_t *strand;

// Initialize the hardware for talking to our strand of pixels.
void start_strand(void) {
  if (strand_type == STRAND_CAROUSHELL) {
    start_caroushell_strand();
  }
  else if (strand_type == STRAND_STARFISH) {
    start_starfish_strand();
  }
  else {  // not supposed to happen
  // do nothing
  }
}

void start_caroushell_strand(void) {
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV16); // 1 MHz max, else flicker supposedly
}

void start_starfish_strand(void) {
  
  // Starfish initialization is a little more elaborate, because we have to
  // send some bits out manually instead of always using the SPI hardware.
  // So we'll set up the pins for manual output, and then switch them back
  // and forth with SPI.begin() and SPI.end() -- or a subset thereof.
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


// "Dynamics" here are pixel-local calculations that are carried out in a
// pipeline with the actual shifting-out of the RGB data to the LED strands.
// Dynamics do not involve changing the pixel values stored in the buffers.
//
// There are three opportunities to calculate:
//   1. dynamics_precalc_tick runs once per tick for pre-calculations.
//   2. dynamics_precalc_pixel gets a pointer to the entire pixel (RGB+D)
//      and does any necessary setup and/or calculations that involve more
//      than one component.
//   3. dynamics_calc_component receives the nominal R, G, or B value from the
//      stored pixel and returns the modified value to be sent to the LED strand.
//
// Dynamics are enabled by bits in the D byte of each pixel, so up to 8 types
// of dynamics are possible. For each type, there can be parameters that
// modify how the dynamic effect is implemented, but the parameters are global
// to every pixel on this slave's strand. They are NOT entity based. The
// parameters are set by slave-addressed command packets, so they do not have
// effective times or repeat counts.
//
// It is possible for multiple dynamic effects to be enabled for any pixel.
// The effects are applied in the order shown here, which is intended to
// preserve all the visual effects to some extent, but exact interactions
// between the effects are left unspecified.
//
// The following types of dynamics are defined:
#define DYNAMICS_BLINK    0x01    // hard on/dim at some duty cycle and rate
#define DYNAMICS_THROB    0x02    // soft brightness modulation at some duty cycle and rate
#define DYNAMICS_SPARKLE  0x04    // full-on for one tick with probability P
//etc. TBD


// These are the parameters for all of the dynamic effects. They are global
// to the entire slave, not entity-specific. They must be set before use by
// sending the corresponding configuration packet. They are given default
// values here just for safety.
uint16_t  dynamics_blink_period = 100;    // period in ticks of the blink effect
uint16_t  dynamics_blink_ontime =  50;    // how many ticks of each period the pixel is normal brightness
uint8_t   dynamics_blink_dimming = 255;   // how much to dim the pixel (in LED physical units)

uint16_t  dynamics_throb_period = 100;    // period in ticks of the throb effect
uint16_t  dynamics_throb_ramptime = 10;   // how many ticks it takes to ramp up or down from normal
uint8_t   dynamics_throb_bright = 10;     // how much to modify the brightness at ramp peaks
uint8_t   dynamics_throb_dim = 10;        // how much to modify the brightness at ramp troughs

uint16_t  dynamics_sparkle_probability;   // probability of a sparkle, per tick, times 0xFFFF

// These values are passed from precalc to calc_component:
uint8_t   dynamics_bitmap;                // bitmap of effects enabled for this pixel
int16_t   dynamics_blink_value;
int16_t   dynamics_throb_value;
bool      dynamics_sparkle_now;

// Once-per-tick precalculations for dynamic effects.
void dynamics_precalc_tick(void) {
  uint16_t time_in_cycle;          // where are we in the cycle
  
  // BLINK
  // If the BLINK bit is set, the pixel will show its buffered value for
  // part of the time, and be dimmer by a certain amount for part of the time.
  // Each cycle has duration dynamics_blink_period (in ticks), and the ON time
  // begins when current_tick is a multiple of that value.
  // The duration of the ON time is dynamics_blink_ontime (in ticks).
  if ((current_tick % dynamics_blink_period) < dynamics_blink_ontime) {
    dynamics_blink_value = 0;  // that is, the pixel is displayed normally
  } else {
    dynamics_blink_value = -dynamics_blink_dimming;
  }

  // THROB
  // If the THROB bit is set, the pixel's brightness will ramp up from its
  // buffered value to a certain amount brighter over a certain period of
  // time, then back down to nominal over the same period, then continue
  // down to a certain amount dimmer over the same period, then back up to
  // nominal over the same period. Then the pixel will stay at nominal until
  // the cycle repeats.
  time_in_cycle = current_tick % dynamics_throb_period;
  if (time_in_cycle < dynamics_throb_ramptime) {
    dynamics_throb_value = dynamics_throb_bright * time_in_cycle / dynamics_throb_ramptime;
  }
  else if (time_in_cycle < 2 * dynamics_throb_ramptime) {
    dynamics_throb_value = dynamics_throb_bright * (dynamics_throb_ramptime*2 - time_in_cycle) / dynamics_throb_ramptime;
  }
  else if (time_in_cycle < 3 * dynamics_throb_ramptime) {
    dynamics_throb_value = - (int16_t) (dynamics_throb_dim * (time_in_cycle - 2*dynamics_throb_ramptime) / dynamics_throb_ramptime);
  }
  else if (time_in_cycle < 4 * dynamics_throb_ramptime) {
    dynamics_throb_value = - (int16_t) (dynamics_throb_dim * (dynamics_throb_ramptime*4 - time_in_cycle) / dynamics_throb_ramptime);
  }
  else {
    dynamics_throb_value = 0;
  }
  
  // SPARKLE
  // no tick precalcs for SPARKLE
  
}


// Once-per-pixel precalculations for dynamic effects
void dynamics_precalc_pixel(uint8_t *pixel) {  
  dynamics_bitmap = pixel[PIXEL_PARAMETERS];
  
  if (dynamics_bitmap & DYNAMICS_BLINK) {
    // no pixel precalc for BLINK
  }
  
  if (dynamics_bitmap & DYNAMICS_THROB) {
    // no pixel precalc for THROB
  }
  
  // If the SPARKLE bit is set, the pixel's brightness will be normal
  // except that, with a certain probability, it will be slammed to white
  // for one tick.
  if (dynamics_bitmap & DYNAMICS_SPARKLE) {
    if (random(0xFFFF) < dynamics_sparkle_probability) {
      dynamics_sparkle_now = 1;
    }
    else {
      dynamics_sparkle_now = 0;
    }
  }
  
}

// Component (R, G, or B) by component calculations for dynamic effects
uint8_t dynamics_calc_component(uint8_t component) {
  
  if (dynamics_bitmap & DYNAMICS_BLINK) {
    component = add_with_limit(component, dynamics_blink_value);
  }
  
  if (dynamics_bitmap & DYNAMICS_THROB) {
    component = add_with_limit(component, dynamics_throb_value);
  }
  
  if (dynamics_bitmap & DYNAMICS_SPARKLE) {
    if (dynamics_sparkle_now) {
      component = 255;
    }    
  }
  
  return component;
}

// Add a signed adjustment to a pixel component value,
// limiting to 0-255.
uint8_t add_with_limit(uint8_t in, int16_t adj) {
  int16_t result = (int16_t)in + adj;
  
  if (result < 0) {
    return 0;
  } else if (result > 255) {
    return 255;
  } else {
    return (uint8_t) result;
  }
}


//*********************************************************************************** 

void update_strand(uint8_t *pixels) {
  if (strand_type == STRAND_CAROUSHELL) {
    update_caroushell_strand(pixels);
  }
  else if (strand_type == STRAND_STARFISH) {
    update_starfish_strand(pixels);
  }
  else {  // not supposed to happen
  // do nothing
  }
}

// This routine transmits the current color component information out to the
// physical strand of LEDs. Since it takes some time (about 8 microseconds)
// for each byte to be shifted out, we use some of that time to compute the value
// needed for the next byte going out. If this calculations runs a little
// overtime, that's OK, the LEDs are happy to wait.
//
// The computation we do here in the pipeline is a simple one applied to each
// color component separately, parameterized by the parameters byte of each
// pixel in the buffer.

void update_starfish_strand(uint8_t *pixels) {
  int i;
  uint8_t pipelined_byte;

  digitalWrite(DEBUG1_PIN, HIGH);
  
  // 50-bit block of zeros before each update of pixels
  // We send 48 as six octets of zero, then two manually.
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
  
  // send the last two zero bits manually
  digitalWriteFast2(DATA_PIN, 0);
  digitalWriteFast2(CLOCK_PIN, HIGH); digitalWriteFast2(CLOCK_PIN, LOW);
  digitalWriteFast2(CLOCK_PIN, HIGH); digitalWriteFast2(CLOCK_PIN, LOW);
  
  // prime the pipeline for component dynamics calculations
  dynamics_precalc_pixel(pixels+0);                  // point to entire pixel
  pipelined_byte = dynamics_calc_component(pixels[0+COMPONENT_0]);    // pass in individual component
  for (i=0; i < pixels_in_strand*BYTES_PER_PIXEL; i+=BYTES_PER_PIXEL) {

    // start bit (sent manually)
    digitalWriteFast2(DATA_PIN, 1);
    digitalWriteFast2(CLOCK_PIN, HIGH); digitalWriteFast2(CLOCK_PIN, LOW);

    // send the pixel values using SPI hardware  
    SPCR |= _BV(SPE);  // the useful part of SPI.begin();  
    SPDR = pipelined_byte;
    pipelined_byte = dynamics_calc_component(pixels[i+COMPONENT_1]);
    while (!(SPSR & (1<<SPIF)));
    SPDR = pipelined_byte;
    pipelined_byte = dynamics_calc_component(pixels[i+COMPONENT_2]);
    while (!(SPSR & (1<<SPIF)));
    SPDR = pipelined_byte;
    // note: on the last time through, the following pipelined calculations address
    // a dummy pixel at the end of the buffer. They're wasted but harmless.
    dynamics_precalc_pixel(pixels+i+BYTES_PER_PIXEL);    // point to entire pixel
    pipelined_byte = dynamics_calc_component(pixels[i+BYTES_PER_PIXEL + COMPONENT_0]);
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
  
  digitalWrite(DEBUG1_PIN, LOW);

}

void update_caroushell_strand(uint8_t *pixels) {
  uint8_t pipelined_byte;
  int i;
  
  static unsigned long earliest_update = 0;
    
  // Each update of the strand has to end with a 1ms pause. Rather than simply delay(1)
  // at the end of each transaction, we write down the value of the micros() counter, and
  // make sure it has advanced by at least 1000 (1ms) before starting again. Unfortunately,
  // the micros() counter can wrap around, and so can the calculation of earliest_update.
  //
  // If micros() and earliest_update are both small or both large, we can just compare
  // and get the right answer. If earliest_update is small but micros() is large, we
  // need to wait for micros() to wrap around and then start comparing. If earliest_update
  // is large but micros() is small, we can proceed because the time has already elapsed.
  
  if ((earliest_update > 0xF0000000UL) && (micros() < 100000UL)) {
    // micros() has wrapped and is well past earliest_update, so we can proceed.
  }
  else {
    if ((earliest_update < 100000UL) && (micros() > 0xF0000000UL)) {
    // earliest_update has wrapped past zero but micros() has not. Wait for micros() to wrap.
      while (micros() > 0xF000000UL) {
       //wait
      }
    }
    // Now we can rely on a straight comparison.
    while (micros() < earliest_update) {
     // wait
    }
  }
  
  // prime the pipeline
  dynamics_precalc_pixel(pixels+0);                  // point to entire 0th pixel
  pipelined_byte = dynamics_calc_component(pixels[0+COMPONENT_0]);    // pass in individual component
  
  for (i=0; i < pixels_in_strand*BYTES_PER_PIXEL; i+=BYTES_PER_PIXEL) {
    SPDR = pipelined_byte;
    pipelined_byte = dynamics_calc_component(pixels[i+COMPONENT_1]);
    while (!(SPSR & (1<<SPIF)));
    SPDR = pipelined_byte;
    pipelined_byte = dynamics_calc_component(pixels[i+COMPONENT_2]);
    while (!(SPSR & (1<<SPIF)));
    SPDR = pipelined_byte;
    // note: on the last time through, the following pipelined calculations address
    // a dummy pixel at the end of the buffer. They're wasted but harmless.
    dynamics_precalc_pixel(pixels+i+BYTES_PER_PIXEL);    // point to entire pixel
    pipelined_byte = dynamics_calc_component(pixels[i + BYTES_PER_PIXEL + COMPONENT_0]);
    while (!(SPSR & (1<<SPIF)));
  }
  
  // we need to impose a 1ms delay here, since that's how the LEDs latch their data.
  // Instead of delay(1), we'll take note of the earliest time we can update again.
  earliest_update = micros() + 1000;  // delay 1ms. (Value can wrap!)
}

/* Non-pipelined version
  
void update_caroushell_strand(uint8_t *pixels) {
  uint8_t i;
  
  for (i=0; i < pixels_in_strand*3; i++) {
    SPDR = pixels[i];
    while (!(SPSR & (1<<SPIF)));
  }
  delay(1);   // Data is latched by holding clock pin low for 1ms
}

*/

// Copy the relevant parts of the entity buffers into the strand buffers
void refresh_segments(void) {
  uint8_t segment;
  uint8_t entity;
  uint8_t *src, *dst;
  uint8_t count;
  uint8_t decrement;
  uint8_t i, j;
  
  for (segment = 0; segment < num_segments; segment++) {
    entity = segment[segments].entity_index;
    src = entities[entity].buffer + segment[segments].first_entity_index*BYTES_PER_PIXEL;
    count = segments[segment].pixels_in_segment;
    dst = strand + segment[segments].first_strand_index*BYTES_PER_PIXEL;
    decrement = segments[segment].reverse_index ? 2*BYTES_PER_PIXEL : 0;
    
    for (i=0; i < count; i++) {
      for (j=0; i < BYTES_PER_PIXEL; j++) {
        *dst++ = *src++;
      }
      dst -= decrement;      
    }
  }
}

// Color utility routines

// Set pixel color from separate 8-bit R, G, B components:
void setPixelRGB(uint8_t *strand, uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
  uint8_t *p = strand + (n*BYTES_PER_PIXEL);
  
  *p++ = r;
  *p++ = g;
  *p   = b;
  // pixel parameters are left alone
}

// Set pixel color from 'packed' 32-bit RGB value:
void setPixelColor(uint8_t *strand, uint16_t n, uint32_t c) {
  uint8_t *p = strand + (n*BYTES_PER_PIXEL);
  *p++ = c >> 16;   // Red
  *p++ = c >>  8;   // Green
  *p   = c;         // Blue
  // pixel parameters are left alone
}

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

//************************************************************************************
// Packet Input Handling

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
byte rcv_buffer[PACKET_MAX];
byte rcv_count = 0;

// simply add a received byte to the serial receive buffer
void add_to_rcv_buffer(byte in) {  
  rcv_buffer[rcv_count] = in;
  rcv_count++;
  if (rcv_count >= PACKET_MAX)              // overflow!
    state = IDLE;
}

// Handle a single received byte.
// The stream of bytes is broken up into packets according to the rules
// for KISS or SLIP framing, by wrapping the packet in flag bytes. The
// encoding is made transparent to binary data by escaping any flag bytes
// that may occur in the data stream. See RFC 1055.
void handle_serial_byte(byte bytein) {
  
  switch (state) {
    case	IDLE:
      if (bytein == FEND) {
        state = RECV;
	rcv_count = 0;
      }
      break;

    case 	RECV:
      if ((bytein == FEND) && (rcv_count == 0)) {
      } else if (bytein == FEND) {         // possible end of packet
        handle_packet();
        state = IDLE;
      } else if (bytein == FESC)      // byte de-stuffing to happen next
        state = STUF;
      else                          // ordinary data byte
        add_to_rcv_buffer(bytein);

      break;

    case	STUF:
      if (bytein == TFESC) {
        add_to_rcv_buffer(FESC);
        state = RECV;
      } else if (bytein == TFEND) {
        add_to_rcv_buffer(FEND);
        state = RECV;
      } else                          // byte stuffing error
        state = IDLE;
      break;

    default:  // "can't happen"
      state = IDLE;
      break;
  }
}

// A complete framed packet has been placed in the receive buffer.
// Check addressing type and see if it's for us.
//
void handle_packet(void) {
  bool entity_addressed = !!(rcv_buffer[PKT_COMMAND_OFFSET] & 0x80);
  uint16_t address = * (uint16_t *)(rcv_buffer + PKT_ADDRESS_OFFSET);
  
  if ((! entity_addressed) && ((address & slave_address_bitmap) != 0)) {
    handle_slave_packet();
  }
  
  if (entity_addressed && ((address & entity_address_bitmap) != 0)) {
    handle_entity_packet();
  }

}  

// A slave-addressed packet that's for us is in the receive buffer.
// Process it.
void handle_slave_packet(void) {
  uint8_t command_code = rcv_buffer[PKT_COMMAND_OFFSET] & 0x7F;
  uint8_t *p = rcv_buffer+PKT_S_DATA_OFFSET;
  
  switch(command_code) {
    case CMD_S_RESET_CLOCK:      // reset the origin of time for event synchronization
      time_origin = millis();
      current_tick = 0;
      clear_deferred_queue();
      break;
   
    case CMD_S_DYN_BLINK:        // set parameters for dynamic effect BLINK
      dynamics_blink_period = * (uint16_t *)p; p += 2;
      dynamics_blink_ontime = * (uint16_t *)p; p += 2;
      dynamics_blink_dimming = *p;
      
      // make sure we have safe values
      if (dynamics_blink_period < 2 || dynamics_blink_ontime >= dynamics_blink_period) {
        dynamics_blink_period = 100;
        dynamics_blink_ontime = 50;
      }
      break;
    
    case CMD_S_DYN_THROB:        // set parameters for dynamic effect THROB
      dynamics_throb_period   = * (uint16_t *)p; p += 2;
      dynamics_throb_ramptime = * (uint16_t *)p; p += 2;
      dynamics_throb_bright = *p++;
      dynamics_throb_dim    = *p;
      
      // make sure we have safe values
      if (dynamics_throb_period < 5 || 4*dynamics_throb_ramptime >= dynamics_throb_period) {
        dynamics_throb_period = 100;
        dynamics_throb_ramptime = 10;
      }
      break;
      
    case CMD_S_DYN_SPARKLE:     // set parameters for dynamic effect SPARKLE
      dynamics_sparkle_probability = * (uint16_t *)p;
      break;
    
    default:
      // don't know how to handle this packet. Shrug!
      break;
  }
  
}

// A valid entity-addressed packet for us is in the receive buffer.
// If it is for non-repeating immediate execution, do so -- this means that
// the designer doesn't have to worry that packets sent for immediate execution
// will fill up the queue.
// If it is for repeating or future execution, look for an empty spot in the queue
// and copy it there.
void handle_entity_packet(void) {
  uint8_t repeat_count = rcv_buffer[PKT_REPEAT_COUNT_OFFSET];
  uint16_t effective_time = * (uint16_t *)(rcv_buffer + PKT_EFFECTIVE_TIME_OFFSET);
  uint8_t i;

  if (repeat_count == 1 && (effective_time == 0 || effective_time <= current_tick)) {
  	execute_entity_packet(rcv_buffer);
  	return;
  }
  
  // Search for an empty spot in the queue
  for (i = 0; i < QUEUE_MAX; i++) {
    if (deferred_queue[i][PKT_REPEAT_COUNT_OFFSET] == 0) {
      // copy the new packet to this spot for later execution
      memcpy(deferred_queue[i], rcv_buffer, rcv_count);
      return;
    }
  }
  
  // No empty spot found. Overflow! Nothing much we can do about it, though.
}

// Check the deferred operations queue for any packets that are due to be executed.
// For each one we find, execute it and then update its schedule for the next execution,
// if any.
void scan_deferred_queue(void) {
  uint16_t effective_time;
  uint8_t  repeat_count;
  uint8_t i;
  
  for (i = 0; i < QUEUE_MAX; i++) {
    repeat_count = deferred_queue[i][PKT_REPEAT_COUNT_OFFSET];
    if (repeat_count != 0) {
      effective_time = * (uint16_t *)(deferred_queue[i] + PKT_EFFECTIVE_TIME_OFFSET);
      if (effective_time <= current_tick) {
        execute_entity_packet(deferred_queue[i]);
        repeat_count--;
        deferred_queue[i][PKT_REPEAT_COUNT_OFFSET] = repeat_count;
        if (repeat_count > 0) {
        effective_time += * (uint16_t *)(deferred_queue[i] + PKT_REPEAT_INTERVAL_OFFSET);
        * (uint16_t *) (deferred_queue[i] + PKT_EFFECTIVE_TIME_OFFSET) = effective_time;
        }
      }
    }
  }
}

// Clear out the deferred operations queue.
void clear_deferred_queue(void) {
  uint8_t i;
  
  for (i=0; i < QUEUE_MAX; i++) {
    deferred_queue[i][PKT_REPEAT_COUNT_OFFSET] = 0;    // set all repeat counts to zero
  }
}

// We have a packet in a buffer (either the receive buffer or one of the deferred
// execution queue buffers) that's due to be executed. Do it.
void execute_entity_packet(uint8_t *buf) {
  uint8_t entity;
  uint8_t addr;
  
  // For each entity that we are handling, check if this entity is being addressed.
  for (entity = 0; entity < num_entities; entity++) {
    addr = entities[entity].address;
    if ((buf[PKT_ADDRESS_OFFSET+1] & (1 << addr)) ||
        (buf[PKT_ADDRESS_OFFSET] & (1 << (addr-8)))) {
          execute_packet_for_entity(entity, buf);
    }
  }
  
  // Some packets need to be updated after all the entities have been processed.
  execute_packet_wrapup(buf);  
}

// A packet that's addressed to a specific entity is in the specified buffer,
// which may be the receive buffer or one of the buffers in the deferred queue.
// Process it.
void execute_packet_for_entity(uint8_t entity, uint8_t *buf) {
  uint8_t command_code = buf[PKT_COMMAND_OFFSET] & 0x7F;
  uint8_t  *p;                 // pointer into the entity's buffer
  uint8_t  pixel;              // counter for stepping through pixels
  uint8_t  r, g, b, d, n;      // data fields that may be in the packet
  uint8_t  dir_is_down;
  uint8_t  delay_in_ticks;
  uint8_t  color;
  uint8_t  start, incr, dir;
    
  // Now process the command on this entity
  switch (command_code) {
    case CMD_E_FILL_RGB:
      r = buf[PKT_E_DATA_OFFSET+0];
      g = buf[PKT_E_DATA_OFFSET+1];
      b = buf[PKT_E_DATA_OFFSET+2];
      
      p = entities[entity].buffer;
      for (pixel = 0; pixel < entities[entity].pixel_count; pixel++) {
        *p++ = r;
        *p++ = g;
        *p++ = b;
        p++;        // don't change the D byte of any pixel
      }
      break;
    
    case CMD_E_FILL_D:
      d = buf[PKT_E_DATA_OFFSET+0];
      
      p = entities[entity].buffer + PIXEL_PARAMETERS;
      for (pixel = 0; pixel < entities[entity].pixel_count; pixel++) {
        *p = d;
        p += BYTES_PER_PIXEL;
      }
      break;
      
    case CMD_E_SHIFT_UP:
      n = buf[PKT_E_DATA_OFFSET+0];
      r = buf[PKT_E_DATA_OFFSET+1];
      g = buf[PKT_E_DATA_OFFSET+2];
      b = buf[PKT_E_DATA_OFFSET+3];
      d = buf[PKT_E_DATA_OFFSET+4];
      
      if (n > entities[entity].pixel_count || n == 0) {    // sanity check
        return;    // can't process nonsense
      }
      
      p = entities[entity].buffer;
      // shift up
      memmove(p + n*BYTES_PER_PIXEL, p, (entities[entity].pixel_count-n)*BYTES_PER_PIXEL);
      // and fill vacated pixels with the specified constant
      for (pixel = 0; pixel < n; pixel++) {
        *p++ = r;
        *p++ = g;
        *p++ = b;
        *p++ = d;
      }
      break;
      
    case CMD_E_SHIFT_DOWN:
      n = buf[PKT_E_DATA_OFFSET+0];
      r = buf[PKT_E_DATA_OFFSET+1];
      g = buf[PKT_E_DATA_OFFSET+2];
      b = buf[PKT_E_DATA_OFFSET+3];
      d = buf[PKT_E_DATA_OFFSET+4];
      
      if (n > entities[entity].pixel_count || n == 0) {    // sanity check
        return;    // can't process nonsense
      }
      
      p = entities[entity].buffer;
      // shift down
      memmove(p, p + n*BYTES_PER_PIXEL, (entities[entity].pixel_count-n)*BYTES_PER_PIXEL);
      // and fill the vacated pixels with the specified constant
      p += n * BYTES_PER_PIXEL;
      for (pixel = 0; pixel < n; pixel++) {
        *p++ = r;
        *p++ = g;
        *p++ = b;
        *p++ = d;
      }
      break;
      
    case CMD_E_ROTATE:
      n = buf[PKT_E_DATA_OFFSET+0];
      dir_is_down = buf[PKT_E_DATA_OFFSET+1];

      if (n > entities[entity].pixel_count || n == 0) {    // sanity check
        return;    // can't process nonsense
      }

      if (dir_is_down) {
        rotate_down(entities[entity].buffer, entities[entity].pixel_count, n);
      } else {
        rotate_up(entities[entity].buffer, entities[entity].pixel_count, n);
      }
      break;
      
    case CMD_E_RANDOMIZE:
      // no arguments
      
      for (pixel = 0; pixel < entities[entity].pixel_count; pixel++) {
        setPixelColor(entities[entity].buffer, pixel, Wheel(random(256)));
      }
      break;

    case CMD_E_LOADONE:
      n = buf[PKT_E_DATA_OFFSET+0];
      r = buf[PKT_E_DATA_OFFSET+1];
      g = buf[PKT_E_DATA_OFFSET+2];
      b = buf[PKT_E_DATA_OFFSET+3];
      d = buf[PKT_E_DATA_OFFSET+4];
      
      p = entities[entity].buffer + n * BYTES_PER_PIXEL;
      *p++ = r;
      *p++ = g;
      *p++ = b;
      *p   = d;
      break;
    
    case CMD_E_RAINBOW:
      start = buf[PKT_E_DATA_OFFSET+0];    // index on 0-255 color wheel for first pixel
      incr  = buf[PKT_E_DATA_OFFSET+1];    // pixel-to-pixel increment on the 0-255 color wheel (wrap OK)
//    dir   = buf[PKT_E_DATA_OFFSET+2];    // 0 = increment start on repeat, 1 = decrement start on repeat
      
      for (color = start, pixel = 0; pixel < entities[entity].pixel_count; pixel++, color += incr) {
        setPixelColor(entities[entity].buffer, pixel, Wheel(color));
      }
      break;
      
    default:
      // don't know how to handle this packet. Shrug!
      break;
  }  
}

void execute_packet_wrapup(uint8_t *buf) {
  uint8_t command_code = buf[PKT_COMMAND_OFFSET] & 0x7F;
  uint8_t  start, incr, dir;

  switch (command_code) {
    case CMD_E_RAINBOW:
      start = buf[PKT_E_DATA_OFFSET+0];    // index on 0-255 color wheel for first pixel
      incr  = buf[PKT_E_DATA_OFFSET+1];    // pixel-to-pixel increment on the 0-255 color wheel (wrap OK)
      dir   = buf[PKT_E_DATA_OFFSET+2];    // 0 = increment start on repeat, 1 = decrement start on repeat

      if (dir) {
        start -= incr;
      }
      else {
        start += incr;
      }
      buf[PKT_E_DATA_OFFSET+0] = start;    // update start index to implement rainbow rotation
      break;
      
    default:
      // No wrapup required for this type of packet.
      break;
  }  
}


void rotate_up(uint8_t *p, uint8_t entity_size, uint8_t n) {
  uint8_t i;
  uint8_t r, g, b, d;
  uint8_t *last_pixel;
  
  for (i = 0; i < n; i++) {
    last_pixel = p + ((entity_size-1) * BYTES_PER_PIXEL);
    r = last_pixel[0];
    g = last_pixel[1];
    b = last_pixel[2];
    d = last_pixel[3];
    memmove(p+BYTES_PER_PIXEL, p, (entity_size-1)*BYTES_PER_PIXEL);
    p[0] = r;
    p[1] = g;
    p[2] = b;
    p[3] = d;
  }
}

void rotate_down(uint8_t *p, uint8_t entity_size, uint8_t n) {
  uint8_t i;
  uint8_t r, g, b, d;
  uint8_t *last_pixel;
  
  for (i = 0; i < n; i++) {
    last_pixel = p + ((entity_size-1) * BYTES_PER_PIXEL);
    r = p[0];
    g = p[1];
    b = p[2];
    d = p[3];
    memmove(p, p+BYTES_PER_PIXEL, (entity_size-1)*BYTES_PER_PIXEL);
    last_pixel[0] = r;
    last_pixel[1] = g;
    last_pixel[2] = b;
    last_pixel[3] = d;
  }
}

// Come here if we can't proceed. Blink the LED frantically.
void fatal_error(void) {
  SPI.end();
  pinMode(13, OUTPUT);
  
  while (1) {
    digitalWrite(13, HIGH);
    delay(200);
    digitalWrite(13, LOW);
    delay(200);
  }
}

// On powerup, we read our configuration from EEPROM.
void read_EEPROM(void) {
  int addr = 0;
  uint8_t segment;
  uint8_t entity_address;
  
  slave_address = EEPROM.read(addr++);
  config_version = EEPROM.read(addr++) * 0x100;  // opposite byte order
  config_version += EEPROM.read(addr++);
  strand_type = EEPROM.read(addr++);
  pixels_in_strand = EEPROM.read(addr++);
  num_segments = EEPROM.read(addr++);
  
  if (slave_address == 255     ||
      config_version != 0x0001 ||
      strand_type > 1          ||
      num_segments > MAX_SEGMENTS) {
    // Error in configuration data!
    fatal_error();
  }
  
  slave_address_bitmap = 1 << slave_address;
  
  for (segment=0; segment < num_segments; segment++) {
    entity_address = EEPROM.read(addr++);
    segments[segment].address = entity_address;
        
    segments[segment].pixels_in_entity = EEPROM.read(addr++);
    segments[segment].first_entity_index = EEPROM.read(addr++);
    segments[segment].pixels_in_segment = EEPROM.read(addr++);
    segments[segment].first_strand_index = EEPROM.read(addr++);
    segments[segment].reverse_index = EEPROM.read(addr++);
  }
  
}

// After we've read our configuration, we need to build some data structures
// for each entity we're handling part(s) of.
void init_entities(void) {
  uint8_t  segment;
  uint8_t  address;
  uint8_t  pixels;
  
  num_entities = 0;
  entity_address_bitmap = 0;

  // For each segment, create an entry in the entity table if there isn't already one.
  for (segment = 0; segment < num_segments; segment++) {
    address = segments[segment].address;
    pixels = segments[segment].pixels_in_entity;
    segment[segments].entity_index = add_to_entity_table(address, pixels);
  }  
}

// Create a new entity table entry if there isn't already one for this address,
// and return its index. If there's already an entry with that address, return its index.
uint8_t add_to_entity_table(uint8_t addr, uint8_t pixels) {
  uint8_t entity;

  for (entity = 0; entity < num_entities; entity++) {
    if (entities[entity].address == addr) {
      return entity;
    }
  }
  num_entities++;
  entities[entity].address = addr;
  entities[entity].pixel_count = pixels;
  
  // build up a bitmap of the entities we support, for quick addressing decisions
  entity_address_bitmap |= (1 << addr);
  
  // allocate a buffer to hold the RGBD data for the entity
  // Use calloc to set them all to zero, which represents black (off).
  entities[entity].buffer = (uint8_t *)calloc(pixels, BYTES_PER_PIXEL);
  if (entities[entity].buffer == NULL) {
    fatal_error();
  }

  return entity;
}

// On Powerup ...
void setup() {
  
  // seed the random number generator, in case we use it
  randomSeed(analogRead(0));
  
  // set up debug output pins, for use with oscilloscope or logic analyzer
  pinMode(DEBUG1_PIN, OUTPUT);
  pinMode(DEBUG2_PIN, OUTPUT);
  digitalWrite(DEBUG1_PIN, LOW);
  digitalWrite(DEBUG2_PIN, LOW);

  // obtain configuration info. Doesn't return if config is invalid.
  read_EEPROM();
  
  // initialize data structures for the entities defined in our configuration
  init_entities();
  
  // allocate a buffer to hold the RGBD data for the physical LED strand
  // Use calloc to set them all to zero, which represents black (off).
  strand = (uint8_t *)calloc(pixels_in_strand+1, BYTES_PER_PIXEL);  // +1 makes pipelining calculations safe
  if (strand == NULL) {
    fatal_error();
  }
  
  // initialize the physical LED strand
  start_strand();
  update_strand(strand);
  
  // set up the serial port for network communications
  Serial.begin(9600);
}

void loop() {
  static unsigned long last_tick_millis = 0;
  unsigned long t;
  
  // process incomding network commands
  if (Serial.available() > 0) {
    handle_serial_byte(Serial.read());
  }
  
  // every 10ms, perform tick-based processing
  t = millis() - time_origin;
  if (t - last_tick_millis >= TICK_LENGTH) {
    last_tick_millis += TICK_LENGTH;      // if we get behind, we'll try to catch up.
    tick();
  }
}

// Periodic time processing
void tick(void) {
  
  // kick off the regular update of the LED strand
  dynamics_precalc_tick();   // Precompute dynamics to be applied during update
  refresh_segments();        // copy entity data to the strand buffer
  update_strand(strand);     // send it out to the physical LED strand  
  
  // keep time-based entity processing cooking along
  scan_deferred_queue();
    
  current_tick++;
}

