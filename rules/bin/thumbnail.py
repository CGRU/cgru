#!/usr/bin/env python
# -*- coding: utf-8 -*-

import json
import re
import os
import sys
import time

import cgruutils

import mediainfo

from optparse import OptionParser

Parser = OptionParser(
    usage="%prog [options]\ntype \"%prog -h\" for help",
    version="%prog 1.0"
)

Parser.add_option('-x', '--xres',       dest='xres',       type='int',          default=160,                help='X Resolution')
Parser.add_option('-y', '--yres',       dest='yres',       type='int',          default=90,                 help='Y Resolution')
Parser.add_option('-n', '--number',     dest='number',     type='int',          default=0,                  help='Number of images')
Parser.add_option('-i', '--input',      dest='input',      type='string',       default='',                 help='Input image')
Parser.add_option('-o', '--output',     dest='output',     type='string',       default='thumbnail.jpg',    help='Output image')
Parser.add_option('-w', '--walk',       dest='walk',       type='string',       default='.rules/walk.json', help='Output mediainfo in walk data')
Parser.add_option('-t', '--time',       dest='time',       type='int',          default=0,                  help='Midification test time')
Parser.add_option('-s', '--skip',       dest='skip',       type='string',       default='matte,mask,layer', help='Skip folders and folders, comma separated list.')
Parser.add_option(      '--nomovie',    dest='nomovie',    action='store_true', default=False,              help='Skip movie files.')
Parser.add_option('-c', '--colorspace', dest='colorspace', type='string',       default='auto',             help='Specity input colorpace.')
Parser.add_option('-f', '--force',      dest='force',      action='store_true', default=False,              help='Force creation, no modification time check.')
Parser.add_option('-a', '--avcmd',      dest='avcmd',      type='string',       default='ffmpeg',           help='AV command (ffmpeg).')
Parser.add_option('-V', '--verbose',    dest='verbose',    action='store_true', default=False,              help='Verbose mode.')
Parser.add_option('-D', '--debug',      dest='debug',      action='store_true', default=False,              help='Debug mode.')

Options, Args = Parser.parse_args()

out = dict()
out['thumbnail'] = Options.output

SkipFolders = Options.skip.split(',')


def errorExit(i_err):
    out['error'] = i_err
    print(json.dumps(out))
    sys.exit(1)


def statusExit(i_msg):
    out['status'] = i_msg
    print(json.dumps(out))
    sys.exit(0)


if Options.input == '':
    errorExit('Input not specified.')

Images = []
Movie = None
MTime = 0
if os.path.isfile(Options.output):
    MTime = os.path.getmtime(Options.output)
    if Options.verbose:
        print('Thumbnail "%s" already exists.' % Options.output)
    if Options.time > 0 and not Options.force:
        if time.time() - MTime < Options.time:
            statusExit('uptodate')

if Options.input.find(',') != -1 or os.path.isdir(Options.input):
    if Options.number == 0:
        Options.number = 3
    folders = [Options.input]
    if folders[0].find(',') != -1:
        folders = folders[0].split(',')
    cur_mtime = 0
    for folder in folders:
        if Options.verbose:
            print('Scanning folder "%s"...' % folder)
        if not os.path.isdir(folder):
            # print('ERROR: folder "%s" does not exist.' % folder)
            continue
        for root, dirs, files in os.walk(folder):
            # Skip folder:
            if len(files) == 0:
                continue
            to_skip = False

            root_basename = os.path.basename(root)
            if len(root_basename):
                if root_basename[0] in '._':
                    to_skip = True
            if not to_skip:
                for skip in SkipFolders:
                    if root_basename.lower().find(skip) != -1:
                        to_skip = True
                        break
            if to_skip:
                if Options.verbose:
                    print('Skipping: "%s"' % root)
                continue

            if Options.verbose:
                print('Scanning folder "%s"...' % root)

            images = []
            for afile in files:
                if afile[0] in '._':
                    continue
                if cgruutils.isImageExt(afile):
                    images.append(afile)
                elif cgruutils.isMovieExt(afile) and not Options.nomovie:
                    new_movie = os.path.join(root, afile)
                    new_mtime = int(os.path.getmtime(new_movie))
                    if new_movie > cur_mtime:
                        Movie = new_movie
                        cur_mtime = new_mtime

            if len(images) == 0:
                continue
            new_mtime = int(os.path.getmtime(os.path.join(root, images[0])))
            if new_mtime > cur_mtime:
                Images = []
                Movie = None
                images.sort()
                for i in range(Options.number):
                    num = int(len(images) * (i + 1.0) / (Options.number + 1.0))
                    Images.append(os.path.join(root, images[num]))
                cur_mtime = new_mtime
