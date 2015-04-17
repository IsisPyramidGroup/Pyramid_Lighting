# Starts with whatever is already in the array, then flows down random colors.
import random

filename("XFLOW.PKT")
comment("Rnd Flow")
cmd_s_reset_clock(ALL)
reset_master_clock()

random.seed(1234)			# yes we want this to be the same every time.

interval = 7


# copied from the Wheel function in the Arduino code
# Input a value 0 to 255 to get a color value.
# The colours are a transition r - g -b - back to r
def cRed(val):
  if val < 85:
  	return val * 3
  elif val < 170:
    val -= 85
    return 255 - val * 3
  else:
    val -= 170
    return 0

def cGreen(val):
  if val < 85:
  	return 255 - val * 3
  elif val < 170:
    return 0
  else:
    val -= 170
    return val * 3

def cBlue(val):
  if val < 85:
    return 0
  elif val < 170:
    val -= 85
    return val * 3
  else:
    val -= 170
    return 255 - val * 3
    
cmd_e_fill_rgb(SIDES, 1,68*interval,0, 200,200,200)		# white when first drop lands

for rain in range(100):
  wait_for_tick(rain*21*interval)
  x = random.randrange(256)
  cmd_e_shift_down(DIAGS, 21, rain*21*interval, interval, 1, cRed(x),cGreen(x),cBlue(x), 0)

ends_at_tick(100*21*interval)
