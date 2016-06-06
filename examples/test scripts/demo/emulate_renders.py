#!/bin/env python
# -*- coding: utf-8 -*-

import json
import os
import random
import sys
import time

import af
import afnetwork

# Actions = ['nimby','Nimby','free']

from optparse import OptionParser

Parser = OptionParser(usage="usage: %prog [options]", version="%prog 1.0")
Parser.add_option('-n', '--name',        dest='name',        type='string', default='_e_r_',  help='Renders name prefix')
Parser.add_option('-c', '--count',       dest='count',       type='int',    default=10,       help='Renders count')
Parser.add_option('-u', '--user',        dest='user',        type='string', default='render', help='User name')
Parser.add_option('-w', '--worktime',    dest='worktime',    type='float',  default=10.0,     help='Renders work period time')
Parser.add_option('-o', '--offlinetime', dest='offlinetime', type='float',  default=10.0,     help='Renders offline period time')
Options, Args = Parser.parse_args()


def sendAction(i_type, i_mask, i_operation, i_params=None):
	action = dict()
	action['type'] = i_type
	action['user_name'] = Options.user
	action['host_name'] = Options.name
	action['mask'] = i_mask

	if i_operation is not None:
		operation = dict()
		operation['type'] = i_operation
		action['operation'] = operation

	if i_params is not None:
		action['params'] = i_params

	# print( json.dumps( {'action': action}))
	status, answer = afnetwork.sendServer(json.dumps({'action': action}), False)
	return status


def rendersExit():
	return sendAction('renders', Options.name + '.*', 'exit')


print('Renders name = "%s"' % Options.name)
print('Renders count = %d' % Options.count)

Cmd = 'afrender -hostname "%s"'
Cmd += ' -username "%s"' % Options.user

cycle = 0
while True:
	cycle += 1

	print('Launching renders: (%d)' % cycle)
	for i in range(0, Options.count):
		hostname = Options.name + ("%03d" % i)
		cmd = Cmd % hostname
		print(cmd)
		os.system(cmd + ' > /dev/null &')

	time.sleep(Options.worktime)

	print('Exiting renders...(%d)' % cycle)
	if not rendersExit():
		break

	time.sleep(Options.offlinetime)

