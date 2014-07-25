# -*- coding: utf-8 -*-
"""
meCheckTexturePaths

ver.1.0.5 5 May 2014
	- fix for PSD file, Arnold
	- restored functionality of 'Replace' command
	- added 'Set prefix' command

ver.1.0.4 26 Jun 2013
	- fix for VRay

ver.1.0.3 20 Apr 2012
	- lookup for textures in mentalrayIblShape
	- texture types and attributes gathered from tuples list

ver.1.0.2 26 Jan 2012
	- lookup for textures in ImagePlanes
	- textures list splitted to 3 frameLayout:
		Image Planes, Maya Textures, MentalRay Textures
	- fix iconTextButton height for Maya2008

ver.1.0.0 30 Oct 2011
ver.0.0.1 28 Oct 2011

Author:

	Yuri Meshalkin (aka mesh)
	mesh@kpp.kiev.ua

	(c) Kiev Post Production 2011,2012,2013,2014

Description:

	This UI script shows lists of texture nodes
	(maya, mentalray) and imagePlanes with indication
	of MotFound/Absolute/Relative location. It helps to select
	texture nodes and to replace strings in texture names

Usage:

	import afanasy.meCheckTexturePaths as tx
	reload( tx )
	tx = tx.meCheckTexturePaths()

"""
import os
from functools import partial
import maya.cmds as cmds

import maya_ui_proc

self_prefix = 'meCheckTexturePaths_'
meCheckTexturePathsVer = "1.0.5"
meCheckTexturePathsMainWnd = self_prefix + "MainWnd"
meCheckTexturePathsReplaceWnd = self_prefix + "ReplaceWnd"
meCheckTexturePathsSetPrefixWnd = self_prefix + "SetPrefixWnd"


