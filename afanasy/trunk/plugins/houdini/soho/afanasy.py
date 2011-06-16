import soho
from soho import SohoParm
import hou
import os
import sys
import time

import af
import afcommon
import services.service

VERBOSE = 2

class BlockParameters:
   def __init__( self, afnode, ropnode, subblock, prefix):
      if VERBOSE==2: print 'Initializing block parameters for "%s" from "%s"' % (ropnode.name(), afnode.name())

      self.ropnode   = ropnode
      self.ropname   = str( ropnode.name())
      self.valid     = True
      self.subblock  = subblock
      self.prefix    = prefix
      self.fullrangedepend = 0

      if afnode.parm('f1') is not None: self.framefirst = int( afnode.parm('f1').eval())
      if afnode.parm('f2') is not None: self.framelast  = int( afnode.parm('f2').eval())
      if afnode.parm('f3') is not None: self.frameinc   = int( afnode.parm('f3').eval())

      self.take              = str( afnode.parm('take').eval())
      self.jobname           = str( afnode.parm('jobname').eval())

      self.start_paused      = int( afnode.parm('start_paused').eval())
      self.priority          = int( afnode.parm('priority').eval())
      self.framefirst        = int( afnode.parm('f1').eval())
      self.framelast         = int( afnode.parm('f2').eval())
      self.frameinc          = int( afnode.parm('f3').eval())
      self.framespertask     = int( afnode.parm('fpr').eval())
      self.maxhosts          = int( afnode.parm('maximum_hosts').eval())
      self.capacity          = int( afnode.parm('capacity').eval())
      self.capacitymin       = int( afnode.parm('capacity_coefficient1').eval())
      self.capacitymax       = int( afnode.parm('capacity_coefficient2').eval())
      self.hostsmask         = str( afnode.parm('hosts_mask').eval())
      self.hostsmaskexclude  = str( afnode.parm('hosts_mask_exclude').eval())
      self.platform          = str( afnode.parm('platform').eval())

      # Find the output driver to process
      ropnode = gen.node(hdriver)
      if not ropnode:
         soho.error('Can\'t find %s for processing' % ropname)
      if not isinstance( ropnode, hou.RopNode):
         soho.error('%s is not a ROP node' % ropname)
      tr = ropnode.parm('trange')
      blockSet = False
      soho_foreground = driver.parm('soho_foreground')
      if tr != None and tr.eval() == 0:
         if soho_foreground != None:
            if soho_foreground.eval() == 0:
               try:
                  soho_foreground.set( 1)
                  blockSet = True
               except:
                  soho.error('Set "Block Until Render Complete" on %s node' % ropname)

   def genJob( self):
      if VERBOSE: print 'Generating job on "%s"' % self.jobname

      if not os.path.isdir(hip): soho.error('Unable to find spool directory "%"' % hip)
      ftime = time.time()
      tmphip = hip + '/' + jobname + time.strftime('.%m%d-%H%M%S-') + str(ftime - int(ftime))[2:5] + ".hip"
      # use mwrite, because hou.hipFile.save(tmphip)
      # changes current scene file name to tmphip, at least in version 9.1.115
      hou.hscript('mwrite -n %s' % tmphip)

      jobcmd = 'hrender_af'
      if ignore_inputs: jobcmd += ' -i'
      jobargs = ' -s @#@ -e @#@ --by %d -t %s %s %s' % (self.frameinc, self.take, tmphip, self.ropname)

      blocktype = 'hbatch'
      preview = ''
      drivertypename = self.ropnode.type().name()
      if drivertypename == 'ifd':
         blocktype = 'hbatch_mantra'
         vm_picture = driver.parm('vm_picture')
         if vm_picture != None:
            preview = afcommon.patternFromPaths( vm_picture.evalAsStringAtFrame(start), vm_picture.evalAsStringAtFrame(end))
      elif drivertypename == 'rib':
         blocktype = 'hbatch_prman'

      job = af.Job()
      job.setName( jobname )
      if start_paused:
         job.offLine()

      if platform != '':
         if platform == 'any': job.setNeedOS('')
         else: job.setNeedOS( platform)

      block = af.Block( hdriver, blocktype)
      block.setWorkingDirectory( os.getenv('PWD', os.getcwd()))
      block.setNumeric( start, end, fpertask, by)
      if preview != '': block.setFiles( preview)
      job.blocks.append( block)

      job.setPriority( self.priority)
      if self.depend_mask != '': job.setDependMask( self.depend_mask)
      if self.depend_mask_global != '': job.setDependMaskGlobal( self.depend_mask_global)
      if self.maximum_hosts > -1: job.setMaxHosts( self.maximum_hosts)
      if self.hosts_mask != '': job.setHostsMask( self.hosts_mask)
      if self.hosts_mask_exclude != '': job.setHostsMaskExclude( self.hosts_mask_exclude)
      block.setCapacity( self.capacity)
      if self.capacity_min != -1 or self.capacity_max != -1 :
         block.setVariableCapacity( self.capacity_min, self.capacity_max)
         jobcmd += ' --numcpus '+ services.service.str_capacity

      block.setCommand( jobcmd + jobargs)

      job.setCmdPost('deletefiles "%s"' % tmphip)

      if VERBOSE: job.stdout( True)

#      job.send()

      if blockSet: soho_foreground.set( 0)

def getJobParameters( afnode):
   inputs = []
   inputs.extend( hou.pwd().inputs())
   inputs.reverse()
   for i in inputs:
      if i.isBypassed(): continue
      print i

afnode = hou.pwd()
getJobParemeters( afnode)
