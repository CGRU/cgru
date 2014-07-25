#!/usr/bin/env python
# -*- coding: utf-8 -*-

filename = 'common/mp4v2/mpeg4ip.h'

import shutil

with open(filename, 'r') as f:
	lines = f.readlines()

shutil.move(filename, filename + '.old')

with open(filename, 'w') as f:
	line_num = 1
	for line in lines:
		if line_num == 126:
			line = '// ' + line
			print('%d: %s' % (line_num, line))
		f.write(line)
		line_num += 1
	f.close()
