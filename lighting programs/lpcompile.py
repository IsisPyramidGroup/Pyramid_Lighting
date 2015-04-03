#! /usr/bin/python

# This program serves as a compiler of sorts for lighting programs for the Isis Pyramid 1.1.
# The designer of a lighting program figures out what command packets need to be sent
# to the slave processors by the master processor, and codes them up in a file using 
# set of Python functions defined herein. The source filename is fed to this program on
# the command line. This program sets up all the support functions, and then executes
# the source file as Python code. That means arbitrary Python code can exist in the
# lighting program file, if that's convenient, but typically the lighting program will
# consist mostly of the predefined function calls. It also means that diagnostic error
# messages will be potentially-cryptic Python errors, which isn't ideal for designers
# who are not also Python programmers.
#
# 2015-03-26 ptw

import sys

ALL = 0xFFFF        # all-call address for either slaves or entities
DIAGS = 0x00FF      # address for all the diagonals
SIDES = 0x0F00      # address for all the bottom edges

# Entity names
E_DIAG_0 = 0x0001
E_DIAG_1 = 0x0002
E_DIAG_2 = 0x0004
E_DIAG_3 = 0x0008
E_DIAG_4 = 0x0010
E_DIAG_5 = 0x0020
E_DIAG_6 = 0x0040
E_DIAG_7 = 0x0080
E_LEFT   = 0x0100
E_BACK   = 0x0200
E_RIGHT  = 0x0400
E_FRONT  = 0x0800

# Slave packet command codes
CMD_S_RESET_CLOCK = 0x00
CMD_S_DYN_BLINK   = 0x01
CMD_S_DYN_THROB   = 0x02
CMD_S_DYN_SPARKLE = 0x03
CMD_S_COMMENT	  = 0x04

# Entity packet command codes
CMD_E_FILL_RGB      = 0xC0
CMD_E_FILL_D        = 0xC1
CMD_E_SHIFT_UP      = 0xC2
CMD_E_SHIFT_DOWN    = 0xC3
CMD_E_ROTATE        = 0xC4
CMD_E_RANDOMIZE     = 0xC5
CMD_E_LOADONE       = 0xC6
CMD_E_RAINBOW       = 0xC7
  
META = 0xFF         # special command code that is interpreted locally and not sent
META_CONSOLE = 0x00     # subcommands of the META command code
META_WAIT    = 0x01
META_ENDS    = 0x02
META_RESET_TIME = 0x03

# special codes for byte stuffing, per KISS or SLIP protocol standards.
FEND = 0xC0
FESC = 0xDB
TFEND = 0xDC
TFESC = 0xDD

def LO(val):
    return val & 0xff

def HI(val):
    return (val >> 8) & 0xff

def filename(name):
    """Create a PKT file with the specified 8.3 filename
    """
    global outfile
    outfile = open(name, 'w')

def console(value):
    """Insert a meta packet instructing the master to display a value on the console.
    """
    write_packet([META, META_CONSOLE, LO(value), HI(value)])

def wait_for_tick(tick):
    """Insert a meta packet instructing the master to wait for a certain tick number
    to come around before proceeding to send the following packets.
    """
    write_packet([META, META_WAIT, LO(tick), HI(tick)])
    
def ends_at_tick(tick):
    """Insert a meta packet instructing the master that the current program, if run to
    completion, will end when the tick number reaches a certain value.
    """
    write_packet([META, META_ENDS, LO(tick), HI(tick)])

def reset_master_clock():
	"""Insert a meta packet instructing the master to reset its own version of time
	at this point in the program.
	"""
	write_packet([META, META_RESET_TIME])
    
def write_packet(bytes):
    """Write a list of bytes into the output file, with SLIP framing.
    """    
    # SLIP framing
    pkt = [FEND]
    for b in bytes:
        if b == FEND:
            pkt.append(FESC)
            pkt.append(TFEND)
        elif b == FESC:
            pkt.append(FESC)
            pkt.append(TFESC)
        else:
            pkt.append(b)
    pkt.append(FEND)            
    
    outfile.write(bytearray(pkt))

def cmd_s_reset_clock(bitmap):
    """Insert a RESET_CLOCK packet addressed to the slaves shown in bitmap.
    """
    write_packet([CMD_S_RESET_CLOCK, LO(bitmap), HI(bitmap)])
    
def cmd_s_dyn_blink(bitmap, period, ontime, dimming):
    """Insert a DYN_BLINK packet addressed to the slaves shown in bitmap.
    """
    write_packet([CMD_S_DYN_BLINK, LO(bitmap), HI(bitmap), LO(period), HI(period),
                  LO(ontime), HI(ontime), dimming])

