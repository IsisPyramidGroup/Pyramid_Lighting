# static poses dividing up the pyramid into vertical layers
# plus roving sparkles
filename("LAYER2.PKT")
comment("Layer 2")
cmd_s_reset_clock(ALL)
reset_master_clock()

cmd_e_fill_d(ALL, 1,0,0, 0)
cmd_e_fill_rgb(ALL, 1,0,0, 0,0,0)

cmd_s_dyn_sparkle(ALL, 300)					# about 1/3 pixel lit each tick

# red on top, green middle, blue lower, white on bottom edge
cmd_e_shift_down(DIAGS, 1,0,0, 18, 0,0,255, 0)
cmd_e_shift_down(DIAGS, 1,0,0, 25, 0,255,0, DYN_SPARKLE)
cmd_e_shift_down(DIAGS, 1,0,0, 25, 255,0,0, 0)
cmd_e_fill_rgb(SIDES, 1,0,0, 80,80,80)

wait_for_tick(800)

# Mark's palette.
# gold on top, blue purple middle, orange lower, red purple on bottom edge
cmd_e_shift_down(DIAGS, 1,0,0, 18, 127,32,0, 0)
cmd_e_shift_down(DIAGS, 1,0,0, 25, 90,0,120, 0)
cmd_e_shift_down(DIAGS, 1,0,0, 25, 122,88,0, DYN_SPARKLE)
cmd_e_fill_rgb(SIDES, 1,0,0, 127,0,64)

wait_for_tick(1600)

# Layers of red brightness
cmd_e_shift_down(DIAGS, 1,0,0, 18, 8,0,0, DYN_SPARKLE)
cmd_e_shift_down(DIAGS, 1,0,0, 25, 32,0,0, 0)
cmd_e_shift_down(DIAGS, 1,0,0, 25, 128,0,0, 0)
cmd_e_fill_rgb(SIDES, 1,0,0, 4,0,0)

wait_for_tick(2400)

# run the sparkle up the stack
cmd_e_shift_down(DIAGS, 1,0,0, 18, 8,0,0, DYN_SPARKLE)
cmd_e_shift_down(DIAGS, 1,0,0, 25, 8,0,0, DYN_SPARKLE)
cmd_e_shift_down(DIAGS, 1,0,0, 25, 32,0,0, 0)
cmd_e_fill_rgb(SIDES, 1,0,0, 8,0,0)

wait_for_tick(3200)

cmd_e_shift_down(DIAGS, 1,0,0, 68, 8,0,0, DYN_SPARKLE)

wait_for_tick(4000)

cmd_e_fill_d(ALL, 1,0,0, 0)
ends_at_tick(4000)