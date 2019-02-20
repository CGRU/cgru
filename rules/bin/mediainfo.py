#!/usr/bin/env python
# -*- coding: utf-8 -*-

import json
import os
import sys
import subprocess
import traceback

import cgruutils

def processMovie(i_file):

    out = dict()

    out['infile'] = i_file

    if not os.path.isfile(out['infile']):
        out['error'] = 'Input file does not exist.'
        return out

    params = {}
    params['bitdepth'] = 'BitDepth'
    params['chromasubsampling'] = 'ChromaSubsampling'
    params['codec'] = 'Codec'
    params['colorspace'] = 'ColorSpace'
    params['fps'] = 'FrameRate'
    params['frame_count'] = 'FrameCount'
    params['width'] = 'Width'
    params['height'] = 'Height'

    inform = ''
    for key in params:
        if len(inform): inform += ','
        inform += '""%s"":""%%%s%%""' % (key, params[key])
    inform = '--Inform=Video;{' + inform + '}'

    data = subprocess.check_output(['mediainfo', inform, out['infile']])
    data = cgruutils.toStr(data)

    inform = None
    try:
        inform = json.loads(data)
    except:
        inform = None
        out['data'] = data
        out['error'] = 'json load error'
        print(traceback.format_exc())
        return out

    # remove empty keys:
    inform = dict([(k,v) for k,v in inform.items() if len(v)>0])

    for key in inform:
        if inform[key].isdigit():
            inform[key] = int(inform[key])
        else:
            try:
                inform[key] = float(inform[key])
            except:
                pass

    # return this object:
    out['mediainfo'] = {'video':inform}
    return out


def processExif(i_file):
    out = dict()

    out['infile'] = i_file

    if not os.path.isfile(out['infile']):
        out['error'] = 'Input file does not exist.'
        return out

    params = {}
    params['BitDepth'] = 'bitdepth'
    params['ColorType'] = 'colortype'
    params['Compression'] = 'compression'
    params['ImageWidth'] = 'width'
    params['ImageHeight'] = 'height'
    params['Artist'] = 'artist'
    params['Comment'] = 'comment'

    cmd_args = ['exiftool','-S','-fast2']
    for key in params:
        cmd_args.append('-'+key)
    cmd_args.append(i_file)

    try:
        data = subprocess.check_output(cmd_args)
    except:
        out['error'] = 'Failed to execute process.'
        return out

    data = cgruutils.toStr(data)
    data = data.splitlines()
    exif = dict()

    for line in data:
        if len(line) == 0: continue
        fields = line.split(': ', 1)
        if len(fields) != 2: continue
        key = fields[0]
        val = fields[1]
        if len(val) == 0: continue
        if not key in params: continue

        if val.isdigit(): val = int(val)
        else:
            try: val = float(val)
            except: pass

        exif[params[key]] = val


    # return this object:
    out['mediainfo'] = {'exif':exif}
    return out


def processFile(i_file):
    if cgruutils.isMovieExt(i_file):
        return processMovie(i_file)
    return processExif(i_file)

if __name__ == '__main__':

    if len(sys.argv) < 2:
        print('Input file is not specified.')
        sys.exit(1)

    out = processFile(sys.argv[1])
    print(json.dumps(out, indent=4))

