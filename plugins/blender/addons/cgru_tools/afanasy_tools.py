# -*- coding: utf-8 -*-
import imp
import shutil
import time
import os
import sys

import bpy

from bpy.props import (PointerProperty, StringProperty, BoolProperty,
					   EnumProperty, IntProperty, CollectionProperty)

bpy.errors = []

layer_text_block = '''# 
# layer '{0}'
#
import bpy
bpy.context.scene.render.use_sequencer = False
bpy.context.scene.render.use_compositing = False
layers = bpy.context.scene.render.layers
for layer in layers:
    layer.use = False
layers['{0}'].use = True
bpy.context.scene.render.filepath = bpy.context.scene.render.filepath + '_' + "{0}" + '_'
'''

## This method creates a string with a list of engines in all scenes
def getSceneEngine():
	#str = ''
	#strEngines = []
	#for scene in bpy.data.scenes:
		#if scene.render.engine not in strEngines:
			#if str != '':
				#str += ','
			#str += scene.render.engine
			#strEngines.append(scene.render.engine)

	#strEngines = None

	return bpy.context.scene.render.engine


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

		layout.label(text="Engine: " + getSceneEngine())
		row = layout.row()
		row.scale_y = 1.5
		row.operator('ore.submit', icon='RENDER_STILL')
		
		layout.separator()		
		col = layout.column()
		col.prop(ore, 'jobname')
		col.prop(ore, 'filepath')

		layout.separator()
		split = layout.split()
		col = split.column()
		col.prop(ore, 'pause')
		col.prop(ore, 'splitRenderLayers')
		col = split.column()
		col.prop(ore, 'packLinkedObjects')
		col.prop(ore, 'relativePaths')
		col.prop(ore, 'packTextures')

		layout.separator()
		col = layout.column(align=True)
		row = col.row(align=True)
		row.scale_y = 1.4
		row.prop(sce, "frame_start")
		row.prop(sce, "frame_end")
		row.prop(ore, 'fpertask')
		row = col.row(align=True)
		row.scale_y = 1.2
		row.prop(ore, 'priority')
		row.prop(ore, 'maxruntasks')
		
		layout.separator()
		col = layout.column()
		col.prop(ore, 'dependmask')
		col.prop(ore, 'dependmaskglobal')
		col.prop(ore, 'hostsmask')
		col.prop(ore, 'hostsmaskexclude')
		


class ORE_Submit(bpy.types.Operator):
	"""Submit job to Afanasy Renderfarm."""

	bl_idname = "ore.submit"
	bl_label = "Submit Job"

	def execute(self, context):
		sce = context.scene
		ore = sce.ore_render
		addon_prefs = context.user_preferences.addons['cgru_tools'].preferences
		#rd = context.scene.render
		images = None
		engineString = getSceneEngine()

		# Calculate temporary scene path:
		scenefile = bpy.data.filepath
		if ".blend" in scenefile:
			scenefile = scenefile[:-6]
		renderscenefile = scenefile + time.strftime('.%m%d-%H%M%S-') + str(
			time.time() - int(time.time()))[2:5] + '.blend'

		# Make all Local and pack all textures and objects
		if ore.packLinkedObjects:
			bpy.ops.object.make_local(type='ALL')
		if ore.relativePaths:
			bpy.ops.file.make_paths_relative()
		if ore.packTextures:
			bpy.ops.file.pack_all()

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
		fstart = sce.frame_start
		fend = sce.frame_end
		finc = sce.frame_step
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
		if not 'CGRU_LOCATION' in os.environ:
			os.environ['CGRU_LOCATION'] = addon_prefs.cgru_location
		
		cgrupython = os.getenv('CGRU_PYTHON')
		if cgrupython is None or cgrupython == '':
			if addon_prefs.cgru_location is None or addon_prefs.cgru_location == '':
				if sys.platform.find('win'):
					cgrupython = 'C:\cgru\lib\python'
				else:
					cgrupython = '/opt/cgru/lib/python'
			else:
				cgrupython = os.path.join(addon_prefs.cgru_location, 'lib', 'python')
		if cgrupython not in sys.path:
			sys.path.append(cgrupython)

		# Check and add Afanasy module in system path:
		afpython = os.getenv('AF_PYTHON')
		if afpython is None or afpython == '':
			if addon_prefs.cgru_location is None or addon_prefs.cgru_location == '':
				if sys.platform.find('win'):
					afpython = 'C:\cgru\afanasy\python'
				else:
					afpython = '/opt/cgru/afanasy/python'
			else:
				afpython = os.path.join(addon_prefs.cgru_location, 'afanasy', 'python')
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

		renderNames = []

		layers = bpy.context.scene.render.layers

		if ore.splitRenderLayers and len(layers) > 1:
			for layer in layers:
				renderNames.append(layer.name)
		else:
			renderNames.append(engineString)

		for renderName in renderNames:
			block = None

			# Create block
			if ore.splitRenderLayers and len(layers) > 1:
				txt_block = bpy.data.texts.new("layer_" + renderName)
				txt_block.write(layer_text_block.format(renderName))
				block = af.Block("layer_" + renderName, servicename)
			else:
				block = af.Block(engineString, servicename)

			# Check current render engine
			if engineString == 'BLENDER_RENDER':
				block.setParser('blender_render')
			elif engineString == 'CYCLES':
				block.setParser('blender_cycles')

			job.blocks.append(block)

			# Set block command and frame range:
			cmd = 'blender -b "%s"' % renderscenefile

			# Only for renderLayers
			if ore.splitRenderLayers and len(layers) > 1:
				cmd += ' --python-text "%s"' % ("layer_" + renderName)

			cmd += ' -y -E "%s"' % engineString
			if images is not None:
				cmd += ' -o "%s"' % (images + renderName + '_')
			cmd += ' -s @#@ -e @#@ -j %d -a' % finc
			block.setCommand(cmd)
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

		# Save Temporary file
		bpy.ops.wm.save_as_mainfile(filepath=renderscenefile, copy=True)

		# Clean up temp text blocks
		if ore.splitRenderLayers and len(layers) > 1:
			for text in bpy.data.texts:
				if "layer_" in text:
					bpy.data.texts.remove(text)

		#  Send job to server:
		job.send()

		# open the file again
		#bpy.ops.wm.open_mainfile(filepath=scenefile)

		return set(['FINISHED'])


def register():
	bpy.utils.register_module(__name__)


def unregister():
	bpy.utils.unregister_module(__name__)


if __name__ == "__main__":
	register()
