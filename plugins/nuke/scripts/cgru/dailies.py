import nuke

import os, sys, time


def dailiesEvaluate( node):

   newNode = False
   
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
   codec = node.knob('codec').value()
   if codec == None or codec == '' or not os.path.isfile( codec):
      newNode = True
      codec = os.getenv('CGRU_DAILIES_CODEC', os.environ['CGRU_LOCATION'] + '/utilities/moviemaker/codecs/photojpg_best.ffmpeg')
      codec.replace('\\','/')
      node.knob('codec').setValue( codec)

   # Template:
   template = node.knob('template').value()
   if template == None or template == '' or not os.path.isfile( template):
      template = os.getenv('CGRU_DAILIES_TEMPLATE', os.environ['CGRU_LOCATION'] + '/utilities/moviemaker/templates/dailies')
      template.replace('\\','/')
      node.knob('template').setValue( template)

   # Slate:
   slate = node.knob('slate').value()
   if slate == None or slate == '' or not os.path.isfile( slate):
      slate = os.getenv('CGRU_DAILIES_SLATE', os.environ['CGRU_LOCATION'] + '/utilities/moviemaker/templates/dailies_slate')
      slate.replace('\\','/')
      node.knob('slate').setValue( slate)

   # Logo:
   logosize = node.knob('logosize').value()
   if logosize < 1:
      logosize = os.getenv('CGRU_DAILIES_LOGOSIZE', '20')
      node.knob('logosize').setValue( float(logosize))

   if newNode:

      logopath = node.knob('logopath').value()
      if logopath == None or logopath == '' or not os.path.isfile( logopath):
         logopath = os.getenv('CGRU_DAILIES_LOGOPATH', os.environ['CGRU_LOCATION'] + '/utilities/moviemaker/logos/logo.png')
         logopath.replace('\\','/')
         node.knob('logopath').setValue( logopath)

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
   movfolder = node.knob('movfolder').value()
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

   # Movie Name:
   if node.knob('movauto').value() or node.knob('movname').value is None or node.knob('movname').value == '':

      movname = movrule

      # Project:
      project = node.knob('project').value()
      if project is None: project = ''
      # Version:
      version = 'v%03d' % int(node.knob('version').value())
      # Date:
      date = time.strftime('%y%m%d')
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
   # Process Input:
   inputnode = None
   for i in range( node.inputs()):
      inputnode = node.input(i)
   if inputnode is None:
      nuke.message('Error:\n%s\nNot connected to Read or Write node.' % node.name())
      return
   if not inputnode.Class() in ['Read','Write']:
      nuke.message('Error:\n%s\nConnected not to Read or Write node.' % node.name())
      return
   images = inputnode.knob('file').value()
   if images is None or images == '':
      nuke.message('Error:\n%s\nFiles are empty.' % inputnode.name())
      return
   images = os.path.abspath(images)

   # Get Movie Name:
   movname  = node.knob('movname' ).value()
   if movname is None or movname == '':
      nuke.message('Error:\n%s\nMovie name is not set.' % node.name())
      return

   # Get Movie Folder:
   movfolder = node.knob('movfolder').value()
   if movfolder is None or movfolder == '':
      nuke.message('Error:\n%s\nMovie folder is not set.' % node.name())
      return

   # Get Parameters:
   format   = node.knob('format'  ).value()
   fps      = node.knob('fps'     ).value()
   codec    = node.knob('codec'   ).value()
   template = node.knob('template').value()
   slate    = node.knob('slate'   ).value()
   company  = node.knob('company' ).value()
   project  = node.knob('project' ).value()
   shot     = node.knob('shot'    ).value()
   artist   = node.knob('artist'  ).value()
   activity = node.knob('activity').value()
   comments = node.knob('comments').value()
   draw169  = node.knob('draw169' ).value()
   draw235  = node.knob('draw235' ).value()
   line169  = node.knob('line169' ).value()
   line235  = node.knob('line235' ).value()
   logopath = node.knob('logopath').value()
   logosize = int(node.knob('logosize').value())
   version  = int(node.knob('version').value())
   fstart   = int(node.knob('fstart').value())
   fend     = int(node.knob('fend').value())
   fffirst  = int(node.knob('fffirst').value())

   # Command Construction:
   cmd = os.environ['CGRU_LOCATION']
   cmd = os.path.join( cmd, 'utilities')
   cmd = os.path.join( cmd, 'moviemaker')
   cmd = os.path.join( cmd, 'mavishky.py')
   cmd = 'python ' + cmd

   cmd += ' -r "%s"' % format
   cmd += ' -f "%s"' % fps
   cmd += ' -c "%s"' % codec
   cmd += ' -t "%s"' % template
   cmd += ' -s "%s"' % slate

   if fstart != -1: cmd += ' --fs %d ' % fstart
   if fend   != -1: cmd += ' --fe %d ' % fend
   if company  is not None and company  != '': cmd += ' --company "%s"'  % company
   if project  is not None and project  != '': cmd += ' --project "%s"'  % project
   if shot     is not None and shot     != '': cmd += ' --shot "%s"'     % shot
   if version  is not None and version  != '': cmd += ' --ver "v%03d"'   % version
   if artist   is not None and artist   != '': cmd += ' --artist "%s"'   % artist
   if activity is not None and activity != '': cmd += ' --activity "%s"' % activity
   if comments is not None and comments != '': cmd += ' --comments "%s"' % comments
   if draw169  is not None and draw169  != '': cmd += ' --draw169 "%s"'  % draw169
   if draw235  is not None and draw235  != '': cmd += ' --draw235 "%s"'  % draw235
   if line169  is not None and line169  != '': cmd += ' --line169 "%s"'  % line169
   if line235  is not None and line235  != '': cmd += ' --line235 "%s"'  % line235
   if logopath is not None and logopath != '':
      cmd += ' --logopath "%s"' % logopath
      cmd += ' --logosize %d'   % logosize
   if fffirst: cmd += ' -fff'

   # Stereo:
   if images.find('%v') != -1 or images.find('%V') != -1:
      if node.knob('forceviews').value():
         views = node.knob('viewsnames').value().split(' ')
      else:
         views_knob = inputnode.knob('views')
         if views_knob is not None:
            views = inputnode.knob('views').value().split(' ')
         else:
            views = nuke.views()
      if len(views) > 0:
         stereo_images = ''
         for view in views:
            view = view.strip()
            if view != '':
               img = images
               img = img.replace('%V', view)
               img = img.replace('%v', view[0])
               if stereo_images != '': stereo_images += ' '
               stereo_images += img
         images = stereo_images
   else:
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
      cmd = 'launchcmd.py ' + cmd + ' &'
   print "\n%s\n" % cmd
   os.system( cmd)
