#!/usr/bin/env python

import os
import shutil

import getenv
import html
import walk

from optparse import OptionParser
parser = OptionParser(usage="usage: %prog [options]", version="%prog 1.0")
parser.add_option('-r', '--remote',  dest='remote',  action='store_true', default=False, help='Make remote site version')
parser.add_option('-v', '--verbose', dest='verbose', type='int',          default=1,     help='Verbose level: [0-3]')
(options, args) = parser.parse_args()
remote  = options.remote
verbose = options.verbose

if remote:
   print "Creating remote site version..."
   fileheader = open( 'header_remote', 'r')
   header = fileheader.readlines()
   fileheader.close()
   filefooter = open( 'footer_remote', 'r')
   footer = filefooter.readlines()
   filefooter.close()
else:
   fileheader = open( 'header_local', 'r')
   header = fileheader.readlines()
   fileheader.close()
   filefooter = open( 'footer_local', 'r')
   footer = filefooter.readlines()
   filefooter.close()

filedoctype = open( 'doctype', 'r')
doctype = filedoctype.readlines()
filedoctype.close()

paths = walk.walk( remote)
dircount = 0

for dirpath in paths[0]:

   filesother = paths[1][dircount]
   fileshtml  = paths[2][dircount]
   dircount += 1

   if verbose >= 1: print 'Procesing directory: "%s"' % dirpath

   # Create directories for remote site:
   if remote:
      newdir = os.path.join( getenv.WWWROOT, dirpath[len(getenv.SRCDIR)+1:len(dirpath)])
      if verbose >= 2: print 'New directory: "%s"' % newdir
      os.makedirs( newdir)
      for afile in filesother:
         afile = os.path.join( dirpath, afile)
         if verbose >= 2: print 'Copying file "%s"' % afile
         shutil.copy( afile, newdir)

   # Find relative to top path:
   relative_path=html.relativePath( verbose, getenv.SRCDIR, dirpath, remote)

   for filename in fileshtml:
      filepath = os.path.join( dirpath, filename)
      if verbose >= 2: print 'Procesing: "%s"' % filepath

      # Read entire file:
      fsrc = open( filepath, 'r')
      lines_src = fsrc.readlines()
      fsrc.close()

      # Search for header and footer markers existance:
      skipping = False
      header_marker_founded = False
      footer_marker_founded = False
      for line in lines_src:
         if header_marker_founded == False and line.find(getenv.HEADER_MARKER) > -1:
            header_marker_founded = True
            continue
         if footer_marker_founded == False and line.find(getenv.FOOTER_MARKER) > -1:
            footer_marker_founded = True
            continue
         if header_marker_founded and footer_marker_founded: break
      # Skip file if it has no markers:
      if header_marker_founded == False or footer_marker_founded == False:
         if verbose >= 1: print 'Skipping: "%s"' % filepath
         skipping = True

      if skipping and not remote: continue

      if skipping == False:
         # Check for custom stylesheet, it must be before title, as we replace all from the end of title to header marker
         stylesheet = html.checkStyleSheet( lines_src)
         if stylesheet != '':
            print 'WARNING: stylesheet = %(stylesheet)s %(filepath)s' % vars()
         # Replace header and footer
         lines_src = html.replaceHeaderFooter( verbose, lines_src, doctype, header, footer, relative_path, stylesheet)

      # Creating result lines:
      lines_dest = []
      if remote and not skipping:
         # Remove local lines and uncomment remote:
         removing_local = False
         for line in lines_src:
            if( removing_local ):
               if( line.find(getenv.LOCAL_END) > -1): removing_local = False
               continue
            if( line.find(getenv.LOCAL_BEGIN) > -1):
               removing_local = True
               continue
            if( line.find(getenv.REMOTE_BEGIN) > -1): line = getenv.REMOTE_BEGIN + '-->\n'
            if( line.find(getenv.REMOTE_END) > -1): line = '<!--' + getenv.REMOTE_END + '\n'
            lines_dest.append( line)
      else:
         # Simple copy for local site:
         lines_dest = lines_src

      # Generate new file name for remote site:
      if remote:
         filepath = os.path.join( newdir, filename)
         if verbose >= 3: print 'New file: "%s"' % filepath

      # Writing all lines in file:
      fdest = open( filepath, 'w')
      for line in lines_dest: fdest.write( line)
      fdest.close()
