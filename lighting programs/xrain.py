# Starts with whatever is already in the array, then rains down random colors.
import random

filename("XRAIN.PKT")
comment("Rnd Rain")
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

for rain in range(25):
  wait_for_tick(rain*21*interval)
  x = random.randrange(256)
  cmd_e_shift_down(DIAGS,  3, rain*21*interval, interval, 1, cRed(x),cGreen(x),cBlue(x), 0)		# random
  cmd_e_shift_down(DIAGS, 18, 3*interval+rain*21*interval, interval, 1, 0,0,0, 0)			# black
  cmd_e_fill_rgb(SIDES, 1, rain*21*interval + 70*interval, 0, cRed(x), cGreen(x), cBlue(x))	# hit bottom!
wait_for_tick(25*21*interval)

org = 25*21*interval;

for rain in range(15):
  wait_for_tick(rain*21*interval)
  x = random.randrange(256)
  cmd_e_shift_down(DIAGS,  6, org+rain*21*interval, interval, 1, cRed(x),cGreen(x),cBlue(x), 0)			# random
  cmd_e_shift_down(DIAGS, 15, org+6*interval+rain*21*interval, interval, 1, 0,0,0, 0)			# black
  cmd_e_fill_rgb(SIDES, 1, org+rain*21*interval + 70*interval, 0, cRed(x), cGreen(x), cBlue(x))	# hit bottom!
wait_for_tick(15*21*interval)

org += 15*21*interval

for rain in range(15):
  wait_for_tick(rain*21*interval)
  x = random.randrange(256)
  cmd_e_shift_down(DIAGS, 12, org+rain*21*interval, interval, 1, cRed(x),cGreen(x),cBlue(x), 0)			# random
  cmd_e_shift_down(DIAGS,  9, org+12*interval+rain*21*interval, interval, 1, 0,0,0, 0)			# black
  cmd_e_fill_rgb(SIDES, 1, org+rain*21*interval + 70*interval, 0, cRed(x), cGreen(x), cBlue(x))	# hit bottom!
wait_for_tick(15*21*interval)

org += 15*21*interval

for rain in range(15):
  wait_for_tick(rain*21*interval)
  x = random.randrange(256)
  cmd_e_shift_down(DIAGS, 21, org+rain*21*interval, interval, 1, cRed(x),cGreen(x),cBlue(x), 0)
  cmd_e_fill_rgb(SIDES, 1, org+rain*21*interval + 70*interval, 0, cRed(x), cGreen(x), cBlue(x))	# hit bottom!

ends_at_tick(15*21*interval)
