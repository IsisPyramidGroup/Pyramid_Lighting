//!!! TODO
// - figure out what if anything to do with the user console buttons
// - better isolate user console interactions

/*
  Master Controller for the Isis Pyramid 1.1
  
  The master is the only talker on a RS-485 bus that connects the master to 16 slave controllers.
  A small but powerful repertoire of packets can be sent to the slaves, telling the slaves how to
  manage their connected strands of RGB LED pixels. Rather than bury the knowledge of these packets
  in this master program, the sequences of packets to send are stored separately in files on an SD
  card. These are called lighting programs. An additional file on the SD card, named PLAYLIST.TXT,
  contains the list of filenames for the lighting programs, and sets the order in which they are
  normally executed. To execute a lighting program, this master program reads packets from the file.
  Most packets are simply sent on to the RS-485 bus, without any interpretation. Certain packets
  begin with the special META command code 0xFF, and these are interpreted locally by the master.
  They give the master just enough information to parcel out the other packets with appropriate
  timing. The designer of the lighting program is responsible for setting up sequences of packets
  that do something interesting and comply with the constraints of the slave program.
 
  Besides the slave controllers, the master may be connected to a user console, which consists of
  a 4-digit 7-segment LED display and five buttons. The master polls the buttons and implements
  TBD interactivity. The display is shared between the button functions (TBD) and the lighting
  programs, which can include a META command to display a number.
 
  RS-485 adapter is connected to TX (RX is not used).
  User console display is a software serial transmitting on pin 8.
  The user console buttons are individually connected to pins 3,4,5,6,7.
 
 */ 

#include <SPI.h>
#include <SD.h>
#include "isis_packets.h"
#include "isis_user.h"

#define  SD_SELECT        10  // SD Card Shield uses pin 10 for SPI select

// All processing in the slaves is based on ticks. It's convenient to do the
// same here in the master.
#define  TICK_LENGTH    10      // milliseconds per tick
uint16_t current_tick = 0;      // counter of ticks processed since powerup or origin reset
                                // designer is supposed to make sure this never wraps around (10+ minutes)
                                
unsigned long last_tick_millis = 0;  // time when we last processed a tick


// Time-based commands in the slaves are processed relative to a local time
// origin. Here in the master it's helpful to keep a time base that's more or less
// synchronized with that. It'll start at zero (as it does in the slaves) and be
// reset to zero at the same times we send reset commands to the slaves.
unsigned long time_origin = 0;

// Sometimes we just have to wait until the slaves are ready to receive the next
// commands. Here we store the value of current_tick when it's OK to start again.
uint16_t waitfor_tick = 0;

// Sometimes the file playout ends long before the program itself has finished
// executing on the slaves. Here's where we keep track of when it is supposed to be done.
uint16_t program_end_tick;


