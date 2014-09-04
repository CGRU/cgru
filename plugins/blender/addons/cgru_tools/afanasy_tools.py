# -*- coding: utf-8 -*-
import imp
import shutil
import time
import os
import sys

import bpy

from bpy.props import (PointerProperty, StringProperty, BoolProperty,
					   EnumProperty, IntProperty, CollectionProperty)
from bpy.types import Operator, AddonPreferences

bpy.errors = []

class OREAddonPreferences(AddonPreferences):
	# this must match the addon name, use '__name__'
	# when defining this in a submodule of a python package.
	bl_idname = __name__

	def draw(self, context):
		layout = self.layout
		row = layout.row()
		row.label(text="Please, set Exchanges Folder and save Preferences")
		row = layout.row()


# This method creates a string with a list of engines in all scenes
def getSceneEngines():
	str = ''
	strEngines = []
	for scene in bpy.data.scenes:
		if scene.render.engine not in strEngines:
			if str != '':
				str += ','
			str += scene.render.engine
			strEngines.append(scene.render.engine)

	strEngines = None
	return str


class RENDER_PT_Afanasy(bpy.types.Panel):
	bl_label = "Afanasy"
	bl_category = 'CGRU'
	bl_space_type = 'PROPERTIES'
	bl_region_type = 'WINDOW'
	bl_context = "render"
	bl_options = {'DEFAULT_CLOSED'}

	def draw(self, context):
		layout = self.layout
		sce = context.scene
		ore = sce.ore_render

		#row = layout.row()

		layout.operator('ore.submit')
		layout.separator()

		layout.label(text="Engines: " + getSceneEngines())
		layout.prop(ore, 'jobname')
		layout.prop(ore, 'filepath')

		layout.separator()
		layout.prop(ore, 'pause')

		layout.separator()
		layout.prop(ore, 'packLinkedObjects')
		layout.prop(ore, 'relativePaths')
		layout.prop(ore, 'packTextures')

		layout.separator()
		layout.prop(ore, 'fpertask')
		layout.prop(ore, 'priority')
		layout.prop(ore, 'maxruntasks')
		layout.prop(ore, 'dependmask')
		layout.prop(ore, 'dependmaskglobal')
		layout.prop(ore, 'hostsmask')
		layout.prop(ore, 'hostsmaskexclude')


class ORE_Submit(bpy.types.Operator):
	"""Missing DocString
	"""

	bl_idname = "ore.submit"
	bl_label = "Submit Job"

	def execute(self, context):
		sce = context.scene
		ore = sce.ore_render
		#rd = context.scene.render
		images = None
		enginesString = getSceneEngines()

		# Calculate temporary scene path:
		scenefile = bpy.data.filepath
		renderscenefile = scenefile + time.strftime('.%m%d-%H%M%S-') + str(
			time.time() - int(time.time()))[2:5] + '.blend'

		# Make all Local and pack all textures and objects
		if ore.packLinkedObjects:
			bpy.ops.object.make_local(type='ALL')
		if ore.relativePaths:
			bpy.ops.file.make_paths_relative()
		if ore.packTextures:
			bpy.ops.file.pack_all()


		# Save Temporary file
		bpy.ops.wm.save_mainfile(filepath=renderscenefile)

		# Get job name:
		jobname = ore.jobname
		# If job name is empty use scene file name:
		if jobname is None or jobname == '':
			jobname = os.path.basename(renderscenefile)
			# Try to cut standart '.blend' extension:
			if len(jobname) > 6:
				if jobname[-6:] == '.blend':
					jobname = jobname[:-6]

		# Get frames settings:
		fstart = bpy.context.scene.frame_start
		fend = bpy.context.scene.frame_end
		finc = bpy.context.scene.frame_step
		fpertask = ore.fpertask
		# Check frames settings:
		if fpertask < 1:
			fpertask = 1
		if fend < fstart:
			fend = fstart
		# Process images:
		if ore.filepath != '':
			images = ore.filepath

		# Check and add CGRU module in system path:
		cgrupython = os.getenv('CGRU_PYTHON')
		if cgrupython is None or cgrupython == '':
			cgrupython = '/opt/cgru/lib/python'
		if cgrupython not in sys.path:
			sys.path.append(cgrupython)

		# Check and add Afanasy module in system path:
		afpython = os.getenv('AF_PYTHON')
		if afpython is None or afpython == '':
			afpython = '/opt/cgru/afanasy/python'
		if afpython not in sys.path:
			sys.path.append(afpython)

		# Import Afanasy module:
		try:
			af = __import__('af', globals(), locals(), [])
		except:  # TODO: Too broad exception clause
			error = str(sys.exc_info()[1])
			print('Unable to import Afanasy Python module:\n' + error)

			self.report(
				set(['ERROR']),
				'An error occurred while sending submission to Afanasy'
			)
			return set(['CANCELLED'])

		imp.reload(af)  # TODO: imp.reload() does not exist in Python 3.x

		# Create a job:
		job = af.Job(jobname)
		servicename = 'blender'
		block = af.Block(enginesString, servicename)

		#if enginesString == 'BLENDER_RENDER':
			#block.setParser('blender_render')
		#if enginesString == 'CYCLES':
			#block.setParser('blender_cycles')

		# set blender_cycles parser for all engines
		block.setParser('blender_cycles')

		job.blocks.append(block)
		# Set block command and frame range:
		cmd = 'blender -b "%s"' % renderscenefile
		cmd += ' -E "%s"' % enginesString
		if images is not None:
			cmd += ' -o "%s"' % images
		cmd += ' -s @#@ -e @#@ -j %d -a' % finc
		block.setCommand(cmd)
		#print(cmd)
		block.setNumeric(fstart, fend, fpertask, finc)
		if images is not None:
			pos = images.find('#')
			if pos > 0:
				images = images[:pos] + '@' + images[pos:]
			pos = images.rfind('#')
			if pos > 0:
				images = images[:pos + 1] + '@' + images[pos + 1:]
			block.setFiles([images])
		# Set job running parameters:
		if ore.maxruntasks > -1:
			job.setMaxRunningTasks(ore.maxruntasks)
		if ore.priority > -1:
			job.setPriority(ore.priority)
		if ore.dependmask != '':
			job.setDependMask(ore.dependmask)
		if ore.dependmaskglobal != '':
			job.setDependMaskGlobal(ore.dependmaskglobal)
		if ore.hostsmask != '':
			job.setHostsMask(ore.hostsmask)
		if ore.hostsmaskexclude != '':
			job.setHostsMaskExclude(ore.hostsmaskexclude)
		if ore.pause:
			job.offLine()
		# Make server to delete temporary file after job deletion:
		job.setCmdPost('deletefiles "%s"' % os.path.abspath(renderscenefile))

		# Print job information:
		job.output(True)

		## Copy scene to render
		#shutil.copy(scenefile, renderscenefile)

		#  Send job to server:
		job.send()

		# open the file again
		bpy.ops.wm.open_mainfile(filepath=scenefile)

		return set(['FINISHED'])


def register():
	bpy.utils.register_module(__name__)


def unregister():
	bpy.utils.unregister_module(__name__)


if __name__ == "__main__":
	register()