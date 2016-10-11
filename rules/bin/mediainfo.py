#!/usr/bin/env python
# -*- coding: utf-8 -*-

import json
import os
import sys
import subprocess

import cgruutils

def processMovie( i_file, o_file = '.rules/walk.json'):

    out = dict()

    out['infile'] = i_file

    if not os.path.isfile( out['infile']):
        out['error'] = 'Input file does not exist.'
        return out

    params = {}
    params['frame_count'] = 'FrameCount'
    params['fps'] = 'FrameRate'
    params['codec'] = 'Codec'
    params['width'] = 'Width'
    params['height'] = 'Height'

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

    # Write to file (to store in walk):
    obj = {}
    # Try to read existing file if any:
    o_file = os.path.join( os.path.dirname( i_file), o_file)
    if os.path.isfile( o_file):
        try:
            file = open( o_file, 'r')
            obj = json.load( file)
            file.close()
        except:
            obj = {}
    # Add object:
    if not 'files' in obj: obj['files'] = {}
    obj['files'][os.path.basename(i_file)] = {'video':inform}
    # Write file:
    with open( o_file, 'w') as file:
        json.dump( obj, file, indent=1)

    # Return this object:
    out['mediainfo'] = {'video':inform}
    return out


if __name__ == '__main__':

    if len(sys.argv) < 2:
        print('Input file is not specified.')
        sys.exit(1)

    out = processMovie( sys.argv[1])
    print( json.dumps( out, indent=4))

