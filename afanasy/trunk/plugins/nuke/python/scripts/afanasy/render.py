import nuke

import os.path
import os
import re
import time

RenderNodeClassName = 'Write'
DailiesNodeClassName = 'cgru_dailies'
AfanasyNodeClassName = 'afanasy'
AfanasyServiceType = 'nuke'
DailiesServiceType = 'movgen'

VERBOSE = 0

af = None

def getNodeName( node): return node.name()

def checkFrameRange( framefirst, framelast, frameinc, framespertask, string = ''):
   if string != '': string = ' on "%s"' % string
   if framefirst > framelast:
      nuke.message('First frame > Last frame' + string)
      return False
   if frameinc < 1:
      nuke.message('Frame increment must be >= 1' + string)
      return False
   if framespertask < 1:
      nuke.message('Frames per task must be >= 1' + string)
      return False
   return True

def getInputNodes( afnode, parent):
   if parent is None:
      print 'Node is "None"'
      return None

   if VERBOSE == 2: print 'Getting inputs of "%s"' % parent.name()

   global LastAfNode
   global InputNumber
   global InputName

   inputnodes = []
   for i in range( parent.inputs()):
      node = parent.input(i)
      if node is None: continue
      inputnodes.append( node)

   if afnode != None:
      LastAfNode = afnode 
      addnodes = afnode.knob('addnodes').value()
      if addnodes != None and addnodes != '':
         if VERBOSE == 2: print 'Adding nodes "%s" to "%s"' % ( addnodes, afnode.name())
         rexpr = re.compile( addnodes + '$')
         addnodes = []
         addnodes.extend( nuke.allNodes( RenderNodeClassName,  nuke.root()))
         addnodes.extend( nuke.allNodes( AfanasyNodeClassName, nuke.root()))
         if len(addnodes) > 1: addnodes.sort( None, getNodeName)
         for node in addnodes:
            if rexpr.match( node.name()):
               inputnodes.append( node)

   nodes = []
   for i in range( len(inputnodes)):
      node = inputnodes[i]
      if afnode != None:
         InputNumber = i + 1
         InputName = node.name()
      if node.Class() == RenderNodeClassName or node.Class() == AfanasyNodeClassName or node.Class() == DailiesNodeClassName:
         disableknob = node.knob('disable')
         if disableknob:
            if not disableknob.value():
               nodes.append( node)
            else:
               if VERBOSE==1: print 'Node "%s" is disabled' % node.name()
               continue
      else:
         if node.inputs() > 0:
            childs = getInputNodes( None, node)
            if childs == None: return None
            if len( childs) > 0: nodes.extend( childs)
         else:
            nuke.message('Leaf node reached "%s"-"%s" on "%s" input #%d.' % ( node.name(), InputName, LastAfNode.name(), InputNumber))
            return None
   return nodes



