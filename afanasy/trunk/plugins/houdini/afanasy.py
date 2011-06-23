import hou
import os
import sys
import time

import af
import afcommon
import services.service

VERBOSE = 2

class BlockParameters:
   def __init__( self, afnode, ropnode, subblock, prefix, frame_range):
      if VERBOSE == 2: print 'Initializing block parameters for "%s" from "%s"' % (ropnode.name(), afnode.name())

      self.ropnode         = ropnode
      self.ropname         = str( ropnode.name())
      self.valid           = False
      self.subblock        = subblock
      self.prefix          = prefix
      self.dependmask      = ''
      self.fullrangedepend = False
      self.blockname       = self.ropname
      if self.prefix != '': self.blockname = self.prefix + '_' + self.blockname

      # Check output driver to process:
      if not ropnode:
         hou.ui.displayMessage('Can\'t find "%s" for processing' % ropname)
      if not isinstance( ropnode, hou.RopNode):
         hou.ui.displayMessage('"%s" is not a ROP node' % ropname)


      # Process frame range:
      self.frame_first, self.frame_last, self.frame_inc, self.frame_pertask = frame_range
      trange = afnode.parm('trange')
      if trange.eval() > 0:
         self.frame_first   = int( afnode.parm('f1').eval())
         self.frame_last    = int( afnode.parm('f2').eval())
         self.frame_inc     = int( afnode.parm('f3').eval())
         self.frame_pertask = int( afnode.parm('frame_pertask').eval())
      if trange.eval() > 1: self.fullrangedepend = True
      trange = ropnode.parm('trange')
      if trange is not None:
         if trange.eval() > 0:
            if ropnode.parm('f1') is not None: self.frame_first = int( ropnode.parm('f1').eval())
            if ropnode.parm('f2') is not None: self.frame_last  = int( ropnode.parm('f2').eval())
            if ropnode.parm('f3') is not None: self.frame_inc   = int( ropnode.parm('f3').eval())
         if trange.eval() > 1: self.fullrangedepend = True
      if self.frame_last < self.frame_first:
         hou.ui.displayMessage('Last frame < first frame for "%s"' % ropname)
         return
      if self.frame_inc < 1:
         hou.ui.displayMessage('Frame increment < 1 for "%s"' % ropname)
         return
      if self.frame_pertask < 1:
         hou.ui.displayMessage('Frames per task < 1 for "%s"' % ropname)
         return
         
      # Get parameters:
      self.job_name           = str( afnode.parm('job_name').eval())
      self.start_paused       = int( afnode.parm('start_paused').eval())
      self.take               = str( afnode.parm('take').eval())
      self.ignore_inputs      = int( afnode.parm('ignore_inputs').eval())
      self.platform           = str( afnode.parm('platform').eval())
      self.subtaskdepend      = int( afnode.parm('subtaskdepend').eval())
      self.priority           = -1
      self.max_runtasks       = -1
      self.capacity           = -1
      self.capacity_min       = -1
      self.capacity_max       = -1
      self.hosts_mask         = ''
      self.hosts_mask_exclude = ''
      self.depend_mask        = ''
      self.depend_mask_global = ''
      if afnode.parm('enable_extended_parameters').eval():
         self.priority           = int( afnode.parm('priority').eval())
         self.max_runtasks       = int( afnode.parm('max_runtasks').eval())
         self.capacity           = int( afnode.parm('capacity').eval())
         self.capacity_min       = int( afnode.parm('capacity_coefficient1').eval())
         self.capacity_max       = int( afnode.parm('capacity_coefficient2').eval())
         self.hosts_mask         = str( afnode.parm('hosts_mask').eval())
         self.hosts_mask_exclude = str( afnode.parm('hosts_mask_exclude').eval())
         self.depend_mask        = str( afnode.parm('depend_mask').eval())
         self.depend_mask_global = str( afnode.parm('depend_mask_global').eval())

      # Try to set driver foreground mode
      self.soho_foreground_set = False
      trange = ropnode.parm('trange')
      soho_foreground = ropnode.parm('soho_foreground')
      if trange != None and trange.eval() == 0:
         if soho_foreground != None:
            if soho_foreground.eval() == 0:
               try:
                  soho_foreground.set( 1)
                  self.soho_foreground_set = True
               except:
                  hou.ui.displayMessage('Set "Block Until Render Complete" on "%s" node' % ropname)
                  return

      self.valid = True

   def genBlock( self, hipfilename):
      if VERBOSE: print 'Generating block on "%s"' % self.job_name

      blockcmd = 'hrender_af'
      if self.ignore_inputs: blockcmd += ' -i'
      blockargs = ' -s @#@ -e @#@ --by %d -t %s %s %s' % (self.frame_inc, self.take, hipfilename, self.ropname)

      blocktype = 'hbatch'
      preview = ''
      drivertypename = self.ropnode.type().name()
      if drivertypename == 'ifd':
         blocktype = 'hbatch_mantra'
         vm_picture = self.ropnode.parm('vm_picture')
         if vm_picture != None:
            preview = afcommon.patternFromPaths( vm_picture.evalAsStringAtFrame( self.frame_first), vm_picture.evalAsStringAtFrame( self.frame_last))
      elif drivertypename == 'rib':
         blocktype = 'hbatch_prman'

      block = af.Block( self.blockname, blocktype)
      block.setWorkingDirectory( os.getenv('PWD', os.getcwd()))
      block.setNumeric( self.frame_first, self.frame_last, self.frame_pertask, self.frame_inc)
      if preview != '': block.setFiles( preview)

      block.setCapacity( self.capacity)
      if self.capacity_min != -1 or self.capacity_max != -1 :
         block.setVariableCapacity( self.capacity_min, self.capacity_max)
         blockcmd += ' --numcpus '+ services.service.str_capacity

      block.setCommand( blockcmd + blockargs)

      if self.subblock:
         if self.max_runtasks > -1: block.setMaxRunningTasks( self.max_runtasks)
         if self.hosts_mask != '': block.setHostsMask( self.hosts_mask)
         if self.hosts_mask_exclude != '': block.setHostsMaskExclude( self.hosts_mask_exclude)
      if self.dependmask != '':
         if self.fullrangedepend:
            block.setDependMask( self.dependmask)
         else:
            block.setTasksDependMask( self.dependmask)
      if self.subtaskdepend: block.setDependSubTask()

      return block

   def genJob( self, blockparams = None):
      if VERBOSE: print 'Generating job on "%s"' % self.job_name

      # Calculate temporary hip name:
      ftime = time.time()
      tmphip = hou.hipFile.name() + '_' + self.job_name + time.strftime('.%m%d-%H%M%S-') + str(ftime - int(ftime))[2:5] + ".hip"
      # use mwrite, because hou.hipFile.save(tmphip)
      # changes current scene file name to tmphip, at least in version 9.1.115
      hou.hscript('mwrite -n %s' % tmphip)

      job = af.Job()
      job.setName( self.job_name )
      if self.start_paused:
         job.offLine()
      if self.platform != '':
         if self.platform == 'any': job.setNeedOS('')
         else: job.setNeedOS( self.platform)
      if self.priority != -1: job.setPriority( self.priority)
      if self.depend_mask != '': job.setDependMask( self.depend_mask)
      if self.depend_mask_global != '': job.setDependMaskGlobal( self.depend_mask_global)
      if self.max_runtasks > -1: job.setMaxRunningTasks( self.max_runtasks)
      if self.hosts_mask != '': job.setHostsMask( self.hosts_mask)
      if self.hosts_mask_exclude != '': job.setHostsMaskExclude( self.hosts_mask_exclude)

      job.blocks.append( self.genBlock( tmphip))
      if blockparams is not None:
         for blockparam in blockparams:
            job.blocks.append( blockparam.genBlock( tmphip))

      job.setCmdPost('deletefiles "%s"' % tmphip)

      if VERBOSE: job.output( True)

      job.send()

   def doPost( self):
      if VERBOSE: print 'doPost: "%s"' % self.ropname
      if self.soho_foreground_set: self.ropnode.parm('soho_foreground').set( 0)

   def addDependMask( self, dependmask):
      if dependmask is None: return
      if dependmask == '': return
      if self.dependmask: self.dependmask += '|' + dependmask
      else: self.dependmask = dependmask

