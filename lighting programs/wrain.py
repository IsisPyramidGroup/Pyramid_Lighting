# Starts with the bottom lit up white and the diagonals dark.
# Then rains down white from top to bottom.

filename("WRAIN.PKT")
comment("Wht Rain")
cmd_s_reset_clock(ALL)
reset_master_clock()

interval = 7

cmd_e_fill_rgb(SIDES, 1,0,0, 255,255,255)		# bright white
cmd_e_fill_rgb(DIAGS, 1,0,0, 0,0,0)				# dark
cmd_e_fill_d(ALL, 1,0,0, 0)					#no dynamics

for rain in range(25):
  wait_for_tick(rain*21*interval)
  cmd_e_shift_down(DIAGS,  3, rain*21*interval, interval, 1, 255, 255, 255, 0)		# white
  cmd_e_shift_down(DIAGS, 18, 3*interval+rain*21*interval, interval, 1, 0,0,0, 0)			# black
wait_for_tick(25*21*interval)

cmd_s_reset_clock(ALL)
reset_master_clock()

for rain in range(15):
  wait_for_tick(rain*21*interval)
  cmd_e_shift_down(DIAGS,  6, rain*21*interval, interval, 1, 255, 255, 255, 0)		# white
  cmd_e_shift_down(DIAGS, 15, 6*interval+rain*21*interval, interval, 1, 0,0,0, 0)			# black

ends_at_tick(15*21*interval)
