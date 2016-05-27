# set primary plane to various geometries
from lib import *
from helpers import *

card, conn, crtc, mode = cccm()
atomic(card)

modes = conn.get_modes()
for i in modes:
  fb = dumb_fb(card, i)
  if set_props(crtc.primary_plane, {
    "FB_ID": fb.id,
    "CRTC_W": fb.width,
    "CRTC_H": fb.height,
    "SRC_W": fb.width << 16,
    "SRC_H": fb.height << 16
  }) != 0:
    exit(1)

exit(0)
