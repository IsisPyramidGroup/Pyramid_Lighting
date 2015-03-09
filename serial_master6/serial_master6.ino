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
  user_setup();
  display_number(1234);
  poll_and_delay(200);
}

void loop() {
  int i;
  uint8_t command[MAX_PACKET];
  
  // make sure all sequencing is shut down.
  command[CMD_FIELD_COMMAND] = CMD_SEQUENCE;
  command[CMD_FIELD_ADDMAP] = 0xFF;  // all call
  command[CMD_FIELD_DATA] = 0;  // turn off sequencing
  send_packet(6, command);
  poll_and_delay(10);
  
  command[CMD_FIELD_COMMAND] = CMD_TEST;
  command[CMD_FIELD_ADDMAP ] = 0x11;      // two slaves
  command[CMD_FIELD_DATA   ] = 'r';
  send_packet(3, command);
  command[CMD_FIELD_ADDMAP ] = 0x22;
  command[CMD_FIELD_DATA   ] = 'g';
  send_packet(3, command);
  command[CMD_FIELD_ADDMAP ] = 0x44;
  command[CMD_FIELD_DATA   ] = 'b';
  send_packet(3, command);
  command[CMD_FIELD_ADDMAP ] = 0x88;
  command[CMD_FIELD_DATA   ] = 'w';
  send_packet(3, command);
  display_number(1);
  poll_and_delay(4000);

  command[CMD_FIELD_ADDMAP ] = 0xFF;      // all call
  command[CMD_FIELD_DATA   ] = 'g';
  send_packet(3, command);
  display_number(2);
  poll_and_delay(4000);

  command[CMD_FIELD_ADDMAP ] = 0x11;      // two slaves
  command[CMD_FIELD_DATA   ] = 'g';
  send_packet(3, command);
  command[CMD_FIELD_ADDMAP ] = 0x22;
  command[CMD_FIELD_DATA   ] = 'b';
  send_packet(3, command);
  command[CMD_FIELD_ADDMAP ] = 0x44;
  command[CMD_FIELD_DATA   ] = 'w';
  send_packet(3, command);
  command[CMD_FIELD_ADDMAP ] = 0x88;
  command[CMD_FIELD_DATA   ] = 'r';
  send_packet(3, command);
  display_number(3);
  poll_and_delay(4000);
  
  command[CMD_FIELD_ADDMAP ] = 0xFF;      // all call
  command[CMD_FIELD_DATA   ] = 'b';
  send_packet(3, command);
  display_number(4);
  poll_and_delay(4000);

  command[CMD_FIELD_ADDMAP ] = 0x11;      // two slaves
  command[CMD_FIELD_DATA   ] = 'b';
  send_packet(3, command);
  command[CMD_FIELD_ADDMAP ] = 0x22;
  command[CMD_FIELD_DATA   ] = 'w';
  send_packet(3, command);
  command[CMD_FIELD_ADDMAP ] = 0x44;
  command[CMD_FIELD_DATA   ] = 'r';
  send_packet(3, command);
  command[CMD_FIELD_ADDMAP ] = 0x88;
  command[CMD_FIELD_DATA   ] = 'g';
  send_packet(3, command);
  display_number(5);
  poll_and_delay(4000);

  command[CMD_FIELD_ADDMAP ] = 0xFF;      // all call
  command[CMD_FIELD_DATA   ] = 'w';
  send_packet(3, command);
  display_number(6);
  poll_and_delay(4000);

  command[CMD_FIELD_DATA   ] = 'o';  // black
  send_packet(3, command);
  display_number(7);
  poll_and_delay(4000);

  command[CMD_FIELD_DATA   ] = 'x';  //rainbow
  send_packet(3, command);
  display_number(8);
  poll_and_delay(12000);

  command[CMD_FIELD_DATA   ] = 'y';  //rainbow cycle
  send_packet(3, command);
  display_number(9);
  poll_and_delay(12000);

//  command[CMD_FIELD_DATA   ] = 'f';  // fastblink doesn't look like anything
//  send_packet(3, command);
//  display_number(7);
//  poll_and_delay(3000);

  command[CMD_FIELD_DATA   ] = 's';  //fast blink
  send_packet(3, command);
  display_number(10);
  poll_and_delay(8000);
  
  command[CMD_FIELD_COMMAND] = CMD_RANDOMIZE;
  command[CMD_FIELD_ADDMAP ] = 0xFF;            // all cal
  command[CMD_FIELD_DATA   ] = 'r';    // not used
  send_packet(3, command);
  display_number(11);
  poll_and_delay(50);    // just time for it to happen
  
  command[CMD_FIELD_COMMAND] = CMD_SEQUENCE;
  command[CMD_FIELD_ADDMAP ] = 0xFF;
  command[CMD_FIELD_DATA    ] = SEQ_ROTATE;
  command[CMD_FIELD_DATA + 1] = 20;      // 10ms ticks per shift
  command[CMD_FIELD_DATA + 2] = 1;        // shift just one LED at a time
  send_packet(6, command);
  display_number(12);
  poll_and_delay(10000);    // one complete rotation
  
  command[CMD_FIELD_COMMAND] = CMD_FILL;
  command[CMD_FIELD_ADDMAP] = 0xFF;   // all call
  command[CMD_FIELD_DATA    ] = 0;    // black
  command[CMD_FIELD_DATA + 1] = 0;
  command[CMD_FIELD_DATA + 2] = 0;
  send_packet(5, command);
  display_number(13);
  poll_and_delay(20);    // just time for it to happen
  
  command[CMD_FIELD_COMMAND] = CMD_SEQUENCE;
  command[CMD_FIELD_ADDMAP ] = 0xFF;
  command[CMD_FIELD_DATA    ] = SEQ_SPARKLE;
  command[CMD_FIELD_DATA + 1] = 1;      // ticks per sparkle (max rate)
  send_packet(6, command);
  display_number(14);
  poll_and_delay(10000);    // sparkle for a while
  
  command[CMD_FIELD_COMMAND] = CMD_TEST;
  command[CMD_FIELD_ADDMAP ] = 0x49;      // RGBRGBRG slaves
  command[CMD_FIELD_DATA   ] = 'r';
  send_packet(3, command);
  command[CMD_FIELD_ADDMAP ] = 0x92;
  command[CMD_FIELD_DATA   ] = 'g';
  send_packet(3, command);
  command[CMD_FIELD_ADDMAP ] = 0x24;
  command[CMD_FIELD_DATA   ] = 'b';
  send_packet(3, command);
  display_number(15);
  poll_and_delay(4000);
  
  command[CMD_FIELD_COMMAND] = CMD_SEQUENCE;
  command[CMD_FIELD_ADDMAP ] = 0xFF;
  command[CMD_FIELD_DATA    ] = SEQ_SPARKLE;
  command[CMD_FIELD_DATA + 1] = 1;      // ticks per sparkle (max rate)
  send_packet(6, command);
  display_number(16);
  poll_and_delay(10000);    // sparkle for a while
  
  // make sure all sequencing is shut down.
  command[CMD_FIELD_COMMAND] = CMD_SEQUENCE;
  command[CMD_FIELD_ADDMAP] = 0xFF;  // all call
  command[CMD_FIELD_DATA] = 0;  // turn off sequencing
  send_packet(6, command);
  poll_and_delay(10);

  command[CMD_FIELD_COMMAND] = CMD_FILL;
  command[CMD_FIELD_ADDMAP ] = 0xFF;  // all call
  command[CMD_FIELD_DATA    ] = 255;    // red
  command[CMD_FIELD_DATA + 1] = 0;
  command[CMD_FIELD_DATA + 2] = 0;
  send_packet(5, command);
  display_number(17);
  poll_and_delay(20);  // let it happen
  
  command[CMD_FIELD_COMMAND] = CMD_LOADONE;
  command[CMD_FIELD_ADDMAP] = 0xFF; // all call  - send to all controllers
  command[CMD_FIELD_DATA + 1 ] = 255;    // white for candy cane/barber pole
  command[CMD_FIELD_DATA + 2] = 255;
  command[CMD_FIELD_DATA + 3] = 255;
  for (i=0; i < 5; i++) {
    command[CMD_FIELD_DATA] = i;
    send_packet(6, command);
  }
  for (i=10; i < 15; i++) {
    command[CMD_FIELD_DATA] = i;
    send_packet(6, command);
  }
  for (i=20; i < 25; i++) {
    command[CMD_FIELD_DATA] = i;
    send_packet(6, command);
  }
  for (i=30; i < 35; i++) {
    command[CMD_FIELD_DATA] = i;
    send_packet(6, command);
  }
  for (i=40; i < 45; i++) {
    command[CMD_FIELD_DATA] = i;
    send_packet(6, command);
  }
  poll_and_delay(200);
  
  command[CMD_FIELD_COMMAND] = CMD_ROTATE;
  command[CMD_FIELD_ADDMAP] = 0xFE;
  command[CMD_FIELD_DATA] = 1;
  send_packet(3, command);
  poll_and_delay(200);
  command[CMD_FIELD_ADDMAP] = 0xFC;
  send_packet(3, command);
  poll_and_delay(200);
  command[CMD_FIELD_ADDMAP] = 0xF8;
  send_packet(3, command);
  poll_and_delay(200);
  command[CMD_FIELD_ADDMAP] = 0xF0;
  send_packet(3, command);
  poll_and_delay(200);
  command[CMD_FIELD_ADDMAP] = 0xE0;
  send_packet(3, command);
  poll_and_delay(200);
  command[CMD_FIELD_ADDMAP] = 0xC0;
  send_packet(3, command);
  poll_and_delay(200);
  command[CMD_FIELD_ADDMAP] = 0x80;
  send_packet(3, command);
  display_number(18);
  poll_and_delay(8000);        // pause to admire candy stripes

  command[CMD_FIELD_COMMAND] = CMD_SEQUENCE;
  command[CMD_FIELD_ADDMAP ] = 0xFF;
  command[CMD_FIELD_DATA    ] = SEQ_ROTATE;
  command[CMD_FIELD_DATA + 1] = 20;      // ticks per shift
  command[CMD_FIELD_DATA + 2] = 1;        // shift just one LED at a time
  send_packet(6, command);
  display_number(19);
  poll_and_delay(10000);    // one complete rotation

  command[CMD_FIELD_DATA + 1] = 10;      // twice as fast
  send_packet(6, command);
  display_number(20);
  poll_and_delay(10000);    // two complete rotations
  
  command[CMD_FIELD_DATA + 1] = 5;      // twice as fast again!
  send_packet(6, command);
  display_number(21);
  poll_and_delay(10000);    // four complete rotations
  
  command[CMD_FIELD_DATA + 1] = 2;      // super fast
  send_packet(6, command);
  display_number(22);
  poll_and_delay(5000);    // a little while
  
  command[CMD_FIELD_COMMAND] = CMD_SEQUENCE;
  command[CMD_FIELD_ADDMAP ] = 0xFF;
  command[CMD_FIELD_DATA    ] = 0;    // just stop.
  send_packet(6, command);
  poll_and_delay(10);
  
  // whew, let's relax for a while. Dim green.
  command[CMD_FIELD_COMMAND] = CMD_FILL;
  command[CMD_FIELD_ADDMAP ] = 0xFF;  // all call
  command[CMD_FIELD_DATA    ] = 0;    
  command[CMD_FIELD_DATA + 1] = 100;  // dim agreen
  command[CMD_FIELD_DATA + 2] = 0;
  send_packet(5, command);
  display_number(23);
  poll_and_delay(14000);

  command[CMD_FIELD_COMMAND] = CMD_SEQUENCE;
  command[CMD_FIELD_ADDMAP] = 0x55;      // half of strands
  command[CMD_FIELD_DATA    ] = SEQ_RAINBOW;
  command[CMD_FIELD_DATA + 1] = 8;      // 10ms ticks per rainbow step (4x slow)
  send_packet(6, command);
  command[CMD_FIELD_ADDMAP] = 0xAA;     // other half of strands
  command[CMD_FIELD_DATA    ] = SEQ_SPARKLE;
  command[CMD_FIELD_DATA + 1] = 20;      // relatively sparse sparkle
  send_packet(6, command);
  display_number(24);
  poll_and_delay(30000);
  
  
  

  


  
}

