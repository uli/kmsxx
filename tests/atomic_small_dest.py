# expect -EINVAL when setting CRTC_* smaller than SRC_*
import pykms, errno
from lib import *
from helpers import *

card, conn, crtc, mode = cccm()
atomic(card)

fb1 = dumb_fb(card, mode)
fb2 = dumb_fb(card, mode)

if crtc.set_mode(conn, fb1, mode):
    exit(1)

if set_props(crtc.primary_plane, {
        "FB_ID": fb2.id,
        "CRTC_W": fb2.width - 1
   }) != -errno.EINVAL:
    exit(1)
if set_props(crtc.primary_plane, {
        "FB_ID": fb2.id,
        "CRTC_W": fb2.width
   }) != 0:
    exit(1)
if set_props(crtc.primary_plane, {
        "FB_ID": fb1.id,
        "CRTC_H": fb1.height - 1
   }) != -errno.EINVAL:
    exit(1)
if set_props(crtc.primary_plane, {
        "FB_ID": fb1.id,
        "CRTC_H": fb1.height
   }) != 0:
    exit(1)
