# static poses dividing up the pyramid into vertical layers
filename("LAYER1.PKT")
comment("Layer 1")
cmd_s_reset_clock(ALL)
reset_master_clock()

cmd_e_fill_d(ALL, 1,0,0, 0)
cmd_e_fill_rgb(ALL, 1,0,0, 0,0,0)

# red on top, green middle, blue lower, white on bottom edge
cmd_e_shift_down(DIAGS, 1,0,0, 18, 0,0,255, 0)
cmd_e_shift_down(DIAGS, 1,0,0, 25, 0,255,0, 0)
cmd_e_shift_down(DIAGS, 1,0,0, 25, 255,0,0, 0)
cmd_e_fill_rgb(SIDES, 1,0,0, 255,255,255)

wait_for_tick(800)

# Mark's palette.
# gold on top, blue purple middle, orange lower, red purple on bottom edge
cmd_e_shift_down(DIAGS, 1,0,0, 18, 127,32,0, 0)
cmd_e_shift_down(DIAGS, 1,0,0, 25, 90,0,120, 0)
cmd_e_shift_down(DIAGS, 1,0,0, 25, 122,88,0, 0)
cmd_e_fill_rgb(SIDES, 1,0,0, 127,0,64)

wait_for_tick(1600)

# Layers of red brightness
cmd_e_shift_down(DIAGS, 1,0,0, 18, 8,0,0, 0)
cmd_e_shift_down(DIAGS, 1,0,0, 25, 32,0,0, 0)
cmd_e_shift_down(DIAGS, 1,0,0, 25, 128,0,0, 0)
cmd_e_fill_rgb(SIDES, 1,0,0, 255,0,0)

wait_for_tick(2400)
