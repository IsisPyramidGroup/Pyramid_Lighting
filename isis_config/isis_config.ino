// Program to write configuration info into slave EEPROM memory
// for Isis Pyramid 1.1 configuration (Lucidity 2015)
// Uses the serial console interactively (at 115200).
//
// The configuration is coded into tables for all slaves; this is the one
// authoritative copy of the entity configuration for the Pyramid.
// The interactive user chooses which slave (by address) is to be programmed.
//
// 2015-03-18 ptw
// 2015-03-29 ptw updated for as-built configuration

/*
Starfish strands are mounted in four configurations:
A = 8 6-foot sticks of 18 pixels + power cable, connectors both ends
B = 3 8-foot sticks of 24 pixels + power cable, connector at input end, joined at output end.
C = 2 8-foot sticks of 24 pixels, no power. No passthru connector at output end.
D = 6 12-foot sticks of 36 pixels, no power.

Caroushell strips are mounted on the upper diagonals.
Type A sticks are mounted on the lower diagonals.
The B, C, and D sticks are arranged on the lower edge as shown:

 B D - D 
D       B
-       D
D       -
B       D
 C     C
The hyphens represent breaks in the wiring. No wires are connected at these places.
The bottom in this picture is the front door.
 
 Number the corners like this:
  3      4 
 2        5
 
 1        6
  0      7

So here's what we have at each corner:

0 = A,C.   42 pixels. Power at pixel 0.
1 = A,B,D. 78 pixels. Power at pixel 0 and 42.
2 = A,D.   54 pixels. Power at pixel 0.
3 = A,B,D. 78 pixels. Power at pixel 0 and 42.
4 = A,D.   54 pixels. Power at pixel 0.
5 = A,B,D. 78 pixels. Power at pixel 0 and 42.
6 = A,D.   54 pixels. Power at pixel 0.
7 = A,C.   42 pixels. Power at pixel 0.
Total:    480 pixels.
*/

#include <EEPROM.h>

#define  NUM_SLAVES  16

int  address = 255;
int  old_address = 255;
int  old_version = 0;
unsigned int bitmap = 0x0000;

// Starting to the left of the door and proceeding clockwise,
// the slaves controlling upper diagonals (Caroushell strips) are 0-7,
// the slaves controlling lower diagonals and bottom edges are 8-15.
//
// The entities are:
// Diagonals 0-7 (pixels count from bottom to top)
// Left edge 8 (pixels count clockwise from front left corner)
// Back edge 9
// Right edge 10
// Front edge (spanning the door) 11
//

#define  CAROUSHELL_STRIP_LENGTH    50  // pixels in the upper diagonal
#define  LOWER_DIAGONAL_LENGTH      18  // pixels in the 6' lower diagonal
#define  DIAGONAL_LENGTH    (CAROUSHELL_STRIP_LENGTH+LOWER_DIAGONAL_LENGTH)
#define  SIDE_LONG_HALF_LENGTH  60      // pixels in the 8'+12' "half" of a side or back
#define  SIDE_SHORT_HALF_LENGTH 36      // pixels in the 12' "half" of a side or back
#define  FRONT_SEGMENT_LENGTH   24      // pixels to left or right of doorway
#define  SIDE_LENGTH        (SIDE_LONG_HALF_LENGTH+SIDE_SHORT_HALF_LENGTH)

uint8_t slave_00[] = {
  0,                        // this slave's address
  0x01, 0x00,               // version number, always 0x0001 for now
  0,                        // Caroushell-style strands
  CAROUSHELL_STRIP_LENGTH,  // pixels in strand
  1,                        // number of entities managed by this slave
  // first entity descriptor
  0,                        // entity address, diagonal 0
  DIAGONAL_LENGTH,          // pixels in the entire entity
  LOWER_DIAGONAL_LENGTH,    // entity index of first pixel of this segment
  CAROUSHELL_STRIP_LENGTH,  // number of pixels in this segment
  0,                        // slave index of first pixel of this segment
  0,                        // indexes are not reversed
};

