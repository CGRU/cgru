#!/usr/bin/env python

import getenv

def relativePath( verbose, srcdir, dirpath):
   path_end=dirpath[len(srcdir):len(dirpath)]
   relative_path=''
   if len(path_end) > 1: 
      relative_count=path_end.count('/')
      for c in range(relative_count):
         relative_path += '../'
   if verbose >= 3: print 'srcdir=%(srcdir)s, dirpath=%(dirpath)s, path_end=%(path_end)s: %(relative_path)s' % vars()
   return relative_path

# return stylesheet line if it is after the title
def checkStyleSheet( lines):
   title_finished = False
   stylesheet=''
   for line in lines:
      if line.find('</title>') > -1:
         title_finished = True
      if line.find('stylesheet') > -1:
         if title_finished:
            stylesheet = line
   return stylesheet

def replaceHeaderFooter( verbose, lines, doctype, header, footer, relative_path, stylesheet):
   lines_dest = []
   for line in doctype: lines_dest.append( line)
   removing_header = False
   html_begins = False
   for line in lines:
      if html_begins == False:
         if line.find('<html') > -1:
            html_begins = True
            continue
         if line.find('<head') > -1:
            html_begins = True
         if html_begins == False: continue

      if line.find('charset=') > -1: continue

      if removing_header:
         if line.find(getenv.HEADER_MARKER) > -1:
            removing_header = False
            line = getenv.HEADER_MARKER + '\n'
         else: continue
      if line.find('</title>') > -1:
         removing_header = True
         lines_dest.append( line)
         if stylesheet !='':
            lines_dest.append( stylesheet)
         for line in header:
            line = line.replace('href="..','href="%s' % relative_path)
            line = line.replace('src="..','src="%s' % relative_path)
            lines_dest.append( line)
         continue

      if( line.find(getenv.FOOTER_MARKER) > -1):
         lines_dest.append( line)
         for line in footer:
            line = line.replace('href="..','href="%s' % relative_path)
            line = line.replace('src="..','src="%s' % relative_path)
            lines_dest.append( line)
         break

      lines_dest.append( line)

   return lines_dest
