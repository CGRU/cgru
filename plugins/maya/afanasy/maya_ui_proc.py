# -*- coding: utf-8 -*-
"""
  maya_ui_proc.py

  ver.0.4.1 (10 May 2013)
    getMayaSceneName ( withoutSubdir = True )
  ver.0.4.0 (13 Mar 2013)
    getRenderLayersList:
    - check for referenced render layers
  ver.0.3.0 (28 Jan 2013)
  ver.0.2.0 (18 Jan 2013)
  ver.0.1.0 (17 Jan 2013)

  Author:

  Yuri Meshalkin (aka mesh)
  mesh@kpp.kiev.ua

  (c) Kiev Post Production 2013

  Description:

  Collection of procedures for working
  with Maya UI

"""

import os

import maya.cmds as cmds
import maya.mel as mel


def getDefaultStrValue(self_prefix, name, value):
	"""getDefaultStrValue

	:param self_prefix:
	:param name:
	:param value:
	:return:
	"""
	var_name = self_prefix + name
	if cmds.optionVar(exists=var_name) == 1:
		ret = cmds.optionVar(q=var_name)
	else:
		cmds.optionVar(sv=(var_name, value))
		ret = value
	return ret


def getDefaultIntValue(self_prefix, name, value):
	"""getDefaultIntValue

	:param self_prefix:
	:param name:
	:param value:
	:return:
	"""
	var_name = self_prefix + name
	if cmds.optionVar(exists=var_name) == 1:
		ret = cmds.optionVar(q=var_name)
	else:
		cmds.optionVar(iv=(var_name, value))
		ret = value
	return ret


def getDefaultFloatValue(self_prefix, name, value):
	"""getDefaultFloatValue

	:param self_prefix:
	:param name:
	:param value:
	:return:
	"""
	var_name = self_prefix + name
	if cmds.optionVar(exists=var_name) == 1:
		ret = cmds.optionVar(q=var_name)
	else:
		cmds.optionVar(fv=(var_name, value))
		ret = value
	return ret


def setDefaultIntValue(self_prefix, name, section=None, value=None):
	"""setDefaultIntValue

	:param self_prefix:
	:param name:
	:param section:
	:param value:
	:return:
	"""
	var_name = self_prefix + name
	cmds.optionVar(iv=(var_name, value))
	if section is not None:
		section[name] = value
	return value


def setDefaultIntValue2(self_prefix, names, section=None, value1=None, value2=None):
	"""setDefaultIntValue2

	:param self_prefix:
	:param names:
	:param section:
	:param value1:
	:param value2:
	:return:
	"""
	var_name = self_prefix + names[0]
	cmds.optionVar(iv=(var_name, value1))
	var_name = self_prefix + names[1]
	cmds.optionVar(iv=(var_name, value2))
	if section is not None:
		section[names[0]] = value1
		section[names[1]] = value2
	return value1, value2


def setDefaultIntValue3(self_prefix, names, section=None, value1=None, value2=None, value3=None):
	"""setDefaultIntValue3

	:param self_prefix:
	:param names:
	:param section:
	:param value1:
	:param value2:
	:param value3:
	:return:
	"""
	var_name = self_prefix + names[0]
	cmds.optionVar(iv=(var_name, value1))
	var_name = self_prefix + names[1]
	cmds.optionVar(iv=(var_name, value2))
	var_name = self_prefix + names[2]
	cmds.optionVar(iv=(var_name, value3))
	if section is not None:
		section[names[0]] = value1
		section[names[1]] = value2
		section[names[2]] = value3
	return value1, value2, value3


def setDefaultFloatValue(self_prefix, name, section=None, value=None):
	"""setDefaultFloatValue

	:param self_prefix:
	:param name:
	:param section:
	:param value:
	:return:
	"""
	var_name = self_prefix + name
	cmds.optionVar(fv=(var_name, value))
	if section is not None:
		section[name] = value
	return value


def setDefaultFloatValue2(self_prefix, names, section=None, value1=None, value2=None):
	"""setDefaultFloatValue2

	:param self_prefix:
	:param names:
	:param section:
	:param value1:
	:param value2:
	:return:
	"""
	var_name = self_prefix + names[0]
	cmds.optionVar(fv=(var_name, value1))
	var_name = self_prefix + names[1]
	cmds.optionVar(fv=(var_name, value2))
	if section is not None:
		section[names[0]] = value1
		section[names[1]] = value2
	return value1, value2


def setDefaultStrValue(self_prefix, name, section=None, value=None):
	"""setDefaultStrValue

	:param self_prefix:
	:param name:
	:param section:
	:param value:
	:return:
	"""
	var_name = self_prefix + name
	cmds.optionVar(sv=(var_name, value))
	if section is not None:
		section[name] = value
	return value