uint8_t slave_01[] = {
  1,                        // this slave's address
  0x01, 0x00,               // version number, always 0x0001 for now
  0,                        // Caroushell-style strands
  CAROUSHELL_STRIP_LENGTH,  // pixels in strand
  1,                        // number of entities managed by this slave
  // first entity descriptor
  1,                        // entity address, diagonal 1
  DIAGONAL_LENGTH,          // pixels in the entire entity
  LOWER_DIAGONAL_LENGTH,    // entity index of first pixel of this segment
  CAROUSHELL_STRIP_LENGTH,  // number of pixels in this segment
  0,                        // slave index of first pixel of this segment
  0,                        // indexes are not reversed
};

uint8_t slave_02[] = {
  2,                        // this slave's address
  0x01, 0x00,               // version number, always 0x0001 for now
  0,                        // Caroushell-style strands
  CAROUSHELL_STRIP_LENGTH,  // pixels in strand
  1,                        // number of entities managed by this slave
  // first entity descriptor
  2,                        // entity address, diagonal 2
  DIAGONAL_LENGTH,          // pixels in the entire entity
  LOWER_DIAGONAL_LENGTH,    // entity index of first pixel of this segment
  CAROUSHELL_STRIP_LENGTH,  // number of pixels in this segment
  0,                        // slave index of first pixel of this segment
  0,                        // indexes are not reversed
};

uint8_t slave_03[] = {
  3,                        // this slave's address
  0x01, 0x00,               // version number, always 0x0001 for now
  0,                        // Caroushell-style strands
  CAROUSHELL_STRIP_LENGTH,  // pixels in strand
  1,                        // number of entities managed by this slave
  // first entity descriptor
  3,                        // entity address, diagonal 3
  DIAGONAL_LENGTH,          // pixels in the entire entity
  LOWER_DIAGONAL_LENGTH,    // entity index of first pixel of this segment
  CAROUSHELL_STRIP_LENGTH,  // number of pixels in this segment
  0,                        // slave index of first pixel of this segment
  0,                        // indexes are not reversed
};

uint8_t slave_04[] = {
  4,                        // this slave's address
  0x01, 0x00,               // version number, always 0x0001 for now
  0,                        // Caroushell-style strands
  CAROUSHELL_STRIP_LENGTH,  // pixels in strand
  1,                        // number of entities managed by this slave
  // first entity descriptor
  4,                        // entity address, diagonal 4
  DIAGONAL_LENGTH,          // pixels in the entire entity
  LOWER_DIAGONAL_LENGTH,    // entity index of first pixel of this segment
  CAROUSHELL_STRIP_LENGTH,  // number of pixels in this segment
  0,                        // slave index of first pixel of this segment
  0,                        // indexes are not reversed
};

uint8_t slave_05[] = {
  5,                        // this slave's address
  0x01, 0x00,               // version number, always 0x0001 for now
  0,                        // Caroushell-style strands
  CAROUSHELL_STRIP_LENGTH,  // pixels in strand
  1,                        // number of entities managed by this slave
  // first entity descriptor
  5,                        // entity address, diagonal 5
  DIAGONAL_LENGTH,          // pixels in the entire entity
  LOWER_DIAGONAL_LENGTH,    // entity index of first pixel of this segment
  CAROUSHELL_STRIP_LENGTH,  // number of pixels in this segment
  0,                        // slave index of first pixel of this segment
  0,                        // indexes are not reversed
};

uint8_t slave_06[] = {
  6,                        // this slave's address
  0x01, 0x00,               // version number, always 0x0001 for now
  0,                        // Caroushell-style strands
  CAROUSHELL_STRIP_LENGTH,  // pixels in strand
  1,                        // number of entities managed by this slave
  // first entity descriptor
  6,                        // entity address, diagonal 6
  DIAGONAL_LENGTH,          // pixels in the entire entity
  LOWER_DIAGONAL_LENGTH,    // entity index of first pixel of this segment
  CAROUSHELL_STRIP_LENGTH,  // number of pixels in this segment
  0,                        // slave index of first pixel of this segment
  0,                        // indexes are not reversed
};

uint8_t slave_07[] = {
  7,                        // this slave's address
  0x01, 0x00,               // version number, always 0x0001 for now
  0,                        // Caroushell-style strands
  CAROUSHELL_STRIP_LENGTH,  // pixels in strand
  1,                        // number of entities managed by this slave
  // first entity descriptor
  7,                        // entity address, diagonal 7
  DIAGONAL_LENGTH,          // pixels in the entire entity
  LOWER_DIAGONAL_LENGTH,    // entity index of first pixel of this segment
  CAROUSHELL_STRIP_LENGTH,  // number of pixels in this segment
  0,                        // slave index of first pixel of this segment
  0,                        // indexes are not reversed
};

