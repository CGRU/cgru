import hou
import os
import sys
import time

import af
import afcommon
import services.service

VERBOSE = 0

class BlockParameters:
   def __init__( self, afnode, ropnode, subblock, prefix, frame_range, for_job_only = False):
      if VERBOSE == 2:
         if ropnode:
            print 'Initializing block parameters for "%s" from "%s"' % (ropnode.path(), afnode.path())
         else:
            print 'Initializing command block parameters from "%s"' % (afnode.path())

      # Init parameters:
      self.valid           = False
      self.afnode          = afnode
      self.ropnode         = None
      self.subblock        = subblock
      self.prefix          = prefix
      self.preview         = ''
      self.name            = ''
      self.type            = ''
      self.cmd             = ''
      self.cmd_useprefix   = True
      self.dependmask      = ''
      self.fullrangedepend = False
      self.numeric         = True
      self.frame_first, self.frame_last, self.frame_inc, self.frame_pertask = frame_range
      self.tasks_names     = []
      self.tasks_cmds      = []
      self.tasks_previews  = []
      # Parameters to restore ROP changes:
      self.soho_foreground = None
      self.soho_outputmode = None

      # Get parameters:
      self.job_name           = str( afnode.parm('job_name').eval())
      self.start_paused       = int( afnode.parm('start_paused').eval())
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

      # Process frame range:
      opname = afnode.path()
      if afnode.parm('trange').eval() > 1: self.fullrangedepend = True
      if ropnode:
         opname = ropnode.path()
         trange = ropnode.parm('trange')
         if trange is not None:
            if int(trange.eval()) > 0:
               if ropnode.parm('f1') is not None: self.frame_first = int( ropnode.parm('f1').eval())
               if ropnode.parm('f2') is not None: self.frame_last  = int( ropnode.parm('f2').eval())
               if ropnode.parm('f3') is not None: self.frame_inc   = int( ropnode.parm('f3').eval())
            if int(trange.eval()) > 1: self.fullrangedepend = True
      if self.frame_last < self.frame_first:
         hou.ui.displayMessage('Last frame < first frame for "%s"' % opname)
         return
      if self.frame_inc < 1:
         hou.ui.displayMessage('Frame increment < 1 for "%s"' % opname)
         return
      if self.frame_pertask < 1:
         hou.ui.displayMessage('Frames per task < 1 for "%s"' % opname)
         return

      # Process output driver type to construct a command:
      if ropnode:
         self.type = 'hbatch'
         if not isinstance( ropnode, hou.RopNode):
            hou.ui.displayMessage('"%s" is not a ROP node' % ropnode.path())
            return
         self.ropnode = ropnode
         self.name    = str( ropnode.name())
         if self.prefix != '': self.name = self.prefix + '_' + self.name
         # Block type and preview:
         roptype = ropnode.type().name()
         if roptype == 'ifd':
            if not ropnode.parm('soho_outputmode').eval(): self.type = 'hbatch_mantra'
            vm_picture = ropnode.parm('vm_picture')
            if vm_picture != None:
               self.preview = afcommon.patternFromPaths( vm_picture.evalAsStringAtFrame( self.frame_first), vm_picture.evalAsStringAtFrame( self.frame_last))
         elif roptype == 'rib':
            self.type = 'hbatch_prman'
         # Block command:
         self.cmd = 'hrender_af'
         if afnode.parm('ignore_inputs').eval(): self.cmd += ' -i'
         if self.capacity_min != -1 or self.capacity_max != -1: self.cmd += ' --numcpus '+ services.service.str_capacity
         self.cmd += ' -s @#@ -e @#@ --by %d -t "%s"' % (self.frame_inc, afnode.parm('take').eval())
         self.cmd += ' "%(hipfilename)s"'
         self.cmd += ' "%s"' % ropnode.path()
      else:
         # Custom command driver:
         if int( afnode.parm('cmd_add').eval()):
            # Command:
            cmd = self.afnode.parm('cmd_cmd')
            self.cmd = afcommon.patternFromPaths( cmd.evalAsStringAtFrame( self.frame_first), cmd.evalAsStringAtFrame( self.frame_last))
            # Name:
            self.name = self.afnode.parm('cmd_name').eval()
            if self.name is None or self.name == '': self.name = self.cmd.split(' ')[0]
            # Service:
            self.type = self.afnode.parm('cmd_service').eval()
            if self.type is None or self.type == '': self.type = self.cmd.split(' ')[0]
            # Prefix:
            self.cmd_useprefix = int( self.afnode.parm('cmd_use_afcmdprefix').eval())
         elif not for_job_only:
            hou.ui.displayMessage('Can\'t process "%s"' % str(afnode.path()))
            return

      # Try to set driver foreground mode
      if ropnode:
         trange = ropnode.parm('trange')
         soho_foreground = ropnode.parm('soho_foreground')
         if trange != None and int(trange.eval()) == 0:
            if soho_foreground != None:
               if soho_foreground.eval() == 0:
                  try:
                     soho_foreground.set( 1)
                     self.soho_foreground = 0
                  except:
                     hou.ui.displayMessage('Set "Block Until Render Complete" on "%s" node' % ropnode.path())
                     return

      self.valid = True

   def genBlock( self, hipfilename):
      if VERBOSE:
         if self.ropnode:
            print 'Generating block for "%s" from "%s"' % (self.ropnode.path(), self.afnode.path())
         else:
            print 'Generating command block from "%s"' % (self.afnode.path())

      block = af.Block( self.name, self.type)
      block.setCommand( self.cmd % vars(), self.cmd_useprefix)
      if self.preview != '': block.setFiles( self.preview)

      if self.numeric: block.setNumeric( self.frame_first, self.frame_last, self.frame_pertask, self.frame_inc)
      else:
         t = 0
         for cmd in self.tasks_cmds:
            task = af.Task( self.tasks_names[t])
            task.setCommand( cmd)
            if len( self.tasks_previews): task.setFiles(self.tasks_previews[t])
            block.tasks.append( task)
            t += 1
         block.setFramesPerTask( self.frame_pertask)

      block.setCapacity( self.capacity)
      if self.capacity_min != -1 or self.capacity_max != -1 :
         block.setVariableCapacity( self.capacity_min, self.capacity_max)

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

   def genJob( self, blockparams):
      if VERBOSE: print 'Generating job on "%s"' % self.job_name

      if len( blockparams) < 1:
         print('Can`t generate job without any blocks on "%s"' % self.afnode.name())
         return

      # Calculate temporary hip name:
      ftime = time.time()
      tmphip = hou.hipFile.name() + '_' + afcommon.filterFileName(self.job_name) + time.strftime('.%m%d-%H%M%S-') + str(ftime - int(ftime))[2:5] + ".hip"
      # use mwrite, because hou.hipFile.save(tmphip)
      # changes current scene file name to tmphip, at least in version 9.1.115
      hou.hscript('mwrite -n "%s"' % tmphip)

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

      for blockparam in blockparams:
         job.blocks.append( blockparam.genBlock( tmphip))

      job.setCmdPost('deletefiles "%s"' % tmphip)

      if VERBOSE: job.output( True)

      job.send()

   def doPost( self):
      if self.ropnode is None: return
      if VERBOSE: print 'doPost: "%s"' % self.ropnode.path()
      if self.soho_foreground is not None: self.ropnode.parm('soho_foreground').set(self.soho_foreground)
      if self.soho_outputmode is not None:
         self.ropnode.parm('soho_outputmode').set(self.soho_outputmode)
         self.ropnode.parm('soho_diskfile').set('')

   def addDependMask( self, dependmask):
      if dependmask is None: return
      if dependmask == '': return
      if self.dependmask: self.dependmask += '|' + dependmask
      else: self.dependmask = dependmask

