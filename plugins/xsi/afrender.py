import os, sys

def log( msg):
   Application.LogMessage('afRender: ' + msg)

def afRender( scene, start, end, step, simulate, renderPass = '', attr = '', value = ''):
   log('  ScenePath : ' + scene)
   log('  StartFrame: ' + start)
   log('  EndFrame  : ' + end)
   log('  Step      : ' + step)
   log('  Simulate  : ' + simulate)
   Application.SetUserPref('RealTimeMessageLogging', True )
   log('Trying to open "%s"...' % scene)
   Application.OpenScene( scene, False )
   log('Trying refresh scene...')
   Application.Refresh()
   Application.Refresh()
   Application.Refresh()
   start = int(start)
   end = int(end)
   step = int(step)
   simulate = int(simulate)
   if not simulate:
      Application.SetValue('PlayControl.Key', start )
      Application.SetValue('PlayControl.Current', end )
      Application.SetValue('PlayControl.In', start )
      Application.SetValue('PlayControl.Out', end )
      log('Trying to render current pass "%s"...' % renderPass)
   if attr != '':
      log('Trying to set value "%s" to "%s"...' % ( attr, value))
      value = int(value)
      Application.SetValue( attr, value)
      outdir = Application.GetValue('Passes.RenderOptions.ResolvedOutputDir')
      outdir = os.path.join( outdir, str(value))
      log('Changing output directory to:')
      log( outdir)
      if not os.path.isdir( outdir): os.mkdir( outdir)
      Application.SetValue('Passes.RenderOptions.OutputDir', outdir)

   Application.Refresh()
   if renderPass != '': renderPass = 'Passes.' + renderPass
   Application.RenderPasses( renderPass, start, end, step, "siRenderVerbosityOn")
   log('Finished.')
