# -*- coding: utf-8 -*-

import json
import re
import os
import sys
import time

import cgruutils

import mediainfo

from argparse import ArgumentParser

def thumbnailArgs(i_args):
    o_out = dict()
    o_out['thumbnail'] = i_args.output

    if i_args.input is None:
        o_out['error'] = ('Input not specified.')
        return o_out

    SkipFolders = i_args.skip.split(',')

    Images = []
    Movie = None
    MTime = 0

    if os.path.isfile(i_args.output):
        MTime = os.path.getmtime(i_args.output)
        if i_args.verbose:
            print('Thumbnail "%s" already exists.' % i_args.output)
        if i_args.time > 0 and not i_args.force:
            if time.time() - MTime < i_args.time:
                o_out['status'] = ('uptodate')
                return o_out

    if i_args.input.find(',') != -1 or os.path.isdir(i_args.input):
        if i_args.number == 0:
            i_args.number = 3
        folders = [i_args.input]
        if folders[0].find(',') != -1:
            folders = folders[0].split(',')
        cur_mtime = 0
        for folder in folders:
            if i_args.verbose:
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
                    if i_args.verbose:
                        print('Skipping: "%s"' % root)
                    continue

                if i_args.verbose:
                    print('Scanning folder "%s"...' % root)

                images = []
                for afile in files:
                    if afile[0] in '._':
                        continue
                    if cgruutils.isImageExt(afile):
                        images.append(afile)
                    elif cgruutils.isMovieExt(afile) and not i_args.nomovie:
                        new_movie = os.path.join(root, afile)
                        new_mtime = int(os.path.getmtime(new_movie))
                        if new_mtime > cur_mtime:
                            Movie = new_movie
                            cur_mtime = new_mtime

                if len(images) == 0:
                    continue
                new_mtime = int(os.path.getmtime(os.path.join(root, images[0])))
                if new_mtime > cur_mtime:
                    Images = []
                    Movie = None
                    images.sort()
                    for i in range(i_args.number):
                        num = int(len(images) * (i + 1.0) / (i_args.number + 1.0))
                        Images.append(os.path.join(root, images[num]))
                    cur_mtime = new_mtime
    else:
        if not os.path.isfile(i_args.input):
            o_out['error'] = ('Input does not exist %s' % i_args.input)
            return o_out

        if i_args.verbose:
            print('Input is a file.')

        if cgruutils.isImageExt(i_args.input):
            if i_args.number == 0:
                i_args.number = 1
            Images.append(i_args.input)
        elif cgruutils.isMovieExt(i_args.input):
            Movie = i_args.input
            if i_args.number == 0:
                i_args.number = 3
        else:
            o_out['status'] = ('skipped')
            return o_out
        cur_mtime = int(os.path.getmtime(i_args.input))

    if len(Images) == 0 and Movie is None:
        o_out['error'] = ('Can`t find images in %s' % i_args.input)
        return o_out

    if i_args.verbose:
        if len(Images):
            print('Images:')
            Movie = None
            for img in Images:
                print(img)
        if Movie is not None:
            print('Movie: ' + Movie)

    if MTime >= cur_mtime:
        if i_args.force:
            if i_args.verbose:
                print('Forcing thumbnail creation')
        else:
            os.utime(i_args.output, None)
            o_out['status'] = ('updated')
            return o_out

    OutDir = os.path.dirname(i_args.output)
    if OutDir != '':
        if not os.path.isdir(OutDir):
            os.makedirs(OutDir)
            if not os.path.isdir(OutDir):
                o_out['error'] = ('Can`t create output folder %s' % OutDir)
                return o_out

    Cmds = []
    Thumbnails = []

    if Movie is not None:

        # Try to get movie frames count:
        frame_count = 3 # < this will be the default value
        inf_obj = mediainfo.processMovie( Movie)

        if 'error' in inf_obj:
            if i_args.verbose: print(inf_obj)
            if 'data' in inf_obj:
                data = inf_obj['data']
                if i_args.verbose: print(data)
                data = re.findall(r'frame_count:\d*',data)
                if len(data):
                    frame_count = int(data[0].split(':')[1])
                    print('frame_count:%d' % frame_count)
            print(inf_obj['error'])
            inf_obj = None

        if inf_obj and 'mediainfo' in inf_obj and 'video' in inf_obj['mediainfo']:
            frame_count = inf_obj['mediainfo']['video']['frame_count']

            # Write to file (to store in walk):
            walk_obj = {}
            # Try to read existing file if any:
            walk_file = os.path.join( os.path.dirname( Movie), i_args.walk)
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
        mod = frame_count / ( i_args.number + 1 )
        # Limit thumbnail interval to 1000 for speed reasons:
        if mod > 1000: mod = 1000

        frame = os.path.join(OutDir, 'frame.%07d.jpg')
        cmd = '%s -y' % i_args.avcmd
        cmd += ' -i "%s"' % Movie
        cmd += ' -f image2'
        cmd += r' -filter:v "select=gte(n\,%d)*not(mod(n\,%d))"' % ( mod, mod)
        cmd += ' -frames:v %d' % i_args.number
        cmd += ' -vsync vfr'
        cmd += ' "%s"' % frame
        
        for i in range(0, i_args.number):
            Images.append(frame % (i + 1))
        Cmds.append(cmd)

    cmd = 'convert'
    cmd += ' "%s"'
    cmd += ' -alpha Deactivate'
    cmd += ' -layers flatten'
    if Movie is None:
        if i_args.colorspace != 'auto':
            colorspace = i_args.colorspace
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

    cmd += ' -resize %dx%d' % (i_args.xres, i_args.yres)
    cmd += ' "%s"'
    if len(Images) == 1:
        Cmds.append(cmd % (Images[0], i_args.output))
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
        cmd += ' "%s"' % i_args.output
        Cmds.append(cmd)

    if i_args.verbose or i_args.debug:
        for cmd in Cmds:
            print(cmd)

    if i_args.debug:
        print('Debug mode. Exiting...')
        return o_out

    for cmd in Cmds:
        if i_args.verbose:
            os.system(cmd)
        else:
            os.system(cmd + ' > /dev/null 2>&1')

    return o_out