class BlockParameters:
   def __init__( self, afnode, wnode, subblock, prefix, fparams):
      if VERBOSE==2: print 'Initializing block parameters for "%s"' % wnode.name()
      self.wnode = wnode
      self.valid = True

      self.subblock          = subblock
      self.prefix            = prefix

      self.framefirst        = nuke.root().firstFrame()
      self.framelast         = nuke.root().lastFrame()
      self.frameinc          =  1
      self.framespertask     =  1
      self.maxhosts          = -1
      self.capacity          = -1
      self.capacitymin       = -1
      self.capacitymax       = -1
      self.hostsmask         = None
      self.hostsmaskexclude  = None
      self.fullrangedepend   = 0
      if afnode is not None:
         self.framefirst        = int( afnode.knob('framefirst').value())
         self.framelast         = int( afnode.knob('framelast').value())
         self.frameinc          = int( afnode.knob('frameinc').value())
         self.framespertask     = int( afnode.knob('framespertask').value())
         self.maxhosts          = int( afnode.knob('maxhosts').value())
         self.capacity          = int( afnode.knob('capacity').value())
         self.capacitymin       = int( afnode.knob('capacitymin').value())
         self.capacitymax       = int( afnode.knob('capacitymax').value())
         self.hostsmask         = afnode.knob('hostsmask').value()
         self.hostsmaskexclude  = afnode.knob('hostsmaskexcl').value()

      self.writename = str( wnode.name())

      if wnode.Class() == RenderNodeClassName:
         # Get images files:
         self.imgfile = ''
         if nuke.toNode('root').knob('proxy').value():
            self.imgfile = str( wnode.knob('proxy').value())
         else:
            self.imgfile = str( wnode.knob('file').value())

         # Get views:
         views = wnode.knob('views')
         if views is not None:
            views = views.value()
            if views is not None:
               if views != '':
                  views = views.split(' ')
                  images = []
                  for view in views:
                     view = view.strip()
                     if view != '':
                        img = self.imgfile
                        img = img.replace('%V', view)
                        img = img.replace('%v', view[0])
                        images.append( img)
                  if len(images) > 0:
                     self.imgfile = ''
                     for img in images:
                        if self.imgfile != '': self.imgfile += ';'
                        self.imgfile += img

         # Check images files:
         if self.imgfile == '':
            nuke.message('Write Node "%s"\nImages are empty.' % self.writename)
            self.valid = False
         else:
            for imgfile in self.imgfile.split(';'):
               folder = os.path.dirname( imgfile)
               if folder != '':
                  if not os.path.isdir(folder):
                     result = nuke.ask('Write Node "%s" Directory\n%s\ndoes not exist.\nCreate it?' % (self.writename,folder))
                     if result:
                        os.makedirs( folder)
                        if not os.path.isdir(folder):
                           nuke.message('Can`t create folder:\n%s' % folder)
                           self.valid = False
                     else:
                        self.valid = False
      elif wnode.Class() == DailiesNodeClassName:
         if VERBOSE: print 'Generating dailies "%s"' % self.writename
      else:
         nuke.message('Node type\n"%s"\nis unsendable.' % self.writename)
         self.valid = False

      for par in fparams:
         if fparams[par] is not None:
            if hasattr( self, par):
               setattr( self, par, fparams[par])

      self.name = self.writename
      if subblock:
         if self.prefix != None:
            if self.prefix != '':
               self.name = self.prefix + self.name

      self.dependmask = ''
      self.tasksdependmask = ''

   def addTasksDependMask( self, mask):
      if self.tasksdependmask == '': self.tasksdependmask = mask
      else:
         self.tasksdependmask = self.tasksdependmask + '|' + mask

   def addDependMask( self, mask):
      if self.dependmask == '': self.dependmask = mask
      else:
         self.dependmask = self.dependmask + '|' + mask

   def genBlock( self, scenename):
      if VERBOSE==2: print 'Generating block "%s"' % self.name

      if not self.valid: return

      if self.wnode.Class() == RenderNodeClassName:
         block = af.Block( self.name, AfanasyServiceType)
         block.setNumeric( self.framefirst, self.framelast, self.framespertask)
         afcommon = __import__('afcommon', globals(), locals(), [])
         self.imgfile = afcommon.patternFromDigits( afcommon.patternFromStdC( self.imgfile))
         block.setFiles( self.imgfile)
         if self.capacity != -1: block.setCapacity( self.capacity)

         threads = os.getenv('NUKE_AF_RENDERTHREADS', '2')
         cmd = os.getenv('NUKE_AF_RENDER', 'nuke -i -m %(threads)s')
         cmdargs = ' -X %s -F@#@-@#@x%d -x \"%s\"' % ( self.writename, self.frameinc, scenename)
         if self.capacitymin != -1 or self.capacitymax != -1:



            block.setVariableCapacity( self.capacitymin, self.capacitymax)
            services = __import__('services.service', globals(), locals(), [])
            threads = services.service.str_capacity
         cmd = cmd.replace('AF_THREADS', threads)
         block.setCommand( cmd + cmdargs)
      elif self.wnode.Class() == DailiesNodeClassName:
         cgru = __import__('cgru', globals(), locals(), [])
         cgru.dailiesEvaluate( self.wnode)
         cmd = cgru.dailiesGenCmd( self.wnode)
         if cmd is None or cmd == '': return
         block = af.Block( os.path.basename( cmd.split(' ')[-1]), DailiesServiceType)
         if self.capacity != -1: block.setCapacity( self.capacity)
         task = af.Task('dailies')
         task.setCommand( cmd)
         block.tasks.append( task)
      else:
         print 'Invalid block node class = "%s"' % self.wnode.Class()
         return


      if self.dependmask != '': block.setDependMask( self.dependmask)
      if self.tasksdependmask != '': block.setTasksDependMask( self.tasksdependmask)

      if self.subblock:
         if self.maxhosts != -1: block.setMaxHosts( self.maxhosts)
         if self.hostsmask != None:
            self.hostsmask = str( self.hostsmask)
            if self.hostsmask != '': block.setHostsMask( self.hostsmask)
         if self.hostsmaskexclude != None:
            self.hostsmaskexclude = str( self.hostsmaskexclude)
            if self.hostsmaskexclude != '': block.setHostsMaskExclude( self.hostsmaskexclude)

      return block



