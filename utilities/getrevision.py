import os, subprocess, sys

# Get SVN revision:
cmd = 'svn info'
if len(sys.argv) > 1: cmd += ' "%s"' % sys.argv[1]

output = str( subprocess.Popen( cmd, shell=True, bufsize=100000, stdout=subprocess.PIPE).stdout.read())

str = 'Last Changed Rev: '
pos = output.find(str)
if pos != -1:
   pos += len(str)
   verlen = 0
   while output[pos+verlen] in '0123456789': verlen += 1
   print( output[pos:pos+verlen])
   sys.exit(0)

print('0')
