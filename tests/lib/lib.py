import pykms
from sys import exit, argv
import helpers
helpers.silent = True

def cccm():
  card = pykms.Card()
  if len(argv) > 1:
    conn = card.connectors[int(argv[1])]
  else:
    conn = card.get_first_connected_connector()
  crtc = conn.get_current_crtc()
  mode = conn.get_default_mode()
  return card, conn, crtc, mode

def dumb_fb(card, mode):
  return pykms.DumbFramebuffer(card, mode.hdisplay, mode.vdisplay, "XR24")

def atomic(card):
  if not card.has_atomic:
    exit(2)
