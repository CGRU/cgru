import re, os, shutil, time

import webbrowser

import af

#Get handles to the xsi application
#Application = win32com.client.Dispatch('XSI.Application')

def CloseButton_OnClicked(): PPG.Close()

def HelpButton_OnClicked():
   path = os.getenv('XSI_AF_PATH')
   path = os.path.join( path, 'doc')
   path = os.path.join( path, 'afanasy.html')
   Application.LogMessage('Opening "%s"' % path)
   webbrowser.open(path)
   

def SubmitButton_OnClicked():
   opSet = Application.ActiveSceneRoot.Properties('afSubmitProperties')
   if(opSet == None):
      Application.LogMessage('Error: Can\'t find options.')
      PPG.Close()

   # Save scene:
   Application.SaveScene()
   scene = Application.ActiveProject.ActiveScene
   scenefile = scene.Filename.Value
   if not os.path.isfile( scenefile):
      Application.LogMessage('Error: Can\'t save scene.')
      return

   range_frompass    = opSet.Parameters('afRange_frompass'  ).Value
   range_forcepass   = opSet.Parameters('afRange_forcepass' ).Value
   frame_start       = opSet.Parameters('afFrame_start'     ).Value
   frame_end         = opSet.Parameters('afFrame_end'       ).Value
   frame_by          = opSet.Parameters('afFrame_by'        ).Value
   frame_fpt         = opSet.Parameters('afFrame_fpt'       ).Value
   passesOption      = opSet.Parameters('afRenderPass'      ).Value
   jobname           = opSet.Parameters('afJobName'         ).Value
   priority          = opSet.Parameters('afPriority'        ).Value
   capacity          = opSet.Parameters('afCapacity'        ).Value
   simulate          = opSet.Parameters('afSimulate'        ).Value
   paused            = opSet.Parameters('afStartPaused'     ).Value
   maxhosts          = opSet.Parameters('afMaxHosts'        ).Value
   maxruntime        = opSet.Parameters('afMaxRunTime'      ).Value
   hostsmask         = opSet.Parameters('afHostsMask'       ).Value
   hostsmaskexclude  = opSet.Parameters('afHostsMaskExclude').Value
   dependmask        = opSet.Parameters('afDependMask'      ).Value
   dependmaskglobal  = opSet.Parameters('afDependMaskGlobal').Value
   varirender        = opSet.Parameters('afVariRender'      ).Value
   varirender_attr   = opSet.Parameters('afVariRenderAttr'  ).Value
   varirender_start  = opSet.Parameters('afVariRenderStart' ).Value
   varirender_step   = opSet.Parameters('afVariRenderStep'  ).Value
   varirender_count  = opSet.Parameters('afVariRenderCount' ).Value

   if frame_end < frame_start: frame_end = frame_start
   if frame_by  < 1: frame_by  = 1
   if frame_fpt < 1: frame_fpt = 1

   passes = []
   if passesOption == '_all_':
      for cpass in scene.Passes:
         passes.append( cpass.Name)
   elif passesOption == '_selected_':
      selection = []
      for selected in Application.Selection:
         selection.append( selected.Name)
      for cpass in scene.Passes:
         if cpass.Name in selection:
            passes.append( cpass.Name)
   elif passesOption == '_current_':
      passes.append( scene.ActivePass.Name)
   else:
      passes.append( passesOption)

   padding = Application.GetValue('Passes.RenderOptions.FramePadding')

   for cpass in passes:

      images = []
      # Get framebuffers:
      for ps in scene.Passes:
         if ps.Name != cpass: continue
         for fb in ps.Framebuffers:
            if fb.Enabled.Value:
               format = fb.Format.Value
               filename = fb.ResolvedFilename.Value
               pattern = r'\d+.' + format + '$'
               match = re.search( pattern, filename)
               if match is not None:
                  part = match.group(0)               
                  match = re.search(r'\d+', part)
                  if match is not None:
                     num = match.group(0)
                     pad = '%'
                     if padding > 1: pad += '0' + str( padding)
                     pad += 'd'
                     newpart = part.replace( num, pad)
                     filename = filename.replace( part, newpart)
                     images.append( filename)
               else:
                  Application.LogMessage('Can`t solve "%s"' % filename)

      # Copy scene to temporary file:
      curjobname = jobname
      if len(passes) > 1: curjobname += '-%s' % cpass
      ftime = time.time()
      tmpscene = scenefile + '.' + curjobname + time.strftime('.%m%d-%H%M%S-') + str(ftime - int(ftime))[2:5] + '.scn'
      try:
         shutil.copyfile( scenefile, tmpscene)
      except:
         Application.LogMessage('Unable to copy temporary scene:')
         Application.LogMessage( tmpscene)
         Application.LogMessage( str(sys.exc_info()[1]))
         return
      if not os.path.isfile( tmpscene):
         Application.LogMessage('Error: Can\'t save temporary scene.')
         return

      # Get frame range:
      cp_frame_start = frame_start
      cp_frame_end = frame_end
      cp_frame_by = frame_by
      if not range_forcepass:
         if range_frompass:
            if Application.GetValue('Passes.%s.FrameRangeSource' % cpass) == 0:
               cp_frame_start = Application.GetValue('Passes.%s.FrameStart' % cpass)
               cp_frame_end = Application.GetValue('Passes.%s.FrameEnd' % cpass)
               cp_frame_by = Application.GetValue('Passes.%s.FrameStep' % cpass)

      # Construct job:
      Application.LogMessage('Sending "%s" pass, range: %d-%d,%d' % (cpass, cp_frame_start, cp_frame_end, cp_frame_by))