#define  BUTTON_GREEN   1
#define  BUTTON_RED     2
#define  BUTTON_BLACK   4
#define  BUTTON_YELLOW  8
#define  BUTTON_BLUE   16

// poll_and_delay() implements some of the button meanings.
// Here are all the meanings:
//   Black center:  hold current pattern (i.e, don't return from function)
//   Red right:     rotating beacon of red
//   Yellow left:   fill with color (+red, +blue, +green in RGB combos, black dims)
//   Blue top:      flashing beacon of blue

void poll_and_delay(unsigned long wait) {
  static int prev_buttons;
  int buttons;
  unsigned long t1 = millis() + wait;
  uint8_t r, g, b;
  uint8_t command[MAX_PACKET];
  uint8_t strand;
 
  
  while (millis() < t1) {
    buttons = scan_buttons();

    // somewhat cryptic binary display of buttons state    
    if (buttons != prev_buttons) {
      prev_buttons = buttons;
      
      display_number(buttons * 10);
    }
    
    // Yellow changes all other button meanings, so check it first
    // and hang here while it's held down.    
    while (buttons &  BUTTON_YELLOW) {
      if (buttons & BUTTON_BLACK) {
        r = (buttons & BUTTON_RED) ? 40 : 0;
        g = (buttons & BUTTON_GREEN) ? 40 : 0;
        b = (buttons & BUTTON_BLUE) ? 40 : 0;
      } else {
        r = (buttons & BUTTON_RED) ? 255 : 0;
        g = (buttons & BUTTON_GREEN) ? 255 : 0;
        b = (buttons & BUTTON_BLUE) ? 255 : 0;
      }
      
      // make sure all sequencing is shut down.
      command[CMD_FIELD_COMMAND] = CMD_SEQUENCE;
      command[CMD_FIELD_ADDMAP] = 0xFF;  // all call
      command[CMD_FIELD_DATA] = 0;  // turn off sequencing
      send_packet(6, command);
      delay(10);
      
      command[CMD_FIELD_COMMAND] = CMD_FILL;
      command[CMD_FIELD_ADDMAP ] = 0xFF;  // all call
      command[CMD_FIELD_DATA    ] = r;    
      command[CMD_FIELD_DATA + 1] = g;
      command[CMD_FIELD_DATA + 2] = b;
      send_packet(5, command);
      delay(10);
      
      while ((buttons = scan_buttons()) == prev_buttons) {
      }
      prev_buttons = buttons;
    } // still perhaps processing in yellow-button mode
    
    // out of yellow-button mode, we have regular button meanings.
    // Black means hold on here, don't do anything until black is released.
    while (buttons & BUTTON_BLACK) {
      // just wait here.
      buttons = scan_buttons();
    }
    
    while (buttons & BUTTON_BLUE) {
      command[CMD_FIELD_COMMAND] = CMD_FILL;
      command[CMD_FIELD_ADDMAP] = 0xFF;   // all call
      command[CMD_FIELD_DATA    ] = 0;    // blue
      command[CMD_FIELD_DATA + 1] = 0;
      command[CMD_FIELD_DATA + 2] = 255;
      send_packet(5, command);
      display_number(666);
      delay(100);
      command[CMD_FIELD_DATA + 2] = 0;    // black
      send_packet(5, command);
      delay(100);
      buttons = scan_buttons();
    }
    
    // Red means rotating beacon of red
    if (buttons & BUTTON_RED) {
      command[CMD_FIELD_COMMAND] = CMD_FILL;
      command[CMD_FIELD_ADDMAP] = 0xFF;   // all call
      command[CMD_FIELD_DATA    ] = 0;    // black
      command[CMD_FIELD_DATA + 1] = 0;
      command[CMD_FIELD_DATA + 2] = 0;
      send_packet(5, command);
      display_number(999);
      delay(10);    // just time for it to happen
      
      strand = 0x11;
      while ((buttons = scan_buttons()) & BUTTON_RED) {
         command[CMD_FIELD_ADDMAP] = strand;
         command[CMD_FIELD_DATA    ] = 255;    // red
         send_packet(5, command);
         delay(100);    // pretty fast
         command[CMD_FIELD_DATA    ] = 0;      // black
         send_packet(5, command);
         delay(10);    // just time for it to happen
         switch (strand) {
           case 0x11:  strand = 0x22;
                       break;
           case 0x22:  strand = 0x44;
                       break;
           case 0x44:  strand = 0x88;
                       break;
           default:    strand = 0x11;
                       break;
         }
      }
      command[CMD_FIELD_ADDMAP] = 0xFF;   // all call
      command[CMD_FIELD_DATA    ] = 0;    // black
      send_packet(5, command);
      delay(10);
    }
  }
}  
