import nuke

import os, sys, time

import afcommon

TimeFromat = '%y-%m-%d %H:%M'
DateFormat = '%y%m%d'

def dailiesEvaluate( node):

   newNode = False
   param = node.knob('codec').value()
   if param is None or param == '': newNode = True

   # Format:
   format = node.knob('format').value()
   if format == None or format == '':
      format = os.getenv('CGRU_DAILIES_FORMAT', '720x576')
      node.knob('format').setValue( format)

   # FPS:
   fps = node.knob('fps').value()
   if fps == None or fps == '':
      fps = os.getenv('CGRU_DAILIES_FPS', '24')
      node.knob('fps').setValue( fps)

   # Codec Preset:
   codec = node.knob('codec').getEvaluatedValue()
   if codec == None or codec == '' or not os.path.isfile( codec):
      codec = os.getenv('CGRU_DAILIES_CODEC', os.environ['CGRU_LOCATION'] + '/utilities/moviemaker/codecs/photojpg_best.ffmpeg')
      codec = codec.replace('\\','/')
      node.knob('codec').setValue( codec)

   # Logo slate:
   lgssize = node.knob('lgssize').value()
   if lgssize < 1:
      lgssize = os.getenv('CGRU_DAILIES_LGSSIZE', '20')
      node.knob('lgssize').setValue( float(lgssize))
      lgsgrav = os.getenv('CGRU_DAILIES_LGSGRAV', 'SouthEast')
      node.knob('lgsgrav').setValue( lgsgrav)
   # Logo frame:
   lgfsize = node.knob('lgfsize').value()
   if lgfsize < 1:
      lgfsize = os.getenv('CGRU_DAILIES_LGFSIZE', '10')
      node.knob('lgfsize').setValue( float(lgfsize))
      lgfgrav = os.getenv('CGRU_DAILIES_LGFGRAV', 'North')
      node.knob('lgfgrav').setValue( lgfgrav)
   # Logo slate file:
   lgspath = node.knob('lgspath').getEvaluatedValue()
   if lgspath != None and lgspath != '' and not os.path.isfile( lgspath):
      lgspath = os.getenv('CGRU_DAILIES_LGSPATH', os.environ['CGRU_LOCATION'] + '/utilities/moviemaker/logos/logo.png')
      lgspath = lgspath.replace('\\','/')
      node.knob('lgspath').setValue( lgspath)
   # Logo frame file:
   lgfpath = node.knob('lgfpath').getEvaluatedValue()
   if lgfpath != None and lgfpath != '' and not os.path.isfile( lgfpath):
      lgfpath = os.getenv('CGRU_DAILIES_LGFPATH', os.environ['CGRU_LOCATION'] + '/utilities/moviemaker/logos/logo.png')
      lgfpath = lgfpath.replace('\\','/')
      node.knob('lgfpath').setValue( lgfpath)

   if newNode:
      # Template:
      template = node.knob('template').getEvaluatedValue()
      if template == None or template == '' or not os.path.isfile( template):
         template = os.getenv('CGRU_DAILIES_TEMPLATE', os.environ['CGRU_LOCATION'] + '/utilities/moviemaker/templates/dailies_withlogo')
         template = template.replace('\\','/')
         node.knob('template').setValue( template)

      # Slate:
      slate = node.knob('slate').getEvaluatedValue()
      if slate == None or slate == '' or not os.path.isfile( slate):
         slate = os.getenv('CGRU_DAILIES_SLATE', os.environ['CGRU_LOCATION'] + '/utilities/moviemaker/templates/dailies_slate')
         slate = slate.replace('\\','/')
         node.knob('slate').setValue( slate)

      # Logo slate file:
      lgspath = os.getenv('CGRU_DAILIES_LGSPATH', os.environ['CGRU_LOCATION'] + '/utilities/moviemaker/logos/logo.png')
      lgspath = lgspath.replace('\\','/')
      node.knob('lgspath').setValue( lgspath)
      # Logo frame file:
      lgfpath = os.getenv('CGRU_DAILIES_LGFPATH', os.environ['CGRU_LOCATION'] + '/utilities/moviemaker/logos/logo.png')
      lgfpath = lgfpath.replace('\\','/')
      node.knob('lgfpath').setValue( lgfpath)

      # Cacher:
      draw169 = node.knob('draw169').value()
      if draw169 == None or draw169 == '':
         draw169 = os.getenv('CGRU_DAILIES_DRAW169', '0')
         node.knob('draw169').setValue( draw169)
      draw235 = node.knob('draw235').value()
      if draw235 == None or draw235 == '':
         draw235 = os.getenv('CGRU_DAILIES_DRAW235', '0')
         node.knob('draw235').setValue( draw235)
      line169 = node.knob('line169').value()
      if line169 == None or line169 == '':
         line169 = os.getenv('CGRU_DAILIES_LINE169', '')
         node.knob('line169').setValue( line169)
      line235 = node.knob('line235').value()
      if line235 == None or line235 == '':
         line235 = os.getenv('CGRU_DAILIES_LINE235', '')
         node.knob('line235').setValue( line235)

   # Shot:
   shot = node.knob('shot').value()
   if shot == None or shot == '':
      try:
         shot = os.path.basename( nuke.root().name())
      except:
         return
      dot = shot.find('.')
      if dot > 0: shot = shot[ : dot]
      node.knob('shot').setValue( shot)

   # Artist:
   artist = node.knob('artist').value()
   if artist == None or artist == '':
      node.knob('artist').setValue( os.getenv('USER', os.getenv('USERNAME','')))

   # Dailies Folder:
   movfolder = node.knob('movfolder').getEvaluatedValue()
   if movfolder is None or movfolder == '' or not os.path.isdir( movfolder):
      inputnode = None
      for i in range( node.inputs()):
         inputnode = node.input(i)
         if inputnode is None: continue
      if inputnode is not None:
         if inputnode.Class() == 'Read' or inputnode.Class() == 'Write':
            images = inputnode.knob('file').value()
            if images is not None and images != '':
               movfolder = os.path.abspath( images)
               movfolder = os.path.dirname( movfolder)
               movfolder = os.path.dirname( movfolder)
               movfolder = movfolder.replace('\\','/')
               node.knob('movfolder').setValue( movfolder)

   # Naming Rule:
   movrule = node.knob('movrule').value()
   if movrule == None or movrule == '':
      movrule = os.getenv('CGRU_DAILIES_NAMING', '(s)_(v)_(d)')
      node.knob('movrule').setValue( movrule)

   # Date:
   localtime = time.localtime()
   faketime_str = node.knob('faketime_str').value()
   faketime_on  = int(node.knob('faketime_on').value())
   if not faketime_on or faketime_str is None or faketime_str == '':
      faketime_str = time.strftime( TimeFromat, localtime)
      node.knob('faketime_str').setValue( faketime_str)
   if faketime_on:
      localtime = time.strptime( faketime_str, TimeFromat)
   date = time.strftime( DateFormat, localtime)

   # Movie Name:
   if node.knob('movauto').value() or node.knob('movname').value is None or node.knob('movname').value == '':

      movname = movrule

      # Project:
      project = node.knob('project').value()
      if project is None: project = ''
      # Version:
      version = node.knob('version').value()
      if version is None: version = ''
      # Activity:
      activity = node.knob('activity').value()
      if activity is None: activity = ''
      # Company:
      company = node.knob('company').value()
      if company is None: company = ''

      movname = movname.replace('(p)', project)
      movname = movname.replace('(P)', project.upper())
      movname = movname.replace('(s)', shot)
      movname = movname.replace('(S)', shot.upper())
      movname = movname.replace('(v)', version)
      movname = movname.replace('(V)', version.upper())
      movname = movname.replace('(d)', date)
      movname = movname.replace('(D)', date.upper())
      movname = movname.replace('(a)', activity)
      movname = movname.replace('(A)', activity.upper())
      movname = movname.replace('(c)', company)
      movname = movname.replace('(C)', company.upper())
      movname = movname.replace('(u)', artist)
      movname = movname.replace('(U)', artist.upper())

      node.knob('movname').setValue( movname)


