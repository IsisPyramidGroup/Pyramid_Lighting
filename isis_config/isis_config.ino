// Program to write configuration info into slave EEPROM memory
// for Isis Pyramid 1.1 configuration (Lucidity 2015)
// Uses the serial console interactively (at 115200).
//
// The configuration is coded into tables for all slaves; this is the one
// authoritative copy of the entity configuration for the Pyramid.
// The interactive user chooses which slave (by address) is to be programmed.
//
// 2015-03-18 ptw

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

#define  EDGE_LENGTH  72           // total pixels of a complete bottom edge
#define  FRONT_SEGMENT_LENGTH  25  // pixels to left or right of doorway

uint8_t slave_00[] = {
  0,             // this slave's address
  0x00, 0x01,    // version number, always 0x0001 for now
  0,             // Caroushell-style strands
  50,            // pixels in strand
  1,             // number of entities managed by this slave
  // first entity descriptor
  0,             // entity address, diagonal 0
  68,            // pixels in the entire entity
  18,            // entity index of first pixel of this segment
  50,            // number of pixels in this segment
  0,             // slave index of first pixel of this segment
  0,             // indexes are not reversed
};

uint8_t slave_01[] = {
  1,             // this slave's address
  0x00, 0x01,    // version number, always 0x0001 for now
  0,             // Caroushell-style strands
  50,            // pixels in strand
  1,             // number of entities managed by this slave
  // first entity descriptor
  1,             // entity address, diagonal 1
  68,            // pixels in the entire entity
  18,            // entity index of first pixel of this segment
  50,            // number of pixels in this segment
  0,             // slave index of first pixel of this segment
  0,             // indexes are not reversed
};

uint8_t slave_02[] = {
  2,             // this slave's address
  0x00, 0x01,    // version number, always 0x0001 for now
  0,             // Caroushell-style strands
  50,            // pixels in strand
  1,             // number of entities managed by this slave
  // first entity descriptor
  2,             // entity address, diagonal 2
  68,            // pixels in the entire entity
  18,            // entity index of first pixel of this segment
  50,            // number of pixels in this segment
  0,             // slave index of first pixel of this segment
  0,             // indexes are not reversed
};

uint8_t slave_03[] = {
  3,             // this slave's address
  0x00, 0x01,    // version number, always 0x0001 for now
  0,             // Caroushell-style strands
  50,            // pixels in strand
  1,             // number of entities managed by this slave
  // first entity descriptor
  3,             // entity address, diagonal 3
  68,            // pixels in the entire entity
  18,            // entity index of first pixel of this segment
  50,            // number of pixels in this segment
  0,             // slave index of first pixel of this segment
  0,             // indexes are not reversed
};

uint8_t slave_04[] = {
  4,             // this slave's address
  0x00, 0x01,    // version number, always 0x0001 for now
  0,             // Caroushell-style strands
  50,            // pixels in strand
  1,             // number of entities managed by this slave
  // first entity descriptor
  4,             // entity address, diagonal 4
  68,            // pixels in the entire entity
  18,            // entity index of first pixel of this segment
  50,            // number of pixels in this segment
  0,             // slave index of first pixel of this segment
  0,             // indexes are not reversed
};

uint8_t slave_05[] = {
  5,             // this slave's address
  0x00, 0x01,    // version number, always 0x0001 for now
  0,             // Caroushell-style strands
  50,            // pixels in strand
  1,             // number of entities managed by this slave
  // first entity descriptor
  5,             // entity address, diagonal 5
  68,            // pixels in the entire entity
  18,            // entity index of first pixel of this segment
  50,            // number of pixels in this segment
  0,             // slave index of first pixel of this segment
  0,             // indexes are not reversed
};

