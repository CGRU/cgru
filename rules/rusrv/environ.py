USER_ID = None
GROUPS = None

AUTH_TYPE = None

HT_ACCESS_FILE_NAME = '.htaccess'
HT_GROUPS_FILE_NAME = '.htgroups'
HT_DIGEST_FILE_NAME = '.htdigest'

FILE_MAX_LENGTH     =  3000000

CGRU_LOCATION = None
CGRU_VERSION = None

SKIPFILES      = ['.', '..', HT_ACCESS_FILE_NAME, HT_GROUPS_FILE_NAME, HT_DIGEST_FILE_NAME]
GUESTCANCREATE = ['status.json', 'comments.json']
GUESTCANEDIT   = ['comments.json']

COUNTER = 0


import os
import sys


from rusrv import functions


def initApp(i_app_root):
    global CGRU_LOCATION
    global CGRU_VERSION

    CGRU_LOCATION = os.path.dirname(i_app_root)
    CGRU_VERSION = functions.fileRead(os.path.join(CGRU_LOCATION,'version.txt'))

    os.environ['CGRU_LOCATION'] = CGRU_LOCATION
    os.environ['CGRU_VERSION'] = CGRU_VERSION
    os.environ['AF_ROOT'] = os.path.join(CGRU_LOCATION, 'afanasy')

    sys.path.append(os.path.join(CGRU_LOCATION, 'lib', 'python'))
    sys.path.append(os.path.join(CGRU_LOCATION, 'afanasy', 'python'))


def initEnv(i_environ):
    global COUNTER
    global SERVER_SOFTWARE
    global MOD_WSGI_VERSION
    global AUTH_TYPE

    COUNTER += 1
    SERVER_SOFTWARE = i_environ['SERVER_SOFTWARE']
    MOD_WSGI_VERSION = i_environ['mod_wsgi.version']
    if 'AUTH_TYPE' in i_environ:
        AUTH_TYPE = i_environ['AUTH_TYPE']


def initSession(i_environ):
    global REMOTE_ADDR
    global USER_ID

    REMOTE_ADDR = i_environ['REMOTE_ADDR']
    if 'REMOTE_USER' in i_environ:
        USER_ID = i_environ['REMOTE_USER']

