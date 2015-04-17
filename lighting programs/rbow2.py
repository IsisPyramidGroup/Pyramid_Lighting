# Rainbow with sparkle
# 36 seconds of relatively fast (max speed with minimum color steps) rainbow rotation,
# but with more and more sparkling!

filename("RBOW2.PKT")
comment("Rbow 2")
cmd_s_reset_clock(ALL)
reset_master_clock()
step = 10		# 10 ticks = 100ms
#def cmd_e_rainbow(bitmap, repeat_count, start_tick, repeat_interval, start, incr, dir):
cmd_e_fill_rgb(SIDES, 1,0,0, 40,40,40)		# dim white on the bottom
cmd_e_rainbow(DIAGS, 100,    0, 1,   0, 1, 0)	# rainbow with rotation
cmd_e_fill_d( DIAGS,   1,    0, 0, DYN_SPARKLE)	# tell all the diag pixels to enable sparkle
cmd_e_fill_d( SIDES,   1,    0, 0, 0)			# and make sure the bottom edge doesn't
cmd_s_dyn_sparkle(ALL, 0)						# no sparkle yet
cmd_e_rainbow(DIAGS, 100,  100, 1, 100, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100,  200, 1, 200, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100,  300, 1,  45, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100,  400, 1, 145, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100,  500, 1, 245, 1, 0)	# rainbow with rotation
wait_for_tick(500)
cmd_s_dyn_sparkle(ALL, 964)					# about 1 pixel lit each tick
cmd_e_fill_rgb(SIDES, 1,0,0, 60,60,60)		# brighter white on the bottom
cmd_e_rainbow(DIAGS, 100,  600, 1,  90, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100,  700, 1, 190, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100,  800, 1,  35, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100,  900, 1, 135, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 1000, 1, 235, 1, 0)	# rainbow with rotation
wait_for_tick(1000)
cmd_s_dyn_sparkle(ALL, 1928)							# about 2 pixels lit
cmd_e_fill_rgb(SIDES, 1,0,0, 100,100,100)		# brighter white on the bottom
cmd_e_rainbow(DIAGS, 100, 1100, 1,  80, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 1200, 1, 180, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 1300, 1,  25, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 1400, 1, 125, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 1500, 1, 225, 1, 0)	# rainbow with rotation
wait_for_tick(1500)
cmd_s_dyn_sparkle(ALL, 3855)							# about 4 pixels lit
cmd_e_fill_rgb(SIDES, 1,0,0, 140,140,140)		# brighter white on the bottom
cmd_e_rainbow(DIAGS, 100, 1600, 1,  70, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 1700, 1, 175, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 1800, 1,  15, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 1900, 1, 115, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 2000, 1, 215, 1, 0)	# rainbow with rotation
wait_for_tick(2000)
cmd_s_dyn_sparkle(ALL, 7710)							# about 8 pixels lit
cmd_e_fill_rgb(SIDES, 1,0,0, 180,180,180)		# brighter white on the bottom
cmd_e_rainbow(DIAGS, 100, 2100, 1,  60, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 2200, 1, 160, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 2300, 1,   5, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 2400, 1, 105, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 2500, 1, 205, 1, 0)	# rainbow with rotation
wait_for_tick(2500)
cmd_s_dyn_sparkle(ALL, 15420)						# about 16 pixels lit
cmd_e_fill_rgb(SIDES, 1,0,0, 220,220,220)		# brighter white on the bottom
cmd_e_rainbow(DIAGS, 100, 2600, 1,  50, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 2700, 1, 150, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 2800, 1, 250, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 2900, 1,  95, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 3000, 1, 195, 1, 0)	# rainbow with rotation
wait_for_tick(3000)
cmd_s_dyn_sparkle(ALL, 30840)						# about 32 pixels lit
cmd_e_fill_rgb(SIDES, 1,0,0, 255,255,255)		# brighter white on the bottom
cmd_e_rainbow(DIAGS, 100, 3100, 1,  40, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 3200, 1, 140, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 3300, 1, 240, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 3400, 1,  85, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 3500, 1, 185, 1, 0)	# rainbow with rotation
ends_at_tick(3600)
