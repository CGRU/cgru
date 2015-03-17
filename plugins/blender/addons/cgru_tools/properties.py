# -*- coding: utf-8 -*-

import bpy
from bpy.props import (
	IntProperty,
	BoolProperty,
	StringProperty,
	PointerProperty,
)


class CGRUProperties(bpy.types.PropertyGroup):
	# General:
	jobname = StringProperty(name='Job Name',
		description='Job Name. Scene name if empty.', maxlen=512, default='')
	fpertask = IntProperty(name='Per Task', description='Frames Per One Task',
		min=1, default=1)
	sequential = IntProperty(name='Sequential',
		description='Solve task with this step at first.',
		default=1)
	pause = BoolProperty(name='Start Job Paused',
		description='Send job in offline state.', default=0)
	packLinkedObjects = BoolProperty(name='Pack Linked Objects',
		description='Make Local All linked Groups and Objects', default=0)
	relativePaths = BoolProperty(name='Relative Paths',
		description='Set Relative Paths for all Textures and Objects', default=0)
	packTextures = BoolProperty(name='Pack Textures',
		description='Pack all Textures into the Blend File.', default=0)
	splitRenderLayers = BoolProperty(name='Split Render Layers',
		description='Split Render layer in blocks. Warning: this option disable post-processing passes (compositing nor seqeuncer are execute).', default=0)

	# Render Settings:
	filepath = StringProperty(name='File Path', description='Set File Path. You can use ### to set number of frames',
		maxlen=512, default='')

	# Paramerets:
	priority = IntProperty(name='Priority',
		description='Job order in user jobs list.', min=-1,
		max=250, default=-1)
	maxruntasks = IntProperty(name='Max Run Tasks',
		description='Maximum number of running tasks.',
		min=-1, max=9999, default=-1)
	dependmask = StringProperty(name='Depend Mask',
		description='Jobs to wait pattern.',
		maxlen=512, default='')
	dependmaskglobal = StringProperty(name='Global Depend',
		description='All users jobs wait pattern.',
		maxlen=512, default='')
	hostsmask = StringProperty(name='Hosts Mask',
		description='Hosts to run pattern.', maxlen=512,
		default='')
	hostsmaskexclude = StringProperty(name='Exclude Hosts',
		description='Hosts to ignore pattern.',
		maxlen=512, default='')


