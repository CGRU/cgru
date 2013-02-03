# -*- coding: utf-8 -*-

import os
import sys

import cgruconfig
import cgruutils

#PathSeparators = ' ";=,\''
PathSeparators = ' ";=,\':'
#if 'unix' in cgruconfig.VARS['platform']: PathSeparators += ':'

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

def replaceSeparators( path, path_from, path_to):
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

class PathMap:
	def __init__( self, UnixSeparators = False, Verbose = False):
		self.initialized = False
		self.UnixSeparators = UnixSeparators
		self.PathClient = []
		self.PathServer = []

		if 'pathsmap' in cgruconfig.VARS:
			self.init( cgruconfig.VARS['pathsmap'], Verbose)

	def init( self, pathsmap, Verbose = False):
		self.initialized = False
		self.PathClient = []
		self.PathServer = []

		for pair in pathsmap:
			if len( pair) != 2:
				print('ERROR: Pathmap is not a pair.')
				return
			path_client = pair[0].replace('\\','/')
			path_server = pair[1].replace('\\','/')
			self.PathClient.append( path_client)
			self.PathServer.append( path_server)
			self.initialized = True

		if Verbose:
			print('Path map:')
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

				if 'windows' in cgruconfig.VARS['platform'] and toserver:
					path_search = path_search.lower()
					path_from = path_from.replace('/','\\').lower()

				if path_search.find( path_from) == 0:
					pathfounded = True
				else:
					path_from = path_from.replace('\\','/')
					if path_search.find( path_from) == 0:
						pathfounded = True

				if pathfounded:
					part1 = newpath[:position]
					part2 = newpath[position+len(path_from):]
					if not self.UnixSeparators:
						if 'windows' in cgruconfig.VARS['platform'] and not toserver:
							path_to = path_to.replace('/','\\')
					part2 = replaceSeparators( part2, path_from, path_to)
					newpath = part1 + path_to + part2
					position = len(part1 + path_to)
					newpath = part1 + path_to + part2
					if Verbose:
						print('Paths mapped:')
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
		outdata = []
		for line in inlines:
			if not isinstance( line, str): line = cgruutils.toStr( line)
			toskip = False
			if len( SearchStrings): toskip = True
			for searchstr in SearchStrings:
#				if line[0:len(searchstr)] == searchstr:
				if line.find( searchstr) != -1:
#					print('"%s" == "%s"' % (line[0:len(searchstr)], searchstr))
					toskip = False
					break
			if toskip:
				outdata.append( line)
			else:
#				print('Translating: "%s"' % line)
				outdata.append( self.translatePath( line, toserver , Verbose))
#				print( outdata[-1])
		fileout = open( outfile, 'w')
		fileout.writelines(outdata)
		fileout.close()
		return True

	def toServerFile( self, infile, outfile, SearchStrings = [], Verbose = False): return self.translateFile( infile, outfile, True,  SearchStrings, Verbose)
	def toClientFile( self, infile, outfile, SearchStrings = [], Verbose = False): return self.translateFile( infile, outfile, False, SearchStrings, Verbose)
