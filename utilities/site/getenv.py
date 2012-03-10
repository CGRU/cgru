#!/usr/bin/env python

import os

def getenv_errorexit( env):
   value = os.getenv(env, '')
   if value == '':
      print 'Error: getenv: %s is not set.' % env
      exit(1)
   return value

DIRS_EXLUDE_FINDSUB  = getenv_errorexit('DIRS_EXLUDE_FINDSUB').split(';')
DIRS_INLUDE_FILEEXT  = getenv_errorexit('DIRS_INLUDE_FILEEXT').split(';')
HEADER_MARKER        = getenv_errorexit('HEADER_MARKER')
FOOTER_MARKER        = getenv_errorexit('FOOTER_MARKER')
LOCAL_BEGIN          = getenv_errorexit('LOCAL_BEGIN')
LOCAL_END            = getenv_errorexit('LOCAL_END')
REMOTE_BEGIN         = getenv_errorexit('REMOTE_BEGIN')
REMOTE_END           = getenv_errorexit('REMOTE_END')

SRCDIR               = getenv_errorexit('SRCDIR')
TMPDIR               = getenv_errorexit('TMPDIR')
CGRUEXPORT           = getenv_errorexit('CGRUEXPORT')
WWWROOT              = getenv_errorexit('WWWROOT')
