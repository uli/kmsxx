# expect -EINVAL when setting mode with bogus dimensions
from lib import *
import errno

card, conn, crtc, mode = cccm()
# allocating buffer with valid width because it may fail otherwise
fb = dumb_fb(card, mode)
mode.hdisplay = 32768
exit(crtc.set_mode(conn, fb, mode) != -errno.EINVAL)