else:
    if not os.path.isfile(Options.input):
        errorExit('Input does not exist %s' % Options.input)

    if Options.verbose:
        print('Input is a file.')

    if cgruutils.isImageExt(Options.input):
        if Options.number == 0:
            Options.number = 1
        Images.append(Options.input)
    elif cgruutils.isMovieExt(Options.input):
        Movie = Options.input
        if Options.number == 0:
            Options.number = 3
    else:
        statusExit('skipped')
    cur_mtime = int(os.path.getmtime(Options.input))

if len(Images) == 0 and Movie is None:
    errorExit('Can`t find images in %s' % Options.input)

if Options.verbose:
    if len(Images):
        print('Images:')
        Movie = None
        for img in Images:
            print(img)
    if Movie is not None:
        print('Movie: ' + Movie)

if MTime >= cur_mtime:
    if Options.force:
        if Options.verbose:
            print('Forcing thumbnail creation')
    else:
        os.utime(Options.output, None)
        statusExit('updated')

OutDir = os.path.dirname(Options.output)
if OutDir != '':
    if not os.path.isdir(OutDir):
        os.makedirs(OutDir)
        if not os.path.isdir(OutDir):
            errorExit('Can`t create output folder %s' % OutDir)

Cmds = []
Thumbnails = []

if Movie is not None:

    # Try to get movie frames count:
    frame_count = 3 # < this will be the default value
    inf_obj = mediainfo.processMovie( Movie)
    if inf_obj and 'mediainfo' in inf_obj and 'video' in inf_obj['mediainfo']:
        frame_count = inf_obj['mediainfo']['video']['frame_count']

        # Write to file (to store in walk):
        walk_obj = {}
        # Try to read existing file if any:
        walk_file = os.path.join( os.path.dirname( Movie), Options.walk)
        if os.path.isfile( walk_file):
            try:
                file = open( walk_file, 'r')
                walk_obj = json.load( file)
                file.close()
            except:
                walk_obj = {}
        # Add file to walk object files:
        if not 'files' in walk_obj: walk_obj['files'] = {}
        walk_obj['files'][os.path.basename( Movie)] = inf_obj['mediainfo']
        # Write walk file:
        o_dir = os.path.dirname( walk_file) 
        if not os.path.isdir( o_dir):
            os.makedirs( o_dir)
        try:
            with open( walk_file, 'w') as file:
                json.dump( walk_obj, file, indent=1)
        except:       
            pass


    # Calculate thumbnail interval:
    mod = frame_count / ( Options.number + 1 )
    # Limit thumbnail interval to 1000 for speed reasons:
    if mod > 1000: mod = 1000

    frame = os.path.join(OutDir, 'frame.%07d.jpg')
    cmd = '%s -y' % Options.avcmd
    cmd += ' -i "%s"' % Movie
    cmd += ' -f image2'
    cmd += ' -filter:v "select=gte(n\,%d)*not(mod(n\,%d))"' % ( mod, mod)
    cmd += ' -frames:v %d' % Options.number
    cmd += ' -vsync vfr'
    cmd += ' "%s"' % frame
    
    for i in range(0, Options.number):
        Images.append(frame % (i + 1))
    Cmds.append(cmd)

cmd = 'convert'
cmd += ' "%s"'
cmd += ' -layers flatten'
cmd += ' -alpha Deactivate'
if Movie is None:
    if Options.colorspace != 'auto':
        colorspace = Options.colorspace
        if colorspace == 'extension':
            imgtype = Images[0].rfind('.')
            if imgtype > 0:
                imgtype = Images[0][imgtype + 1:].lower()
            if imgtype == 'exr':
                colorspace = 'RGB'
            elif imgtype == 'dpx':
                colorspace = 'Log'
            elif imgtype == 'cin':
                colorspace = 'Log'
            else:
                colorspace = 'sRGB'
        cmd += ' -set colorspace ' + colorspace
    cmd += ' -colorspace sRGB'

cmd += ' -resize %dx%d' % (Options.xres, Options.yres)
cmd += ' "%s"'
if len(Images) == 1:
    Cmds.append(cmd % (Images[0], Options.output))
else:
    for i in range(len(Images)):
        thumbnail = os.path.join(OutDir, 'thumbnail_%d.jpg' % i)
        Cmds.append(cmd % (Images[i], thumbnail))
        Thumbnails.append(thumbnail)

if len(Images) > 1:
    cmd = 'montage'
    cmd += ' -geometry +0+0'
    for img in Thumbnails:
        cmd += ' "%s"' % img
    cmd += ' -alpha Off -strip'
    cmd += ' "%s"' % Options.output
    Cmds.append(cmd)

if Options.verbose or Options.debug:
    for cmd in Cmds:
        print(cmd)

if Options.debug:
    print('Debug mode. Exiting...')
    sys.exit(0)

for cmd in Cmds:
    os.system(cmd)

print(json.dumps(out))
