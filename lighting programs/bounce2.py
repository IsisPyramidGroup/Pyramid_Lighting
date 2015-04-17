DOES NOT WORK.

# Bouncing columns of color, top and bottom
filename("BOUNCE2.PKT")
comment("Bounce2")
cmd_s_reset_clock(ALL)
reset_master_clock()
step = 20		# 10 ticks = 100ms
cmd_e_fill_rgb(ALL, 1, 0, 0, 0, 0, 0)		# fill everything with black
cmd_e_fill_d(ALL, 1,0,0, 0)

#mark's gold
r = 122
g = 88
b = 0

wait_until = 0

def wait(howlong):
  global wait_until
  wait_until += howlong
  wait_for_tick(wait_until)


# fill diagonals with rgb from top and bottom to depth of ht
def columns(ht):
  cmd_e_shift_up(DIAGS, 1,0,0, ht, r,g,b, 0)
  cmd_e_shift_up(DIAGS, 1,0,0, 68-2*ht, 0,0,0, 0)
  cmd_e_shift_up(DIAGS, 1,0,0, ht, r,b,b, 0)

def bounceup(ht):
  for h in range(1,ht+1):
  	columns(h)
  	wait(step)
  for h in range(ht,0):
    columns(h)
    wait(step)

for b in [5,15,25,33,25,15,5]:
  bounceup(b)
  
ends_at_tick(wait_until)
