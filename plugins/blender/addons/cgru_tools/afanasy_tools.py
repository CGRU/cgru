# -*- coding: utf-8 -*-
import imp
import time
import os
import sys
import re

import bpy


# bpy.errors = []

LAYER_TEXT_BLOCK = '''#
# layer '{0}'
#
import bpy
bpy.context.scene.render.use_sequencer = False
bpy.context.scene.render.use_compositing = False
layers = bpy.context.scene.render.layers
for layer in layers:
	layer.use = False
layers['{0}'].use = True
bpy.context.scene.render.filepath = bpy.context.scene.render.filepath \
	+ '_' + "{0}" + '_'
'''

CMD_TEMPLATE = "blender -b {blend_scene} -y -E {render_engine} " \
		"{python_options}" "{output_options} -s @#@ " \
		"-e @#@ -j {frame_inc} -a"


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

		layout.label(text="Engine: %s" % sce.render.engine)
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
		row.prop(sce, 'frame_start')
		row.prop(sce, 'frame_end')
		row.prop(sce, 'frame_step')
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
		rd = context.scene.render
		images = None
		engine_string = sce.render.engine
		sceneModified = False  # if the opriginal scene modified checker

		# Check and add CGRU module in system path:
		if 'CGRU_LOCATION' not in os.environ:
			os.environ['CGRU_LOCATION'] = addon_prefs.cgru_location

		cgrupython = os.getenv('CGRU_PYTHON')
		if cgrupython is None or cgrupython == '':
			if addon_prefs.cgru_location is None or addon_prefs.cgru_location == '':
				if sys.platform.find('win'):
					cgrupython = r'C:\cgru\lib\python'
				else:
					cgrupython = r'/opt/cgru/lib/python'
			else:
				cgrupython = os.path.join(addon_prefs.cgru_location, 'lib', 'python')
		if cgrupython not in sys.path:
			sys.path.append(cgrupython)

		# Check and add Afanasy module in system path:
		afpython = os.getenv('AF_PYTHON')
		if afpython is None or afpython == '':
			if addon_prefs.cgru_location is None or addon_prefs.cgru_location == '':
				if sys.platform.find('win'):
					afpython = r'C:\cgru\afanasy\python'
				else:
					afpython = r'/opt/cgru/afanasy/python'
			else:
				afpython = os.path.join(addon_prefs.cgru_location, 'afanasy', 'python')
		if afpython not in sys.path:
			sys.path.append(afpython)

		# Import Afanasy module:
		try:
			af = __import__('af', globals(), locals(), [])
		except ImportError as err:
			print('Unable to import Afanasy Python module: ' % err)
			self.report(
				{'ERROR'},
				'An error occurred while sending submission to Afanasy'
			)
			return {'CANCELLED'}

		imp.reload(af)

		# Calculate temporary scene path:
		scenefile = bpy.data.filepath
		if scenefile.endswith('.blend'):
			scenefile = scenefile[:-6]
		renderscenefile = "%s.%s.blend" % (scenefile, time.strftime('%Y%m%d%H%M%S'))

		# Make all Local and pack all textures and objects
		if ore.packLinkedObjects:
			bpy.ops.object.make_local(type='ALL')
			sceneModified = True
		if ore.relativePaths:
			bpy.ops.file.make_paths_relative()
			sceneModified = True
		if ore.packTextures:
			bpy.ops.file.pack_all()
			sceneModified = True

		# Get job name:
		jobname = ore.jobname
		# If job name is empty use scene file name:
		if jobname is None or jobname == '':
			jobname = os.path.basename(scenefile)
			# Try to cut standart '.blend' extension:
			if jobname.endswith('.blend'):
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

		# Create a job:
		job = af.Job(jobname)

		servicename = 'blender'
		renderlayer_names = []
		layers = bpy.context.scene.render.layers

		if ore.splitRenderLayers and len(layers) > 1:
			for layer in layers:
				if layer.use:
					renderlayer_names.append(layer.name)
		else:
			renderlayer_names.append('')

		for renderlayer_name in renderlayer_names:
			block = None
			images = None

			# Create block
			if ore.splitRenderLayers and len(layers) > 1:
				txt_block = bpy.data.texts.new("layer_%s" % renderlayer_name)
				txt_block.write(LAYER_TEXT_BLOCK.format(renderlayer_name))
				block = af.Block("layer_%s" % renderlayer_name, servicename)
			else:
				block = af.Block(engine_string, servicename)

			# Check current render engine
			if engine_string == 'BLENDER_RENDER':
				block.setParser('blender_render')
			elif engine_string == 'CYCLES':
				block.setParser('blender_cycles')

			if ore.filepath != '':
				pos = ore.filepath.find('#')
				if pos != -1:
					if ore.filepath[pos-1] in '._- ':
						images = "{0}{1}{2}".format(ore.filepath[:pos-1],
							renderlayer_name, ore.filepath[pos-1:])
					else:
						images = "{0}{1}{2}".format(ore.filepath[:pos],
							renderlayer_name, ore.filepath[pos:])
				else:
					images = "{0}{1}".format(ore.filepath, renderlayer_name)

				output_images = re.sub(r'(#+)', r'@\1@', images)
				if output_images.startswith('//'):
					output_images = os.path.join(os.path.dirname(renderscenefile),
							output_images.replace('//', ''))

				if rd.file_extension not in output_images:
					block.setFiles([output_images + rd.file_extension])
				else:
					block.setFiles([output_images])

			cmd = CMD_TEMPLATE.format(
					blend_scene=renderscenefile,
					render_engine=engine_string,
					python_options=' --python-text "layer_%s"' % renderlayer_name
							if ore.splitRenderLayers and len(layers) > 1 else '',
					output_options=' -o "%s" ' % images if images else '',
					frame_inc=finc)

			block.setCommand(cmd)
			block.setNumeric(fstart, fend, fpertask, finc)
			job.blocks.append(block)

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

		# Save Temporary file
		bpy.ops.wm.save_as_mainfile(filepath=renderscenefile, copy=True)

		# Clean up temp text blocks
		if ore.splitRenderLayers and len(layers) > 1:
			for text in bpy.data.texts:
				if "layer_" in text:
					bpy.data.texts.remove(text)

		#  Send job to server:
		job.send()

		# if opriginal scene is modified - we need to reload the scene file
		if sceneModified:
			bpy.ops.wm.open_mainfile(filepath=scenefile + ".blend")

		return {'FINISHED'}


def register():
	bpy.utils.register_module(__name__)


def unregister():
	bpy.utils.unregister_module(__name__)


if __name__ == "__main__":
	register()
