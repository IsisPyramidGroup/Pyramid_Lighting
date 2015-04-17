# Rainbow mellow
# 52 seconds of relatively fast (max speed with minimum color steps) rainbow rotation.

filename("RBOW1.PKT")
comment("Rbow 1")
cmd_s_reset_clock(ALL)
reset_master_clock()
step = 10		# 10 ticks = 100ms
#def cmd_e_rainbow(bitmap, repeat_count, start_tick, repeat_interval, start, incr, dir):
cmd_e_fill_d(ALL, 1, 0, 0, 0)		# no dynamics
cmd_e_fill_rgb(SIDES, 1,0,0, 40,0,0)		# dim red on the bottom
cmd_e_rainbow(DIAGS, 100,    0, 1,   0, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100,  100, 1, 100, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100,  200, 1, 200, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100,  300, 1,  45, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100,  400, 1, 145, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100,  500, 1, 245, 1, 0)	# rainbow with rotation
wait_for_tick(550)
cmd_e_rainbow(DIAGS, 100,  600, 1,  90, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100,  700, 1, 190, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100,  800, 1,  35, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100,  900, 1, 135, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 1000, 1, 235, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 1100, 1,  80, 1, 0)	# rainbow with rotation
wait_for_tick(1150)
cmd_e_rainbow(DIAGS, 100, 1200, 1, 180, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 1300, 1,  25, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 1400, 1, 125, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 1500, 1, 225, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 1600, 1,  70, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 1700, 1, 175, 1, 0)	# rainbow with rotation
wait_for_tick(1750)
cmd_e_rainbow(DIAGS, 100, 1800, 1,  15, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 1900, 1, 115, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 2000, 1, 215, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 2100, 1,  60, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 2200, 1, 160, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 2300, 1,   5, 1, 0)	# rainbow with rotation
wait_for_tick(2350)
cmd_e_rainbow(DIAGS, 100, 2400, 1, 105, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 2500, 1, 205, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 2600, 1,  50, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 2700, 1, 150, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 2800, 1, 250, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 2900, 1,  95, 1, 0)	# rainbow with rotation
wait_for_tick(2950)
cmd_e_rainbow(DIAGS, 100, 3000, 1, 195, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 3100, 1,  40, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 3200, 1, 140, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 3300, 1, 240, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 3400, 1,  85, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 3500, 1, 185, 1, 0)	# rainbow with rotation
wait_for_tick(3550)
cmd_e_rainbow(DIAGS, 100, 3600, 1,  30, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 3700, 1, 130, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 3800, 1, 230, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 3900, 1,  75, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 4000, 1, 175, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 4100, 1,  20, 1, 0)	# rainbow with rotation
wait_for_tick(4150)
cmd_e_rainbow(DIAGS, 100, 4200, 1, 120, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 4300, 1, 220, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 4400, 1,  65, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 4500, 1, 165, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 4600, 1,  10, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 4700, 1, 110, 1, 0)	# rainbow with rotation
wait_for_tick(4750)
cmd_e_rainbow(DIAGS, 100, 4800, 1, 210, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 4900, 1,  55, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 5000, 1, 155, 1, 0)	# rainbow with rotation
cmd_e_rainbow(DIAGS, 100, 5100, 1, 255, 1, 0)	# rainbow with rotation
ends_at_tick(5200)
