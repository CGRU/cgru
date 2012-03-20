import os, subprocess, sys

# Look for a file:
revfile='../revision.txt'
if os.path.isfile( revfile):
    f = open(revfile)
    if( f):
        print(f.readline())
        f.close()
        sys.exit(0)



# Get Git SHA:
cmd = "git log --pretty=format:%H -n 1"
if len(sys.argv) > 1: cmd += ' "%s"' % sys.argv[1]
proc = subprocess.Popen( cmd, shell=True, bufsize=100000, stdout=subprocess.PIPE)
gitsha = proc.stdout.read()
proc.wait()
if proc.returncode == 0:
	if not isinstance( gitsha, str):
		gitsha = str( gitsha, 'utf-8')
	print( gitsha)
	sys.exit(0)

#Get git logs count:
#cmd = "git log --pretty=format:''"
#if len(sys.argv) > 1: cmd += ' "%s"' % sys.argv[1]
#output = str( subprocess.Popen( cmd, shell=True, bufsize=100000, stdout=subprocess.PIPE).stdout.read())
#count = output.count('\n')
#if count > 2:
#	print( count)
#	sys.exit(0)



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
