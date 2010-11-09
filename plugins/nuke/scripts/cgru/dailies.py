import nuke

import os, sys, time


def dailiesEvaluate( node):

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
   if movfolder is None or movfolder == '':
      inputnode = None
      for i in range( node.inputs()):
         inputnode = node.input(i)
         if inputnode is None: continue
      if inputnode is not None and inputnode.Class() == 'Write':
         images = inputnode.knob('file').value()
         if images is not None and images != '':
            movfolder = os.path.abspath( images)
            movfolder = os.path.dirname( movfolder)
            movfolder = os.path.dirname( movfolder)
            node.knob('movfolder').setValue( movfolder)

   if node.knob('movauto').value() or node.knob('movname').value is None or node.knob('movname').value == '':
      movname = ''
      # Shot:
      shot = node.knob('shot').value()
      if shot != None and shot != '': movname = shot
      # Acivity:
      activity = node.knob('activity').value()
      if activity != None and activity != '': movname += '_' + activity
      # Suffix:
      suffix = node.knob('suffix').value()
      if suffix != None and suffix != '': movname += '_' + suffix
      # Version:
      version = int(node.knob('version').value())
      if version != None and version != '': movname += '_v%02d' % version
      # Date:
      movname += time.strftime('_%y%m%d')

      node.knob('movname').setValue( movname)


def dailiesGenCmd( node):
   # Process Input:
   inputnode = None
   for i in range( node.inputs()):
      inputnode = node.input(i)
      if inputnode is None: continue
   if inputnode is None or inputnode.Class() != 'Write':
      nuke.message('Error:\n%s\nNot connected to "Write" node.' % node.name())
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
   codec    = node.knob('codec'   ).value().lower()
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
   version  = int(node.knob('version').value())

   # Command Construction:
   cmd = os.environ['CGRU_LOCATION']
   cmd = os.path.join( cmd, 'utilities')
   cmd = os.path.join( cmd, 'moviemaker')
   cmd = os.path.join( cmd, 'mavishky.py')
   cmd = 'python ' + cmd

   cmd += ' -r "%s"' % format
   cmd += ' -f "%s"' % fps
   cmd += ' -c "%s"' % codec

   cmd += ' -t "%s"' % 'dailies'
   cmd += ' -s "%s"' % 'dailies_slate'

   if company  is not None and company  != '': cmd += ' --company "%s"'  % company
   if project  is not None and project  != '': cmd += ' --project "%s"'  % project
   if shot     is not None and shot     != '': cmd += ' --shot "%s"'     % shot
   if version  is not None and version  != '': cmd += ' --ver "v%02d"'   % version
   if artist   is not None and artist   != '': cmd += ' --artist "%s"'   % artist
   if activity is not None and activity != '': cmd += ' --activity "%s"' % activity
   if comments is not None and comments != '': cmd += ' --comments "%s"' % comments
   if draw169  is not None and draw169  != '': cmd += ' --draw169 "%s"'  % draw169
   if draw235  is not None and draw235  != '': cmd += ' --draw235 "%s"'  % draw235
   if line169  is not None and line169  != '': cmd += ' --line169 "%s"'  % line169
   if line235  is not None and line235  != '': cmd += ' --line235 "%s"'  % line235

   # Stereo:
   if images.find('%v') != -1 or images.find('%V') != -1:
      views = inputnode.knob('views').value().split(' ')
      if len(views) > 1:
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
      cmd = 'start launchcmd.cmd ' + cmd
   else:
      cmd = 'launchcmd.py ' + cmd + ' &'
   print "\n%s\n" % cmd
   os.system( cmd)