uint8_t slave_08[] = {
  8,                                            // this slave's address
  0x01, 0x00,                                   // version number, always 0x0001 for now
  1,                                            // Starfish-style strands
  LOWER_DIAGONAL_LENGTH+FRONT_SEGMENT_LENGTH,   // pixels in strand
  2,                                            // number of entities managed by this slave
  // first entity descriptor
  0,                        // entity address, diagonal 0
  DIAGONAL_LENGTH,          // pixels in the entire entity
  0,                        // entity index of first pixel of this segment
  LOWER_DIAGONAL_LENGTH,    // number of pixels in this segment
  LOWER_DIAGONAL_LENGTH-1,  // slave index of first pixel of this segment
  1,                        // indexes are reversed
  // second entity descriptor
  11,                                           // entity address, front edge
  SIDE_LENGTH,                                  // pixels in the entire entity
  SIDE_LENGTH-FRONT_SEGMENT_LENGTH-1,           // entity index of first pixel of this segment
  FRONT_SEGMENT_LENGTH,                         // number of pixels in this segment
  LOWER_DIAGONAL_LENGTH+FRONT_SEGMENT_LENGTH-1, // slave index of first pixel of this segment
  1,                                            // indexes are reversed
};

uint8_t slave_09[] = {
  9,                                            // this slave's address
  0x01, 0x00,                                   // version number, always 0x0001 for now
  1,                                            // Starfish-style strands
  LOWER_DIAGONAL_LENGTH+SIDE_LONG_HALF_LENGTH,  // pixels in strand
  2,                                            // number of entities managed by this slave
  // first entity descriptor
  1,                        // entity address, diagonal 1
  DIAGONAL_LENGTH,          // pixels in the entire entity
  0,                        // entity index of first pixel of this segment
  LOWER_DIAGONAL_LENGTH,    // number of pixels in this segment
  LOWER_DIAGONAL_LENGTH-1,  // slave index of first pixel of this segment
  1,                        // indexes are reversed
  // second entity descriptor
  8,                        // entity address, left edge
  SIDE_LENGTH,              // pixels in the entire entity
  0,                        // entity index of first pixel of this segment
  SIDE_LONG_HALF_LENGTH,    // number of pixels in this segment
  LOWER_DIAGONAL_LENGTH,    // slave index of first pixel of this segment
  0,                        // indexes are not reversed
};

uint8_t slave_10[] = {
  10,                                           // this slave's address
  0x01, 0x00,                                   // version number, always 0x0001 for now
  1,                                            // Starfish-style strands
  LOWER_DIAGONAL_LENGTH+SIDE_SHORT_HALF_LENGTH, // pixels in strand
  2,                                            // number of entities managed by this slave
  // first entity descriptor
  2,                        // entity address, diagonal 2
  DIAGONAL_LENGTH,          // pixels in the entire entity
  0,                        // entity index of first pixel of this segment
  LOWER_DIAGONAL_LENGTH,    // number of pixels in this segment
  LOWER_DIAGONAL_LENGTH-1,  // slave index of first pixel of this segment
  1,                        // indexes are reversed
  // second entity descriptor
  8,                                                // entity address, left edge
  SIDE_LENGTH,                                      // pixels in the entire entity
  SIDE_LONG_HALF_LENGTH,                            // entity index of first pixel of this segment
  SIDE_SHORT_HALF_LENGTH,                           // number of pixels in this segment
  LOWER_DIAGONAL_LENGTH+SIDE_SHORT_HALF_LENGTH-1,   // slave index of first pixel of this segment
  1,                                                // indexes are reversed
};

uint8_t slave_11[] = {
  11,                                           // this slave's address
  0x01, 0x00,                                   // version number, always 0x0001 for now
  1,                                            // Starfish-style strands
  LOWER_DIAGONAL_LENGTH+SIDE_LONG_HALF_LENGTH,  // pixels in strand
  2,                                            // number of entities managed by this slave
  // first entity descriptor
  3,                        // entity address, diagonal 3
  DIAGONAL_LENGTH,          // pixels in the entire entity
  0,                        // entity index of first pixel of this segment
  LOWER_DIAGONAL_LENGTH,    // number of pixels in this segment
  LOWER_DIAGONAL_LENGTH-1,  // slave index of first pixel of this segment
  1,                        // indexes are reversed
  // second entity descriptor
  9,                        // entity address, back edge
  SIDE_LENGTH,              // pixels in the entire entity
  0,                        // entity index of first pixel of this segment
  SIDE_LONG_HALF_LENGTH,    // number of pixels in this segment
  LOWER_DIAGONAL_LENGTH,    // slave index of first pixel of this segment
  0,                        // indexes are not reversed
};