def getMayaSceneName(withoutSubdir=True):
	"""getMayaSceneName

	:param withoutSubdir: If ``withoutSubdir`` is **False**, then return scene
	  relative path to current project (include subdirectory in scene dir).
	:return:
	"""
	fullName = cmds.file(q=True, sceneName=True)

	if withoutSubdir:
		sceneName = os.path.basename(fullName)
	else:
		sceneName = cmds.workspace(projectPath=fullName)

	sceneName, ext = os.path.splitext(sceneName)

	if sceneName == '':
		sceneName = 'untitled'

	return sceneName


def isRelative(fileName):
	"""isRelative

	:param fileName:
	:return:
	"""
	ret = True
	if fileName != '':
		if fileName[1] == ':' or fileName[0] == '/' or fileName[0] == '\\':
			ret = False
	return ret


def fromNativePath(nativePath):
	"""fromNativePath

	:param nativePath:
	:return:
	"""
	return str(nativePath).replace('\\', '/')


def getDirName(image_name):
	"""getDirName

	:param image_name:
	:return:
	"""
	filename = cmds.workspace(expandName=image_name)
	image_dir = os.path.dirname(filename)
	return image_dir


def getPadStr(padding, perframe):
	"""Get padding string
	"""
	if perframe :
		pad_str = '#'
		if padding > 1 and perframe:
			for i in range(1, padding):
				pad_str += '#'
	else :
		pad_str = ''
	return pad_str


def browseDirectory(rootDir, control):
	"""browseDirectory

	:param rootDir:
	:param control:
	:return:
	"""
	path = cmds.textFieldButtonGrp(control, q=True, text=True)
	startDir = path

	if isRelative(path):
		startDir = os.path.join(rootDir, path)

	dirNames = cmds.fileDialog2(
		fileMode=3,
		startingDirectory=startDir,
		dialogStyle=1
	)

	if dirNames is not None:
		dirName = cmds.workspace(projectPath=fromNativePath(dirNames[0]))
		cmds.textFieldButtonGrp(
			control,
			e=True,
			text=dirName,
			forceChangeCommand=True
		)


def browseFile(rootDir, control, extFilter):
	"""browseFile

	:param rootDir:
	:param control:
	:param extFilter:
	:return:
	"""
	path = cmds.textFieldButtonGrp(control, q=True, text=True)
	startDir = path
	if isRelative(path):
		startDir = os.path.join(rootDir, path)

	fileNames = cmds.fileDialog2(
		fileMode=1,
		startingDirectory=os.path.dirname(startDir),
		dialogStyle=1,
		fileFilter=extFilter
	)

	if fileNames is not None:
		fileName = cmds.workspace(projectPath=fromNativePath(fileNames[0]))
		cmds.textFieldButtonGrp(
			control, e=True, text=fileName, forceChangeCommand=True
		)


def getRenderCamerasList():
	"""getRenderCamerasList
	"""
	renderCamerasList = []
	cameras = cmds.listCameras()
	for cam in cameras:
		camShape = cam
		if cmds.objectType(cam) == 'transform':  # != 'camera' :
			camShapes = cmds.listRelatives(str(cam), shapes=True)
			camShape = camShapes[0]
		if cmds.getAttr(camShape + '.renderable'):
			renderCamerasList.append(str(cam))
	return renderCamerasList


def getRenderLayersList(renderable=False):
	"""Returns list of render layers sorted as in RenderLayer Editor

	:param renderable:
	:return:
	"""
	renderLayers = {}
	layers = cmds.ls(type='renderLayer')
	for layer in layers:
		if not cmds.referenceQuery(layer, isNodeReferenced=True):
			id_ = cmds.getAttr(layer + '.displayOrder')
			# print '* setup : %s (%d)' % ( layer, id )
			while id_ in renderLayers:
				id_ += 1  # try to find unique id

			if renderable and (not cmds.getAttr(layer + '.renderable')):
				continue

			renderLayers[id_] = layer
			# get sorted by id list

	labels = renderLayers.values()
	labels.reverse()
	return labels


def maya_render_globals(arg):
	"""Open Maya Render Settings window

	:param arg:
	:return:
	"""
	mel.eval("unifiedRenderGlobalsWindow")


def mrShaderManager(arg):
	"""Open MentalRay Shaders window

	:param arg:
	:return:
	"""
	mel.eval("mrShaderManager")


def checkTextures(arg):
	"""Run check project's texture script

	:param arg:
	:return:
	"""
	import afanasy.meCheckTexturePaths as tx
	# reload( tx )
	tx.meCheckTexturePaths()
