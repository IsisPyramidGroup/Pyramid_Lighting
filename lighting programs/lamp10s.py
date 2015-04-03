# Simple but tricky lamp test for all pixels.
# We want all the pixels to go: red, black, green, black, blue, black, and repeat.
# We can use several instance of the FILL_RGB command with interleaved repeats.
filename("LAMP10S.PKT")
comment("10-sec lamptest")
console(1234)
cmd_s_reset_clock(ALL)
reset_master_clock()
cmd_e_fill_rgb(ALL, 3, 0, 300, 255, 0, 0)		# turn pixels red at 0, 300, 600, ...
cmd_e_fill_rgb(ALL, 9, 50, 100, 0, 0, 0)		# turn pixels black at 50, 150, 250, ...
cmd_e_fill_rgb(ALL, 3, 100, 300, 0, 255, 0)	# turn pixels green at 100, 400, 700, ...
cmd_e_fill_rgb(ALL, 3, 200, 300, 0, 0, 255)	# turn pixels blue at 200, 500, 800, ...
ends_at_tick(9*100)