// Transmit a buffered packet on the RS-485 bus to the slaves.
// This is a blocking transmission, i.e., the function doesn't return until the
// transmission has been completely sent (at least as far as the Serial driver anyway).
// This function adds transparent framing according to the SLIP protocol standard,
// RFC 1055.
void send_packet(byte count, byte *buffer) {
  byte i;
  byte out;
  
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

// Handle a single byte from a lighting program file.
// The stream of bytes is broken up into packets according to the rules
// for KISS or SLIP framing, by wrapping the packet in flag bytes. The
// encoding is made transparent to binary data by escaping any flag bytes
// that may occur in the data stream. See RFC 1055.

// States of the "receive" state machine
#define	IDLE	0			// we're waiting for an initial flag
#define	RECV	1			// putting bytes into the buffer
#define	STUF	2			// next byte is a stuffed byte

uint8_t pkt_buffer[PACKET_MAX];
uint8_t pkt_count;
byte state = IDLE;

void handle_file_byte(byte bytein) {
  
  switch (state) {
    case	IDLE:
      if (bytein == FEND) {
        state = RECV;
	    pkt_count = 0;
      }
      break;

    case 	RECV:
      if ((bytein == FEND) && (pkt_count == 0)) {   // back-to-back FENDs, no action
      
      } else if (bytein == FEND) {         // end of packet
        handle_file_packet();
        state = IDLE;
      } else if (bytein == FESC)           // byte de-stuffing to happen next
        state = STUF;
      else                          // ordinary data byte
        add_to_pkt_buffer(bytein);
      break;

    case	STUF:
      if (bytein == TFESC) {
        add_to_pkt_buffer(FESC);
        state = RECV;
      } else if (bytein == TFEND) {
        add_to_pkt_buffer(FEND);
        state = RECV;
      } else {                           // byte stuffing error
        state = IDLE;
        }
      break;

    default:  // "can't happen"
      state = IDLE;
      break;
  }
}


// simply add a "received" byte to the packet re-assembly buffer
void add_to_pkt_buffer(byte in) {
  pkt_buffer[pkt_count] = in;
  pkt_count++;
  if (pkt_count >= PACKET_MAX) {              // overflow! Shouldn't happen.
    state = IDLE;
  }
}


// We've fetched a packet from a lighting program file. Process it.
// Either it's a META command, which we have to process locally, or else it's a
// packet we need to forward on to the slaves.
void handle_file_packet(void) {
  uint16_t value;

  if (pkt_buffer[PKT_COMMAND_OFFSET] == CMD_META) {
    value = * (uint16_t *)(pkt_buffer + PKT_META_DATA_OFFSET);
    switch(pkt_buffer[PKT_META_CMD_OFFSET]) {
      case META_CONSOLE:
        display_number(value);    // put the value on the user console display
        break;
        
      case META_WAIT:
        waitfor_tick = value;    // note when we can resume sending out packets
        break;
        
      case META_ENDS:
        program_end_tick = value;  // note when the current program is supposed to end
        waitfor_tick = value;      // also implies that we wait for the program to finish
        ready_next_file();         // take advantage of any extra to to prep the next program    
        break;
        
      case META_RESET_TIME:
        time_origin = millis();    // reset the master's time; might or might not coincide with slave reset
        current_tick = 0;
        last_tick_millis = 0;
        waitfor_tick = 0;          // cancels any waiting we're doing.
        break;
              
      default:      // unknown meta command
        break;
    }
  } else {  // we have a packet to forward to the slaves
    send_packet(pkt_count, pkt_buffer);
  }
}  

// At the beginning of any timeline of predefined activities, it's a good idea
// to resynchronize (if it isn't too disruptive). This routine restarts the local
// clock and tells all the slaves to do the same, which cancels any and all pending
// operations within the slaves (but does not reset the entity pixel buffers).
void reset_time_origin(void) {
  uint8_t packet[PACKET_MAX];
  
  // send reset command to all slaves
  packet[PKT_COMMAND_OFFSET] = CMD_S_RESET_CLOCK;
  * (uint16_t *)(packet+PKT_ADDRESS_OFFSET) = PKT_ADDRESS_ALL_CALL;
  send_packet(3, packet);
    
  // reset local timeline
  time_origin = millis();
  waitfor_tick = 0;
  
  // Note: we don't try to get absolute synchrony between the slave time and the
  // master time. The packet will take a short while to go out, and a short while
  // to be processed, so chances are the slaves will lag a bit. That's OK.
}



// The SD card contains a file named PLAYLIST.TXT, which contains a list of filenames.
// Each filename listed in PLAYLIST.TXT contains a sequence of packets that implements
// a particular sequence of lighting effects, called a lighting program. We play out
// the commands in each file named in PLAYLIST.TXT in the order they're named, and then
// go back to the top, forever.

// Index in PLAYLIST.TXT of the filename we'll try to open next.
int playlist_index = 0;
#define FILENAME_MAX    50      // arbitrary, big enough for 8.3 plus a modest path
char filename[FILENAME_MAX];

// Initialize the SD card for reading.
void file_setup(void) {
  if (!SD.begin(SD_SELECT)) {
    fatal_error();
  }
}

// Open up PLAYLIST.TXT and read out the Nth filename.
// If there aren't that many, reset playlist_index to 0 and read out the 0th one.
void file_get_filename(int index) {
  int lines_skipped = 0;
  int fn_index = 0;
  char chr;

  File playlist = SD.open("PLAYLIST.TXT");
  if (!playlist) {
    fatal_error();
  }

  while (playlist.available()) {
    chr = playlist.read();
    if (chr == '\n') {        // end of line
      if (fn_index != 0) {    // and we have some characters in the buffer
        if (lines_skipped == index) {       // name in the buffer is the one we want
          filename[fn_index] = '\0';        // null terminate the string
          playlist.close();
          playlist_index++;
          return;             // success!
        } else {  // that was one of the lines we need to skip
          lines_skipped++;
          fn_index = 0;       // start over with the next line
        }
      } else {                // buffer is empty, it was a blank line
        // disregard it
      }
    } else if (isspace(chr)) {    // not end of line but other white space
      // disregard it
    } else {  // non-whitespace character in the line
      filename[fn_index++] = chr;
      if (fn_index >= FILENAME_MAX) {   // line too long. Ugh.
        fatal_error();
      }
    }
  } // end of file (or error)
  
  // if we get here, unless there has been an error, we're trying to read
  // one filename past the last one. Since we want to repeat the list over
  // and over, the thing to do now is go back to the 0th filename and try
  // again.
  playlist.close();
  playlist_index = 0;
  file_get_filename(playlist_index);      // call this function again.
}

// File object for reading a lighting program from the SD card.
File lp_file;

// Read from the currently open file; open a file if there isn't one.
void next_file_byte(void) {
  
  if (lp_file && lp_file.available()) {    // if we already have a file open and ready
    handle_file_byte(lp_file.read());      // get a byte and process it.
  } else {    // no file currently open. Open one.
    ready_next_file();
    // we'll start reading from it next time around, assuming it was opened successfully.
  }
}

// We're at or near the end of the current lighting program, so we need to get ready
// to begin executing the next one.
void ready_next_file(void) {
  if (lp_file) {
    lp_file.close();
  }
  file_get_filename(playlist_index);      // obtain the next filename
  lp_file = SD.open(filename);
  display_number(playlist_index);
}
            

void fatal_error(void) {
  //!!!
  
  while (1);
}
      
  
// At powerup ...
void setup() {
  
  Serial.begin(9600);       // RS-485 network
  
  user_setup();
  display_number(1234);

  file_setup();
  
  delay(2000);              // wait for all the slaves to be awake and listening
  reset_time_origin();
}

void loop() {
  unsigned long t;
    
  // every 10ms, perform tick-based processing
  t = millis() - time_origin;
  if (t - last_tick_millis >= TICK_LENGTH) {
    last_tick_millis += TICK_LENGTH;      // if we get behind, we'll try to catch up.
    tick();
  }
  
  // If we're not waiting for time to elapse, grab the next packet from the file
  // and process it.
  if (current_tick >= waitfor_tick) {
    next_file_byte();
  }
}

// Periodic time processing
void tick(void) {
  
  buttons();
  
  current_tick++;
}
  
// debounce the buttons before processing them
void buttons(void) {
  uint8_t buttons;
  static uint8_t button_state;
  static uint8_t prev_buttons = 0;
  static long prev_button_time = 0;
  
  buttons = scan_buttons();
  
  if (buttons != prev_buttons) {
    prev_button_time = millis();
  }
  
  if ((millis() - prev_button_time) > 50) {
    
    if (buttons != button_state) {
      button_state = buttons;
      process_new_button_state(buttons);
    }
  }
  
  prev_buttons = buttons;
}

// !!!
// Here are all the old button meanings:
//   Black center:  hold current pattern (i.e, don't return from function)
//   Red right:     rotating beacon of red
//   Yellow left:   fill with color (+red, +blue, +green in RGB combos, black dims)
//   Blue top:      flashing beacon of blue


// Dispatch button processing based on new button-down events
void process_new_button_state(uint8_t buttons) {
  static uint8_t old_buttons;
  uint8_t new_buttons = buttons & ~old_buttons;
  
  if (new_buttons & BUTTON_RED) {
    button_down_red();
  }

  if (new_buttons & BUTTON_BLACK) {
    button_down_black();
  }
  
  if (new_buttons & BUTTON_BLUE) {
    button_down_blue;
    }
    
  if (new_buttons & BUTTON_GREEN) {
    button_down_green();
    }
    
  if (new_buttons & BUTTON_YELLOW) {
    button_down_yellow();
    }
    
  old_buttons = buttons;
}

// The red button jumps ahead to begin the next lighting program.
void button_down_red(void) {
  
  waitfor_tick = 0;
  ready_next_file();
  
}

// The black button restarts the current lighting program.
void button_down_black(void) {
  
  waitfor_tick = 0;
  playlist_index--;
  ready_next_file();
  
}

// The yellow button goes back to the very first lighting program
void button_down_yellow(void) {
  
  waitfor_tick = 0;
  playlist_index = 0;
  ready_next_file();
  
}

void button_down_blue(void) {
  
}

void button_down_green(void) {
  
}