def dailiesGenCmd( node):
   # Process Input Node:
   inputnode = None
   for i in range( node.inputs()):
      inputnode = node.input(i)
   if inputnode is None:
      nuke.message('Error:\n%s\nNot connected to Read or Write node.' % node.name())
      return
   if not inputnode.Class() in ['Read','Write']:
      nuke.message('Error:\n%s\nConnected not to Read or Write node.' % node.name())
      return


   # Process Images:
   images = ''
   root_frame_first = nuke.Root().firstFrame()
   root_frame_last  = nuke.Root().lastFrame()
   if root_frame_first == root_frame_last: root_frame_last += 100
   # Get needed views from dailies node if forced:
   if node.knob('forceviews').value():
      views = node.knob('viewsnames').value().split(' ')
   else:
      # Get needed views write node:
      views_knob = inputnode.knob('views')
      if views_knob is not None:
         views = inputnode.knob('views').value().split(' ')
      else:
         # Get all scene views:
         views = nuke.views()
   # Generate input pattern from each view:
   for view in views:
      if not len(view): continue # skip empty view, may be after split(' ')
      if not view in nuke.views():
         print('Error: Skipping invalid view: "%s"' % view)
         continue
      octx = nuke.OutputContext()
      octx.setView( 1 + nuke.views().index(view))
      octx.setFrame( root_frame_first)
      images1 = inputnode.knob('file').getEvaluatedValue( octx)
      if images1 is None or images1 == '':
         nuke.message('Error:\n%s\nFiles are empty.\nView "%s", frame %d.' % (inputnode.name(), view, root_frame_first))
         return
      octx.setFrame( root_frame_last)
      images2 = inputnode.knob('file').getEvaluatedValue( octx)
      if images2 is None or images2 == '':
         nuke.message('Error:\n%s\nFiles are empty.\nView "%s", frame %d.' % (inputnode.name(), view, root_frame_last))
         return
      part1, padding, part2 = afcommon.splitPathsDifference( images1, images2)
      if padding < 1:
         nuke.message('Error:\n%s\Invalid files pattern.\nView "%s".' % (inputnode.name(), view))
         return
      if len(images): images += ' '
      images += part1 + '#'*padding + part2

   if images == '':
      nuke.message('Error:\n%s\No valid views founded.' % inputnode.name())
      return

   # Get Movie Name:
   movname  = node.knob('movname' ).value()
   if movname is None or movname == '':
      nuke.message('Error:\n%s\nMovie name is not set.' % node.name())
      return

   # Get Movie Folder:
   movfolder = node.knob('movfolder').getEvaluatedValue()
   if movfolder is None or movfolder == '':
      nuke.message('Error:\n%s\nMovie folder is not set.' % node.name())
      return

   # Get Parameters:
   format   = node.knob('format'  ).value()
   fps      = node.knob('fps'     ).value()
   codec    = node.knob('codec'   ).value()
   template = node.knob('template').getEvaluatedValue()
   slate    = node.knob('slate'   ).getEvaluatedValue()
   company  = node.knob('company' ).value()
   project  = node.knob('project' ).value()
   shot     = node.knob('shot'    ).value()
   version  = node.knob('version' ).value()
   artist   = node.knob('artist'  ).value()
   activity = node.knob('activity').value()
   comments = node.knob('comments').value()
   cach_op  = node.knob('cach_op' ).value()
   line_clr = node.knob('line_clr').value()
   draw169  = node.knob('draw169' ).value()
   draw235  = node.knob('draw235' ).value()
   line169  = node.knob('line169' ).value()
   line235  = node.knob('line235' ).value()
   lgspath  = node.knob('lgspath' ).getEvaluatedValue()
   lgfpath  = node.knob('lgfpath' ).getEvaluatedValue()
   lgsgrav  = node.knob('lgsgrav' ).value()
   lgfgrav  = node.knob('lgfgrav' ).value()
   lgssize  = int(node.knob('lgssize').value())
   lgfsize  = int(node.knob('lgfsize').value())
   fstart   = int(node.knob('fstart').value())
   fend     = int(node.knob('fend').value())
   fffirst  = int(node.knob('fffirst').value())
   faketime_on    = int(node.knob('faketime_on').value())
   faketime_str   = node.knob('faketime_str').value()
   encodeonly     = node.knob('encodeonly').value()
   tmpformat      = node.knob('tmpformat').value()
   tmpquality     = node.knob('tmpquality').value()
   autocolorspace = int(node.knob('autocolorspace').value())
   asp_in         = float(node.knob('asp_in' ).value())
   gamma          = float(node.knob('gamma').value())
   cach_as        = float(node.knob('cach_as' ).value())
   line_as        = float(node.knob('line_as' ).value())


   # Command Construction:
   cmd = os.environ['CGRU_LOCATION']
   cmd = os.path.join( cmd, 'utilities')
   cmd = os.path.join( cmd, 'moviemaker')
   cmd = os.path.join( cmd, 'makemovie.py')
   cmd = 'python ' + cmd

   cmd += ' -f "%s"' % fps
   cmd += ' -c "%s"' % codec

   if faketime_on and faketime_str is not None and faketime_str != '':
      cmd += ' --faketime %d' % int(time.mktime( time.strptime( faketime_str, TimeFromat)))
   if tmpformat  is not None and tmpformat  != '': cmd += ' --tmpformat "%s"'   % tmpformat
   if tmpquality is not None and tmpquality != '': cmd += ' --tmpquality "%s"'  % tmpquality
   if not autocolorspace: cmd += ' --noautocorr'
   if gamma  != 1.0: cmd += ' -g %03f' % gamma
   if asp_in  > 0.0: cmd += ' --aspect_in %f' % asp_in

   if fstart !=  -1: cmd += ' --fs %d' % fstart
   if fend   !=  -1: cmd += ' --fe %d' % fend
   if fffirst      : cmd += ' --fff'
   if not encodeonly:
	   cmd += ' -r "%s"' % format
	   if template is not None and template != '': cmd += ' -t "%s"'  % template
	   if slate    is not None and slate    != '': cmd += ' -s "%s"'  % slate
	   if company  is not None and company  != '': cmd += ' --company "%s"'  % company
	   if project  is not None and project  != '': cmd += ' --project "%s"'  % project
	   if shot     is not None and shot     != '': cmd += ' --shot "%s"'     % shot
	   if version  is not None and version  != '': cmd += ' --ver "%s"'      % version
	   if artist   is not None and artist   != '': cmd += ' --artist "%s"'   % artist
	   if activity is not None and activity != '': cmd += ' --activity "%s"' % activity
	   if comments is not None and comments != '': cmd += ' --comments "%s"' % comments
	   if draw169  is not None and draw169  != '': cmd += ' --draw169 "%s"'  % draw169
	   if draw235  is not None and draw235  != '': cmd += ' --draw235 "%s"'  % draw235
	   if line169  is not None and line169  != '': cmd += ' --line169 "%s"'  % line169
	   if line235  is not None and line235  != '': cmd += ' --line235 "%s"'  % line235
	   if line_clr is not None and line_clr != '':
	      cmd += ' --line_aspect %f' % line_as
	      cmd += ' --line_color "%s"' % line_clr
	   if cach_op  is not None and cach_op  != '':
	      cmd += ' --cacher_aspect %f' % cach_as
	      cmd += ' --cacher_opacity "%s"' % cach_op
	   if lgspath  is not None and lgspath  != '':
	      cmd += ' --lgspath "%s"' % lgspath
	      cmd += ' --lgssize %d'   % lgssize
	      cmd += ' --lgsgrav %s'   % lgsgrav
	   if lgfpath  is not None and lgfpath  != '':
	      cmd += ' --lgfpath "%s"' % lgfpath
	      cmd += ' --lgfsize %d'   % lgfsize
	      cmd += ' --lgfgrav %s'   % lgfgrav
   if node.knob('stereodub').value(): cmd += ' --stereo'

   cmd += ' ' + images
   cmd += ' ' + os.path.join( os.path.abspath( movfolder), movname)

   return cmd

def dailies( node):
   cmd = dailiesGenCmd( node)
   if cmd is None or cmd == '': return
   if sys.platform.find('win') == 0:
      cmd = 'start ' + cmd
   else:
      cmd = 'python "%s" ' % (os.environ['CGRU_LOCATION'] + '/utilities/launchcmd.py') + cmd + ' &'
   print('\n%s\n' % cmd)
   os.system( cmd)
