# Test of shifting up and down on entities!
# Also uses LOADONE and FILL_RGB
# Needs whole pyramid to validate visually
filename("ROVERH.PKT")
comment("Rover H")
cmd_s_reset_clock(ALL)
reset_master_clock()
step = 10		# 10 ticks = 100ms
cmd_e_fill_rgb(ALL, 1, 0, 0, 0, 0, 0)		# fill everything with black
cmd_e_loadone(E_FRONT, 1, 0, 0, 95-24, 255,255,255, 0)	# one white pixel by doorway
cmd_e_shift_up(E_FRONT, 24, step, step, 1, 0,0,0, 0)		# travels to the left front corner
cmd_e_loadone(E_LEFT, 1, 24*step, 0, 0, 255,255,255, 0)	# round the corner to left side
cmd_e_shift_up(E_LEFT, 96, 25*step, step, 1, 0,0,0, 0)		# travels to the left rear corner
cmd_e_loadone(E_BACK, 1, 120*step, 0, 0, 255,255,255, 0)	# round the corner to the back
cmd_e_shift_up(E_BACK, 96, 121*step, step, 1, 0,0,0, 0)		# travels across the back
cmd_e_loadone(E_RIGHT, 1, 216*step, 0, 0, 255,255,255, 0)  # round the corner to the back
cmd_e_shift_up(E_RIGHT, 96, 217*step, step, 1, 0,0,0, 0)	# travels to the front right corner
cmd_e_loadone(E_FRONT, 1, 312*step, 0, 0, 255,255,255, 0)	# round the corner to the front
cmd_e_shift_up(E_FRONT, 23, 313*step, step, 1, 0,0,0, 0)	# travels to the doorway edge
wait_for_tick(315*step);		# don't queue up too many packets in each slave
cmd_e_shift_down(E_FRONT, 24, 336*step, step, 1, 0,0,0, 0)	# bounces back to the right
cmd_e_loadone(E_RIGHT, 1, 360*step, 0, 95, 255,255,255, 0)	# round the corner
cmd_e_shift_down(E_RIGHT, 96, 361*step, step, 1, 0,0,0, 0)	# back to rear right corner
cmd_e_loadone(E_BACK, 1, 456*step, 0, 95, 255,255,255, 0)	# round the corner
cmd_e_shift_down(E_BACK, 96, 457*step, step, 1, 0,0,0, 0)	# across the back again
cmd_e_loadone(E_LEFT, 1, 552*step, 0, 95, 255,255,255, 0)	# round the corner
cmd_e_shift_down(E_LEFT, 96, 553*step, step, 1, 0,0,0, 0)	# back up to the front left corner
wait_for_tick(362*step)
cmd_e_loadone(E_FRONT, 1, 648*step, 0, 95, 255,255,255, 0)	# round the corner
cmd_e_shift_down(E_FRONT, 22, 649*step, step, 1, 0,0,0, 0)  # almost to the end
ends_at_tick(671*step)
