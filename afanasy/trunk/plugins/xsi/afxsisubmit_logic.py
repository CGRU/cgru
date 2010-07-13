import re, os, shutil, time

import webbrowser

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

      images = ''
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
                     if images != '': images += ';'
                     images += filename
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
      cmd = os.environ['AF_ROOT']
      cmd = os.path.join( cmd, 'python')
      cmd = os.path.join( cmd, 'afjob.py')
      cmd = 'python "%s"' % cmd
      cmd += ' "%s"' % tmpscene
      cmd += ' %d' % cp_frame_start
      cmd += ' %d' % cp_frame_end
      cmd += ' -fpr %d' % frame_fpt
      cmd += ' -by %d' % cp_frame_by
      cmd += ' -take "%s"' % cpass
      cmd += ' -name "%s"' % curjobname
      if images    != '': cmd += ' -images "%s"'   % images
      if capacity  != -1: cmd += ' -capacity %d'   % capacity
      if priority  != -1: cmd += ' -priority %d'   % priority
      if maxhosts  != -1: cmd += ' -maxhosts %d'   % maxhosts
      if maxruntime >  0: cmd += ' -maxruntime %d' % maxruntime
      if hostsmask         != None and hostsmask         != '': cmd += ' -hostsmask "%s"' % hostsmask
      if hostsmaskexclude  != None and hostsmaskexclude  != '': cmd += ' -hostsexcl "%s"' % hostsmaskexclude
      if dependmask        != None and dependmask        != '': cmd += ' -depmask "%s"'   % dependmask
      if dependmaskglobal  != None and dependmaskglobal  != '': cmd += ' -depglbl "%s"'   % dependmaskglobal

      if simulate: cmd += ' -simulate'
      if paused: cmd += ' -pause'
      if varirender:
         cmd += ' -varirender %s %d %d %d' % (varirender_attr, varirender_start, varirender_step, varirender_count)
      cmd += ' -deletescene'
      Application.LogMessage(cmd)

      # Send job:
      os.system(cmd)
