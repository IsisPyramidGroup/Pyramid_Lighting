# Starts with everything bright white.
# Promptly fades the bottom to black.
# Then slowly fades down the top as the bottom comes back on. 

filename("WFADE.PKT")
comment("Wht Fade")
cmd_s_reset_clock(ALL)
reset_master_clock()

cmd_e_fill_rgb(ALL, 1,0,0, 255,255,255)		# bright white
cmd_e_fill_d(ALL, 1,0,0, 0)					#no dynamics

# fade the bottom down over 1.28 seconds
for wfade in range(0, 256, 2):
  wait_for_tick(wfade/2)
  cmd_e_fill_rgb(SIDES, 1,0,0, 255-wfade,255-wfade,255-wfade)

# now over 12.8 seconds, cross fade
for wfade in range(256):
  wait_for_tick(256+5*wfade)
  cmd_e_fill_rgb(SIDES, 1,0,0, wfade,wfade,wfade)
  cmd_e_fill_rgb(DIAGS, 1,0,0, 255-wfade,255-wfade,255-wfade)

ends_at_tick(2816)