uint8_t slave_06[] = {
  6,             // this slave's address
  0x00, 0x01,    // version number, always 0x0001 for now
  0,             // Caroushell-style strands
  50,            // pixels in strand
  1,             // number of entities managed by this slave
  // first entity descriptor
  6,             // entity address, diagonal 6
  68,            // pixels in the entire entity
  18,            // entity index of first pixel of this segment
  50,            // number of pixels in this segment
  0,             // slave index of first pixel of this segment
  0,             // indexes are not reversed
};

uint8_t slave_07[] = {
  7,             // this slave's address
  0x00, 0x01,    // version number, always 0x0001 for now
  0,             // Caroushell-style strands
  50,            // pixels in strand
  1,             // number of entities managed by this slave
  // first entity descriptor
  7,             // entity address, diagonal 7
  68,            // pixels in the entire entity
  18,            // entity index of first pixel of this segment
  50,            // number of pixels in this segment
  0,             // slave index of first pixel of this segment
  0,             // indexes are not reversed
};

uint8_t slave_08[] = {
  8,             // this slave's address
  0x00, 0x01,    // version number, always 0x0001 for now
  1,             // Starfish-style strands
  18+FRONT_SEGMENT_LENGTH, // pixels in strand
  2,             // number of entities managed by this slave
  // first entity descriptor
  0,             // entity address, diagonal 0
  68,            // pixels in the entire entity
  0,             // entity index of first pixel of this segment
  18,            // number of pixels in this segment
  17,            // slave index of first pixel of this segment
  1,             // indexes are reversed
  // second entity descriptor
  11,            // entity address, front edge
  EDGE_LENGTH,   // pixels in the entire entity
  EDGE_LENGTH-FRONT_SEGMENT_LENGTH-1,  // entity index of first pixel of this segment
  FRONT_SEGMENT_LENGTH,  // number of pixels in this segment
  18+FRONT_SEGMENT_LENGTH,  // slave index of first pixel of this segment
  1,             // indexes are reversed
};

uint8_t slave_09[] = {
  9,             // this slave's address
  0x00, 0x01,    // version number, always 0x0001 for now
  1,             // Starfish-style strands
  18+EDGE_LENGTH/2, // pixels in strand
  2,             // number of entities managed by this slave
  // first entity descriptor
  1,             // entity address, diagonal 1
  68,            // pixels in the entire entity
  0,             // entity index of first pixel of this segment
  18,            // number of pixels in this segment
  17,            // slave index of first pixel of this segment
  1,             // indexes are reversed
  // second entity descriptor
  8,             // entity address, left edge
  EDGE_LENGTH,   // pixels in the entire entity
  0,             // entity index of first pixel of this segment
  EDGE_LENGTH/2, // number of pixels in this segment
  18,            // slave index of first pixel of this segment
  0,             // indexes are not reversed
};

uint8_t slave_10[] = {
  10,            // this slave's address
  0x00, 0x01,    // version number, always 0x0001 for now
  1,             // Starfish-style strands
  18+EDGE_LENGTH/2, // pixels in strand
  2,             // number of entities managed by this slave
  // first entity descriptor
  2,             // entity address, diagonal 2
  68,            // pixels in the entire entity
  0,             // entity index of first pixel of this segment
  18,            // number of pixels in this segment
  17,            // slave index of first pixel of this segment
  1,             // indexes are reversed
  // second entity descriptor
  8,             // entity address, left edge
  EDGE_LENGTH,   // pixels in the entire entity
  EDGE_LENGTH/2, // entity index of first pixel of this segment
  EDGE_LENGTH/2, // number of pixels in this segment
  18+EDGE_LENGTH/2,  // slave index of first pixel of this segment
  1,             // indexes are reversed
};

uint8_t slave_11[] = {
  11,            // this slave's address
  0x00, 0x01,    // version number, always 0x0001 for now
  1,             // Starfish-style strands
  18+EDGE_LENGTH/2, // pixels in strand
  2,             // number of entities managed by this slave
  // first entity descriptor
  3,             // entity address, diagonal 3
  68,            // pixels in the entire entity
  0,             // entity index of first pixel of this segment
  18,            // number of pixels in this segment
  17,            // slave index of first pixel of this segment
  1,             // indexes are reversed
  // second entity descriptor
  9,             // entity address, back edge
  EDGE_LENGTH,   // pixels in the entire entity
  0,             // entity index of first pixel of this segment
  EDGE_LENGTH/2, // number of pixels in this segment
  18,            // slave index of first pixel of this segment
  0,             // indexes are not reversed
};

