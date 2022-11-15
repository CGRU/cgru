AUTH_TYPE = None

GUEST_SITES = ['rules.cgru.info', '127.0.0.1']
SERVER_NAME = None
AUTH_RULES = False

CGRU_LOCATION = None
CGRU_VERSION = None

COUNTER = 0


import os
import sys


from rulib import functions


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
    COUNTER += 1

    if COUNTER > 1:
        return

    global SERVER_SOFTWARE
    global MOD_WSGI_VERSION
    global AUTH_TYPE
    global SERVER_NAME
    global AUTH_RULES
    SERVER_SOFTWARE = i_environ['SERVER_SOFTWARE']
    MOD_WSGI_VERSION = i_environ['mod_wsgi.version']
    if 'AUTH_TYPE' in i_environ:
        AUTH_TYPE = i_environ['AUTH_TYPE']
    SERVER_NAME = i_environ['SERVER_NAME']
    if SERVER_NAME in GUEST_SITES:
        AUTH_RULES = True
