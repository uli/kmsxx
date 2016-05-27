# expect -ENOSPC when using fb smaller than the plane

import pykms, errno
from lib import *
from helpers import *

card, conn, crtc, mode = cccm()
atomic(card)

fb1 = dumb_fb(card, mode)
mode.hdisplay -= 1
fb2 = dumb_fb(card, mode)
mode.hdisplay += 1

if crtc.set_mode(conn, fb1, mode):
    exit(1)

if set_prop(crtc.primary_plane, "FB_ID", fb2.id) != -errno.ENOSPC:
    exit(1)
if set_prop(crtc.primary_plane, "FB_ID", fb1.id) != 0:
    exit(1)