uint8_t slave_12[] = {
  12,                                           // this slave's address
  0x01, 0x00,                                   // version number, always 0x0001 for now
  1,                                            // Starfish-style strands
  LOWER_DIAGONAL_LENGTH+SIDE_SHORT_HALF_LENGTH, // pixels in strand
  2,                                            // number of entities managed by this slave
  // first entity descriptor
  4,                        // entity address, diagonal 4
  DIAGONAL_LENGTH,          // pixels in the entire entity
  0,                        // entity index of first pixel of this segment
  LOWER_DIAGONAL_LENGTH,    // number of pixels in this segment
  LOWER_DIAGONAL_LENGTH-1,  // slave index of first pixel of this segment
  1,                        // indexes are reversed
  // second entity descriptor
  9,                                                // entity address, back edge
  SIDE_LENGTH,                                      // pixels in the entire entity
  SIDE_LONG_HALF_LENGTH,                            // entity index of first pixel of this segment
  SIDE_SHORT_HALF_LENGTH,                           // number of pixels in this segment
  LOWER_DIAGONAL_LENGTH+SIDE_SHORT_HALF_LENGTH-1,   // slave index of first pixel of this segment
  1,                                                // indexes are reversed
};

uint8_t slave_13[] = {
  13,                                           // this slave's address
  0x01, 0x00,                                   // version number, always 0x0001 for now
  1,                                            // Starfish-style strands
  LOWER_DIAGONAL_LENGTH+SIDE_LONG_HALF_LENGTH,  // pixels in strand
  2,                                            // number of entities managed by this slave
  // first entity descriptor
  5,                        // entity address, diagonal 5
  DIAGONAL_LENGTH,          // pixels in the entire entity
  0,                        // entity index of first pixel of this segment
  LOWER_DIAGONAL_LENGTH,    // number of pixels in this segment
  LOWER_DIAGONAL_LENGTH-1,  // slave index of first pixel of this segment
  1,                        // indexes are reversed
  // second entity descriptor
  10,                       // entity address, right edge
  SIDE_LENGTH,              // pixels in the entire entity
  0,                        // entity index of first pixel of this segment
  SIDE_LONG_HALF_LENGTH,    // number of pixels in this segment
  LOWER_DIAGONAL_LENGTH,    // slave index of first pixel of this segment
  0,                        // indexes are not reversed
};

uint8_t slave_14[] = {
  14,                                           // this slave's address
  0x01, 0x00,                                   // version number, always 0x0001 for now
  1,                                            // Starfish-style strands
  LOWER_DIAGONAL_LENGTH+SIDE_SHORT_HALF_LENGTH, // pixels in strand
  2,                                            // number of entities managed by this slave
  // first entity descriptor
  6,                        // entity address, diagonal 6
  DIAGONAL_LENGTH,          // pixels in the entire entity
  0,                        // entity index of first pixel of this segment
  LOWER_DIAGONAL_LENGTH,    // number of pixels in this segment
  LOWER_DIAGONAL_LENGTH-1,  // slave index of first pixel of this segment
  1,                        // indexes are reversed
  // second entity descriptor
  10,                                               // entity address, right edge
  SIDE_LENGTH,                                      // pixels in the entire entity
  SIDE_LONG_HALF_LENGTH,                            // entity index of first pixel of this segment
  SIDE_SHORT_HALF_LENGTH,                           // number of pixels in this segment
  LOWER_DIAGONAL_LENGTH+SIDE_SHORT_HALF_LENGTH-1,   // slave index of first pixel of this segment
  1,                                                // indexes are reversed
};

