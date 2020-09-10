#!/usr/bin/env python3

import os
import shutil

tmp = 'tmp'

if os.path.isdir(tmp):
	shutil.rmtree(tmp)
