# -*- coding: utf-8 -*-

import os
import re

def newSequence( i_name = None, i_prefix = None, i_suffix = None, i_padding = None, i_num = None):
	seq = dict()

	seq['seq']     = False
	seq['name']    = i_name
	seq['prefix']  = i_prefix
	seq['suffix']  = i_suffix
	seq['padding'] = i_padding
	seq['first']   = i_num
	seq['last']    = i_num
	seq['count']   = 1

	return seq

def isNextFrame( i_seq, i_prefix, i_suffix, i_padding, i_num):
	if i_seq['prefix']  != i_prefix:  return False
	if i_seq['suffix']  != i_suffix:  return False
	if i_seq['padding'] != i_padding: return False
	if i_seq['last']    != i_num - 1: return False

	i_seq['seq'] = True
	i_seq['last'] += 1
	i_seq['count'] += 1

	return True

def cgruSequence( i_files, i_verbose = False):

	if len(i_files) == 0: return []

	i_files.sort()

	out = []
	seq = None

	for afile in i_files:

		name = os.path.basename( afile)

		digits = re.findall('\d+', name)

		if len(digits) == 0:
			if seq is not None: out.append(seq)
			seq = newSequence(afile)
			out.append(seq)
			seq = None
			continue

		digits = digits[-1]
		dpos = name.rfind(digits)
		prefix = os.path.join( os.path.dirname( afile), name[:dpos])
		suffix = name[dpos+len(digits):]
		padding = len(digits)
		num = int(digits)

		if seq is not None:
			if isNextFrame( seq, prefix, suffix, padding, num):
				continue
			else:
				out.append(seq)

		seq = newSequence( afile, prefix, suffix, padding, num)

	if seq is not None:
		out.append(seq)

	if i_verbose:
		for seq in out:
			if seq['seq']:
				print("%s%s%s %d-%d=%d" % (seq['prefix'],('#'*seq['padding']),seq['suffix'],seq['first'],seq['last'],seq['count']))
			else:
				print('%s' % seq['name'])

	return out

