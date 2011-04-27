#!/usr/bin/env python

import sys
import struct, socket

for addr in sys.argv:
   if addr == sys.argv[0]: continue
   if len(addr) != 12:
      print 'Invalid address: %s' % addr
      sys.exit(1)

   hw_addr = struct.pack('BBBBBB',
      int(addr[ 0: 2], 16),
      int(addr[ 2: 4], 16),
      int(addr[ 4: 6], 16),
      int(addr[ 6: 8], 16),
      int(addr[ 8:10], 16),
      int(addr[10:12], 16))

   msg = '\xff' * 6 + hw_addr * 16

   s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
   s.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
   s.sendto(msg, ('<broadcast>', 9))
   s.close()

#   for char in msg: print '%X' % struct.unpack('B',char),
