# Starts with whatever is already in the array, then rains down red.
filename("RRAIN.PKT")
comment("Red Rain")
cmd_s_reset_clock(ALL)
reset_master_clock()

interval = 7

cmd_e_fill_rgb(SIDES, 1,68*interval,0, 255,0,0)		# bright red when first raindrop lands

for rain in range(25):
  wait_for_tick(rain*21*interval)
  cmd_e_shift_down(DIAGS,  3, rain*21*interval, interval, 1, 255,0,0, 0)			# red
  cmd_e_shift_down(DIAGS, 18, 3*interval+rain*21*interval, interval, 1, 0,0,0, 0)			# black
wait_for_tick(25*21*interval)

cmd_s_reset_clock(ALL)
reset_master_clock()

for rain in range(15):
  wait_for_tick(rain*21*interval)
  cmd_e_shift_down(DIAGS,  6, rain*21*interval, interval, 1, 255,0,0, 0)			# red
  cmd_e_shift_down(DIAGS, 15, 6*interval+rain*21*interval, interval, 1, 0,0,0, 0)			# black

ends_at_tick(15*21*interval)
