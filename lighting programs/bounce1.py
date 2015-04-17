# Bouncing columns of color
filename("BOUNCE1.PKT")
comment("Bounce1")
cmd_s_reset_clock(ALL)
reset_master_clock()
step = 1		# 10 ticks = 100ms
cmd_e_fill_rgb(ALL, 1, 0, 0, 0, 0, 0)		# fill everything with black
cmd_e_fill_d(ALL, 1,0,0, 0)

#mark's red-purple
r = 90
g = 0
b = 120

cmd_e_shift_up(DIAGS, 5,0,step, 1, r,g,b, 0)
cmd_e_shift_down(DIAGS, 5,5*step,step, 1, 0,0,0, 0)
cmd_e_shift_up(DIAGS, 10,10*step,step, 1, r,g,b, 0)
cmd_e_shift_down(DIAGS, 10,20*step,step, 1, 0,0,0, 0)
cmd_e_shift_up(DIAGS, 15,30*step,step, 1, r,g,b, 0)
cmd_e_shift_down(DIAGS, 15,45*step,step, 1, 0,0,0, 0)
wait_for_tick(55*step)
cmd_e_shift_up(DIAGS, 20,60*step,step, 1, r,g,b, 0)
cmd_e_shift_down(DIAGS, 20,80*step,step, 1, 0,0,0, 0)
cmd_e_shift_up(DIAGS, 40,100*step,step, 1, r,g,b, 0)
cmd_e_shift_down(DIAGS, 40,140*step,step, 1, 0,0,0, 0)
cmd_e_shift_up(DIAGS, 68,180*step,step, 1, r,g,b, 0)
cmd_e_shift_down(DIAGS, 68,242*step,step, 1, 0,0,0, 0)
ends_at_tick(310*step)
