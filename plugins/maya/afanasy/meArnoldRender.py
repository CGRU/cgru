# -*- coding: utf-8 -*-
"""
	meArnoldRender

	Usage:
	You can add this code as shelf button :

from afanasy import meArnoldRender
reload( meArnoldRender ) # keep this just for debug purposes
meArnoldRender = meArnoldRender.meArnoldRender()

	Important!!!
	Do not use another object name.
	Only meArnoldRender.renderLayerSelected() will work with script job.

	Add %CGRU_LOCATION%\plugins\maya to %PYTHONPATH%

"""

import sys
import os
import string
import maya.OpenMaya as OpenMaya
import maya.cmds as cmds
import maya.mel as mel

from functools import partial

from maya_ui_proc import *
from afanasyRenderJob import *

self_prefix = 'meArnoldRender_'
meArnoldRenderVer = '0.3.4'
meArnoldRenderMainWnd = self_prefix + 'MainWnd'

job_separator_list = ['none', '.', '_' ]
ar_verbosity_list = [
	'none',
	'fatal',
	'error',
	'warning',
	'info',
	'progress',
	'debug',
	'details'
]

class meArnoldRender ( object ) :
	"""
	meArnoldRender
	
	"""
	def __init__ (self, NoGUI = False) :
		"""
		In case if NoGUI = True, it's possible to use external "wrapper"
		e.g. for Shotgun to call "generate_ass" or "submitJob" previously
		assign global variables  
		"""
		self.NoGUI = NoGUI
		self.winMain = ''

		self.os = sys.platform
		if self.os.startswith('linux'):
			self.os = 'linux'
		elif self.os == 'darwin':
			self.os = 'mac'
		elif self.os == 'win32':
			self.os = 'win'

		print ('sys.platform = %s self.os = %s' % (sys.platform, self.os))

		if cmds.getAttr('defaultRenderGlobals.currentRenderer') != 'arnold':
			cmds.warning('Arnold is not current renderer!')
			return

		self.rootDir = cmds.workspace(q=True, rootDirectory=True)
		self.rootDir = self.rootDir[:-1]
		self.layer = cmds.editRenderLayerGlobals(
			query=True,
			currentRenderLayer=True
		)

		self.job = None

		self.job_param = {}
		self.ass_param = {}
		self.ar_param = {}
		self.img_param = {}
		self.afanasy_param = {}
		self.bkburn_param = {}

		self.assgenCommand = 'arnoldExportAss'
		self.def_assgenCommand = 'Render -r arnold'
		# maya scene name used for deferred .ass generation
		self.def_scene_name = ''  
		
		# 'deletefiles' script is located in $CGRU_LOCATION\bin
		self.cleanup_cmd = 'deletefiles' 
		
		self.save_frame_bgc = [0, 0, 0]
		self.def_frame_bgc = [0.75, 0.5, 0]

		self.initParameters()
		self.ui = self.setupUI()

		# def __del__( self ):

	# print( ">> meArnoldRender: Class deleted" )

	def initParameters(self):
		"""
		Init all parameters
		"""
		#
		# Job parameters
		#
		self.job_param['job_dispatcher'] = \
			getDefaultStrValue(self_prefix, 'job_dispatcher', 'afanasy')

		self.job_param['job_name'] = getMayaSceneName()
		self.job_param['job_description'] = \
			getDefaultStrValue(self_prefix, 'job_description', '')

		self.job_param['job_animation'] = \
			getDefaultIntValue(self_prefix, 'job_animation', 1) is 1

		self.job_param['job_start'] = \
			getDefaultIntValue(self_prefix, 'job_start', 1)

		self.job_param['job_end'] = \
			getDefaultIntValue(self_prefix, 'job_end', 100)

		self.job_param['job_step'] = \
			getDefaultIntValue(self_prefix, 'job_step', 1)

		self.job_param['job_size'] = \
			getDefaultIntValue(self_prefix, 'job_size', 1)

		self.job_param['job_paused'] = \
			getDefaultIntValue(self_prefix, 'job_paused', 1) is 1

		self.job_param['job_priority'] = \
			getDefaultIntValue(self_prefix, 'job_priority', -1)

		self.job_param['job_cleanup_ass'] = \
			getDefaultIntValue(self_prefix, 'job_cleanup_ass', 0) is 1

		self.job_param['job_cleanup_maya'] = \
			getDefaultIntValue(self_prefix, 'job_cleanup_maya', 0) is 1
			
		self.job_param['job_padding'] = \
			getDefaultIntValue(self_prefix, 'job_padding', 4)
		
		# job_separator = { 'none', '.', '_' }
		self.job_param['job_separator'] = \
			getDefaultStrValue(self_prefix, 'job_separator', '.') 

		#
		# .ass generation parameters
		#
		self.ass_param['ass_reuse'] = \
			getDefaultIntValue(self_prefix, 'ass_reuse', 0) is 1

		ass_dirname = cmds.workspace( fileRuleEntry='ASS' )
		if ass_dirname == '' :
			ass_dirname = 'ass'
			cmds.workspace( fileRule=('ASS',ass_dirname) )
			cmds.workspace( saveWorkspace=True )
				
		self.ass_param['ass_dirname'] = \
			getDefaultStrValue(self_prefix, 'ass_dirname', ass_dirname ) 

		#self.ass_param['ass_perframe'] = \
		#	getDefaultIntValue(self_prefix, 'ass_perframe', 1) is 1

		self.ass_param['ass_selection'] = \
			getDefaultIntValue(self_prefix, 'ass_selection', 0) is 1

		self.ass_param['ass_binary'] = \
			getDefaultIntValue(self_prefix, 'ass_binary', 
				cmds.getAttr( 'defaultArnoldRenderOptions.binaryAss' ) ) is 1

		self.ass_param['ass_verbosity'] = \
			getDefaultStrValue(self_prefix, 'ass_verbosity', 'none')

		self.ass_param['ass_deferred'] = \
			getDefaultIntValue(self_prefix, 'ass_deferred', 0) is 1

		self.ass_param['ass_local_assgen'] = \
			getDefaultIntValue(self_prefix, 'ass_local_assgen', 1) is 1

		self.ass_param['ass_def_task_size'] = \
			getDefaultIntValue(self_prefix, 'ass_def_task_size', 4)

		self.ass_param['ass_export_all_layers'] = \
			getDefaultIntValue(self_prefix, 'ass_export_all_layers', 0) is 1

		self.ass_param['ass_compressed'] = \
			getDefaultStrValue(self_prefix, 'ass_compressed',
			cmds.getAttr('defaultArnoldRenderOptions.output_ass_compressed' )) is 1
			
		# defaultArnoldRenderOptions.expandProcedurals
		self.ass_param['ass_expand_procedurals'] = \
			getDefaultIntValue(self_prefix, 'ass_expand_procedurals', 
			cmds.getAttr('defaultArnoldRenderOptions.expandProcedurals' )) is 1
		
		# defaultArnoldRenderOptions.outputAssBoundingBox
		self.ass_param['ass_export_bounds'] = \
			getDefaultIntValue(self_prefix, 'ass_export_bounds', 
			cmds.getAttr('defaultArnoldRenderOptions.outputAssBoundingBox')) is 1  
			
		
		#
		# Arnold parameters
		#
		self.ar_param['ar_options'] = \
			getDefaultStrValue(self_prefix, 'ar_options', '')

		self.ar_param['ar_verbosity'] = \
			getDefaultStrValue(self_prefix, 'ar_verbosity', 'none')

		self.ar_param['ar_threads'] = \
			getDefaultIntValue(self_prefix, 'ar_threads', 0)
			
		# defaultArnoldRenderOptions.absoluteTexturePaths
		self.ar_param['ar_abs_tex_path'] = \
			getDefaultIntValue(self_prefix, 'ar_abs_tex_path', 
			cmds.getAttr('defaultArnoldRenderOptions.absoluteTexturePaths')) is 1
			
		# defaultArnoldRenderOptions.absoluteProceduralPaths
		self.ar_param['ar_abs_proc_path'] = \
			getDefaultIntValue(self_prefix, 'ar_abs_proc_path', 
			cmds.getAttr('defaultArnoldRenderOptions.absoluteProceduralPaths')) is 1
			
		# defaultArnoldRenderOptions.plugins_path
		self.ar_param['ar_plugin_path'] = \
			getDefaultStrValue(self_prefix, 'ar_plugin_path',
			cmds.getAttr('defaultArnoldRenderOptions.plugins_path') )
		
		# defaultArnoldRenderOptions.procedural_searchpath	
		self.ar_param['ar_proc_search_path'] = \
			getDefaultStrValue(self_prefix, 'ar_proc_search_path',
			cmds.getAttr('defaultArnoldRenderOptions.procedural_searchpath'))
		
		# defaultArnoldRenderOptions.shader_searchpath	
		self.ar_param['ar_shader_search_path'] = \
			getDefaultStrValue(self_prefix, 'ar_shader_search_path',
			cmds.getAttr('defaultArnoldRenderOptions.shader_searchpath'))
		
		# defaultArnoldRenderOptions.texture_searchpath
		self.ar_param['ar_tex_search_path'] = \
			getDefaultStrValue(self_prefix, 'ar_tex_search_path',
			cmds.getAttr('defaultArnoldRenderOptions.texture_searchpath'))
		#
		# Image parameters
		#
		self.img_param['img_filename'] = self.getImageFileNamePrefix()
		self.img_param['img_format'] = self.getImageFormat()
		#
		# Afanasy parameters
		#
		self.afanasy_param['af_capacity'] = \
			getDefaultIntValue(self_prefix, 'af_capacity', 1000)

		self.afanasy_param['af_deferred_capacity'] = \
			getDefaultIntValue(self_prefix, 'af_deferred_capacity', 1000)

		self.afanasy_param['af_use_var_capacity'] = \
			getDefaultIntValue(self_prefix, 'af_use_var_capacity', 0) is 1

		self.afanasy_param['af_cap_min'] = \
			getDefaultFloatValue(self_prefix, 'af_cap_min', 1.0)

		self.afanasy_param['af_cap_max'] = \
			getDefaultFloatValue(self_prefix, 'af_cap_max', 1.0)

		self.afanasy_param['af_max_running_tasks'] = \
			getDefaultIntValue(self_prefix, 'af_max_running_tasks', -1)

		self.afanasy_param['af_max_tasks_per_host'] = \
			getDefaultIntValue(self_prefix, 'af_max_tasks_per_host', -1)

		self.afanasy_param['af_service'] = \
			getDefaultStrValue(self_prefix, 'af_service', 'arnold')

		self.afanasy_param['af_deferred_service'] = \
			getDefaultStrValue(
				self_prefix,
				'af_deferred_service',
				'mayatoarnold'
			)

		self.afanasy_param['af_os'] = \
			getDefaultStrValue(self_prefix, 'af_os', '')  #linux mac windows
	
		self.afanasy_param['af_hostsmask'] = \
			getDefaultStrValue(self_prefix, 'af_hostsmask', '')

		self.afanasy_param['af_hostsexcl'] = \
			getDefaultStrValue(self_prefix, 'af_hostsexcl', '')

		self.afanasy_param['af_depmask'] = \
			getDefaultStrValue(self_prefix, 'af_depmask', '')

		self.afanasy_param['af_depglbl'] = \
			getDefaultStrValue(self_prefix, 'af_depglbl', '')

	def getImageFileNamePrefix ( self ) :
		""" Get image file prefix name from RenderGlobals or use just maya scene name
				if name is empty string 
		"""
		fileNamePrefix = cmds.getAttr('defaultRenderGlobals.imageFilePrefix')
		if fileNamePrefix == None or fileNamePrefix == '':
			fileNamePrefix = getMayaSceneName()
		return fileNamePrefix

	def getImageFormat ( self ) :
		"""Get image format extensions
		
		return: string of image format extension
		"""
		imageFormatStr = mel.eval ( 'getImfImageType' )
		if imageFormatStr == 'deepexr' : 
			imageFormatStr = 'exr'
		return imageFormatStr

	def getDeferredCmd ( self, layer=None ) :
		"""Get command for deferred .ass generation
		
		:param layer: current render layer
		:return: string for deferred render command
		"""
		gen_cmd = self.def_assgenCommand
		gen_cmd += ' -proj "' + self.rootDir + '"'
		gen_cmd += ' -rt 1' # Render type : 
		# 0 = render,
		# 1 = export ass,
		# 2 = export and kick
		if layer is not None:
			gen_cmd += ' -rl ' + layer
		else:
			gen_cmd += ' -rl 1'
		gen_cmd += self.get_assgen_options(layer)
		return gen_cmd

	def getRenderCmd ( self, layer=None ) :
		"""Get command for render

		:param layer: current render layer
		:return: string for render command
		"""
		ar_verbosity_level = \
			ar_verbosity_list.index(self.ar_param['ar_verbosity'])
		options = str(self.ar_param['ar_options']).strip()
		ar_threads = self.ar_param['ar_threads']

		if ar_verbosity_level < 2:
			ar_verbosity_level = 2

		cmd_buffer = ['kick']
		if ar_verbosity_level != 0:
			cmd_buffer.append('-v %s' % ar_verbosity_level)

		if ar_threads != 0:
			cmd_buffer.append('-t %s' % ar_threads)

		# cmd_buffer.append('-nstdin -dw -dp -nocrashpopup -i')
		# !!! -nocrashpopup flag doesn't support in kick linux version

		cmd_buffer.append('-nstdin -dw -dp -i')

		if len(options):
			cmd_buffer.append(options)

		return ' '.join(cmd_buffer)

	def get_ass_name ( self, suffix=True, layer=None, decorator = '@' ) :
		"""get_ass_name

		:param suffix: if it is True -- return pattern string name name.@###@.ass
									 otherwise -- return just .ass filename
		:param layer: current render layer
		:param decorator: symbol for padding string decoration
		:return: .ass filename 
		"""
		ass_deferred = self.ass_param['ass_deferred']
		separator = self.job_param['job_separator']
		if separator == 'none' :
			separator = ''
			
		filename = self.ass_param['ass_dirname']
		if layer is not None:
			if layer == 'defaultRenderLayer':
				layer = 'masterLayer'

			if not (len(getRenderLayersList(False)) == 1):
				filename += '/' + layer

		scenename = getMayaSceneName()

		if ass_deferred:
			scenename += '_deferred'
			#scenename, ext = os.path.splitext(
			#	os.path.basename(self.def_scene_name)
			#)

		filename += '/%s' % scenename
		filename = cmds.workspace(expandName=filename)

		if suffix:
			pad_str = getPadStr( self.job_param['job_padding'], True )

			#if self.ass_param['ass_perframe'] :
			filename += '%s%s%s%s.ass' % ( separator, decorator, pad_str, decorator )
			#else :	
			#	filename += '.ass'	
			if self.ass_param['ass_compressed'] :
				# !!! There is error in maya translator 
				# for "defaultArnoldRenderOptions.output_ass_compressed" flag
				if not ass_deferred : 
					filename += '.gz'

		return filename

	def get_image_name ( self ) :
		"""get_image_name
		"""
		pad_str = getPadStr( self.job_param['job_padding'], True )

		images = cmds.renderSettings(
			fullPath=True,
			genericFrameImageName=('@%s@' % pad_str)
		)

		#imageFileName = ';'.join (images)
		imageFileName = str(images[0])
		( fileName, ext ) = os.path.splitext ( imageFileName )
		imageFileName = fileName + '.' + self.getImageFormat ()
		return fromNativePath(imageFileName)

	def get_assgen_options ( self, layer=None ) :
		"""get_assgen_options

		:param layer:
		:return:
		"""
		animation = self.job_param['job_animation']
		start = self.job_param['job_start']
		stop = self.job_param['job_end']
		step = self.job_param['job_step']
		ass_reuse = self.ass_param['ass_reuse']
		ass_selection = self.ass_param['ass_selection']
		ass_dirname = self.ass_param['ass_dirname']
		ass_padding = self.job_param['job_padding']
		#ass_perframe = self.ass_param['ass_perframe']
		ass_deferred = self.ass_param['ass_deferred']  
		
		ass_binary = self.ass_param['ass_binary']
		ass_compressed = self.ass_param['ass_compressed']
		ass_expand_procedurals = self.ass_param['ass_expand_procedurals']
		ass_export_bounds = self.ass_param['ass_export_bounds']
		
		ar_abs_tex_path = self.ar_param['ar_abs_tex_path']
		ar_abs_proc_path = self.ar_param['ar_abs_proc_path']
		
		ar_plugin_path = self.ar_param['ar_plugin_path']
		ar_proc_search_path = self.ar_param['ar_proc_search_path']
		ar_shader_search_path = self.ar_param['ar_shader_search_path']
		ar_tex_search_path = self.ar_param['ar_tex_search_path']
		
		assgen_cmd = ''
		filename = cmds.workspace(expandName=ass_dirname)
		filename, ext = os.path.splitext(filename)

		if ext == '' or ext == '.':
			ext = '.ass'

		if ass_deferred:
			if ass_binary:
				assgen_cmd += ' -ai:bass 1'
			if ass_export_bounds:
				assgen_cmd += ' -ai:exbb 1'
			
			assgen_cmd += ' -ai:lve 1' # ' -ai:lfv 2'
			assgen_cmd += ' -ai:sppg "' + ar_plugin_path + '"'
			assgen_cmd += ' -ai:sppr "' + ar_proc_search_path + '"'
			assgen_cmd += ' -ai:spsh "' + ar_shader_search_path + '"'
			assgen_cmd += ' -ai:sptx "' + ar_tex_search_path + '"'
			
			#assgen_cmd += ' -filename \"' + filename + '\"'
		else:
			if layer is not None:
				layer_in_filename = layer
				if layer == 'defaultRenderLayer':
					layer_in_filename = 'masterLayer'

				filename += '_' + layer_in_filename

				# for deferred generation we do not add layer name to filename
				# this will be done by Maya

			filename = self.get_ass_name(False, layer)

			if ass_compressed :
				assgen_cmd += ' -compressed'

			if not ass_binary :
				assgen_cmd += ' -asciiAss'

			if ass_selection :
				assgen_cmd += ' -selected'

			if ass_expand_procedurals :
				assgen_cmd += ' -expandProcedurals'
				
			if ass_export_bounds:
				assgen_cmd += ' -boundingBox'
				
			assgen_cmd += ' -filename \"' + filename + '\"'

		return assgen_cmd

	def generate_ass ( self, isSubmitingJob=False ) :
		"""generate_ass

		:param isSubmitingJob: if job assumed to be submited after .ass generation
		"""
		skipExport = False
		exportAllRenderLayers = self.ass_param['ass_export_all_layers']

		animation = self.job_param['job_animation']
		start = self.job_param['job_start']
		stop = self.job_param['job_end']
		step = self.job_param['job_step']
		separator = self.job_param['job_separator']
		ass_reuse = self.ass_param['ass_reuse']
		ass_selection = self.ass_param['ass_selection']
		ass_dirname = self.ass_param['ass_dirname']
		ass_padding = self.job_param['job_padding']
		#ass_perframe = self.ass_param['ass_perframe']
		ass_deferred = self.ass_param['ass_deferred']
		
		ass_binary = self.ass_param['ass_binary']
		ass_expand_procedurals = self.ass_param['ass_expand_procedurals']
		ass_export_bounds = self.ass_param['ass_export_bounds']
		
		ar_abs_tex_path = self.ar_param['ar_abs_tex_path']
		ar_abs_proc_path = self.ar_param['ar_abs_proc_path']
		
		ar_plugin_path = self.ar_param['ar_plugin_path']
		ar_proc_search_path = self.ar_param['ar_proc_search_path']
		ar_shader_search_path = self.ar_param['ar_shader_search_path']
		ar_tex_search_path = self.ar_param['ar_tex_search_path']

		filename = cmds.workspace(expandName=ass_dirname)
		filename, ext = os.path.splitext(filename)

		if ext == '' or ext == '.':
			ext = '.ass'
		filename += ext

		dirname = os.path.dirname(filename)
		if not os.path.exists(dirname):
			print('path %s not exists... create it!' % dirname)
			os.mkdir(dirname)

		# TODO!!! check if files are exist and have to be overwritten
		if isSubmitingJob and ass_reuse:
			skipExport = True
			print('Skipping .ass files generation ...')

		if not skipExport:
			if not animation:
				start = stop = int(cmds.currentTime(q=True))
				step = 1
			#
			# save RenderGlobals
			#
			defGlobals = 'defaultRenderGlobals'
			aiGlobals = 'defaultArnoldRenderOptions'
			saveGlobals = {}
			#
			# override RenderGlobals
			#
			cmds.setAttr(defGlobals + '.extensionPadding', ass_padding)
			cmds.setAttr(defGlobals + '.animation', 1) # always use 'name.#.ext' format
			cmds.setAttr(defGlobals + '.outFormatControl', 0)
			cmds.setAttr(defGlobals + '.putFrameBeforeExt', 1 )
			if separator == 'none' :
				cmds.setAttr(defGlobals + '.periodInExt', 0 )		
			elif separator == '.' :
				cmds.setAttr(defGlobals + '.periodInExt', 1 )	
			else :
				cmds.setAttr(defGlobals + '.periodInExt', 2 )	
			
			image_name = self.getImageFileNamePrefix()
			
			cmds.setAttr(aiGlobals + '.binaryAss', ass_binary )
			cmds.setAttr(aiGlobals + '.expandProcedurals', ass_expand_procedurals )
			cmds.setAttr(aiGlobals + '.outputAssBoundingBox', ass_export_bounds )
			cmds.setAttr(aiGlobals + '.absoluteTexturePaths', ar_abs_tex_path )
			cmds.setAttr(aiGlobals + '.absoluteProceduralPaths', ar_abs_proc_path )
			cmds.setAttr(aiGlobals + '.plugins_path', ar_plugin_path, type='string' )
			cmds.setAttr(aiGlobals + '.procedural_searchpath', ar_proc_search_path, type='string' )
			cmds.setAttr(aiGlobals + '.shader_searchpath', ar_shader_search_path, type='string' )
			cmds.setAttr(aiGlobals + '.texture_searchpath', ar_tex_search_path, type='string' )
			#
			# Clear .output_ass_filename to force using default filename from RenderGlobals
			#
			cmds.setAttr(aiGlobals + '.output_ass_filename', '', type='string' )
			
			cmds.workspace( fileRule=('ASS',self.ass_param['ass_dirname']) )
			cmds.workspace( saveWorkspace=True )
		
			if ass_deferred:
				# generate unique maya scene name and save it
				# with current render and .ass generation settings
				print('Use deferred .ass generation')
				saveGlobals['imageFilePrefix'] = \
					str(cmds.getAttr(defGlobals + '.imageFilePrefix'))

				cmds.setAttr(
					defGlobals + '.imageFilePrefix',
					image_name,
					type='string'
				)
				
				# get scene name without extension
				scene_name = getMayaSceneName( withoutSubdir=False)  
				def_scene_name = scene_name + '_deferred'
				cmds.file(rename=def_scene_name)
				# save it with default extension
				self.def_scene_name = cmds.file(save=True, de=True)  
				# rename scene back
				cmds.file(rename=scene_name)  

				cmds.setAttr(
					defGlobals + '.imageFilePrefix',
					saveGlobals['imageFilePrefix'],
					type='string'
				)
			else:
				renderLayers = []
				# save current layer
				current_layer = \
					cmds.editRenderLayerGlobals(
						q=True,
						currentRenderLayer=True
					)
				if exportAllRenderLayers:
					renderLayers = getRenderLayersList(True)  # renderable only
				else:
					# use only current layer
					renderLayers.append(current_layer)

				for layer in renderLayers:
					# if layer == 'masterLayer' :
					#     layer = 'defaultRenderLayer' 
					saveGlobals['renderableLayer'] = \
						cmds.getAttr(layer + '.renderable')

					cmds.setAttr(layer + '.renderable', True)
					# print 'set current layer renderable (%s)' % layer

					cmds.editRenderLayerGlobals(currentRenderLayer=layer)

					assgen_cmd = \
						self.assgenCommand + self.get_assgen_options(layer)

					assgen_cmd += ' -startFrame %d' % start
					assgen_cmd += ' -endFrame %d' % stop
					assgen_cmd += ' -frameStep %d' % step

					#cmds.setAttr( defGlobals + '.imageFilePrefix', image_name, type='string' ) 
					# will use MayaSceneName if empty 

					print(assgen_cmd)
					mel.eval(assgen_cmd)

					cmds.setAttr(
						layer + '.renderable',
						saveGlobals['renderableLayer']
					)

				if exportAllRenderLayers:
					# restore current layer
					cmds.editRenderLayerGlobals(
						currentRenderLayer=current_layer
					)

	def submitJob ( self, param=None ) :
		"""
		submitJob

		:param param: dummy parameter
		"""

		job_dispatcher = self.job_param['job_dispatcher']
		job_description = self.job_param['job_description']
		job_name = str(self.job_param['job_name']).strip()
		if job_name == '':
			job_name = getMayaSceneName()
		job_cleanup_ass = self.job_param['job_cleanup_ass']
		job_cleanup_maya = self.job_param['job_cleanup_maya']
		
		ass_deferred = self.ass_param['ass_deferred']
		ass_local_assgen = self.ass_param['ass_local_assgen']
		ass_def_task_size = self.ass_param['ass_def_task_size']
		ass_reuse = self.ass_param['ass_reuse']
		exportAllRenderLayers = self.ass_param['ass_export_all_layers']

		if job_dispatcher == 'afanasy':
			self.job = AfanasyRenderJob(job_name, job_description)
			self.job.use_var_capacity = self.afanasy_param[
				'af_use_var_capacity']
			self.job.capacity_coeff_min = self.afanasy_param['af_cap_min']
			self.job.capacity_coeff_max = self.afanasy_param['af_cap_max']
			self.job.max_running_tasks = self.afanasy_param[
				'af_max_running_tasks']
			self.job.max_tasks_per_host = self.afanasy_param[
				'af_max_tasks_per_host']
			self.job.hostsmask = str(
				self.afanasy_param['af_hostsmask']).strip()
			self.job.hostsexcl = str(
				self.afanasy_param['af_hostsexcl']).strip()
			self.job.depmask = str(self.afanasy_param['af_depmask']).strip()
			self.job.depglbl = str(self.afanasy_param['af_depglbl']).strip()
			self.job.need_os = str(self.afanasy_param['af_os']).strip()

			service = str(self.afanasy_param['af_service']).strip()
			deferred_service = \
				str(self.afanasy_param['af_deferred_service']).strip()

			capacity = self.afanasy_param['af_capacity']
			deferred_capacity = self.afanasy_param['af_deferred_capacity']

		elif job_dispatcher == 'backburner':
			print('backburner not supported in this version')
			#self.job = ArnoldBackburnerJob ( job_name, job_description )
			return
		else:
			ass_deferred = False
			self.job = RenderJob(job_name, job_description)

		self.job.work_dir  = self.rootDir
		self.job.padding = self.job_param['job_padding']
		self.job.priority = self.job_param['job_priority']
		self.job.paused = self.job_param['job_paused']
		self.job.task_size = self.job_param['job_size']
		self.job.animation = self.job_param['job_animation']
		self.job.start = self.job_param['job_start']
		self.job.stop = self.job_param['job_end']
		self.job.step = self.job_param['job_step']

				
		self.generate_ass(True)  # isSubmitingJob=True

		self.job.setup_range(int(cmds.currentTime(q=True)))
		self.job.setup()

		# save current layer
		current_layer = \
			cmds.editRenderLayerGlobals(
				q=True,
				currentRenderLayer=True
			)

		if job_dispatcher == 'afanasy':
			if ass_deferred and not ass_reuse:
				if exportAllRenderLayers:
					gen_cmd = self.getDeferredCmd(None)
				else:
					gen_cmd = self.getDeferredCmd(current_layer)
				gen_cmd += ' -s @#@'
				gen_cmd += ' -e @#@'
				gen_cmd += ' -b %s' % self.job.step
				print('gen_cmd = %s %s' % (gen_cmd, self.def_scene_name))

				self.job.gen_block = \
					AfanasyRenderBlock(
						'generate_ass',
						deferred_service,
						self.job,
						ass_local_assgen
					)
				self.job.gen_block.capacity = deferred_capacity
				self.job.gen_block.cmd = gen_cmd
				self.job.gen_block.input_files = '"%s"' % self.def_scene_name
				self.job.gen_block.task_size = min(ass_def_task_size,
													 self.job.num_tasks)
				# Set block Post command for cleanup 
				if job_cleanup_maya :
					self.job.gen_block.af_block.setCmdPost( 
					self.cleanup_cmd + ' ' + self.job.gen_block.input_files )										 
				
				self.job.gen_block.setup()

			renderLayers = []
			if exportAllRenderLayers:
				renderLayers = getRenderLayersList(True)  # renderable only
			else:
				# use only current layer
				renderLayers.append(current_layer)
			
			for layer in renderLayers:
				cmds.editRenderLayerGlobals(currentRenderLayer=layer)
				layer_name = layer
				if layer == 'defaultRenderLayer':
					layer_name = 'masterLayer'

				frame_block = \
					AfanasyRenderBlock(
						'render_%s' % layer_name,
						service,
						self.job
					)
				frame_block.capacity = capacity
				frame_block.input_files = '"%s"' % self.get_ass_name(
					True, layer_name)
				frame_block.out_files = self.get_image_name()
				frame_block.cmd = self.getRenderCmd(layer)
				
				# Set block Post command for cleanup
				if job_cleanup_ass :
					import re
					# replace frame number '@####@' with '*' wildcard
					input_files = re.sub ( '@[#]+@', '*', frame_block.input_files )
					frame_block.af_block.setCmdPost(
						self.cleanup_cmd + ' ' + input_files )	
					
				frame_block.setup()
				
				self.job.frames_blocks.append(frame_block)

			self.job.process()

		if exportAllRenderLayers:
			# restore current layer
			cmds.editRenderLayerGlobals(currentRenderLayer=current_layer)

	def jobFileNameOptionsChanged ( self, name, value ) :
		"""jobFileNameOptionsChanged

		:param name:
		:param value:
		"""
		if name == 'job_padding' :
			setDefaultIntValue(self_prefix, name, self.job_param, value)
			cmds.setAttr('defaultRenderGlobals.extensionPadding', value)
		elif name == 'job_separator' :
			setDefaultStrValue(self_prefix, name,	self.job_param, value)
			cmds.setAttr('defaultRenderGlobals.outFormatControl', 0)
			if value == 'none' :
				cmds.setAttr('defaultRenderGlobals.periodInExt', 0 )		
			elif value == '.' :
				cmds.setAttr('defaultRenderGlobals.periodInExt', 1 )	
			else :
				cmds.setAttr('defaultRenderGlobals.periodInExt', 2 )	
		self.setResolvedPath()
		
	def assDirNameChanged ( self, name, value ) :
		"""assDirNameChanged

		:param name:
		:param value:
		
		"""
		if name == 'ass_dirname' :
			setDefaultStrValue(self_prefix, name, self.ass_param, value)
			cmds.workspace( fileRule=('ASS',value) )
			cmds.workspace( saveWorkspace=True )
		elif name == 'ass_compressed' :
			setDefaultIntValue(self_prefix, name, self.ass_param, value)
			cmds.setAttr( 'defaultArnoldRenderOptions.output_ass_compressed', value )
		else:  # ass_padding, ass_perframe, 
			setDefaultIntValue(self_prefix, name, self.ass_param, value)
		self.setResolvedPath()

	def setResolvedPath(self) :
		"""Displays preview of current .ass full filename in textFieldGrp
		"""
		filename = self.get_ass_name ( True, self.layer, '' )
		
		cmds.textFieldGrp(
			'%s|f0|t0|tc1|fr2|fc2|ass_resolved_path' % self.winMain,
			edit=True,
			text=filename
		)

	def enable_range ( self, arg ) :
		"""enable_range

		:param arg: Missing documentation of arg argument
		:return:
		"""
		setDefaultIntValue(self_prefix, 'job_animation', self.job_param, arg)
		cmds.intFieldGrp(
			'%s|f0|t0|tc0|fr1|fc1|job_range' % self.winMain,
			edit=True,
			enable=arg
		)

	def enable_var_capacity(self, arg) :
		"""enable_var_capacity

		:param arg: True if enable_var_capacity
		:return:
		"""
		setDefaultIntValue(
			self_prefix,
			'af_use_var_capacity',
			self.afanasy_param,
			arg
		)

		cmds.floatFieldGrp(
			'%s|f0|t0|tc3|fr1|fc1|af_var_capacity' % self.winMain,
			edit=True,
			enable=arg
		)

	def enable_deferred(self, arg) :
		"""enable_deferred

		:param arg: True if enabled
		"""
		ass_def_frame = self.winMain + '|f0|t0|tc1|fr3'
		ass_def = ass_def_frame + '|fc3|'
		ass_compressed = self.winMain + '|f0|t0|tc1|fr1|fc1|ass_compressed'

		setDefaultIntValue(
			self_prefix,
			'ass_deferred',
			self.ass_param,
			arg
		)
		cmds.checkBoxGrp(
			ass_def + 'ass_local_assgen',
			edit=True,
			enable=arg
		)
		cmds.intFieldGrp(
			ass_def + 'ass_def_task_size',
			edit=True,
			enable=arg
		)
		cmds.checkBoxGrp(
			ass_compressed,
			e=True,
			enable=not arg
		)

		bg_color = self.save_frame_bgc
		if arg:
			bg_color = self.def_frame_bgc

		cmds.frameLayout(
			ass_def_frame,
			edit=True,
			bgc=bg_color
		)  # , enableBackground=False
		
		self.setResolvedPath()
	
	def set_needed_os ( self, arg ) :
		"""set_needed_os

		:param arg: dummy boolean value
		"""
		af_os_chk = self.winMain + '|f0|t0|tc3|fr1|fc1|af_os'

		val1 = cmds.checkBoxGrp( af_os_chk, q=True, value1=True )
		val2 = cmds.checkBoxGrp( af_os_chk, q=True, value2=True )
		val3 = cmds.checkBoxGrp( af_os_chk, q=True, value3=True )
		af_os_lst = []
		if val1 :
			af_os_lst.append( 'windows' )
		if val2 :
			af_os_lst.append( 'linux' )	
		if val3 :
			af_os_lst.append( 'mac' )	
		af_os_str = '|'.join ( af_os_lst )

		setDefaultStrValue( self_prefix, 'af_os', self.afanasy_param, af_os_str )
		
	def onRenderLayerSelected(self, arg) :
		"""onRenderLayerSelected

		:param arg: selected render layer
		"""
		#
		self.layer = arg
		if self.layer == 'masterLayer':
			arg = 'defaultRenderLayer'

		#print '* onRenderLayerSelected %s' % self.layer

		cmds.evalDeferred(
			'import maya.OpenMaya; '
			'maya.cmds.editRenderLayerGlobals('
			'   currentRenderLayer = "%s"'
			')' % arg,
			lowestPriority=True
		)

	def renderLayerSelected(self) :
		"""renderLayerSelected
		"""
		self.layer = cmds.editRenderLayerGlobals(
			query=True,
			currentRenderLayer=True
		)

		if self.layer == 'defaultRenderLayer':
			self.layer = 'masterLayer'

		cmds.optionMenuGrp(
			self.winMain + '|f0|c0|r0|' + 'layer_selector',
			e=True,
			value=self.layer
		)

		self.setResolvedPath()

	def renderLayerRenamed(self) :
		"""renderLayerRenamed
		"""
		self.layer = cmds.editRenderLayerGlobals(
			query=True,
			currentRenderLayer=True
		)
		cmds.evalDeferred(
			partial(self.updateRenderLayerMenu),
			lowestPriority=True
		)

	def renderLayerChanged(self) :
		"""renderLayerChanged
		"""
		self.layer = cmds.editRenderLayerGlobals(
			query=True,
			currentRenderLayer=True
		)
		cmds.evalDeferred(
			partial(self.updateRenderLayerMenu),
			lowestPriority=True
		)
		return True

	def updateRenderLayerMenu(self) :
		"""updateRenderLayerMenu
		"""
		list_items = cmds.optionMenuGrp(
			self.winMain + '|f0|c0|r0|' + 'layer_selector',
			q=True,
			itemListLong=True
		)

		if list_items is not None:
			# clear OptionMenu
			for item in list_items:
				cmds.deleteUI(item)

		renderLayers = getRenderLayersList(False)
		for layer in renderLayers:
			if layer == 'defaultRenderLayer':
				layer = 'masterLayer'
			cmds.menuItem(
				label=layer,
				parent=(
				self.winMain + '|f0|c0|r0|' + 'layer_selector|OptionMenu')
			)

		self.layer = cmds.editRenderLayerGlobals(
			query=True,
			currentRenderLayer=True
		)

		if self.layer == 'defaultRenderLayer':
			self.layer = 'masterLayer'

		cmds.optionMenuGrp(
			self.winMain + '|f0|c0|r0|' + 'layer_selector',
			e=True,
			value=self.layer
		)

		cmds.evalDeferred(
			partial(self.renderLayersSetup, renderLayers),
			lowestPriority=True
		)

	def renderLayersSetup(self, layers) :
		"""renderLayersSetup

		:param layers:
		:return:
		"""
		# add script job for renaming render layer
		selector = self.winMain + '|f0|c0|r0|layer_selector'
		firstRun = True
		for layer in layers:
			if layer != 'defaultRenderLayer':
				# cmds.scriptJob(
				#     nodeNameChanged=[layer, partial(self.renderLayerRenamed)],
				#     parent=top,
				#     replacePrevious=firstRun
				# )
				cmds.scriptJob(
					nodeNameChanged=[
						layer,
						'import maya.OpenMaya; '
						'maya.cmds.evalDeferred('
						'    "meArnoldRender.renderLayerRenamed()",'
						'    lowestPriority=True'
						')'
					],
					parent=selector,
					replacePrevious=firstRun
				)
				firstRun = False

	def setupUI ( self ) :
		"""
		setupUI
		
		Main window setup
		
		"""
		self.deleteUI(True)
		self.winMain = \
			cmds.window(
				meArnoldRenderMainWnd,
				title='meArnoldRender ver.%s (%s)' % (meArnoldRenderVer,
														self.os),
				menuBar=True,
				retain=False,
				widthHeight=(420, 460)
			)

		self.mainMenu = cmds.menu(label='Commands', tearOff=False)
		cmds.menuItem(label='Render Globals ...', command=maya_render_globals)
		cmds.menuItem(label='Check Texture Paths ...', command = checkTextures)
		cmds.menuItem(label='Generate .ass', command=self.generate_ass)
		cmds.menuItem(label='Submit Job', command=self.submitJob)
		cmds.menuItem(divider=True)
		cmds.menuItem(label='Close', command=self.deleteUI)

		#
		# setup render layers script jobs
		#
		cmds.scriptJob(
			attributeChange=[
				'renderLayerManager.currentRenderLayer',
				'import maya.OpenMaya; '
				'maya.cmds.evalDeferred('
				'    "meArnoldRender.renderLayerSelected()",'
				'    lowestPriority=True'
				')'
			],
			parent=self.winMain
		)
		cmds.scriptJob(
			event=[
				'renderLayerChange',
				'import maya.OpenMaya; '
				'maya.cmds.evalDeferred('
				'    "meArnoldRender.renderLayerChanged()",'
				'    lowestPriority=True'
				')'
			],
			parent=self.winMain
		)

		cw1 = 120
		cw2 = 60
		cw3 = 20

		ar_hi = 8

		form = cmds.formLayout('f0', numberOfDivisions=100)
		proj = cmds.columnLayout(
			'c0',
			columnAttach=('left', 0),
			rowSpacing=2,
			adjustableColumn=True,
			height=50
		)
		cmds.textFieldGrp(
			cw=(1, 70),
			adj=2,
			label='Project Root ',
			text=self.rootDir,
			editable=False
		)  # , bgc=(0,0,0)

		cmds.rowLayout('r0', numberOfColumns=2)
		layer_selector = cmds.optionMenuGrp(
			'layer_selector',
			cw=((1, 70),),
			cal=(1, 'right'),
			label='Render Layer ',
			cc=partial(self.onRenderLayerSelected)
		)
		self.updateRenderLayerMenu()

		cmds.checkBoxGrp(
			'ass_export_all_layers',
			cw=((1, cw1), (2, cw1 * 2)),
			label='Export All Renderable ',
			value1=self.ass_param['ass_export_all_layers'],
			cc=partial(
				setDefaultIntValue,
				self_prefix,
				'ass_export_all_layers',
				self.ass_param
			)
		)

		cmds.setParent('..')
		cmds.setParent('..')

		#
		# setup tabs
		#
		tab = cmds.tabLayout(
			't0',
			scrollable=True,
			childResizable=True
		)  # tabLayout -scr true -cr true  tabs; //

		#
		# Job tab
		#
		tab_job = cmds.columnLayout(
			'tc0',
			columnAttach=('left', 0),
			rowSpacing=0,
			adjustableColumn=True
		)

		cmds.frameLayout(
			'fr1',
			label=' Parameters ',
			borderVisible=True,
			borderStyle='etchedIn',
			marginHeight=ar_hi
		)

		cmds.columnLayout(
			'fc1',
			columnAttach=('left', 0),
			rowSpacing=0,
			adjustableColumn=True
		)

		job_dispatcher = cmds.optionMenuGrp(
			'job_dispatcher',
			cw=((1, cw1),),
			cal=(1, 'right'),
			label='Job Dispatcher ',
			cc=partial(
				setDefaultStrValue,
				self_prefix,
				'job_dispatcher',
				self.job_param
			)
		)

		for name in ('none', 'afanasy'):
			cmds.menuItem(label=name)  # 'backburner',

		cmds.optionMenuGrp(
			job_dispatcher,
			e=True,
			value=self.job_param['job_dispatcher']
		)

		cmds.text(label='')

		cmds.textFieldGrp(
			'job_name',
			cw=(1, cw1),
			adj=2,
			label='Job Name ',
			text=self.job_param['job_name'],
			cc=partial(
				setDefaultStrValue,
				self_prefix,
				'job_name',
				self.job_param
			)
		)

		cmds.textFieldGrp(
			'job_description',
			cw=(1, cw1),
			adj=2,
			label='Description ',
			text=self.job_param['job_description'],
			cc=partial(
				setDefaultStrValue,
				self_prefix,
				'job_description',
				self.job_param
			)
		)

		cmds.checkBoxGrp(
			'job_paused',
			cw=((1, cw1), (2, cw1 * 2)),
			label='Start Paused ',
			value1=self.job_param['job_paused'],
			cc=partial(
				setDefaultIntValue,
				self_prefix,
				'job_paused',
				self.job_param
			)
		)

		cmds.text(label='')
		cmds.checkBoxGrp(
			'job_animation',
			cw=((1, cw1), (2, cw1 * 2)),
			label='Animation ',
			value1=self.job_param['job_animation'],
			cc=partial(self.enable_range)
		)

		cmds.intFieldGrp(
			'job_range',
			cw=((1, cw1), (2, cw2), (3, cw2), (4, cw2)),
			nf=3,
			label='Start/Stop/By ',
			value1=self.job_param['job_start'],
			value2=self.job_param['job_end'],
			value3=self.job_param['job_step'],
			enable=self.job_param['job_animation'],
			cc=partial(
				setDefaultIntValue3,
				self_prefix,
				('job_start', 'job_end', 'job_step'),
				self.job_param
			)
		)

		cmds.intFieldGrp(
			'job_size',
			cw=((1, cw1), (2, cw2)),
			label='Task Size ',
			ann='Should be smaller then number of frames to render',
			value1=self.job_param['job_size'],
			cc=partial(
				setDefaultIntValue,
				self_prefix,
				'job_size',
				self.job_param
			)
		)

		cmds.intFieldGrp(
			'job_priority',
			cw=((1, cw1), (2, cw2)),
			label='Priority ',
			value1=self.job_param['job_priority'],
			cc=partial(
				setDefaultIntValue,
				self_prefix,
				'job_priority',
				self.job_param
			)
		)

		cmds.setParent('..')
		cmds.setParent('..')
		
		cmds.frameLayout(
			'fr2',
			label=' File Names Options ',
			borderVisible=True,
			borderStyle='etchedIn',
			marginHeight=ar_hi,
			cll=True,
			cl=True
		)

		cmds.columnLayout(
			'fc2',
			columnAttach=('left', 0),
			rowSpacing=0,
			adjustableColumn=True
		)
		
		cmds.intFieldGrp(
			'job_padding',
			cw=((1, cw1), (2, cw2)),
			label='Frame Padding ',
			value1=self.job_param['job_padding'],
			cc=partial(self.jobFileNameOptionsChanged, 'job_padding')
		)
		
		job_separator = cmds.optionMenuGrp(
			'job_separator',
			cw=((1, cw1),),
			cal=(1, 'right'),
			label='Separator ',
			cc=partial( self.jobFileNameOptionsChanged, 'job_separator' ) 
		)

		for name in job_separator_list:
			cmds.menuItem(label=name)  

		cmds.optionMenuGrp(
			job_separator,
			e=True,
			value=self.job_param['job_separator']
		)
		
		cmds.setParent('..')
		cmds.setParent('..')

		cmds.frameLayout(
			'fr3',
			label=' Cleanup ',
			borderVisible=True,
			borderStyle='etchedIn',
			marginHeight=ar_hi,
			cll=True,
			cl=True
		)

		cmds.columnLayout(
			'fc3',
			columnAttach=('left', 0),
			rowSpacing=0,
			adjustableColumn=True
		)
		
		cmds.checkBoxGrp(
			'job_cleanup_ass',
			cw=((1, cw1), (2, cw1 * 2)),
			label='',
			label1=' .ass files',
			value1=self.job_param['job_cleanup_ass'],
			enable=True,
			cc=partial(
				setDefaultIntValue,
				self_prefix,
				'job_cleanup_ass',
				self.job_param
			)
		)

		cmds.checkBoxGrp(
			'job_cleanup_maya',
			cw=((1, cw1), (2, cw1 * 2)),
			label='',
			label1=' maya _deferred file',
			value1=self.job_param['job_cleanup_maya'],
			enable=True,
			cc=partial(
				setDefaultIntValue,
				self_prefix,
				'job_cleanup_maya',
				self.job_param
			)
		)
		
		cmds.text( label ='\n   Note: Files will be cleaned after the job delete', al='left' )
		
		cmds.setParent('..')
		cmds.setParent('..')
		
		cmds.setParent('..')

		#
		# .ass files generation tab
		#
		tab_assparam = cmds.columnLayout(
			'tc1',
			columnAttach=('left', 0),
			rowSpacing=0,
			adjustableColumn=True
		)

		cmds.frameLayout(
			'fr3',
			label=' Deferred .ass generation ',
			borderVisible=True,
			borderStyle='etchedIn',
			marginHeight=ar_hi,
			cll=True,
			cl=True
		)

		cmds.columnLayout(
			'fc3',
			columnAttach=('left', 0),
			rowSpacing=0,
			adjustableColumn=True
		)

		cmds.checkBoxGrp(
			'ass_deferred',
			cw=((1, cw1), (2, cw1 * 2)),
			label='Use deferred ',
			ann='Generate .ass files in background process',
			value1=self.ass_param['ass_deferred'],
			cc=partial(self.enable_deferred)
		)

		cmds.checkBoxGrp(
			'ass_local_assgen',
			cw=((1, cw1), (2, cw1 * 2)),
			label='',
			label1=" Only on localhost ",
			ann="Do not use remote hosts",
			value1=self.ass_param['ass_local_assgen'],
			enable=self.ass_param['ass_deferred'],
			cc=partial(
				setDefaultIntValue,
				self_prefix,
				'ass_local_assgen',
				self.ass_param
			)
		)

		cmds.intFieldGrp(
			'ass_def_task_size',
			cw=((1, cw1), (2, cw2)),
			label='Task Size ',
			value1=self.ass_param['ass_def_task_size'],
			enable=self.ass_param['ass_deferred'],
			cc=partial(
				setDefaultIntValue,
				self_prefix,
				'ass_def_task_size',
				self.ass_param
			)
		)

		#self.save_frame_bgc = cmds.frameLayout ( 'fr3', query = True, bgc = True )
		#self.def_frame_bgc = [ 0.75, 0.5, 0 ]
		bg_color = self.save_frame_bgc

		if self.ass_param['ass_deferred']:
			bg_color = self.def_frame_bgc

		cmds.frameLayout(
			self.winMain + '|f0|t0|tc1|fr3',
			edit=True,
			bgc=bg_color
		)  # , enableBackground=False

		cmds.setParent('..')
		cmds.setParent('..')

		cmds.frameLayout(
			'fr1',
			label=' Export Settings ',
			borderVisible=True,
			borderStyle='etchedIn',
			marginHeight=ar_hi
		)

		cmds.columnLayout(
			'fc1',
			columnAttach=('left', 0),
			rowSpacing=0,
			adjustableColumn=True
		)

		cmds.checkBoxGrp(
			'ass_reuse',
			cw=((1, cw1), (2, cw1 * 2)),
			label='Use existing .ass files ',
			ann='Do not generate .ass files if they are exist',
			value1=self.ass_param['ass_reuse'],
			cc=partial(
				setDefaultIntValue,
				self_prefix,
				'ass_reuse',
				self.ass_param
			)
		)

		cmds.text(label='')

		ass_dirname = cmds.textFieldButtonGrp(
			'ass_dirname',
			cw=(1, cw1),
			enable=True,
			adj=2,
			label='Directory Name ',
			buttonLabel='...',
			text=self.ass_param['ass_dirname'],
			cc=partial(self.assDirNameChanged, 'ass_dirname')
		)

		cmds.textFieldButtonGrp(
			ass_dirname,
			edit=True,
			bc=partial(
				browseDirectory,
				self.rootDir,
				ass_dirname
			),
			cc=partial(self.assDirNameChanged, 'ass_dirname')
		)
		"""
		cmds.checkBoxGrp(
			'ass_perframe',
			cw=((1, cw1), (2, cw1 * 2)),
			label='',
			label1=' File Per Frame ',
			value1=self.ass_param['ass_perframe'],
			cc=partial(self.assDirNameChanged, 'ass_perframe')
		)
		"""
		cmds.checkBoxGrp(
			'ass_selection',
			cw=((1, cw1), (2, cw1 * 2)),
			label='',
			label1=' Export Only Selected Objects',
			value1=self.ass_param['ass_selection'],
			cc=partial(
				setDefaultIntValue,
				self_prefix,
				'ass_selection',
				self.ass_param
			)
		)

		cmds.checkBoxGrp(
			'ass_binary',
			cw=((1, cw1), (2, cw1 * 2)),
			label='',
			label1=' Binary',
			value1=self.ass_param['ass_binary'],
			cc=partial(
				setDefaultIntValue,
				self_prefix,
				'ass_binary',
				self.ass_param
			)
		)

		cmds.checkBoxGrp(
			'ass_compressed',
			cw=((1, cw1), (2, cw1 * 2)),
			label='',
			label1=' Compression',
			value1=self.ass_param['ass_compressed'],
			enable=not self.ass_param['ass_deferred'],
			cc=partial(self.assDirNameChanged, 'ass_compressed')
		)
	
		cmds.checkBoxGrp(
			'ass_expand_procedurals',
			cw=((1, cw1), (2, cw1 * 2)),
			label='',
			label1=' Expand Procedurals',
			value1=self.ass_param['ass_expand_procedurals'],
			cc=partial(
				setDefaultIntValue,
				self_prefix,
				'ass_expand_procedurals',
				self.ass_param
			)
		)
		
		cmds.checkBoxGrp(
			'ass_export_bounds',
			cw=((1, cw1), (2, cw1 * 2)),
			label='',
			label1=' Export Bounding Box (.asstoc)',
			value1=self.ass_param['ass_export_bounds'],
			cc=partial(
				setDefaultIntValue,
				self_prefix,
				'ass_export_bounds',
				self.ass_param
			)
		)

		cmds.setParent('..')
		cmds.setParent('..')
		

		cmds.frameLayout(
			'fr2',
			label=' Resolved Path ',
			borderVisible=True,
			borderStyle='etchedIn',
			marginHeight=ar_hi
		)

		cmds.columnLayout(
			'fc2',
			columnAttach=('left', 0),
			rowSpacing=0,
			adjustableColumn=True
		)

		cmds.textFieldGrp(
			'ass_resolved_path',
			cw=(1, 0),
			adj=2,
			label='',
			text='',
			editable=False
		)

		self.setResolvedPath()

		cmds.setParent('..')
		cmds.setParent('..')

		cmds.setParent('..')

		#
		# Renderer tab
		#
		tab_render = cmds.columnLayout(
			'tc2',
			columnAttach=('left', 0),
			rowSpacing=0,
			adjustableColumn=True
		)


		cmds.frameLayout(
			'fr1',
			label=' Arnold options ',
			borderVisible=True,
			borderStyle='etchedIn',
			marginHeight=ar_hi
		)

		cmds.columnLayout(
			'fc1',
			columnAttach=('left', 0),
			rowSpacing=0,
			adjustableColumn=True
		)

		cmds.textFieldGrp(
			'ar_options',
			cw=(1, cw1),
			adj=2,
			label='Additional Options ',
			text=self.ar_param['ar_options'],
			cc=partial(
				setDefaultStrValue,
				self_prefix,
				'ar_options',
				self.ar_param
			)
		)

		cmds.text(label='')

		ar_verbosity = cmds.optionMenuGrp(
			'ar_verbosity',
			cw=((1, cw1),),
			cal=(1, 'right'),
			label='Verbosity ',
			cc=partial(
				setDefaultStrValue,
				self_prefix,
				'ar_verbosity',
				self.ar_param
			)
		)

		for name in ar_verbosity_list:
			cmds.menuItem(label=name)

		cmds.optionMenuGrp(
			ar_verbosity,
			e=True,
			value=self.ar_param['ar_verbosity']
		)

		cmds.intFieldGrp(
			'ar_threads',
			cw=((1, cw1), (2, cw2)),
			label='Threads ',
			ann='The number of threads',
			value1=self.ar_param['ar_threads'],
			cc=partial(
				setDefaultIntValue,
				self_prefix,
				'ar_threads',
				self.ar_param
			)
		)

		cmds.setParent('..')
		cmds.setParent('..')
		
		cmds.frameLayout(
			'fr2',
			label=' Search Paths ',
			borderVisible=True,
			borderStyle='etchedIn',
			marginHeight=ar_hi,
			cll=True,
			cl=True
		)

		cmds.columnLayout(
			'fc2',
			columnAttach=('left', 0),
			rowSpacing=0,
			adjustableColumn=True
		)

		cmds.checkBoxGrp(
			'ar_abs_tex_path',
			cw=((1, cw1), (2, cw1 * 2)),
			label='',
			label1=' Absolute Texture Paths',
			value1=self.ar_param['ar_abs_tex_path'],
			cc=partial(
				setDefaultIntValue,
				self_prefix,
				'ar_abs_tex_path',
				self.ar_param
			)
		)
		
		cmds.checkBoxGrp(
			'ar_abs_proc_path',
			cw=((1, cw1), (2, cw1 * 2)),
			label='',
			label1=' Absolute Procedural Paths',
			value1=self.ar_param['ar_abs_proc_path'],
			cc=partial(
				setDefaultIntValue,
				self_prefix,
				'ar_abs_proc_path',
				self.ar_param
			)
		)
		
		cmds.textFieldGrp(
			'ar_plugin_path',
			cw=(1, cw1),
			adj=2,
			label='Plug-ins Path ',
			text=self.ar_param['ar_plugin_path'],
			cc=partial(
				setDefaultStrValue,
				self_prefix,
				'ar_plugin_path',
				self.ar_param
			)
		)
		
		cmds.textFieldGrp(
			'ar_proc_search_path',
			cw=(1, cw1),
			adj=2,
			label='Procedural Search Path ',
			text=self.ar_param['ar_proc_search_path'],
			cc=partial(
				setDefaultStrValue,
				self_prefix,
				'ar_proc_search_path',
				self.ar_param
			)
		)
		
		cmds.textFieldGrp(
			'ar_shader_search_path',
			cw=(1, cw1),
			adj=2,
			label='Shaders Search Path ',
			text=self.ar_param['ar_shader_search_path'],
			cc=partial(
				setDefaultStrValue,
				self_prefix,
				'ar_shader_search_path',
				self.ar_param
			)
		)
		
		cmds.textFieldGrp(
			'ar_tex_search_path',
			cw=(1, cw1),
			adj=2,
			label='Textures Search Path ',
			text=self.ar_param['ar_tex_search_path'],
			cc=partial(
				setDefaultStrValue,
				self_prefix,
				'ar_tex_search_path',
				self.ar_param
			)
		)
		
		cmds.setParent('..')
		cmds.setParent('..')

		cmds.setParent('..')

		#
		# Afanasy tab
		#
		tab_afanasy = cmds.columnLayout(
			'tc3',
			columnAttach=('left', 0),
			rowSpacing=0,
			adjustableColumn=True
		)

		cmds.frameLayout(
			'fr1',
			label=' Parameters ',
			borderVisible=True,
			borderStyle='etchedIn',
			marginHeight=ar_hi
		)

		cmds.columnLayout(
			'fc1',
			columnAttach=('left', 4),
			rowSpacing=0,
			adjustableColumn=True
		)

		cmds.intFieldGrp(
			'af_capacity',
			cw=((1, cw1), (2, cw2)),
			label='Task Capacity ',
			value1=self.afanasy_param['af_capacity'],
			cc=partial(
				setDefaultIntValue,
				self_prefix,
				'af_capacity',
				self.afanasy_param
			)
		)

		cmds.intFieldGrp(
			'af_deferred_capacity',
			cw=((1, cw1), (2, cw2)),
			label='Deferred Capacity ',
			value1=self.afanasy_param['af_deferred_capacity'],
			cc=partial(
				setDefaultIntValue,
				self_prefix,
				'af_deferred_capacity',
				self.afanasy_param
			)
		)

		cmds.checkBoxGrp(
			'af_use_var_capacity',
			cw=((1, cw1), (2, cw1 * 2)),
			label='Use Variable Capacity ',
			ann='Block can generate tasks with capacity*coefficient to fit '
				'free render capacity',
			value1=self.afanasy_param['af_use_var_capacity'],
			cc=partial(self.enable_var_capacity)
		)

		cmds.floatFieldGrp(
			'af_var_capacity',
			cw=((1, cw1), (2, cw2), (3, cw2), (4, cw2)),
			nf=2,
			pre=2,
			label='Min/Max coefficient ',
			value1=self.afanasy_param['af_cap_min'],
			value2=self.afanasy_param['af_cap_max'],
			enable=self.afanasy_param['af_use_var_capacity'],
			cc=partial(
				setDefaultFloatValue2,
				self_prefix,
				('af_cap_min', 'af_cap_max'),
				self.afanasy_param
			)
		)

		cmds.intFieldGrp(
			'af_max_running_tasks',
			cw=((1, cw1), (2, cw2)),
			label='Max Running Tasks ',
			value1=self.afanasy_param['af_max_running_tasks'],
			cc=partial(
				setDefaultIntValue,
				self_prefix,
				'af_max_running_tasks',
				self.afanasy_param
			)
		)

		cmds.intFieldGrp(
			'af_max_tasks_per_host',
			cw=((1, cw1), (2, cw2)),
			label='Max Tasks Per Host ',
			value1=self.afanasy_param['af_max_tasks_per_host'],
			cc=partial(
				setDefaultIntValue,
				self_prefix,
				'af_max_tasks_per_host',
				self.afanasy_param
			)
		)

		cmds.textFieldGrp(
			'af_service',
			cw=(1, cw1),
			adj=2,
			label='Service ',
			text=self.afanasy_param['af_service'],
			enable=False,
			cc=partial(
				setDefaultStrValue,
				self_prefix,
				'af_service',
				self.afanasy_param
			)
		)

		cmds.textFieldGrp(
			'af_deferred_service',
			cw=(1, cw1),
			adj=2,
			label='Deferred Service ',
			text=self.afanasy_param['af_deferred_service'],
			enable=False,
			cc=partial(
				setDefaultStrValue,
				self_prefix,
				'af_deferred_service',
				self.afanasy_param
			)
		)

		cmds.textFieldGrp(
			'af_hostsmask',
			cw=(1, cw1),
			adj=2,
			label='Hosts Mask ',
			ann='Job run only on renders which host name matches this mask\n'
				'e.g.  .* or host.*',
			text=self.afanasy_param['af_hostsmask'],
			cc=partial(
				setDefaultStrValue,
				self_prefix,
				'af_hostsmask',
				self.afanasy_param
			)
		)

		cmds.textFieldGrp(
			'af_hostsexcl',
			cw=(1, cw1),
			adj=2,
			label='Exclude Hosts Mask ',
			ann='Job can not run on renders which host name matches this '
				'mask\n e.g.  host.* or host01|host02',
			text=self.afanasy_param['af_hostsexcl'],
			cc=partial(
				setDefaultStrValue,
				self_prefix,
				'af_hostsexcl',
				self.afanasy_param
			)
		)

		cmds.textFieldGrp(
			'af_depmask',
			cw=(1, cw1),
			adj=2,
			label='Depend Mask ',
			ann='Job will wait other user jobs which name matches this mask',
			text=self.afanasy_param['af_depmask'],
			cc=partial(
				setDefaultStrValue,
				self_prefix,
				'af_depmask',
				self.afanasy_param
			)
		)

		cmds.textFieldGrp(
			'af_depglbl',
			cw=(1, cw1),
			adj=2,
			label='Global Depend Mask ',
			ann='Job will wait other jobs from any user which name matches '
				'this mask',
			text=self.afanasy_param['af_depglbl'],
			cc=partial(
				setDefaultStrValue,
				self_prefix,
				'af_depglbl',
				self.afanasy_param
			)
		)

		"""
		cmds.textFieldGrp(
			'af_os',
			cw=(1, cw1),
			adj=2,
			label='Needed OS ',
			ann='windows linux mac',
			text=self.afanasy_param['af_os'],
			cc=partial(
				setDefaultStrValue,
				self_prefix,
				'af_os',
				self.afanasy_param
			)
		)
		"""
		cmds.checkBoxGrp(
			'af_os',
			cw=((1, cw1), (2, cw1/2), (3, cw1/2), (4, cw1/2)),
			numberOfCheckBoxes = 3,
			label='Needed OS  ',
			label1='windows',
			value1=( self.afanasy_param['af_os'].find('windows') != -1 ),
			label2='linux',
			value2=( self.afanasy_param['af_os'].find('linux') != -1 ),
			label3='mac',
			value3=( self.afanasy_param['af_os'].find('mac') != -1 ),
			cc=partial( self.set_needed_os )
		)

		cmds.setParent('..')
		cmds.setParent('..')
		cmds.setParent('..')

		cmds.tabLayout(
			tab,
			edit=True,
			tabLabel=(
				(tab_job, "Job"),
				(tab_assparam, ".ass"),
				(tab_render, "Renderer"),
				(tab_afanasy, "Afanasy")
			)
		)

		cmds.setParent(form)

		btn_sbm = cmds.button(
			label='Submit',
			command=self.submitJob,
			ann='Generate .ass files and submit to dispatcher'
		)

		btn_gen = cmds.button(
			label='Generate .ass',
			command=self.generate_ass,
			ann='Force .ass files generation'
		)
		btn_cls = cmds.button(label='Close', command=self.deleteUI)

		cmds.formLayout(
			form,
			edit=True,
			attachForm=(
				(proj, 'top', 0),
				(proj, 'left', 0),
				(proj, 'right', 0),
				(tab, 'left', 0),
				(tab, 'right', 0),
				(btn_cls, 'bottom', 0),
				(btn_gen, 'bottom', 0),
				(btn_sbm, 'bottom', 0),
				(btn_sbm, 'left', 0),
				(btn_cls, 'right', 0)
			),
			attachControl=(
				(tab, 'top', 0, proj),
				(tab, 'bottom', 0, btn_sbm),
				(btn_gen, 'left', 0, btn_sbm),
				(btn_gen, 'right', 0, btn_cls)
			),
			attachPosition=(
				(btn_sbm, 'right', 0, 33),
				(btn_gen, 'right', 0, 66),
				(btn_cls, 'left', 0, 66)
			)
		)

		if not self.NoGUI :
			cmds.showWindow(self.winMain)

		return form

	def deleteUI ( self, param ) :
		"""deleteUI

		:param param: dummy parameter
		"""
		winMain = meArnoldRenderMainWnd

		if cmds.window(winMain, exists=True):
			cmds.deleteUI(winMain, window=True)

		if cmds.windowPref(winMain, exists=True):
			cmds.windowPref(winMain, remove=True)


print('meArnoldRender sourced ...')
