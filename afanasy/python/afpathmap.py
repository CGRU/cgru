# -*- coding: utf-8 -*-

import os
import sys

import afenv

import cgruutils

#PathSeparators = ' ";=,\''
PathSeparators = ' ";=,\':'
#if sys.platform.find('win') != 0: PathSeparators += ':'

def findPathEnd( path):
	position = 0
	pathlen = len(path)
	if pathlen <= 1: return 1
	while position < pathlen:
		position += 1
		if position >= pathlen: break
		if path[position] in PathSeparators: break
	return position

def findNextPosition( position, path):
	pathlen = len(path)
	if position >= pathlen: return -1
	if path[position] != ' ': position += findPathEnd(path[position:])
	while position < pathlen:
		if path[position] in PathSeparators: position += 1
		else: break
	if position >= pathlen: return -1
	return position

def findSeparator( path):
	sep = ''
	seppos1 = path.find('\\')
	seppos2 = path.find('/')
	if seppos1 != -1:
		if seppos2 != -1:
			if seppos1 < seppos2: sep = path[seppos1]
			if seppos2 < seppos1: sep = path[seppos2]
		else: sep = path[seppos1]
	elif seppos2 != -1: sep = path[seppos2]
#    print('Separator for "%s" = "%s"' % ( path, sep))
	return sep

def replaceSeperators( path, path_from, path_to):
	newpath = path
	sep_from = findSeparator( path_from)
	sep_to    = findSeparator( path_to)
	if sep_from == sep_to: return newpath
	if sep_from == '' or sep_to == '': return newpath
	pathend = findPathEnd( newpath)
#    print('Path end for "%s" = %d' % (newpath, pathend))
	if pathend > 1 and pathend <= len(newpath):
		part1 = newpath[:pathend]
		part2 = newpath[pathend:]
		part1 = part1.replace( sep_from, sep_to)
		newpath = part1 + part2
	return newpath

def findPathMapFiles( folder):
	PathMapFile = 'pathmap'
	pathmap_files = []
	if folder is not None:
		folder = cgruutils.toStr( folder)
		if os.path.isdir( folder):
			files = os.listdir( folder)
			for afile in files:
				afile = cgruutils.toStr( afile)
				if afile.find( PathMapFile) != 0: continue
				filename = os.path.join( folder, afile)
				if not os.path.isfile( filename): continue
				pathmap_files.append( filename)
	return pathmap_files

class PathMap:
	def __init__( self, UnixSeparators = False, Verbose = False):
		self.initialized = False
		PathMapFile = 'pathmap'
		self.UnixSeparators = UnixSeparators
		self.PathClient = []
		self.PathServer = []
		pathmap_files = []

		pathmap_files.extend( findPathMapFiles( afenv.VARS['HOME_AFANASY']))
		pathmap_files.extend( findPathMapFiles( '.'))
		pathmap_files.extend( findPathMapFiles( afenv.VARS['AF_ROOT']))

		for filename in pathmap_files:
			if Verbose:
				print('Opening file: "%s"' % filename)

			file = open( filename, 'r')
			for line in file:
				if not isinstance( line, str): line = cgruutils.toStr( line)
				line = line.strip()
				linelen = len(line)
				if linelen <= 3: continue
				if line[0] == '#': continue
				pos = line.rfind(' ')
				if pos <= 2 or pos >= linelen:
					print('Error: Ivalid line in "%s":' % filename)
					print(line)
					continue
				path_client = line[:pos].strip()
				path_server = line[pos:].strip()
				if sys.platform.find('win') == 0:
					path_client = path_client.lower()
				if self.UnixSeparators:
					path_client = path_client.replace('\\','/')
					path_server = path_server.replace('\\','/')
				self.PathClient.append( path_client)
				self.PathServer.append( path_server)
				self.initialized = True

			file.close()

		if Verbose:
			print('Pathes map:')
			n = 0
			for path in self.PathClient:
				print('    "%s" <-> "%s"' % (path, self.PathServer[n]))
				n += 1
			if self.UnixSeparators: print('Using UNIX separators.')


	def translatePath( self, path, toserver, Verbose):
		newpath = cgruutils.toStr( path)
		if len(newpath) < 1: return newpath
		if not self.initialized: return newpath
		position = 0
		while newpath[position] in PathSeparators:
			position += 1
			if position >= len(newpath): return newpath
		maxcycles = len(newpath)
		cycle = 0
		while position != -1:
			path_search = newpath[position:]
#            print('position # %d/%d : "%s"' % (position, len(newpath), path_search))
			for i in range( 0, len( self.PathServer)):
				if toserver:
					path_from = self.PathClient[i]
					path_to   = self.PathServer[i]
				else:
					path_from = self.PathServer[i]
					path_to   = self.PathClient[i]
				pathfounded = False
				if sys.platform.find('win') == 0 and toserver:
					path_search = path_search.lower()
					path_search = path_search.replace('/','\\')
					if path_search.find(path_from) == 0:
						pathfounded = True
					else:
						path_search = path_search.replace('\\','/')
						if path_search.find(path_from) == 0:
							pathfounded = True
				else:
#                    print('finding "%s" in "%s"' % (path_from,path_search))
					if path_search.find(path_from) == 0:
						pathfounded = True
				if pathfounded:
					part1 = newpath[:position]
					part2 = newpath[position+len(path_from):]
					if not self.UnixSeparators:
						part2 = replaceSeperators( part2, path_from, path_to)
					newpath = part1 + path_to + part2
					position = len(part1 + path_to)
					newpath = part1 + path_to + part2
					if Verbose:
						print('Pathes mapped:')
						print(path)
						print(newpath)
					break
			old_position = position
			position = findNextPosition( position, newpath)
			if position != -1 and position <= old_position:
				print('Path translation error: Eldless cycle, position = %d.' % position)
				break
			cycle += 1
			if cycle > maxcycles:
				print('Path translation error: Cycle > maxcycles (%d>%d).' % (cycle, maxcycles))
				break
		return newpath

	def toServer( self, path, Verbose = False): return self.translatePath( path, True , Verbose)
	def toClient( self, path, Verbose = False): return self.translatePath( path, False, Verbose)

	def translateFile( self, infile, outfile, toserver, SearchStrings, Verbose):
		if not self.initialized: return True
		if Verbose:
			print('TranslateFile:')
			print('Input file: "%s"' % infile)
			print('Output file: "%s"' % outfile)
		filein = open( infile, 'r')
		inlines = filein.readlines()
		filein.close()
		outdata = ''
		for line in inlines:
			if not isinstance( line, str): line = cgruutils.toStr( line)
			toskip = False
			if len( SearchStrings): toskip = True
			for searchstr in SearchStrings:
				if line.find( searchstr) != -1:
					toskip = False
					continue
			if toskip:
				lineout = line
			else:
				lineout = self.translatePath( line, toserver , Verbose)
			outdata += lineout + '\n'
		fileout = open( outfile, 'w')
		fileout.write(outdata)
		fileout.close()
		return True

	def toServerFile( self, infile, outfile, SearchStrings = [], Verbose = False): return self.translateFile( infile, outfile, True,  SearchStrings, Verbose)
	def toClientFile( self, infile, outfile, SearchStrings = [], Verbose = False): return self.translateFile( infile, outfile, False, SearchStrings, Verbose)
