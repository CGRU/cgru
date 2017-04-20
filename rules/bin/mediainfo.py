#!/usr/bin/env python
# -*- coding: utf-8 -*-

import json
import os
import sys
import subprocess

import cgruutils

def processMovie( i_file):

    out = dict()

    out['infile'] = i_file

    if not os.path.isfile( out['infile']):
        out['error'] = 'Input file does not exist.'
        return out

    params = {}
    params['bitdepth'] = 'BitDepth'
    params['chromasubsampling'] = 'ChromaSubsampling'
    params['codec'] = 'Codec'
    params['colorspace'] = 'ColorSpace'
    params['fps'] = 'FrameRate'
    params['frame_count'] = 'FrameCount'
    params['height'] = 'Height'
    params['width'] = 'Width'

    inform = ''
    for key in params:
        if len( inform): inform += ','
        inform += '"%s":"%%%s%%"' % ( key, params[key])
    inform = '--inform=Video;{' + inform + '}'

    data = subprocess.check_output(['mediainfo', inform, out['infile']])
    data = cgruutils.toStr( data)

    inform = None
    try:
        inform = json.loads( data)
    except:
        inform = None
        out['data'] = data
        out['error'] = 'JSON load error'
        return out

    for key in inform:
        if inform[key].isdigit():
            inform[key] = int(inform[key])
        else:
            try:
                inform[key] = float( inform[key])
            except:
                pass

    # Return this object:
    out['mediainfo'] = {'video':inform}
    return out


if __name__ == '__main__':

    if len(sys.argv) < 2:
        print('Input file is not specified.')
        sys.exit(1)

    out = processMovie( sys.argv[1])
    print( json.dumps( out, indent=4))