def getBlockParameters( afnode, ropnode, subblock, prefix, frame_range):
   params = []
   if ropnode is not None and ropnode.type().name() == 'ifd' and afnode.parm('sep_enable').eval():

      # Mantra separate render:
      block_generate = BlockParameters( afnode, ropnode, subblock, prefix, frame_range)
      blockname = block_generate.name
      block_generate.name += '-G'
      if not block_generate.valid: return None

      run_rop = afnode.parm('sep_run_rop').eval()
      read_rop = afnode.parm('sep_read_rop_params').eval()
      join_render = afnode.parm('sep_join').eval()
      tile_render = afnode.parm('sep_tile').eval()
      tile_divx = afnode.parm('sep_tile_divx').eval()
      tile_divy = afnode.parm('sep_tile_divy').eval()
      use_tmp_img_folder = afnode.parm('sep_use_tmp_img_folder').eval()
      del_rop_files = afnode.parm('sep_del_rop_files').eval()

      if read_rop or run_rop:
         if not block_generate.ropnode:
            hou.ui.displayMessage('Can`t find ROP for processing "%s"' % afnode.path())
         if not isinstance( ropnode, hou.RopNode):
            hou.ui.displayMessage('"%s" is not a ROP node' % block_generate.ropnode.path())
      if not run_rop: join_render = False
      if join_render:
         tile_render = False
         delete_files = True
      else:
         if block_generate.ropnode.parm('soho_outputmode').eval() == 0:
            # Set output mode to produce ifd files:
            block_generate.ropnode.parm('soho_outputmode').set(1)
            block_generate.soho_outputmode = 0
            block_generate.ropnode.parm('soho_diskfile').set( block_generate.ropnode.parm('vm_picture').unexpandedString() + '.ifd')

      if read_rop:
         images = ropnode.parm('vm_picture')
         files  = ropnode.parm('soho_diskfile')
         afnode.parm('sep_images').set(images.unexpandedString())
         afnode.parm('sep_files' ).set( files.unexpandedString())

      images = afcommon.patternFromPaths( afnode.parm('sep_images').evalAsStringAtFrame(block_generate.frame_first), afnode.parm('sep_images').evalAsStringAtFrame(block_generate.frame_last))
      files  = afcommon.patternFromPaths( afnode.parm('sep_files' ).evalAsStringAtFrame(block_generate.frame_first), afnode.parm('sep_files' ).evalAsStringAtFrame(block_generate.frame_last))

      if run_rop:
         if join_render: block_generate.preview = images
         if not join_render:
            block_generate.type = 'hbatch'
         else:
            block_generate.type = 'hbatch_mantra'
            block_generate.cmd = block_generate.cmd.replace( 'hrender_af', 'hrender_separate')
            if use_tmp_img_folder: block_generate.cmd += ' --tmpimg'

      if not join_render:
         tiles = tile_divx * tile_divy
         block_render = BlockParameters( afnode, ropnode, subblock, prefix, frame_range)
         block_render.name = blockname + '-R'
         block_render.cmd = 'mantra'
         block_render.type = block_render.cmd
         if run_rop: block_render.dependmask = block_generate.name
         if tile_render or del_rop_files or use_tmp_img_folder: block_render.cmd = 'mantrarender '
         if del_rop_files and not tile_render: block_render.cmd += 'd'
         if use_tmp_img_folder: block_render.cmd += 't'
         if tile_render:
            block_render.numeric = False
            block_render.cmd += 'c %(tile_divx)d %(tile_divy)d' % vars()
            block_render.cmd += ' @#@'
            block_render.frame_pertask = -tiles
            for frame in range( block_generate.frame_first, block_generate.frame_last + 1, block_generate.frame_inc):
               arguments = afnode.parm('sep_render_arguments').evalAsStringAtFrame( frame)
               for tile in range( 0, tiles):
                  block_render.tasks_names.append('%d tile %d' % ( frame, tile))
                  block_render.tasks_cmds.append('%d -R %s' % ( tile, arguments))
         else:
            if del_rop_files or use_tmp_img_folder: block_render.cmd += ' -R '
            else: block_render.cmd +=  ' -V a '
            block_render.cmd += afcommon.patternFromPaths( afnode.parm('sep_render_arguments').evalAsStringAtFrame(block_generate.frame_first), afnode.parm('sep_render_arguments').evalAsStringAtFrame(block_generate.frame_last))
            block_render.previw = images

      if tile_render:
         cmd = 'exrjoin %(tile_divx)d %(tile_divy)d %(images)s d' % vars()
         if del_rop_files: cmd += ' && deletefiles -s "%s"' % files
         block_join = BlockParameters( afnode, ropnode, subblock, prefix, frame_range)
         block_join.name = blockname + '-J'
         block_join.type = 'generic'
         block_join.dependmask = block_render.name
         block_join.cmd = cmd
         block_join.cmd_useprefix = False
         block_join.preview = images

      if read_rop:
         afnode.parm('sep_images').set('')
         afnode.parm('sep_files' ).set('')

      if tile_render: params.append( block_join)
      if not join_render: params.append( block_render)
      if run_rop: params.append( block_generate)

   else:
      params.append( BlockParameters( afnode, ropnode, subblock, prefix, frame_range))
   return params

