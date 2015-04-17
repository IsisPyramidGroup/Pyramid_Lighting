# turns on blinking as a dynamic effect, leaves it on (can be overwritten of course)
filename("BLINKON.PKT");
comment("blink on");
cmd_s_dyn_blink(ALL, 50, 30, 128)			# reduce by half range 7/10 of the time
cmd_e_fill_d(ALL, 1, 0, 0, DYN_BLINK)		# immediate fill with all blinking
