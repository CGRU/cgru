import cgi
import json
import os
import sys
import traceback

import cgitb; cgitb.enable()

os.umask(0000)

APP_FILE = os.path.realpath(__file__)
APP_ROOT = os.path.dirname(APP_FILE)
sys.path.append(APP_ROOT)

import rusrv

rusrv.functions.initApp(APP_ROOT)

os.chdir(rusrv.environ.CGRU_LOCATION)

def application(environ, start_response):

    out = dict()

    rusrv.functions.initEnv(environ)
    rusrv.functions.initSession(environ)

    request = None

    if 'CONTENT_TYPE' in environ and environ['CONTENT_TYPE'].find('multipart/form-data') != -1:
        upload(environ, out)
    else:
        content_length = 0
        if 'CONTENT_LENGTH' in environ:
            content_length = int(environ['CONTENT_LENGTH'])

        if content_length:
            try:
                request = json.load(environ['wsgi.input'])
            except:
                request = None
                out['error'] = 'Request json load error.'
                out['info'] = '%s' % traceback.format_exc()

    rawout = None
    if request:
        if len(request):
            if 'walkdir' in request:
                out['walkdir'] = []
                for path in request['walkdir']:
                    for rem in ['../', '../', '..']:
                        path = path.replace(rem, '')
                    walkdir = dict()
                    rusrv.functions.walkDir(request, path, walkdir, 0)
                    out['walkdir'].append(walkdir)
            else:
                found = False
                for key in request:
                    func = 'req_%s' % key
                    if hasattr(rusrv.requests, func):
                        found = True
                        rawout = getattr(rusrv.requests, func)(request[key], out)
                    else:
                        out['error'] = 'Request not recognized: ' + key
                        break
        else:
            out['error'] = 'Request is empty.'

    status = '200 OK'
    if rawout is None:
        rawout = json.dumps(out)
    rawout = rawout.encode()
    response_headers = [('Content-type', 'text/plain'), ('Content-Length', str(len(rawout)))]
    start_response(status, response_headers)

    return [rawout]

