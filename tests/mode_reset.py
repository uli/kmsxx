# get default mode, then set it again
from lib import *
import pykms

card, conn, crtc, mode = cccm()
fb = dumb_fb(card, mode)
crtc = conn.get_current_crtc()
exit(crtc.set_mode(conn, fb, mode))
