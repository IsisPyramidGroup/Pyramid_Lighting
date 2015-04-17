#!/usr/bin/python
import sys

# Special codes for byte stuffing. Per KISS or SLIP protocol standards.
FEND =	0xC0
FESC =	0xDB
TFEND =	0xDC
TFESC =	0xDD


if len(sys.argv) != 3:
  print "Text to SLIP-frames converter"
  print "  Usage: slipframe infile outfile"
  sys.exit(1)
  
f = open(sys.argv[1], 'r')
fo = open(sys.argv[2], 'wb')

lines = f.readlines()
for line in lines:
  packet = bytearray([FEND])
  for c in line:
    if c == FEND:
      packet.append(FESC)
      packet.append(TFEND)
    elif c == FESC:
      packet.append(FESC)
      packet.append(TFESC)
    else:
      packet.append(c)
  packet.append(FEND)
  fo.write(packet)