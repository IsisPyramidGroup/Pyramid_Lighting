/*
Test master for electrical test of CarouShell tower LEDs

Connect RS-485 adapter to RX and TX

Connect User console display to pin 8.
Connect User console buttons to pins 3,4,5,6,7

*/
#include "QdS_User.h"

#define MIN_PACKET  3
#define MAX_PACKET  10

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


#define  SEQ_ROTATE  1
#define  SEQ_SPARKLE  2
#define  SEQ_RAINBOW  3


// Special codes for byte stuffing. Per KISS or SLIP protocol standards.
#define	FEND	0xC0
#define FESC	0xDB
#define TFEND	0xDC
#define	TFESC	0xDD

// blocking transmission
void send_packet(byte count, byte *buffer) {
  byte i;
  byte out;
  
  if (count < MIN_PACKET)
    return;
  
  Serial.write(FEND);
  for (i=0; i < count; i++) {
    out = buffer[i];
    if (out == FEND) {
      Serial.write(FESC);
      Serial.write(TFEND);
    } else if (out == FESC) {
      Serial.write(FESC);
      Serial.write(TFESC);
    } else {
      Serial.write(out);
    }
  }
  Serial.write(FEND);
}

void setup() {
  Serial.begin(9600);
  //user_setup();
  //display_number(1234);
  delay(200);
}

void loop() {
  int i;
  uint8_t command[MAX_PACKET];
  
  // make sure all sequencing is shut down.
  command[CMD_FIELD_COMMAND] = CMD_SEQUENCE;
  command[CMD_FIELD_ADDMAP] = 0xFF;  // all call
  command[CMD_FIELD_DATA] = 0;  // turn off sequencing
  send_packet(6, command);
  delay(10);
  
  //send the same pattern to all controllers for testing
  command[CMD_FIELD_COMMAND] = CMD_TEST;
  command[CMD_FIELD_ADDMAP ] = 0xFF;     // all call
  command[CMD_FIELD_DATA   ] = 'r';
  send_packet(3, command);
  delay(500);
  //
  command[CMD_FIELD_ADDMAP ] = 0xFF;     // all call
  command[CMD_FIELD_DATA   ] = 'g';
  send_packet(3, command);
  delay(500);
  //
  command[CMD_FIELD_ADDMAP ] = 0xFF;
  command[CMD_FIELD_DATA   ] = 'b';
  send_packet(3, command);
  delay(500);
  //
  command[CMD_FIELD_ADDMAP ] = 0x88;
  command[CMD_FIELD_DATA   ] = 'w';
  send_packet(3, command);
  delay(500);
  //
}
 