def getBlocksParameters( afnode, subblock, prefix, fparams):
   if VERBOSE==2: print 'Getting block parameters "%s"' % afnode.name()

   # Get parameters:
   framefirst        = int( afnode.knob('framefirst').value())
   framelast         = int( afnode.knob('framelast').value())
   frameinc          = int( afnode.knob('frameinc').value())
   framespertask     = int( afnode.knob('framespertask').value())
   independent       = int( afnode.knob('independent').value())
   reversedepends    = int( afnode.knob('reversedeps').value())
   forceframes       = int( afnode.knob('forceframes').value())
   if checkFrameRange( framefirst, framelast, frameinc, framespertask, afnode.name()) == False: return
   if forceframes:
      fparams = dict({'framefirst':framefirst, 'framelast':framelast, 'frameinc':frameinc, 'framespertask':framespertask})

   # MutiWrite parameters:
   if subblock:
      newprefix = afnode.name()
      jobname = afnode.knob('jobname').value()
      if jobname != None and jobname != '': newprefix = jobname
      newprefix += '-'
      if prefix is None or prefix == '': prefix = newprefix
      else: prefix += newprefix

   blocksparams = []

   # Get input render nodes:
   nodes = getInputNodes( afnode, afnode)
   if nodes == None: return None
   if len( nodes) == 0: return blocksparams

   prevparams = []
   # Process input nodes:
   for node in nodes:
      newparams = []
      if node.Class() == AfanasyNodeClassName:
         # Recursion if input node class is "afanasy" too
         newparams = getBlocksParameters( node, True, prefix, fparams)
         # Get fullrangedepend only if afanasy connected to afanasy node
         for newparam in newparams: newparam.fullrangedepend = int( node.knob('fullrange').value())
         if newparams is None: return
         if len( newparams) == 0: continue
      else:
         # Get block parameters from node:
         bparams = BlockParameters( afnode, node, subblock, prefix, fparams)
         if not bparams.valid: return None
         newparams.append( bparams)

      # Set dependences:
      if not independent:
         if len(prevparams) > 0:
            if reversedepends:
               if len( newparams) > 1:
                  mask = newparams[0].prefix + '.*'
               else:
                  mask = newparams[0].name
               for ppar in prevparams: 
                  if newparams[0].fullrangedepend:
                     ppar.addDependMask( mask)
                  else:
                     ppar.addTasksDependMask( mask)
            else:
               if len( prevparams) > 1:
                  mask = prevparams[0].prefix + '.*'
               else:
                  mask = prevparams[0].name
               for nparam in newparams:
                  if prevparams[0].fullrangedepend:
                     nparam.addDependMask( mask)
                  else:
                     nparam.addTasksDependMask( mask)

      # Store previous parameters for dependences:
      prevparams = newparams

      # Append parameters array:
      blocksparams.extend( newparams)

   return blocksparams