#xsibatch -script "%XSI_CGRU_PATH%\afrender.py" -lang Python -main afRenderCurPass -args
#-scenePath "%CD%\project\Scenes\scene.scn" -startFrame 1 -endFrame 2 -step 1 -simulate 0 -setAttr torus.polymsh.geom.enduangle -setValue 120

      blocknames = []
      blockcmds = []
      blockimages = []

      cmd = os.environ['XSI_CGRU_PATH']
      cmd = os.path.join( cmd, 'afrender.py')
      cmd = 'xsibatch -script %s' % cmd
      cmd += ' -lang Python -main afRender -args'
      cmd += ' -scene "%s"' % tmpscene
      cmd += ' -start @#@ -end @#@ -step ' + str(cp_frame_by)
      cmd += ' -simulate'
      if simulate:   cmd += ' 1'
      else:          cmd += ' 0'
      cmd += ' -renderPass ' + cpass
      if varirender:
         cmd += ' -attr ' + varirender_attr + ' -value '
         value = varirender_start
         for i in range( 0, varirender_count):
            blockcmds.append( cmd + str(value))
            blocknames.append( 'variant[%d]' % value)
            images_str = ''
            for img in images:
               img_dir = os.path.dirname( img)
               img_name = os.path.basename( img)
               img_dir = os.path.join( img_dir, str(value))
               img = os.path.join( img_dir, img_name)
               if images_str != '': images_str += ';'
               images_str += img
            blockimages.append( images_str)
            value += varirender_step
      else:
         blockname = 'xsi'
         images_str = ''
         for img in images:
            if images_str != '': images_str += ';'
            images_str += img
 
      job=af.Job( curjobname)
      job.setCmdPost( str('deletefiles "%s"' % os.path.abspath(tmpscene)))
      if priority  != -1: job.setPriority( priority)
      if maxhosts  != -1: job.setMaxHosts( maxhosts)
      if hostsmask         != None and hostsmask         != '': job.setHostsMask( hostsmask)
      if hostsmaskexclude  != None and hostsmaskexclude  != '': job.setHostsMaskExclude( hostsmaskexclude)
      if dependmask        != None and dependmask        != '': job.setDependMask( dependmask)
      if dependmaskglobal  != None and dependmaskglobal  != '': job.setDependMaskGlobal( dependmaskglobal)
      if paused: job.offLine()

      if len( blocknames) == 0:
         blocknames.append( blockname)
         blockcmds.append( cmd)
         blockimages.append( images_str)

      i = 0
      for blockname in blocknames:
         block = af.Block( blockname, 'xsi')
         block.setCommand( str( blockcmds[i]))
         block.setFiles( str( blockimages[i]))
         block.setNumeric( cp_frame_start, cp_frame_end, frame_fpt, cp_frame_by)
         if capacity   != -1: block.setCapacity( capacity)
         if maxruntime !=  0: block.setTasksMaxRunTime( int( maxruntime * 3600))
         job.blocks.append( block)
         i += 1

      # Send job:
      if not job.send():
         Application.LogMessage('Error: Can\'t send job to server.')
         os.remove( tmpscene)
