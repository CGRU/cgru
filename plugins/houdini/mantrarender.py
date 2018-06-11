#!/usr/bin/env python

import subprocess
import os
import sys
import signal
import tempfile
import shutil

tmpdir = ''


def interrupt(signum, frame):
    """Interrupt function to delete temp directory
    """
    if tmpdir != '':
        if os.path.isdir(tmpdir):
            shutil.rmtree(tmpdir)
    exit('\nInterrupt received (mantrarender.py)...')

# Set interrupt function:
signal.signal(signal.SIGTERM, interrupt)
signal.signal(signal.SIGABRT, interrupt)
if sys.platform.find('win') != 0:
    signal.signal(signal.SIGQUIT, interrupt)
signal.signal(signal.SIGINT, interrupt)

print('')

# Check environment:
filter_ = os.getenv('HOUDINI_CGRU_PATH')
if filter_ is None or filter_ == '':
    print('Error: HOUDINI_CGRU_PATH is not set, can`t find mantra python '
          'filer location.')
    exit(1)


def UsageExit(msg=None):
    if msg is not None:
        print(msg)
    else:
        print('Error: Invalid arguments.')
    print('Usage: %s [dtc] [divx divy numtile] -R [arguments to mantra]' %
          os.path.basename(sys.argv[0]))
    print('d - Delete ROP file after successful render '
          '(don`t use it with tile render!).')
    print('t - Enable render in temporary folder.')
    print('c - Crop render region to render one tile.')
    print('divx, divy, numtile - Tile render options: X, Y division and tile '
          'number.')
    print('-R - separator for arguments to mantra ("-v A" will be added '
          'automatically).')
    exit(1)


tilerender = False

# Mantra arguments position:
argspos = 0
if '-R' not in sys.argv:
    UsageExit()
for arg in sys.argv:
    if arg != '-R':
        argspos += 1
    else:
        break
if argspos >= (len(sys.argv) - 1):
    UsageExit('No arguments for render command specified.')

# Tile render:
if 'c' in sys.argv[1]:
    divx = int(sys.argv[2])
    divy = int(sys.argv[3])
    numtile = int(sys.argv[4])
    if divx < 1:
        divx = 1
    if divy < 1:
        divy = 1
    if numtile < 0:
        numtile = 0
    if numtile >= divx * divy:
        numtile = divx * divy - 1
    tilerender = True

# Temp directory:
if 't' in sys.argv[1]:
    tmpdir = tempfile.mkdtemp('.afrender.mantra')
    if os.path.exists(tmpdir):
        print('Rendering in temporary directory:')
        print(tmpdir)
        sys.stdout.flush()
    else:
        print('Error creating temp directory.')
        sys.exit(1)

# Delete files:
ropfile = ''
if 'd' in sys.argv[1]:
    for a in range(argspos, len(sys.argv)):
        if sys.argv[a] == '-f':
            a += 1
            if a < len(sys.argv):
                ropfile = sys.argv[a]
                break

# Construct a command:
mantra = 'mantra'
app_dir = os.getenv('APP_DIR')
if app_dir is not None:
    app_dir = os.path.join(app_dir, 'bin')
    mantra = os.path.join(app_dir, 'mantra')
    if sys.platform.find('win') == 0:
        mantra += '.exe'
if argspos > 1:
    filter_ = filter_.replace('\\', '/')
    filter_ += '/mantrafilter.py'
    if tilerender:
        filter_ += ' %d %d %d' % (divx, divy, numtile)
    if tmpdir != '':
        filter_ += ' ' + tmpdir
    cmd = [mantra, '-P', filter_, '-V', '1a']
else:
    cmd = [mantra, '-V', '1a']

# Append arguments for mantra:
for i in range(argspos + 1, len(sys.argv)):
    cmd.append(sys.argv[i])

# Launch mantra:
p = subprocess.Popen(cmd)
exitcode = p.wait()

if tmpdir != '':
    # Remove temp directory:
    try:
        shutil.rmtree(tmpdir)
    except:
        print('Unable to remove temporary directory:')
        print(tmpdir)
        print(str(sys.exc_info()[1]))

if exitcode == 0:
    # Remove ROP file:
    if ropfile != '':
        print('Removing ROP file "%s"' % ropfile)
        if os.path.isfile(ropfile):
            try:
                os.remove(ropfile)
            except:
                print('Unable to remove ROP file:')
                print( ropfile)
                print( str(sys.exc_info()[1]))
        else:
            print('ROP file does not exist.')

sys.exit(exitcode)
