#!/usr/bin/python

import os
import shutil

tmp = 'tmp'

if os.path.isdir( tmp):
   shutil.rmtree( tmp)
