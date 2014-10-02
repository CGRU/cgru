# -*- coding: utf-8 -*-

import cgruconfig
import cgruutils

# PathSeparators = ' ";=,\''
PathSeparators = ' ";=,\':'

# if 'unix' in cgruconfig.VARS['platform']:
#    PathSeparators += os.pathsep


def findPathEnd(path):
	"""Missing DocString

	:param str path:
	:return:
	"""
	position = 0
	pathlen = len(path)
	if pathlen <= 1:
		return 1
	while position < pathlen:
		position += 1
		if position >= pathlen:
			break
		if path[position] in PathSeparators:
			break
	return position


def findNextPosition(position, path):
	"""Missing DocString

	:param position:
	:param path:
	:return:
	"""
	pathlen = len(path)
	if position >= pathlen:
		return -1

	if path[position] != ' ':
		position += findPathEnd(path[position:])

	while position < pathlen:
		if path[position] in PathSeparators:
			position += 1
		else:
			break

	if position >= pathlen:
		return -1

	return position


def findSeparator(path):
	"""Missing DocString

	:param path:
	:return:
	"""
	sep = ''
	seppos1 = path.find('\\')
	seppos2 = path.find('/')

	if seppos1 != -1:
		if seppos2 != -1:
			if seppos1 < seppos2:
				sep = path[seppos1]
			if seppos2 < seppos1:
				sep = path[seppos2]
		else:
			sep = path[seppos1]
	elif seppos2 != -1:
		sep = path[seppos2]
	#    print('Separator for "%s" = "%s"' % ( path, sep))
	return sep


def replaceSeparators(path, path_from, path_to):
	"""Missing DocString

	:param path:
	:param path_from:
	:param path_to:
	:return:
	"""
	new_path = path

	sep_from = findSeparator(path_from)
	sep_to = findSeparator(path_to)

	if sep_from == sep_to:
		return new_path

	if sep_from == '' or sep_to == '':
		return new_path

	path_end = findPathEnd(new_path)

	#    print('Path end for "%s" = %d' % (new_path, path_end))

	if 1 < path_end <= len(new_path):
		part1 = new_path[:path_end]
		part2 = new_path[path_end:]
		part1 = part1.replace(sep_from, sep_to)
		new_path = part1 + part2

	return new_path