def parseArgs(i_args = None):
    parser = ArgumentParser(usage="%(prog)s [options]\ntype \"%(prog)s -h\" for help")
    parser.add_argument('-x', '--xres',       dest='xres',       type=int,            default=160,                help='X Resolution')
    parser.add_argument('-y', '--yres',       dest='yres',       type=int,            default=90,                 help='Y Resolution')
    parser.add_argument('-n', '--number',     dest='number',     type=int,            default=0,                  help='Number of images')
    parser.add_argument('-i', '--input',      dest='input',      type=str,            default=None,               help='Input image')
    parser.add_argument('-o', '--output',     dest='output',     type=str,            default='thumbnail.jpg',    help='Output image')
    parser.add_argument('-w', '--walk',       dest='walk',       type=str,            default='.rules/walk.json', help='Output mediainfo in walk data')
    parser.add_argument('-t', '--time',       dest='time',       type=int,            default=0,                  help='Midification test time')
    parser.add_argument('-s', '--skip',       dest='skip',       type=str,            default='matte,mask,layer', help='Skip folders and folders, comma separated list.')
    parser.add_argument(      '--nomovie',    dest='nomovie',    action='store_true', default=False,              help='Skip movie files.')
    parser.add_argument('-c', '--colorspace', dest='colorspace', type=str,            default='auto',             help='Specity input colorpace.')
    parser.add_argument('-f', '--force',      dest='force',      action='store_true', default=False,              help='Force creation, no modification time check.')
    parser.add_argument('-a', '--avcmd',      dest='avcmd',      type=str,            default='ffmpeg',           help='AV command (ffmpeg).')
    parser.add_argument('-V', '--verbose',    dest='verbose',    action='store_true', default=False,              help='Verbose mode.')
    parser.add_argument('-D', '--debug',      dest='debug',      action='store_true', default=False,              help='Debug mode.')
    return parser.parse_args(i_args)

def thumbnail(i_args):
    args = parseArgs([])
    for arg in i_args:
        if arg in args:
            setattr(args, arg, i_args[arg])
    return thumbnailArgs(args)

if __name__ == "__main__":
    print(json.dumps(thumbnailArgs(parseArgs())))
