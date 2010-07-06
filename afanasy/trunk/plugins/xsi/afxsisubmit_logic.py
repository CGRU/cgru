import os, shutil, time

#import win32com.client
#from win32com.client import constants

#Get handles to the xsi application
#Application = win32com.client.Dispatch('XSI.Application')

def CloseButton_OnClicked(): PPG.Close()

def SubmitButton_OnClicked():
   opSet = Application.ActiveSceneRoot.Properties('afSubmitProperties')
   if(opSet == None):
      Application.LogMessage('Error: Can\'t find options.')
      PPG.Close()

   Application.SaveScene()
   scene = Application.ActiveProject.ActiveScene

   # Copy scene to temporary file:
   scenefile = scene.Filename.Value
   if not os.path.isfile( scenefile):
      Application.LogMessage('Error: Can\'t save scene.')
      return
   ftime = time.time()
   tmpscene = scenefile + time.strftime('.%m%d-%H%M%S-') + str(ftime - int(ftime))[2:5] + '.scn'
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

#   python "%AF_ROOT%/python/afjob.py" "%CD%\project\Scenes\scene.scn" 1 2 -fpr 1 -take Default_Pass -name XSI_Job

   for cpass in passes:
      cp_frame_start = frame_start
      cp_frame_end = frame_end
      cp_frame_by = frame_by
      if not range_forcepass:
         if range_frompass:
            if Application.GetValue('Passes.%s.FrameRangeSource' % cpass) == 0:
               cp_frame_start = Application.GetValue('Passes.%s.FrameStart' % cpass)
               cp_frame_end = Application.GetValue('Passes.%s.FrameEnd' % cpass)
               cp_frame_by = Application.GetValue('Passes.%s.FrameStep' % cpass)
      Application.LogMessage('Sending "%s" pass, range: %d-%d,%d' % (cpass, cp_frame_start, cp_frame_end, cp_frame_by))
      curjobname = jobname
      if len(passes) > 1: curjobname += '-%s' % cpass
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
      os.system(cmd)
