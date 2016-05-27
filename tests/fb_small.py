# expect -ENOSPC when using framebuffer that is too small
from lib import *
import pykms
import errno

card, conn, crtc, mode = cccm()
mode.hdisplay -= 1
fb = dumb_fb(card, mode)
mode.hdisplay += 1
exit(crtc.set_mode(conn, fb, mode) != -errno.ENOSPC) 
