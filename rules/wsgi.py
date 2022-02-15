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

rusrv.environ.initApp(APP_ROOT)

os.chdir(rusrv.environ.CGRU_LOCATION)

def application(environ, start_response):

    out = dict()

    rusrv.environ.initEnv(environ)
    rusrv.environ.initSession(environ)

    request = None

    if 'CONTENT_TYPE' in environ and environ['CONTENT_TYPE'].find('multipart/form-data') != -1:
        rusrv.functions.upload(environ, out)
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
            # Authentication
            if rusrv.environ.AUTH_RULES:
                if 'digest' in request:
                    if rusrv.admin.http_digest_validate(request['digest'], out):
                        rusrv.environ.USER_ID = request['digest']['username']
                    else:
                        out['auth_status'] = 'Wrong credentials.'
                        out['auth_error'] = True
                        rusrv.environ.USER_ID = None
                    out['nonce'] = rusrv.functions.randMD5()
                    del request['digest']

            if 'login' in request and rusrv.environ.AUTH_RULES and rusrv.environ.AUTH_TYPE != 'AUTH_DIGEST':
                if not 'digest' in request['login']:
                    out['error'] = 'No digest in login object.'
                else:
                    if rusrv.admin.http_digest_validate(request['login']['digest'], out):
                        out['status'] = 'success'
                    else:
                        out['error'] = 'Invalid login.'
            elif 'walkdir' in request:
                out['walkdir'] = []
                for path in request['walkdir']:
                    for rem in ['../', '../', '..']:
                        path = path.replace(rem, '')
                    walkdir = dict()
                    rusrv.functions.walkDir(request, path, walkdir, 0)
                    out['walkdir'].append(walkdir)
            elif 'afanasy' in request:
                rusrv.afanasy.sendJob(request, out)
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