def getJobParameters( afnode, subblock = False, frame_range = None, prefix = ''):
   if VERBOSE: print 'Getting Job Parameters from "%s":' % afnode.path()

   # Process frame range:
   if frame_range is None:
      frame_first   = hou.frame()
      frame_last    = frame_first
      frame_inc     = 1
      frame_pertask = 1
   else:
      frame_first, frame_last, frame_inc, frame_pertask = frame_range
   trange = afnode.parm('trange')
   if int(trange.eval()) > 0:
      frame_first   = int( afnode.parm('f1').eval())
      frame_last    = int( afnode.parm('f2').eval())
      frame_inc     = int( afnode.parm('f3').eval())
      frame_pertask = int( afnode.parm('frame_pertask').eval())
   frame_range = frame_first, frame_last, frame_inc, frame_pertask

   params = []
   connections = []
   connections.extend( afnode.inputs())
   nodes = []
   for node in connections:
      if node is not None:
         nodes.append( node)
   if afnode.parm('cmd_add').eval(): nodes.append(None)
   nodes.reverse()
   dependmask = ''
   prevparams = []
   for node in nodes:
      if node and node.isBypassed(): continue
      newparams = []
      if node and node.type().name() == 'afanasy':
         newprefix = node.name()
         if prefix != '': newprefix = prefix + '_' + newprefix
         newparams = getJobParameters( node, True, frame_range, newprefix)
         dependmask = newprefix + '_.*'
         if newparams is None: return None
      else:
         newparams = getBlockParameters( afnode, node, subblock, prefix, frame_range)
         if newparams is None: return None
         dependmask = newparams[0].name
         for param in newparams:
            if not param.valid: return None
      if len(newparams): params.extend(newparams)
      else: return None
      
      if not afnode.parm('independent').eval() and dependmask != '':
         for prevparam in prevparams:
            prevparam.addDependMask( dependmask)

      prevparams = newparams

   # Last parameter needed to generate a job.
   if not subblock: params.append( BlockParameters( afnode, None, False, '', frame_range, True))
   
   return params

def render( afnode):
	if VERBOSE: print '\nRendering "%s":' % afnode.path()
	params = getJobParameters( afnode)
	if params is not None and len(params) > 1:
		params[-1].genJob( params[:-1])
		for parm in params: parm.doPost()
	else:
		hou.ui.displayMessage('No tasks founded for\n%s\nIs it conncetted to some valid ROP node?' % afnode.path())