class JobParameters:
   def __init__( self, afnode, jobname, blocks, fparams):
      self.valid = True
      if VERBOSE==2:
         nodename = '???'
         if afnode is not None: nodename = afnode.name()
         elif len(blocks): nodename = blocks[0].name
         print 'Initializing job parameters: "%s"' % nodename

      self.startpaused        = 0
      self.maxhosts           = -1
      self.priority           = -1
      self.platform           = None
      self.hostsmask          = None
      self.hostsmaskexclude   = None
      self.dependmask         = None
      self.dependmaskglobal   = None
      self.nodename           = None
      if afnode != None:
         self.startpaused        = int(afnode.knob('startpaused').value())
         self.maxhosts           = int(afnode.knob('maxhosts').value())
         self.priority           = int(afnode.knob('priority').value())
         self.platform           = afnode.knob('platform').value()
         self.hostsmask          = afnode.knob('hostsmask').value()
         self.hostsmaskexclude   = afnode.knob('hostsmaskexcl').value()
         self.dependmask         = afnode.knob('dependmask').value()
         self.dependmaskglobal   = afnode.knob('dependmaskglbl').value()
         self.nodename           = afnode.name()

      self.blocksparameters   = []

      self.jobname = jobname
      self.prefix = self.jobname

      for par in fparams:
         if fparams[par] is not None:
            if hasattr( self, par):
               setattr( self, par, fparams[par])

      if blocks is None:
         self.blocksparameters = getBlocksParameters( afnode, False, '', fparams)
         if self.blocksparameters is None: self.valid = False
      else:
         self.blocksparameters.extend( blocks)
         blocksnames = blocks[0].name
         if len( blocks) > 1:
            for i in range( 1,len(blocks)): blocksnames += '-' + blocks[i].name
         self.jobname += '-' + blocksnames
         self.prefix += '-'
         if self.nodename is None: self.nodename = blocksnames

   def addDependMask( self, mask):
      if self.dependmask is None or self.dependmask == '':
         self.dependmask = mask
      else:
         self.dependmask = self.dependmask + '|' + mask


   def genJob( self, renderscenename):
      if VERBOSE==2: print 'Generating job on: "%s"' % self.nodename
      if not self.valid: return None

      if self.blocksparameters == None:
         if VERBOSE: print 'Block parameters generation error on "%s"' % self.nodename
         return
      if len( self.blocksparameters) == 0:
         if VERBOSE: print 'No blocks parameters generated on "%s"' % self.nodename
         return

      afcommon = __import__('afcommon', globals(), locals(), [])
      self.scenename = renderscenename + afcommon.filterFileName('.%s.nk' % self.jobname)

      blocks = []
      for bparams in self.blocksparameters:
         block = bparams.genBlock( self.scenename)
         if block is None:
            if VERBOSE: print 'Block generation error on "%s" - "%s"' % ( self.nodename, bparams.name)
            return
         blocks.append( block)
      if len( blocks) == 0:
         if VERBOSE: print 'No blocks generated error on "%s"' % self.nodename
         return

      job = af.Job( str( self.jobname))
      if self.priority != -1: job.setPriority( self.priority)
      if self.maxhosts != -1: job.setMaxHosts( self.maxhosts)
      if self.hostsmask != None:
         self.hostsmask = str( self.hostsmask)
         if self.hostsmask != '': job.setHostsMask( self.hostsmask)
      if self.hostsmaskexclude != None:
         self.hostsmaskexclude = str( self.hostsmaskexclude)
         if self.hostsmaskexclude != '': job.setHostsMaskExclude( self.hostsmaskexclude)
      if self.dependmask != None:
         self.dependmask = str( self.dependmask)
         if self.dependmask != '': job.setDependMask( self.dependmask)
      if self.dependmaskglobal != None:
         self.dependmaskglobal = str( self.dependmaskglobal)
         if self.dependmaskglobal != '': job.setDependMaskGlobal( self.dependmaskglobal)
      if self.startpaused: job.offLine()
      if self.platform is None or self.platform == 'Any': job.setNeedOS('')
      job.setCmdPost('deletefiles "%s"' % self.scenename)
      job.blocks = blocks

      return job



def getJobsParameters( afnode, prefix, fparams):
   if VERBOSE==2: print 'Generating jobs parameters on "%s"' % afnode.name()
   jobsparameters = []
   if afnode.knob('disable') is True:
      if VERBOSE==1: print 'Node "%s" is disabled' % afnode.name()
      return jobsparameters

   # Get parameters:
   framefirst        = int( afnode.knob('framefirst').value())
   framelast         = int( afnode.knob('framelast').value())
   frameinc          = int( afnode.knob('frameinc').value())
   framespertask     = int( afnode.knob('framespertask').value())
   independent       = int( afnode.knob('independent').value())
   reversedepends    = int( afnode.knob('reversedeps').value())
   forceframes       = int( afnode.knob('forceframes').value())
   singlejob         = int( afnode.knob('singlejob').value())
   jobname           = afnode.knob('jobname').value()
   if jobname == None or jobname == '':
      jobname = afnode.name()
   jobname = prefix + '-' + jobname
   if checkFrameRange( framefirst, framelast, frameinc, framespertask, afnode.name()) == False: return
   if forceframes:
      fparams = dict({'framefirst':framefirst, 'framelast':framelast, 'framespertask':framespertask})

   # Construct single job (no jobs recursion)
   if singlejob:
      jobparams = JobParameters( afnode, jobname, None, fparams)
      if not jobparams.valid: return None
      jobsparameters.append( jobparams)
      return jobsparameters

   # Construct a job for each connection
   nodes = getInputNodes( afnode, afnode)
   if nodes is None: return None
   if len( nodes) == 0: return jobsparameters

   # Reverse nodes, to send most depended job last
   if reversedepends: nodes.reverse()

   # Construct job parameters
   dependname = None
   for node in nodes:
      if node.Class() == AfanasyNodeClassName:
         # Recursion if "afanasy" node connected
         newjobparams = getJobsParameters( node, jobname, fparams)
         if newjobparams is None: return
         if len( newjobparams) == 0: continue
         # Set dependences
         if not independent:
            if dependname is not None:
               for jobparams in newjobparams:
                  jobparams.addDependMask( dependname + '.*')
            dependname = newjobparams[0].prefix
         # Extend parameters array
         jobsparameters.extend( newjobparams)
      else:
         # Generate a job with one block if "write" node connected
         blocksparameters = []
         bparams = BlockParameters( afnode, node, False, '', fparams)
         if not bparams.valid: return None
         blocksparameters.append( bparams)
         jobparams = JobParameters( afnode, prefix, blocksparameters, fparams)
         if not jobparams.valid: return None
         # Set dependences
         if not independent:
            if dependname is not None:
               jobparams.addDependMask( dependname + '.*')
            dependname = jobparams.jobname
         # Append parameters array
         jobsparameters.append( jobparams)

   return jobsparameters



