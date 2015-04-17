# tests throbbing as a dynamic effect, then leaves it on
filename("THROBON.PKT");
comment("throb on");
cmd_s_reset_clock(ALL)
reset_master_clock()
cmd_s_dyn_throb(ALL, 80, 20, 0, 100)		# throb dimmer only
cmd_e_fill_d(ALL, 1, 0, 0, DYN_THROB)		# immediate fill with all throbbing
cmd_e_fill_rgb(ALL, 1, 0, 0, 128,0,0)		# fill with half-bright red
wait_for_tick(500)
cmd_e_fill_rgb(ALL, 1, 0, 0, 0,128,0)		# half-bright green
wait_for_tick(1000)
cmd_e_fill_rgb(ALL, 1, 0, 0, 0,0,128)		# half-bright blue
wait_for_tick(1500)
cmd_e_fill_rgb(ALL, 1, 0, 0, 128,128,128)	# half-bright white
wait_for_tick(2000)
ends_at_tick(2010)
