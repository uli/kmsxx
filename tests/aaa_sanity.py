# basic sanity check: get card, connector, crtc, default mode
from lib import *

card, conn, crtc, mode = cccm()
print('\nTesting connector ' + conn.fullname)

exit(0)