def renderNodes( nodes, fparams, storeframes):
   global af
   af = __import__('af', globals(), locals(), [])

   scenepath = nuke.root().name()
   if scenepath == 'Root': scenepath = os.getenv('NUKE_AF_TMPSCENE', 'tmp')
   scenepath = os.path.abspath( scenepath)
   scenename = os.path.basename( scenepath)
   ftime = time.time()
   tmp_suffix = time.strftime('.%m%d-%H%M%S-') + str(ftime - int(ftime))[2:5]
   renderscenename = scenepath + tmp_suffix

   jobsparameters = []
   for node in nodes:
      newjobparameters = []
      newjobparameters = None
      if node.Class() == AfanasyNodeClassName:
         oldparams = dict()
         for key in fparams:
            oldparams[key] = node.knob(key).value()
            node.knob(key).setValue(fparams[key])
         newjobparameters = getJobsParameters( node, scenename, dict())
         if newjobparameters is None: return
         if storeframes == False:
            for key in oldparams:
               node.knob(key).setValue(oldparams[key])
      if node.Class() == RenderNodeClassName or node.Class() == DailiesNodeClassName:
         blocksparameters = []
         bparams = BlockParameters( None, node, False, '', fparams)
         if not bparams.valid: return
         blocksparameters.append( bparams)
         jobparams = JobParameters( None, scenename, blocksparameters, fparams)
         if not jobparams.valid: return
         newjobparameters = []
         newjobparameters.append( jobparams)
      if newjobparameters is None:
         if VERBOSE: print 'Job(s) parameters generatiton error on "%s"' % node.name()
         return
      if len( newjobparameters) > 0: jobsparameters.extend( newjobparameters)

   jobs = []
   for jobparams in jobsparameters:
      job = jobparams.genJob( renderscenename)
      if job is None:
         if VERBOSE: print 'Job generatiton error on "%s"' % jobparams.nodename
         return
      jobs.append( job)

   if len( jobs) == 0:
      nuke.message('No jobs generated.')
      return

   
   afpathmap = __import__('afpathmap', globals(), locals(), [])
   pm = afpathmap.PathMap( os.environ['AF_ROOT'], UnixSeparators = True, Verbose = True)

   changed = nuke.modified()
   for i in range(len(jobs)):
      scenename = jobsparameters[i].scenename
      if pm.initialized:
         pm_scenename = scenename + '.pm'
         nuke.scriptSave( pm_scenename)
         pm.toServerFile( pm_scenename, scenename, SearchString = 'file ', Verbose = False)
         os.remove( pm_scenename)
      else:
         nuke.scriptSave( scenename)
      if jobs[i].send() == False:
         nuke.message('Unable to send job to server.')
         os.remove( scenename)
         break
      time.sleep( 0.1)
   nuke.modified( changed)



