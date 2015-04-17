# Rotation variant 1, second and subsequent repeats

filename("ROT2.PKT")
comment("Rotate 2")
cmd_s_reset_clock(ALL)
reset_master_clock()


wait_until = 0

def wait(howlong):
  global wait_until
  wait_until += howlong
  wait_for_tick(wait_until)
  
# a rough attempt at linearizing the subjective brightness 0-9
table = [1,2,4,8,16,32,64,128,255]
def brightening(val):
  if val == 0:
  	return 0
  else:
    return table[val-1]
  
def dimming(val):
  if val == 9:
  	return 0
  else:
    return table[8-val]

# starting position: front left diagonal and bottom left of the door are red. Else black.
#cmd_e_fill_d(ALL, 1,0,0, 0)
#cmd_e_fill_rgb(E_DIAG_0|E_DIAG_1, 1,0,0, 255,0,0)
#cmd_e_fill_rgb(E_FRONT, 1,0,0, 255,0,0)
#cmd_e_shift_up(E_FRONT, 1,0,0, 96-24, 0,0,0, 0)

#wait(300)

# now move the red around the corner
cmd_e_shift_up(E_FRONT, 24,0,1, 1, 0,0,0, 0)
cmd_e_shift_up(E_LEFT, 72,0,1, 1, 255,0,0, 0)

wait(24*1)

# the left side is still filling up
# fade down the first diagonal and up the second, over 190 ticks
for slider in range(0,10):
  cmd_e_fill_rgb(E_DIAG_0|E_DIAG_1, 1,0,0, dimming(slider),0,0)
  cmd_e_fill_rgb(E_DIAG_2|E_DIAG_3, 1,0,0, brightening(slider),0,0)
  wait(4)
wait(8)
# the left side is 3/4 full of red, start shifting in black now
cmd_e_shift_up(E_LEFT, 96,0,1, 1, 0,0,0, 0)

wait(72*1)

# left side is 3/4 full of black and the last 1/4 is red, so start around the corner
cmd_e_shift_up(E_BACK, 72,0,1, 1, 255,0,0, 0)

wait(24*1)

for slider in range(0,10):
  cmd_e_fill_rgb(E_DIAG_2|E_DIAG_3, 1,0,0, dimming(slider),0,0)
  cmd_e_fill_rgb(E_DIAG_4|E_DIAG_5, 1,0,0, brightening(slider),0,0)
  wait(4)
wait(8)

# the back is now 3/4 full of red, start shifting in black now
cmd_e_shift_up(E_BACK, 96,0,1, 1, 0,0,0, 0)

wait(72*1)

# back is 3/4 full of black and last 1/4 is red, so start around the corner
cmd_e_shift_up(E_RIGHT, 72,0,1, 1, 255,0,0, 0)

wait(24*1)

for slider in range(0,10):
  cmd_e_fill_rgb(E_DIAG_4|E_DIAG_5, 1,0,0, dimming(slider),0,0)
  cmd_e_fill_rgb(E_DIAG_6|E_DIAG_7, 1,0,0, brightening(slider),0,0)
  wait(4)
wait(8)

# the right side is now 3/4 full of red, start shifting in black now
cmd_e_shift_up(E_RIGHT, 96,0,1, 1, 0,0,0, 0)

wait(72*1)

# right side is 3/4 full of black and last 1/4 is red, so start around the last corner
cmd_e_shift_up(E_FRONT, 72,0,1, 1, 255,0,0, 0)

wait(24*1)

for slider in range(0,10):
  cmd_e_fill_rgb(E_DIAG_6|E_DIAG_7, 1,0,0, dimming(slider),0,0)
  cmd_e_fill_rgb(E_DIAG_0|E_DIAG_1, 1,0,0, brightening(slider),0,0)
  wait(4)
wait(8)

# the front is now 3/4 full of red, start shifting in black now
cmd_e_shift_up(E_FRONT, 72,0,1, 1, 0,0,0, 0)

wait(72*1)
wait(13)  #why?

# now we should be set up for a repeat of the same program.
ends_at_tick(wait_until)