def cmd_s_dyn_throb(bitmap, period, ramptime, bright, dim):
    """Insert a DYN_THROB packet addressed to the slaves shown in bitmap.
    """
    write_packet([CMD_S_DYN_THROB, LO(bitmap), HI(bitmap), LO(period), HI(period),
                  LO(ramptime), HI(ramptime), bright, dim])

def cmd_s_dyn_sparkle(bitmap, probability):
    """Insert a DYN_SPARKLE packet addressed to the slaves shown in bitmap.
    """
    write_packet([CMD_S_DYN_SPARKLE, LO(bitmap), HI(bitmap), probability])

def comment(string):
	"""Insert a comment packet, nominally addressed to all slaves. This does nothing
	at the slave, but inserts some readable text into the bitstream for debug.
	"""
	packet = [CMD_S_COMMENT, 255, 255]
	packet += string[:11]
	write_packet(packet)

def cmd_e_fill_rgb(bitmap, repeat_count, start_tick, repeat_interval, red, green, blue):
    """Insert a CMD_E_FILL_RGB packet with the specified contents.
    """
    write_packet([CMD_E_FILL_RGB, LO(bitmap), HI(bitmap), repeat_count, LO(start_tick), HI(start_tick),
                  LO(repeat_interval), HI(repeat_interval), red, green, blue])

def cmd_e_fill_d(bitmap, repeat_count, start_tick, repeat_interval, dynamics):
    """Insert a CMD_E_FILL_D packet with the specified contents.
    """
    write_packet([CMD_E_FILL_D, LO(bitmap), HI(bitmap), repeat_count, LO(start_tick), HI(start_tick),
                  LO(repeat_interval), HI(repeat_interval), dynamics])
    

def cmd_e_shift_up(bitmap, repeat_count, start_tick, repeat_interval, count, red, green, blue, dynamics):
    """Insert a CMD_E_SHIFT_IP packet with the specified contents.
    """
    write_packet([CMD_E_SHIFT_UP, LO(bitmap), HI(bitmap), repeat_count, LO(start_tick), HI(start_tick),
                  LO(repeat_interval), HI(repeat_interval), count, red, green, blue, dynamics])
                  
def cmd_e_shift_down(bitmap, repeat_count, start_tick, repeat_interval, count, red, green, blue, dynamics):
    """Insert a CMD_E_SHIFT_DOWN packet with the specified contents.
    """
    write_packet([CMD_E_SHIFT_DOWN, LO(bitmap), HI(bitmap), repeat_count, LO(start_tick), HI(start_tick),
                  LO(repeat_interval), HI(repeat_interval), count, red, green, blue, dynamics])
                  
def cmd_e_rotate(bitmap, repeat_count, start_tick, repeat_interval, count, down):
    """Insert a CMD_E_ROTATE packet with the specified contents.
    """
    write_packet([CMD_E_ROTATE, LO(bitmap), HI(bitmap), repeat_count, LO(start_tick), HI(start_tick),
                  LO(repeat_interval), HI(repeat_interval), count, down])
                  
def cmd_e_randomize(bitmap, repeat_count, start_tick, repeat_interval):
    """Insert a CMD_E_RANDOMIZE packet with the specified contents.
    """
    write_packet([CMD_E_RANDOMIZE, LO(bitmap), HI(bitmap), repeat_count, LO(start_tick), HI(start_tick),
                  LO(repeat_interval), HI(repeat_interval)])
                  
def cmd_e_loadone(bitmap, repeat_count, start_tick, repeat_interval, index, red, green, blue, dynamics):
    """Insert a CMD_E_LOADONE packet with the specified contents.
    """
    write_packet([CMD_E_LOADONE, LO(bitmap), HI(bitmap), repeat_count, LO(start_tick), HI(start_tick),
                  LO(repeat_interval), HI(repeat_interval), index, red, green, blue, dynamics]);
                  
def cmd_e_rainbow(bitmap, repeat_count, start_tick, repeat_interval, start, incr, dir):
    """Insert a CMD_E_RAINBOW packet with the specified contents.
    """
    write_packet([CMD_E_RAINBOW, LO(bitmap), HI(bitmap), repeat_count, LO(start_tick), HI(start_tick),
                  LO(repeat_interval), HI(repeat_interval), start, incr, dir]);


if len(sys.argv) != 2:
    print "Isis Pyramid Packet Compiler 0.01"
    print "  Usage: lpcompile infile"
    sys.exit(1)

execfile(sys.argv[1])

outfile.close()