def render( node = None):
   nodes = []
   fparams = dict()

   if node is not None:
      # Render only specified node:
      nodes.append( node)
      renderNodes( nodes, fparams, False)
      return

   # Store minimum and maximum frames to show in dialog
   hasafanasynodes = False
   framefirst_min = None
   framefirst_max = None
   framelast_min = None
   framelast_max = None
   framespertask_min = None
   framespertask_max = None
   selectednodes = nuke.selectedNodes()
   selectednodes.sort( None, getNodeName)
   for node in selectednodes:
      if node.Class() == AfanasyNodeClassName or node.Class() == RenderNodeClassName or node.Class() == DailiesNodeClassName:
         nodes.append( node)
         # Check for minimum and maximum
         if node.Class() == AfanasyNodeClassName:
            hasafanasynodes = True
            framefirst = int( node.knob('framefirst').value())
            framelast  = int( node.knob('framelast').value())
            framespertask   = int( node.knob('framespertask').value())
         else:
            framefirst = nuke.root().firstFrame()
            framelast  = nuke.root().lastFrame()
            framespertask   = 1
         if framefirst_min is None: framefirst_min = framefirst
         else: framefirst_min = min(framefirst_min,  framefirst)
         if framefirst_max is None: framefirst_max = framefirst
         else: framefirst_max = max(framefirst_max,  framefirst)
         if framelast_min is None: framelast_min = framelast
         else: framelast_min = min(framelast_min,  framelast)
         if framelast_max is None: framelast_max = framelast
         else: framelast_max = max(framelast_max,  framelast)
         if framespertask_min is None: framespertask_min = framespertask
         else: framespertask_min = min(framespertask_min,  framespertask)
         if framespertask_max is None: framespertask_max = framespertask
         else: framespertask_max = max(framespertask_max,  framespertask)

   if len( nodes) < 1:
      nuke.message('No nodes to render founded.\nSelect "%s" or "%s" node(s) to render.' % (AfanasyNodeClassName, RenderNodeClassName))
      return

   nodesstring = nodes[0].name()
   if len( nodes) > 1:
      for i in range(1,len(nodes)):
         nodesstring += ' ' + nodes[i].name()

   # Construct frame ranges:
   if framefirst_min != framefirst_max: framefirst = '%s..%s' % ( framefirst_min, framefirst_max)
   else: framefirst = framefirst_min
   if framelast_min != framelast_max: framelast = '%s..%s' % ( framelast_min, framelast_max)
   else: framelast = framelast_min
   if framespertask_min != framespertask_max: framespertask = '%s..%s' % ( framespertask_min, framespertask_max)
   else: framespertask = framespertask_min

   # Dialog:
   panel = nuke.Panel('Afanasy Render')
   panel.addSingleLineInput('Nodes:', nodesstring)
   panel.addSingleLineInput('First Frame:', framefirst)
   panel.addSingleLineInput('Last Frame:', framelast)
   panel.addSingleLineInput('Frames Per Task:', framespertask)
   if hasafanasynodes: panel.addBooleanCheckBox('Store Frames Settings', 0)
   panel.addBooleanCheckBox('Start Paused', 0)
   panel.addButton("Cancel")
   panel.addButton("OK")
   result = panel.show()
   if not result: return

   # Check for selected nodes:
   nodesstring = panel.value('Nodes:')
   selectednodes = nodesstring.split()
   nodes = []
   for name in selectednodes:
      node = nuke.toNode( name)
      if node is None:
         nuke.message('Node "%s" not founded.' % name)
         return
      if node.Class() == AfanasyNodeClassName or node.Class() == RenderNodeClassName or node.Class() == DailiesNodeClassName:
         nodes.append( node)

   if len( nodes) < 1:
      nuke.message('No nodes to render founded.\nSelect "%s" or "%s" node(s) to render.' % (AfanasyNodeClassName, RenderNodeClassName))
      return

   # Get parameters:
   sframefirst    = str(panel.value('First Frame:'))
   sframelast     = str(panel.value('Last Frame:'))
   sframespertask = str(panel.value('Frames Per Task:'))
   storeframes = False
   if hasafanasynodes: storeframes = int(panel.value('Store Frames Settings'))
   if panel.value('Start Paused'): fparams['startpaused'] = 1
   # Check frame range was set:
   if sframefirst.find('..') == -1:
      try: framefirst = int(sframefirst)
      except:
         nuke.message('Invalid first frame "%s"' % sframefirst)
         return
      fparams['framefirst'] = framefirst
   if sframelast.find('..') == -1:
      try: framelast = int(sframelast)
      except:
         nuke.message('Invalid last frame "%s"' % sframelast)
         return
      fparams['framelast'] = framelast
   if sframespertask.find('..') == -1:
      try: framespertask = int(sframespertask)
      except:
         nuke.message('Invalid frames per task "%s"' % sframespertask)
         return
      fparams['framespertask'] = framespertask
   if checkFrameRange( framefirst, framelast, 1, framespertask) == False: return

   # Render selected nodes:
   renderNodes( nodes, fparams, storeframes)
