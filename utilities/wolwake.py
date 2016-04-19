#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import struct
import socket

from optparse import OptionParser
Parser = OptionParser(usage="%prog [options] path\ntype \"%prog -h\" for help", version="%prog 1.  0")
Parser.add_option('--wake',     dest='wake',     action='store_true', default=False, help='Wake mode, not used.')
Parser.add_option('--sleep',    dest='sleep',    action='store_true', default=False, help='Sleep mode, not used.')
Parser.add_option('--ip',       dest='ip',       type='string',       default=None,  help='IP address, not used.')
Parser.add_option('--hostname', dest='hostname', type='string',       default=None,  help='Hostname, not used.')
Options, Args = Parser.parse_args()

print(sys.argv)

def usageErrorExit():
	print('Usage %s --(wake|sleep) --ip IP mac1 mac2 .. macN' % sys.argv[0])
	print('MAC address(es) should be without any separators, just a string of '
		  '12 characters.')
	sys.exit(1)


if len(Args) < 2:
	print('No mac address(es) specified.')
	usageErrorExit()


for addr in Args:
	if addr == Args[0]:
		continue
	if len(addr) != 12:
		print('Invalid mac address: "%s"' % addr)
		usageErrorExit()

	hw_addr = struct.pack('BBBBBB',
						  int(addr[0: 2], 16),
						  int(addr[2: 4], 16),
						  int(addr[4: 6], 16),
						  int(addr[6: 8], 16),
						  int(addr[8:10], 16),
						  int(addr[10:12], 16))

	msg = b'\xff' * 6 + hw_addr * 16
	# print((('%X'+':%X'*5+' ')*17) % struct.unpack('B'*6*17, msg))

	s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	s.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
	s.sendto(msg, ('<broadcast>', 9))
	s.close()

