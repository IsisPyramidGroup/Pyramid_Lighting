// Isis Pyramid 1.1 Packet Definitions

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

// Packet command code reserved for use in the canned packet file format.
#define	 CMD_META		   0xFF


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
                                          
#define  PKT_ADDRESS_ALL_CALL        0xFFFF  // address all slaves or all entities

// Offsets into the packet format for parsing meta packets in the canned packet file.
#define	 PKT_META_CMD_OFFSET	     1
#define  PKT_META_DATA_OFFSET        2  

// The following meta commands are defined:
#define	META_CONSOLE	0x00
#define	META_WAIT	0x01
#define	META_ENDS	0x02


// The following types of dynamics are defined:
#define DYNAMICS_BLINK    0x01    // hard on/dim at some duty cycle and rate
#define DYNAMICS_THROB    0x02    // soft brightness modulation at some duty cycle and rate
#define DYNAMICS_SPARKLE  0x04    // full-on for one tick with probability P
//etc. TBD

// Special codes for byte stuffing. Per KISS or SLIP protocol standards.
#define	FEND	0xC0
#define FESC	0xDB
#define TFEND	0xDC
#define	TFESC	0xDD