class meCheckTexturePaths(object):
	"""meCheckTexturePaths
	"""

	def __init__(self, selection=''):
		# print ">> meCheckTexturePaths: Class created"
		self.selection = selection
		self.winMain = ''
		self.winReplace = ''
		self.listTextures = ''
		self.rootDir = cmds.workspace(q=True, rootDirectory=True)

		self.strToFind = \
			maya_ui_proc.getDefaultStrValue(self_prefix, 'strToFind', '')
		self.strToReplace = \
			maya_ui_proc.getDefaultStrValue(self_prefix, 'strToReplace', '')
		self.strPrefix = \
			maya_ui_proc.getDefaultStrValue(self_prefix, 'strPrefix', '')

		self.fileTextures = []

		self.ui = self.setupUI()
		self.jobOnDelete = \
			cmds.scriptJob(uiDeleted=[self.winMain, self.onDeleteMainWin])

	def __del__(self):
		# print( ">> meCheckTexturePaths: Class deleted" )
		if cmds.scriptJob(exists=self.jobOnDelete):
			cmds.scriptJob(kill=self.jobOnDelete, force=True)

	def onStrToFindChanged(self, value):
		self.strToFind = \
			maya_ui_proc.setDefaultStrValue(self_prefix, 'strToFind', None,
											value)

	def onStrToReplaceChanged(self, value):
		self.strToReplace = \
			maya_ui_proc.setDefaultStrValue(self_prefix, 'strToReplace', None,
											value)

	def onStrPrefixChanged(self, value):
		self.strPrefix = \
			maya_ui_proc.setDefaultStrValue(self_prefix, 'strPrefix', None,
											value)

	def onFileNameChanged(self, fileNodeName, attr, value):
		# print( ">> meCheckTexturePaths: onFileNameChanged %s = %s" % ( fileNodeName, value ) )
		cmds.setAttr(
			fileNodeName + '.' + attr,
			value,
			type='string'
		)  # ".fileTextureName"

	def selectFileNode(self, fileNodeName):
		cmds.select(fileNodeName, r=True)

	def drawFrameLayout(self, frameTitle, textureTypeAttrList):  # , attrNameList
		cw1 = 100
		cw2 = 60
		cmds.setParent(self.listTextures)

		cmds.frameLayout(label=frameTitle, borderVisible=True,
						 borderStyle='etchedIn', marginHeight=0, cll=True,
						 cl=False)
		cmds.columnLayout(columnAttach=('left', 0), rowSpacing=0,
						  adjustableColumn=True)

		for i in range(len(self.fileTextures)):
			labelType = 'Not Found'
			labelColor = (0.5, 0.0, 0.0)
			fileNodeName = self.fileTextures[i]
			fileNodeType = cmds.objectType(fileNodeName)
			fileName = ''

			for (textureType, attrName) in textureTypeAttrList:
				#print "textureType = %s attrName = %s" % ( textureType, attrName )

				if fileNodeType != textureType:
					continue

				fileTextureName = cmds.getAttr(fileNodeName + "." + attrName)
				print('>> fileTextureName = %s' % fileTextureName)

				if fileTextureName is not None \
						and os.path.basename(fileTextureName) != '':
					fileName = str(fileTextureName)
					if cmds.file(fileTextureName, q=True, exists=True):
						labelType = 'Absolute'
						labelColor = (1.0, 0.5, 0.0)
						fileName = cmds.workspace(projectPath=fileTextureName)
						if maya_ui_proc.isRelative(fileName):
							labelType = 'Relative'
							labelColor = (0.0, 0.5, 0.0)

				cmds.rowLayout(numberOfColumns=2, columnWidth1=cw2,
							   adjustableColumn2=2)
				cmds.iconTextButton(style='textOnly', label=labelType,
									width=cw2, h=16, bgc=labelColor)
				cmds.textFieldButtonGrp(cw=(1, cw1), adj=2,
										label=fileNodeName,
										buttonLabel='select',
										text=fileName,
										cc=partial(self.onFileNameChanged,
												   fileNodeName, attrName),
										bc=partial(self.selectFileNode,
												   fileNodeName))
				cmds.setParent('..')

	def refreshUI(self, param):
		"""Missing DocString

		:param param:
		"""
		currentRenderer = cmds.getAttr('defaultRenderGlobals.currentRenderer')

		# get list of textures and store it to self.fileTextures
		special_type_str_list = ['imagePlane']
		if currentRenderer == 'mentalRay':
			special_type_str_list.append('mentalrayIblShape')
		self.fileTextures = cmds.ls(textures=True, type=special_type_str_list)

		print(self.fileTextures)

		if cmds.columnLayout(self.listTextures, q=True,
							 numberOfChildren=True) > 0:
			controls = \
				cmds.columnLayout(self.listTextures, q=True, childArray=True)
			for i in range(len(controls)):
				cmds.deleteUI(controls[i])

		self.drawFrameLayout(' Image Planes ', [('imagePlane', 'imageName')])
		self.drawFrameLayout(
			' Maya Textures ',
			[
				('file', 'fileTextureName'),
				('psdFileTex', 'fileTextureName')
			]
		)

		if currentRenderer == 'mentalRay':
			self.drawFrameLayout(
				' MentalRay Textures ',
				[
					('mentalrayTexture', 'fileTextureName'),
					('mentalrayIblShape', 'texture')
				]
			)
		elif currentRenderer == 'arnold':
			self.drawFrameLayout(' Arnold Textures ',
								 [('aiImage', 'filename')])

	def setupUI(self):
		"""Missing DocString
		"""
		cw1 = 100
		cw2 = 60
		self.deleteUI(True)
		# Main window setup
		self.winMain = cmds.window(
			meCheckTexturePathsMainWnd,
			title="meCheckTexturePaths ver." + meCheckTexturePathsVer,
			menuBar=True,
			retain=False,
			widthHeight=(500, 400)
		)

		form = cmds.formLayout('f0', numberOfDivisions=100)
		proj = cmds.columnLayout(
			'c0',
			columnAttach=('left', 0),
			rowSpacing=2,
			adjustableColumn=True,
			height=32
		)

		cmds.textFieldGrp(
			cw=(1, cw2),
			columnAlign=(1, 'center'),
			adj=2,
			label=" Project ",
			text=self.rootDir,
			editable=False
		)

		cmds.setParent('..')

		scr = cmds.scrollLayout('scr', childResizable=True)
		self.listTextures = cmds.columnLayout(
			'c1',
			rowSpacing=0,
			adjustableColumn=True
		)  # columnAttach=('left',0 ), bgc=(0.5, 0.5, 0.0)

		self.refreshUI(True)

		cmds.setParent(form)

		btn_rep = cmds.button(label='Replace ...', command=self.onReplace)
		btn_prefix = cmds.button(label='Set prefix ...',
								 command=self.onSetPrefix)
		btn_ref = cmds.button(label='Refresh', command=self.refreshUI)
		btn_cls = cmds.button(label='Close', command=self.deleteUI)

		cmds.formLayout(
			form,
			edit=True,
			attachForm=(
				(proj, 'top', 0),
				(proj, 'left', 0),
				(proj, 'right', 0),
				(scr, 'left', 0),
				(scr, 'right', 0),
				(btn_cls, 'bottom', 0),
				(btn_prefix, 'bottom', 0),
				(btn_rep, 'bottom', 0),
				(btn_ref, 'bottom', 0),
				(btn_rep, 'left', 0),
				(btn_cls, 'right', 0)
			),
			attachControl=(
				(scr, 'top', 0, proj),
				(scr, 'bottom', 0, btn_rep),
				(btn_ref, 'left', 0, btn_prefix),
				(btn_rep, 'right', 0, btn_prefix),
				(btn_ref, 'right', 0, btn_cls)
			),
			attachPosition=(
				(btn_rep, 'right', 0, 25),
				(btn_prefix, 'left', 0, 25),
				(btn_prefix, 'right', 0, 50),
				(btn_ref, 'right', 0, 75),
				(btn_cls, 'left', 0, 75)
			)
		)

		cmds.showWindow(self.winMain)
		return form

	def onDeleteMainWin(self):
		"""Missing DocString
		"""
		# print (">> meCheckTexturePaths: onDeleteMainWin() " )
		#cmds.delete( self.listTextures)
		self.deleteReplaceUI(True)
		self.deletePrefixUI(True)

	def deleteReplaceUI(self, param):
		"""Missing DocString
		"""
		winReplace = meCheckTexturePathsReplaceWnd
		if cmds.window(winReplace, exists=True):
			cmds.deleteUI(winReplace, window=True)

		if cmds.windowPref(winReplace, exists=True):
			cmds.windowPref(winReplace, remove=True)

	def deletePrefixUI(self, param):
		"""Missing DocString
		"""
		winPrefix = meCheckTexturePathsSetPrefixWnd
		if cmds.window(winPrefix, exists=True):
			cmds.deleteUI(winPrefix, window=True)

		if cmds.windowPref(winPrefix, exists=True):
			cmds.windowPref(winPrefix, remove=True)

	def deleteUI(self, param):
		"""Missing DocString
		"""
		winMain = meCheckTexturePathsMainWnd
		if cmds.window(winMain, exists=True):
			cmds.deleteUI(winMain, window=True)

		if cmds.windowPref(winMain, exists=True):
			cmds.windowPref(winMain, remove=True)

	def getTextureName(self, fileNodeName):
		"""Missing DocString
		"""
		fileTextureName = None
		attrName = None

		fileNodeType = cmds.objectType(fileNodeName)
		if fileNodeType == 'file' \
				or fileNodeType == 'mentalrayTexture' \
				or fileNodeType == 'psdFileTex':
			attrName = "fileTextureName"
		elif fileNodeType == 'mentalrayIblShape':
			attrName = "texture"
		elif fileNodeType == 'aiImage':
			attrName = "filename"
		elif fileNodeType == 'imagePlane':
			attrName = "imageName"

		if attrName is not None:
			fileTextureName = cmds.getAttr(fileNodeName + "." + attrName)
		return fileTextureName, attrName

	def doFindReplace(self, param):
		"""Missing DocString
		"""
		if self.strToFind != '':
			print(">> self.strToFind = %s self.strToReplace = %s" %
				  (self.strToFind, self.strToReplace))

			for i in range(len(self.fileTextures)):
				fileNodeName = self.fileTextures[i]
				fileTextureName, attrName = self.getTextureName(fileNodeName)
				if fileTextureName is not None:
					fileName = str(fileTextureName)
					newName = maya_ui_proc.fromNativePath(
						fileName.replace(
							str(self.strToFind),
							str(self.strToReplace),
							1
						)
					)
					if newName != fileName:
						cmds.setAttr(
							fileNodeName + "." + attrName,
							newName,
							type="string"
						)
						print(">> fileName = %s new = %s" %
							  (fileName, newName))
			self.refreshUI(True)

	def doSetPrefix(self, param):
		"""Missing DocString
		"""
		print (">> self.strPrefix = %s" % self.strPrefix)
		for i in range(len(self.fileTextures)):
			fileNodeName = self.fileTextures[i]
			fileTextureName, attrName = self.getTextureName(fileNodeName)
			if fileTextureName is not None:
				fileName = os.path.basename(fileTextureName)
				if fileName != '':
					newName = \
						maya_ui_proc.fromNativePath(
							os.path.join(self.strPrefix, fileName)
						)
					cmds.setAttr(
						fileNodeName + "." + attrName,
						newName,
						type="string"
					)
					print(">> new fileName = %s" % newName)
		self.refreshUI(True)

	def onReplace(self, param):
		"""Missing DocString
		"""
		cw1 = 100
		cw2 = 60

		if cmds.window(meCheckTexturePathsReplaceWnd, exists=True):
			cmds.deleteUI(meCheckTexturePathsReplaceWnd, window=True)

		self.winReplace = cmds.window(
			meCheckTexturePathsReplaceWnd,
			title='Find and Replace',
			menuBar=False,
			retain=False,
			widthHeight=(400, 100)
		)

		form = cmds.formLayout('f0', numberOfDivisions=100)
		cmds.formLayout(form, e=True, width=500)
		col = cmds.columnLayout(
			'c0',
			columnAttach=('left', 0),
			rowSpacing=2,
			adjustableColumn=True,
			height=40
		)

		cmds.textFieldGrp(
			'find_str',
			cw=(1, cw1),
			adj=2,
			label='Find string : ',
			text=self.strToFind,
			cc=self.onStrToFindChanged
		)

		cmds.textFieldGrp(
			'replace_str',
			cw=(1, cw1),
			adj=2,
			label='Replace with : ',
			text=self.strToReplace,
			cc=self.onStrToReplaceChanged
		)

		cmds.text(label='Press ENTER after finish editing')
		cmds.setParent('..')

		# cmds.text( label='' )
		btn_rep = cmds.button(label='Replace ...', command=self.doFindReplace)
		btn_cls = cmds.button(label='Close', command=self.deleteReplaceUI)

		cmds.formLayout(
			form,
			edit=True,
			attachForm=[
				(col, 'top', 10),
				(col, 'left', 0),
				(col, 'right', 0),
				(btn_rep, 'left', 0),
				(btn_cls, 'right', 0),
				(btn_cls, 'bottom', 0),
				(btn_rep, 'bottom', 0)
			],
			attachControl=[
				(col, 'bottom', 0, btn_rep),
				(btn_cls, 'left', 0, btn_rep)
			],
			attachPosition=[
				(btn_rep, 'right', 0, 50),
				(btn_cls, 'left', 0, 50)
			]
		)

		cmds.showWindow(self.winReplace)

	def onSetPrefix(self, param):
		"""Missing DocString
		"""
		cw1 = 100
		cw2 = 60

		if cmds.window(meCheckTexturePathsSetPrefixWnd, exists=True):
			cmds.deleteUI(meCheckTexturePathsSetPrefixWnd, window=True)

		self.winSetPrefix = cmds.window(
			meCheckTexturePathsSetPrefixWnd,
			title='Set Prefix',
			menuBar=False,
			retain=False,
			widthHeight=(400, 100)
		)

		form = cmds.formLayout('f0', numberOfDivisions=100)
		cmds.formLayout(form, e=True, width=500)
		col = cmds.columnLayout('c0', columnAttach=( 'left', 0 ), rowSpacing=2,
								adjustableColumn=True, height=40)
		cmds.textFieldGrp('prefix_str', cw=(  1, cw1 ), adj=2,
						  label='Prefix string : ', text=self.strPrefix,
						  cc=self.onStrPrefixChanged)
		cmds.text(label='Press ENTER after finish editing')
		cmds.setParent('..')
		# cmds.text( label='' )
		btn_set = cmds.button(label='Set prefix ...', command=self.doSetPrefix)
		btn_cls = cmds.button(label='Close', command=self.deletePrefixUI)

		cmds.formLayout(
			form,
			edit=True,
			attachForm=[
				(col, 'top', 10),
				(col, 'left', 0),
				(col, 'right', 0),
				(btn_set, 'left', 0),
				(btn_cls, 'right', 0),
				(btn_cls, 'bottom', 0),
				(btn_set, 'bottom', 0)
			],
			attachControl=[
				(col, 'bottom', 0, btn_set),
				(btn_cls, 'left', 0, btn_set)
			],
			attachPosition=[
				(btn_set, 'right', 0, 50),
				(btn_cls, 'left', 0, 50)
			]
		)

		cmds.showWindow(self.winSetPrefix)


#print 'meCheckTexturePaths sourced ...'
