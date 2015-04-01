# Mark's Tasty Rainbow palette 
# Ported to the lamptest template

filename("MTR_TEST.PKT")
console(1234)
cmd_s_reset_clock(ALL)

# I want the pixels to go blue purple, white, icy blue, white, green, white, etc...

#def cmd_e_fill_rgb(bitmap, repeat_count, start_tick, repeat_interval, red, green, blue):
cmd_e_fill_rgb(ALL, 84, 0, 700, 90, 0, 120)		# blue purple r=90 g=0 b=120 at 0, 700, 1400, ...
cmd_e_fill_rgb(ALL, 252, 50, 100, 255, 255, 255)		# turn pixels white at 50, 150, 250, ...
cmd_e_fill_rgb(ALL, 84, 100, 700, 0, 0, 1000)		# icy blue at 100, 800, 1500, ...
cmd_e_fill_rgb(ALL, 84, 200, 700, 0, 100, 0)	# green at 200, 900, 1600, ...
cmd_e_fill_rgb(ALL, 84, 300, 700, 122, 88, 0)	# gold
cmd_e_fill_rgb(ALL, 84, 400, 700, 127, 32, 0)	# orange
cmd_e_fill_rgb(ALL, 84, 500, 700, 127, 0, 0)	# red
cmd_e_fill_rgb(ALL, 84, 600, 700, 127, 0, 64)	# red purple at 600, 1300, 2000, ...
ends_at_tick(252*100)

