import os, socket, sys

import cgruconfig

def usageExit( exitstatus = 1, msg = None):
   print('Usage: %s [port] [host] command' % sys.argv[0])
   sys.exit( exitstatus)

def send( data, host = 'localhost', port = -1, verbose = False):
   if port == -1: port = int( cgruconfig.VARS['keeper_port'])
   s = None
   for res in socket.getaddrinfo( host, port, socket.AF_UNSPEC, socket.SOCK_STREAM):
      af, socktype, proto, canonname, sa = res
      if verbose: print('Trying to connect to "%s"' % str(sa[0]))
      try:
         s = socket.socket(af, socktype, proto)
      except:
         print(str(sys.exc_info()[1]))
         s = None
         continue
      try:
         s.connect(sa)
      except:
         print(str(sys.exc_info()[1]))
         s.close()
         s = None
         continue
      break

   if s is None: print('Could not open socket.')
   else: s.sendall( data)


if len( sys.argv) < 2: usageExit()

try:
   cmd = bytes( sys.argv[-1],'utf-8')
except:
   cmd = bytes( sys.argv[-1])
port = -1
host = 'localhost'

if len( sys.argv) > 2: host = sys.argv[-2]
if len( sys.argv) > 3: port = int(sys.argv[-3])

send( cmd, host, port)
