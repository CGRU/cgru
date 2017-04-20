#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
import shutil
import parsers.mantra

import mantra

TmpDir = ''
TileRender = False

# Process arguments:
argspos = 1

if len(sys.argv) > 3:
    divx = int(sys.argv[1])
    divy = int(sys.argv[2])
    tilenum = int(sys.argv[3])
    argspos = 4
    TileRender = True
    # Calculate tilenum crop region:
    numx = tilenum % divx
    numy = tilenum / divx
    stepx = 1.0 / divx
    stepy = 1.0 / divy
    tilecrop = [
        numx * stepx,
        (numx + 1) * stepx,
        numy * stepy,
        (numy + 1) * stepy
    ]

if len(sys.argv) > argspos:
    TmpDir = sys.argv[argspos]

# Store filtered names:
FilteredNames = []

# Store images dir:
ImagesDir = ''


def filterPlane():
    """This one is called for each image plane - change name here
    NB! only additional image plane files can be changed here - main image
    filename will have to be set through command line
    """
    global ImagesDir
    filename = mantra.property('image:filename')[0]
    print('FilterPlane = "%s"' % filename)
    if filename is None:
        return
    if filename == '':
        return
    if filename.find('nul') == 0:
        return
    if filename in FilteredNames:
        return

    FilteredNames.append( filename)

    if ImagesDir == '':
        ImagesDir = os.path.dirname( filename)

    if TileRender:
        filename += '.tile_%d.exr' % tilenum

    if TmpDir != '':
        filename = os.path.join( TmpDir, os.path.basename( filename))

    print('Mantra plane filename:\n%s' % filename)

    mantra.setproperty('plane:planefile', filename)

    # Check images folder:
    if not os.path.isdir( ImagesDir):
        print('Images folder does not exist:')
        print( ImagesDir)
        print( parsers.mantra.ParserErrorStr)
        sys.stdout.flush()

    if not os.access( ImagesDir, os.W_OK):
        print('Images folder is not writeable:')
        print( ImagesDir)
        print( parsers.mantra.ParserErrorStr)
        sys.stdout.flush()


# A callback to change image crop and write down main image filename:
def filterCamera():
    if TileRender:
        resolution = mantra.property('image:resolution')
        oldcrop = mantra.property('image:crop')
        if oldcrop[0]>oldcrop[1]: # no crop in ifd file
            oldcrop = [0, 1, 0, 1]
        newcrop = [max(tilecrop[0], oldcrop[0]), min(tilecrop[1], oldcrop[1]),
               max(tilecrop[2], oldcrop[2]), min(tilecrop[3], oldcrop[3])]
        # convert to pixels
        newpixelcrop = [int(resolution[0] * newcrop[0]), int(resolution[0] * newcrop[1])-1,
                int(resolution[1] * newcrop[2]), int(resolution[1] * newcrop[3])-1]
        mantra.setproperty('image:pixelcrop', newpixelcrop)


def filterQuit():
    if TmpDir == '':
        return

    if ImagesDir == '':
        return

    badresult = False

    # Copy image files from temp directory:
    allitems = os.listdir(TmpDir)
    if len(allitems) < 1:
        print('Error: No images generated.')
        print(parsers.mantra.ParserErrorStr)
        sys.stdout.flush()

    images_count = 0
    for item in allitems:
        src = os.path.join(TmpDir, item)
        dest = os.path.join(ImagesDir, item)
        if os.path.isfile(dest):
            try:
                print('Deleting old "%s"' % dest)
                os.remove(dest)
            except (IOError, OSError) as e:
                print(str(e))
                print('Unable to remove destination file:')
                print(dest)
                print(parsers.mantra.ParserErrorStr)
                sys.stdout.flush()
        try:
            print('Moving "%s"' % dest)
            shutil.move(src, ImagesDir)
        except (IOError, OSError) as e:
            print('File moved with error:')
            print(str(sys.exc_info()[1]))
            print(src)
            print(dest)

        if os.path.isfile(dest):
            images_count += 1
            print(('@IMAGE@%s' % dest))
        else:
            print('Error: Destination file does not exist.')
            print(parsers.mantra.ParserErrorStr)
            sys.stdout.flush()
    if images_count < len(FilteredNames):
        print('Error: Not enough images generated (%d of %d).' %
              (images_count, len(FilteredNames)))
        print(parsers.mantra.ParserErrorStr)
        sys.stdout.flush()
    else:
        print('Images processed: %d' % images_count)
