# -*- coding: utf-8 -*-
import nuke

import os

knobnames = ['file', 'proxy']


def do(tocut):
	cwd = nuke.toNode('root').knob('project_directory').getEvaluatedValue()
	if len(cwd) == 0:
		cwd = os.getcwd()
	if len(cwd) == 0:
		return

	nodes = []
	nodes.extend(nuke.allNodes('Read'))
	nodes.extend(nuke.allNodes('Write'))

	for node in nodes:
		for knobname in knobnames:
			knob = node.knob(knobname)
			if knob is not None:
				value = knob.value()
				if value is not None and value != '':
					if tocut:
						knob.setValue(value.replace(cwd + '/', ''))
					elif value[0] != '/':
						knob.setValue(cwd + '/' + value)


def cut():
	do(True)


def add():
	do(False)
