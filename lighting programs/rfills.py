# Test of random and rainbow fills

filename("RFILLS.PKT")
comment("R Fills")
cmd_s_reset_clock(ALL)
reset_master_clock()
step = 10		# 10 ticks = 100ms
cmd_e_randomize(ALL, 10, 0, 200)		# 10 random fills of everything
cmd_e_rainbow(ALL, 100, 2000, 4, 0, 1, 0)	# rainbow with rotation
cmd_e_rainbow(ALL, 100, 2400, 4, 100, 1, 0)	# rainbow with rotation
cmd_e_rainbow(ALL, 100, 2800, 4, 200, 1, 0)	# rainbow with rotation
cmd_e_rainbow(ALL, 100, 3200, 4, 45, 1, 0)	# rainbow with rotation
cmd_e_rainbow(ALL, 100, 3600, 4, 145, 1, 0)	# rainbow with rotation
wait_for_tick(3800)
cmd_e_rainbow(ALL, 100, 4000, 4, 245, 1, 1)	# rotate the opposite way
cmd_e_rainbow(ALL, 100, 4400, 4, 145, 1, 1)	# rotate the opposite way
cmd_e_rainbow(ALL, 100, 4800, 4, 45, 1, 1)	# rotate the opposite way
cmd_e_rainbow(ALL, 100, 5200, 4, 200, 1, 1)	# rotate the opposite way
cmd_e_rainbow(ALL, 100, 5400, 4, 100, 1, 1)	# rotate the opposite way
ends_at_tick(6000)