uint8_t slave_15[] = {
  15,                                           // this slave's address
  0x01, 0x00,                                   // version number, always 0x0001 for now
  1,                                            // Starfish-style strands
  LOWER_DIAGONAL_LENGTH+FRONT_SEGMENT_LENGTH,   // pixels in strand
  2,                                            // number of entities managed by this slave
  // first entity descriptor
  7,                        // entity address, diagonal 7
  DIAGONAL_LENGTH,          // pixels in the entire entity
  0,                        // entity index of first pixel of this segment
  LOWER_DIAGONAL_LENGTH,    // number of pixels in this segment
  LOWER_DIAGONAL_LENGTH-1,  // slave index of first pixel of this segment
  1,                        // indexes are reversed
  // second entity descriptor
  11,                       // entity address, front edge
  SIDE_LENGTH,              // pixels in the entire entity
  0,                        // entity index of first pixel of this segment
  FRONT_SEGMENT_LENGTH,     // number of pixels in this segment
  LOWER_DIAGONAL_LENGTH,    // slave index of first pixel of this segment
  0,                        // indexes are not reversed
};


uint8_t *slave_config[NUM_SLAVES] = {
  slave_00,
  slave_01,
  slave_02,
  slave_03,
  slave_04,
  slave_05,
  slave_06,
  slave_07,
  slave_08,
  slave_09,
  slave_10,
  slave_11,
  slave_12,
  slave_13,
  slave_14,
  slave_15
};

unsigned int slave_config_length[NUM_SLAVES] = {
  sizeof(slave_00),
  sizeof(slave_01),
  sizeof(slave_02),
  sizeof(slave_03),
  sizeof(slave_04),
  sizeof(slave_05),
  sizeof(slave_06),
  sizeof(slave_07),
  sizeof(slave_08),
  sizeof(slave_09),
  sizeof(slave_10),
  sizeof(slave_11),
  sizeof(slave_12),
  sizeof(slave_13),
  sizeof(slave_14),
  sizeof(slave_15)
};


void setup() {
  Serial.begin(115200);
  
  Serial.println("Pyramid slave config");
  Serial.println("");
  Serial.println("? for help");
  
  old_address = EEPROM.read(0);
  if (old_address != 255) {
    address = old_address;
    bitmap = 1 << address;
  }
  old_version = EEPROM.read(1) * 256 + EEPROM.read(2);
    
  

}

void loop() {
  int key, digit, i, errors;
  
  if (Serial.available() > 0) {
    key = Serial.read();
    
    switch (key) {
      case 'C':
        if (old_address != 255) {
          Serial.print("Address in EEPROM is ");
          Serial.println(old_address);
          Serial.print("New address will be  ");
          Serial.println(address);
          Serial.print("Version is 0x");
          Serial.println(old_version, HEX);
        }
        break;
      
      case 'A':
        Serial.print("Set address to: ");
        while (!Serial.available());
        digit = Serial.read();
        if (!isdigit(digit)) {
          Serial.println("\nYou must enter two decimal digits");
          break;
        }
        else {
          Serial.write(digit);  //echo
        }
        address = 10 * (digit - '0');
        while (!Serial.available());
        digit = Serial.read();
        if (!isdigit(digit)) {
          Serial.println("\nYou must enter two decimal digits");
          break;
        }
        else {
          Serial.write(digit);  //echo
        }
        address += (digit - '0');
        bitmap = 1 << address;
        Serial.print("\nUsing address ");
        Serial.print(address);
        Serial.print(", bitmap = 0x");
        Serial.println(bitmap, HEX);
        break;
        
      case 'W':
        if (address >= NUM_SLAVES) {
          Serial.println("Address not set!");
          break;
        }
        for (i=0; i < slave_config_length[address]; i++) {
          EEPROM.write(i, slave_config[address][i]);
        }
        Serial.print("Configuration written for address ");
        Serial.println(address);        
        break;
      
      case 'V':
         if (address >= NUM_SLAVES) {
          Serial.println("Address not set!");
          break;
        }
        errors = 0;
        for (i=0; i < slave_config_length[address]; i++) {
          if (EEPROM.read(i) != slave_config[address][i]) {
            errors++;
        }
        if (errors != 0) {
          Serial.println("Configuration does not match!");
          Serial.print(errors);
          Serial.println(" bytes in error.");
        }
        else {
          Serial.println("Configuration verified!");
        }
      
        break;
        
      default:
        Serial.println("C to check current address");
        Serial.println("Axx to set the address to xx (decimal)");
        Serial.println("W to write configuration");
        Serial.println("V to verify configuration");
        break;
      }
    }
  }
}