def getJobParameters( afnode, subblock = False, frame_range = None, prefix = ''):
   if VERBOSE: 'getJobParameters: "%s"' % afnode.name()

   # Process frame range:
   if frame_range is None:
      frame_first   = hou.frame()
      frame_last    = frame_first
      frame_inc     = 1
      frame_pertask = 1
   else:
      frame_first, frame_last, frame_inc, frame_pertask = frame_range
   trange = afnode.parm('trange')
   if trange.eval() > 0:
      frame_first   = int( afnode.parm('f1').eval())
      frame_last    = int( afnode.parm('f2').eval())
      frame_inc     = int( afnode.parm('f3').eval())
      frame_pertask = int( afnode.parm('frame_pertask').eval())
   frame_range = frame_first, frame_last, frame_inc, frame_pertask

   params = []
   nodes = []
   nodes.extend( afnode.inputs())
   nodes.reverse()
   dependmask = ''
   prevparams = []
   for node in nodes:
      if node.isBypassed(): continue
      newparams = []
      if node.type().name() == 'afanasy':
         newprefix = afnode.name()
         if prefix != '': newprefix = prefix + '_' + newprefix
         newparams = getJobParameters( node, True, frame_range, newprefix)
         dependmask = newprefix + '_.*'
         if newparams is None: return None
      else:
         param = BlockParameters( afnode, node, subblock, prefix, frame_range)
         if not param.valid: return None
         newparams.append( param)
         dependmask = param.blockname
      if len(newparams): params.extend(newparams)
      else: return None
      
      if not afnode.parm('independent').eval() and dependmask != '':
         for prevparam in prevparams:
            prevparam.addDependMask( dependmask)

      prevparams = newparams

   return params

def render( afnode):
   if VERBOSE: print 'render: "%s"' % afnode.name()
   params = getJobParameters( afnode)
   if params is not None:
      if len(params) == 1:
         params[0].genJob()
      elif len(params) > 1:
         params[0].genJob( params[1:])
      for parm in params: parm.doPost()
