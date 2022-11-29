AUTH_TYPE = None

GUEST_SITES = ['rules.cgru.info', '127.0.0.1']
SERVER_NAME = None
AUTH_RULES = False

COUNTER = 0


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
