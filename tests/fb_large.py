# try framebuffer large than the mode
# XXX: is this really legal?
from lib import *

card, conn, crtc, mode = cccm()
mode.hdisplay += 1
fb = dumb_fb(card, mode)
mode.hdisplay -= 1
exit(crtc.set_mode(conn, fb, mode))
