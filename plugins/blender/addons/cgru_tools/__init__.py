# -*- coding: utf-8 -*-
bl_info = {
	"name": "CGRU Tools",
	"author": "CGRU Team",
	"version": (1,),
	"blender": (2, 7, 1),
	"location": "Properties > Render > Afanasy",
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


class OREAddonPreferences(bpy.types.AddonPreferences):
	# this must match the addon name, use '__name__'
	# when defining this in a submodule of a python package.
	bl_idname = __name__

	cgru_location = bpy.props.StringProperty(
		name="CGRU Root location",
		subtype="DIR_PATH")
    
	def draw(self, context):
		layout = self.layout
		row = layout.row()
		row.label(text="Please, set Exchanges Folder and save Preferences")
		row = layout.row()
		row.prop(self, "cgru_location")


class ORESettings(bpy.types.PropertyGroup):
	"""Missing DocString
	"""
	# General:
	jobname = StringProperty(name='Job Name',
		description='Job Name. Scene name if empty.', maxlen=512, default='')
	fpertask = IntProperty(name='Per Task', description='Frames Per One Task',
		min=1, default=1)
	pause = BoolProperty(name='Start Job Paused',
		description='Send job in offline state.', default=0)
	packLinkedObjects = BoolProperty(name='Pack Linked Objects',
		description='Make Local All linked Groups and Objects', default=0)
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
	bpy.utils.register_module(__name__)
	bpy.types.Scene.ore_render = PointerProperty(
		type=ORESettings,
		name='ORE Render',
		description='ORE Render Settings'
	)


def unregister():
	del bpy.types.Scene.ore_render
	bpy.utils.unregister_module(__name__)


if __name__ == "__main__":
	register()