class PathMap:
	"""Missing DocString

	:param UnixSeparators:
	:param Verbose:
	"""

	def __init__(self, UnixSeparators=False, Verbose=False):
		self.initialized = False
		self.UnixSeparators = UnixSeparators
		self.PathClient = []
		self.PathServer = []

		if 'pathsmap' in cgruconfig.VARS:
			self.init(cgruconfig.VARS['pathsmap'], Verbose)

	def init(self, pathsmap, Verbose=False):
		"""Missing DocString

		:param pathsmap:
		:param Verbose:
		:return:
		"""
		self.initialized = False
		self.PathClient = []
		self.PathServer = []

		for pair in pathsmap:
			if len(pair) != 2:
				print('ERROR: Pathmap is not a pair.')
				return
			path_client = (cgruutils.toStr(pair[0])).replace('\\', '/')
			path_server = (cgruutils.toStr(pair[1])).replace('\\', '/')
			self.PathClient.append(path_client)
			self.PathServer.append(path_server)
			self.initialized = True

		if Verbose:
			print('Path map:')
			n = 0
			for path in self.PathClient:
				print('    "%s" <-> "%s"' % (path, self.PathServer[n]))
				n += 1
			if self.UnixSeparators:
				print('Using UNIX separators.')

	def translatePath(self, path, toserver, Verbose):
		"""Missing DocString

		:param path:
		:param toserver:
		:param Verbose:
		:return:
		"""
		newpath = cgruutils.toStr(path)
		if len(newpath) < 1:
			return newpath

		if not self.initialized:
			return newpath

		position = 0

		while newpath[position] in PathSeparators:
			position += 1
			if position >= len(newpath):
				return newpath

		maxcycles = len(newpath)
		cycle = 0
		while position != -1:
			path_search = newpath[position:]
			# print('position # %d/%d : "%s"' % (position, len(newpath), path_search))
			for i in range(0, len(self.PathServer)):
				if toserver:
					path_from = self.PathClient[i]
					path_to = self.PathServer[i]
				else:
					path_from = self.PathServer[i]
					path_to = self.PathClient[i]

				path_found = False

				if 'windows' in cgruconfig.VARS['platform'] and toserver:
					path_search = path_search.lower()
					path_from = path_from.replace('/', '\\').lower()

				if path_search.find(path_from) == 0:
					path_found = True
				else:
					path_from = path_from.replace('\\', '/')
					if path_search.find(path_from) == 0:
						path_found = True

				if path_found:
					part1 = newpath[:position]
					part2 = newpath[position + len(path_from):]
					if not self.UnixSeparators:
						if 'windows' in cgruconfig.VARS['platform'] \
								and not toserver:
							path_to = path_to.replace('/', '\\')
					part2 = replaceSeparators(part2, path_from, path_to)
					position = len(part1 + path_to)
					newpath = part1 + path_to + part2
					if Verbose:
						print('Paths mapped:')
						print(path)
						print(newpath)
					break

			old_position = position
			position = findNextPosition(position, newpath)
			if position != -1 and position <= old_position:
				print('Path translation error: Eldless cycle, '
					  'position = %d.' % position)
				break

			cycle += 1

			if cycle > maxcycles:
				print(
					'Path translation error: Cycle > maxcycles (%d>%d).' %
					(cycle, maxcycles)
				)
				break

		return newpath

	def toServer(self, path, Verbose=False):
		"""Missing DocString

		:param path:
		:param Verbose:
		:return:
		"""
		return self.translatePath(path, True, Verbose)

	def toClient(self, path, Verbose=False):
		"""Missing DocString

		:param path:
		:param Verbose:
		:return:
		"""
		return self.translatePath(path, False, Verbose)

	def translateFile(self, infile, outfile, toserver, SearchStrings, Verbose):
		"""Missing DocString

		:param infile:
		:param outfile:
		:param toserver:
		:param SearchStrings:
		:param Verbose:
		:return:
		"""
		if not self.initialized:
			return True

		if Verbose:
			print('TranslateFile:')
			print('Input file: "%s"' % infile)
			print('Output file: "%s"' % outfile)

		filein = open(infile, 'r')
		inlines = filein.readlines()
		filein.close()
		outdata = []

		for line in inlines:
			if not isinstance(line, str):
				line = cgruutils.toStr(line)
			to_skip = False
			if len(SearchStrings):
				to_skip = True

			for search_str in SearchStrings:
				# if line[0:len(search_str)] == search_str:
				if line.find(search_str) != -1:
					# print('"%s" == "%s"' % (line[0:len(search_str)], search_str))
					to_skip = False
					break
			if to_skip:
				outdata.append(line)
			else:
				# print('Translating: "%s"' % line)
				outdata.append(self.translatePath(line, toserver, Verbose))
				# print( outdata[-1])
		fileout = open(outfile, 'w')
		fileout.writelines(outdata)
		fileout.close()
		return True

	def toServerFile(self, infile, outfile, SearchStrings=None, Verbose=False):
		"""Missing DocString

		:param infile:
		:param outfile:
		:param SearchStrings:
		:param Verbose:
		:return:
		"""
		if SearchStrings is None:
			SearchStrings = []

		return self.translateFile(infile, outfile, True, SearchStrings,
								  Verbose)

	def toClientFile(self, infile, outfile, SearchStrings=None, Verbose=False):
		"""Missing DocString

		:param infile:
		:param outfile:
		:param SearchStrings:
		:param Verbose:
		:return:
		"""
		if SearchStrings is None:
			SearchStrings = []

		return self.translateFile(
			infile, outfile, False, SearchStrings, Verbose
		)
