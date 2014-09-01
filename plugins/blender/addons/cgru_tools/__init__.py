# -*- coding: utf-8 -*-
bl_info = {
	"name": "CGRU Tools",
	"author": """Timur Hairulin <timurhai@gmail.com>, Paul Geraskin <paulgeraskin@gmail.com> """,
	"version": (1,),
	"blender": (2, 7, 1),
	"location": "3D Viewport",
	"description": "CGRU Tools",
	"warning": "",
	"wiki_url": "",
	"tracker_url": "",
	"category": "Tools"
}


if "bpy" in locals():
	import imp
	imp.reload(afanasy_tools)
else:
	from . import afanasy_tools

import bpy
from bpy.props import *


class ORESettings(bpy.types.PropertyGroup):
	"""Missing DocString
	"""
	# General:
	jobname = StringProperty(name='Job Name',
							 description='Job Name. Scene name if empty.',
							 maxlen=512, default='')
	#engine = StringProperty(name='Use Engine',
							 #description='Engine to render scene with.',
							 #maxlen=512, default='')
	fstart = IntProperty(name='Start', description='Start Frame', default=1)
	fend = IntProperty(name='End', description='End Frame', default=11)
	finc = IntProperty(name='By', description='Frames Increment', min=1,
					   default=1)
	fpertask = IntProperty(name='Per Task', description='Frames Per One Task',
						   min=1, default=1)
	pause = BoolProperty(name='Start Job Paused',
						 description='Send job in offline state.', default=0)

	packLinkedObjects = BoolProperty(name='Pack Linked Objects',
						 description='Make Loacal All linked Groups and Objects', default=0)
	relativePaths = BoolProperty(name='Relative Paths',
						 description='Set Relative Paths for all Textures and Objects', default=0)
	packTextures = BoolProperty(name='Pack Textures',
						 description='Pack all Textures into the Blend File.', default=0)

	# Render Settings:
	filepath = StringProperty(name='File Path', description='Set File Path.',
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


def register():
	bpy.CGRUTools = dict()

	bpy.utils.register_module(__name__)
	bpy.types.Scene.ore_render = PointerProperty(
		type=ORESettings,
		name='ORE Render',
		description='ORE Render Settings'
	)


def unregister():
	import bpy

	del bpy.types.Scene.ore_render
	del bpy.CGRUTools
	bpy.utils.unregister_module(__name__)


if __name__ == "__main__":
	register()
