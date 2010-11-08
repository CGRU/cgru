import os, subprocess, sys

# Try version file:
version_filename = 'cgru_version.txt'
if len(sys.argv) > 1: version_filename = os.path.join( sys.argv[1], version_filename)
if os.path.isfile( version_filename):
   version_file = open( version_filename, 'r')
   print version_file.readline()
   version_file.close()
   sys.exit(0)

# Get SVN revision:
cmd = 'svn info'
if len(sys.argv) > 1: cmd += ' "%s"' % sys.argv[1]

output = subprocess.Popen( cmd, shell=True, bufsize=100000, stdout=subprocess.PIPE).stdout.read()

str = 'Last Changed Rev: '
pos = output.find(str)
if pos != -1:
   pos += len(str)
   verlen = 0
   while output[pos+verlen] in '0123456789': verlen += 1
   print output[pos:pos+verlen],
   sys.exit(0)

print 0
