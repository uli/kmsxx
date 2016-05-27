# set each available mode
from lib import *

card, conn, crtc, mode = cccm()

modes = conn.get_modes()

for i in modes:
  fb = dumb_fb(card, i)
  if crtc.set_mode(conn, fb, i):
    exit(1)

exit(0)
