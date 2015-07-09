# -*- coding: utf-8 -*-
import os
import sys
import traceback

import cgruutils

str_warning = '[ PARSER WARNING ]'
str_error = '[ PARSER ERROR ]'
str_badresult = '[ PARSER BAD RESULT ]'
str_finishedsuccess = '[ PARSER FINISHED SUCCESS ]'


class parser(
	object):  # TODO: Class names should follow CamelCase naming convention
	"""This is base class, not to be instanced
	"""

	def __init__(self):
		self.percent = 0
		self.frame = 0
		self.percentframe = 0
		self.activity = ''
		self.error = False
		self.warning = False
		self.badresult = False
		self.finishedsuccess = False

		self.files = []
		self.files_onthefly = []

	def setTaskInfo(self, taskInfo):
		"""Missing DocString

		:param taskInfo:
		:return:
		"""
		self.taskInfo = taskInfo
		self.numframes = taskInfo['frames_num']
		if self.numframes < 1:
			self.numframes = 1

	def appendFile(self, i_file, i_onthefly = False):
		"""Missing DocString

		:param i_file:
		:return:
		"""
		i_file = os.path.join(self.taskInfo['wdir'], i_file)
		if not i_file in self.files:
			if os.path.isfile(i_file):
				if i_onthefly:
					self.files_onthefly.append(i_file)
				else:
					self.files.append(i_file)
			# print('PARSED FILE APPENDED:\n' + i_file)

	def getFiles(self):
		"""Missing DocString

		:return:
		"""
		return self.files

	def getFilesOnTheFly(self):
		"""Missing DocString

		:return:
		"""
		files = self.files_onthefly
		self.files_onthefly = []
		return files

	def do(self, data, mode):
		"""Missing DocString

		:param data:
		:param mode:
		:return:
		"""
		print('Error: parser.do: Invalid call, this method must be '
			  'implemented.')

	def parse(self, data, mode):
		"""Missing DocString

		:param data:
		:param mode:
		:return:
		"""

		data = cgruutils.toStr(data)
		mode = cgruutils.toStr(mode)

		self.activity = ''
		self.warning = False
		self.error = False
		self.badresult = False
		result = None
		if data.find(str_warning) != -1:
			self.warning = True
		if data.find(str_error) != -1:
			self.error = True
		if data.find(str_badresult) != -1:
			self.badresult = True
		if data.find(str_finishedsuccess) != -1:
			self.finishedsuccess = True

		lines = data.split('\n')
		for line in lines:
			if line.find('@IMAGE@') != -1: # Will be used in CGRU render scripts
				line = line[7:]
				self.appendFile(line.strip(), False)
			if line.find('Image: ') == 0: # ImageMagick
				line = line[7:]
				self.appendFile(line.strip(), False)
			if line.find('@IMAGE!@') != -1: # Will be used in CGRU render scripts to generate thumb while task is still running
				line = line[8:]
				self.appendFile(line.strip(), True)

		try:
			result = self.do(data, mode)
		except:  # TODO: too broad exception clause
			print('Error parsing output:')
			#print(str(sys.exc_info()[1]))
			traceback.print_exc(file=sys.stdout)
		return result, self.percent, self.frame, self.percentframe, \
			   self.warning, self.error, self.badresult, self.finishedsuccess, \
			   self.activity

	def calculate(self):
		"""Missing DocString
		"""
		if self.frame < 0:
			self.frame = 0
		if self.frame > self.numframes:
			self.frame = self.numframes
		if self.percentframe < 0:
			self.percentframe = 0
		if self.percentframe > 100:
			self.percentframe = 100

		if self.numframes > 1:
			self.percent = \
				int((100.0 * self.frame + self.percentframe) / self.numframes)
		else:
			self.percent = self.percentframe

		if self.percent < 0:
			self.percent = 0
		if self.percent > 100:
			self.percent = 100
