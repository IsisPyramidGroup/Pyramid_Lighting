# Test of shifting up and down on entities!
# Also uses LOADONE and FILL_RGB
# Needs whole pyramid to validate visually
filename("ROVERV.PKT")
comment("Rover V")
cmd_s_reset_clock(ALL)
reset_master_clock()
step = 10		# 10 ticks = 100ms
cmd_e_fill_rgb(ALL, 1, 0, 0, 0, 0, 0)		# fill everything with black
cmd_e_loadone(DIAGS, 1, 0, 0, 67, 255,255,255, 0)	# start at the top
cmd_e_shift_down(DIAGS, 67, step, step, 1, 0,0,0, 0)	# run them down to the bottom
cmd_e_shift_up(DIAGS, 68, 68*step, step, 1, 0,0,0, 0)	# and then back up
ends_at_tick(136*step)
