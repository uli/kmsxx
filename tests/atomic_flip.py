# atomic page flip
import pykms
from lib import *
from helpers import *

card, conn, crtc, mode = cccm()
atomic(card)

fb1 = dumb_fb(card, mode)
fb2 = dumb_fb(card, mode)

if crtc.set_mode(conn, fb1, mode):
    exit(1)

if set_prop(crtc.primary_plane, "FB_ID", fb2.id) != 0:
    exit(1)
if set_prop(crtc.primary_plane, "FB_ID", fb1.id) != 0:
    exit(1)
