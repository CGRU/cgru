# -*- coding: utf-8 -*-

import af
import cgruconfig
import render

from datetime import datetime, timedelta

nimby_set = False
free_set = False


def setnimby():
	cmd = af.Cmd().renderSetNimby()
	render.refreshAfter()


def setNIMBY():
	cmd = af.Cmd().renderSetNIMBY()
	render.refreshAfter()


def setFree():
	cmd = af.Cmd().renderSetFree()
	render.refreshAfter()

def setFreeUnpause():
	cmd = af.Cmd().renderSetFreeUnpause()
	render.refreshAfter()


def ejectTasks():
	cmd = af.Cmd().renderEjectTasks()
	render.refreshAfter()


def ejectNotMyTasks():
	cmd = af.Cmd().renderEjectNotMyTasks()
	render.refreshAfter()


def refresh(reset=False):
	global nimby_set
	global free_set

	if reset:
		nimby_set = False
		free_set = False

	toset_nimby = False
	toset_free = False
	toallow_tasks = False
	toeject_tasks = False

	# Get current time and day:
	daytime = datetime.now()
	cur_time = daytime.strftime('%H:%M')

	# Check yesterday free:
	yesterday = (daytime - timedelta(days=1)).strftime('%a').lower()
	var = 'nimby_' + yesterday
	if var in cgruconfig.VARS:
		values = cgruconfig.VARS[var].split(' ')
		time_begin = values[0]
		time_end = values[1]
		if time_end < time_begin and 'Enable' in values:
			if cur_time > time_end:
				toset_free = True
			else:
				toset_nimby = True
				if 'Eject' in values:
					toeject_tasks = True
				if 'nimby' in values:
					toallow_tasks = True
				if 'NIMBY' in values:
					toallow_tasks = False

	# Check today:
	var = 'nimby_' + daytime.strftime('%a').lower()
	if var in cgruconfig.VARS:
		values = cgruconfig.VARS[var].split(' ')
		time_begin = values[0]
		time_end = values[1]
		if time_begin != time_end and 'Enable' in values:
			# Check free if it not tomorrow:
			if cur_time > time_end > time_begin:
				toset_free = True
			# Check Nimby:
			elif cur_time > time_begin:
				toset_nimby = True
				if 'Eject' in values:
					toeject_tasks = True
				if 'nimby' in values:
					toallow_tasks = True
				if 'NIMBY' in values:
					toallow_tasks = False

	# Set state:
	if toset_nimby:
		if not nimby_set:
			if toallow_tasks:
				setnimby()
				if toeject_tasks:
					ejectNotMyTasks()
			else:
				setNIMBY()
				if toeject_tasks:
					ejectTasks()
			nimby_set = True
			free_set = False
	elif toset_free:
		if not free_set:
			setFree('(keeper nimby schedule)')
			free_set = True
			nimby_set = False