uint8_t slave_12[] = {
  12,            // this slave's address
  0x00, 0x01,    // version number, always 0x0001 for now
  1,             // Starfish-style strands
  18+EDGE_LENGTH/2, // pixels in strand
  2,             // number of entities managed by this slave
  // first entity descriptor
  4,             // entity address, diagonal 4
  68,            // pixels in the entire entity
  0,             // entity index of first pixel of this segment
  18,            // number of pixels in this segment
  17,            // slave index of first pixel of this segment
  1,             // indexes are reversed
  // second entity descriptor
  9,             // entity address, back edge
  EDGE_LENGTH,   // pixels in the entire entity
  EDGE_LENGTH/2, // entity index of first pixel of this segment
  EDGE_LENGTH/2, // number of pixels in this segment
  18+EDGE_LENGTH/2,  // slave index of first pixel of this segment
  1,             // indexes are reversed
};

uint8_t slave_13[] = {
  13,            // this slave's address
  0x00, 0x01,    // version number, always 0x0001 for now
  1,             // Starfish-style strands
  18+EDGE_LENGTH/2, // pixels in strand
  2,             // number of entities managed by this slave
  // first entity descriptor
  5,             // entity address, diagonal 5
  68,            // pixels in the entire entity
  0,             // entity index of first pixel of this segment
  18,            // number of pixels in this segment
  17,            // slave index of first pixel of this segment
  1,             // indexes are reversed
  // second entity descriptor
  10,            // entity address, right edge
  EDGE_LENGTH,   // pixels in the entire entity
  0,             // entity index of first pixel of this segment
  EDGE_LENGTH/2, // number of pixels in this segment
  18,            // slave index of first pixel of this segment
  0,             // indexes are not reversed
};

uint8_t slave_14[] = {
  14,            // this slave's address
  0x00, 0x01,    // version number, always 0x0001 for now
  1,             // Starfish-style strands
  18+EDGE_LENGTH/2, // pixels in strand
  2,             // number of entities managed by this slave
  // first entity descriptor
  6,             // entity address, diagonal 6
  68,            // pixels in the entire entity
  0,             // entity index of first pixel of this segment
  18,            // number of pixels in this segment
  17,            // slave index of first pixel of this segment
  1,             // indexes are reversed
  // second entity descriptor
  10,            // entity address, right edge
  EDGE_LENGTH,   // pixels in the entire entity
  EDGE_LENGTH/2, // entity index of first pixel of this segment
  EDGE_LENGTH/2, // number of pixels in this segment
  18+EDGE_LENGTH/2,  // slave index of first pixel of this segment
  1,             // indexes are reversed
};

uint8_t slave_15[] = {
  15,            // this slave's address
  0x00, 0x01,    // version number, always 0x0001 for now
  1,             // Starfish-style strands
  18+FRONT_SEGMENT_LENGTH, // pixels in strand
  2,             // number of entities managed by this slave
  // first entity descriptor
  7,             // entity address, diagonal 7
  68,            // pixels in the entire entity
  0,             // entity index of first pixel of this segment
  18,            // number of pixels in this segment
  17,            // slave index of first pixel of this segment
  1,             // indexes are reversed
  // second entity descriptor
  11,            // entity address, front edge
  EDGE_LENGTH,   // pixels in the entire entity
  0,             // entity index of first pixel of this segment
  FRONT_SEGMENT_LENGTH,  // number of pixels in this segment
  18+FRONT_SEGMENT_LENGTH,  // slave index of first pixel of this segment
  0,             // indexes are not reversed
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

