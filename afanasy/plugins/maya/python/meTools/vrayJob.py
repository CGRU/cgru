"""
  vrayJob.py

  ver.0.2.0 13 Jan 2013
  ver.0.1.0 10 Jan 2013
  ver.0.0.1 27 Dec 2012

  Author:

  Yuri Meshalkin (aka mesh)
  mesh@kpp.kiev.ua

  (c) Kiev Post Production 2012

"""
import maya.cmds as cmds
import maya.mel as mel

import af
#
# Abstract VRayJob class
#
class VRayJob ( object ) :
  #
  #
  def __init__( self, name='', description='' ):
    #
    self.dispatcher = None
    self.name = name
    self.description = description

    self.script_name = ''
    self.work_dir = ''

    self.priority = 50
    self.paused = False

    self.animation = True
    self.start = 0
    self.stop = 10
    self.step = 1

    self.padding = 1

    self.task_size = 1
    self.task_number = 1

    self.cmd = 'vray'
    
    self.use_gen_cmd = False
    self.use_local_gen = False
    self.gen_cmd = 'render'
    self.gen_scene_name = ''
    self.gen_task_size = 1
    
    self.vrscene_files = ''
    self.images = ''

    self.use_distributed = False
    self.hosts = ''
    self.min_hosts = 1
    self.max_hosts = 1
    
    self.cleanup = {}
    self.cleanup ['vrscene'] = False
    self.cleanup ['script'] = False
    self.cleanup ['scene'] = False
    
    self.del_cmd = ''
  #
  #
  def begin ( self ) : print '>> VRayJob begin...'
  #
  #
  def once_per_job_tasks ( self ) : print '>> VRayJob once_per_job_tasks...'
  #
  #
  def frame_tasks ( self ) : print '>> VRayJob frame_tasks...'
  #
  #
  def end ( self ) : print '>> VRayJob end...'

#
# VRayAfanasyJob class
# used with Afanasy render manager
#
class VRayAfanasyJob ( VRayJob ) :
  #
  #
  def __init__ ( self, name='', description='' ):
    #
    VRayJob.__init__ ( self, name, description )
    self.dispatcher = 'afanasy'

    self.service = 'vray'
    self.deferred_service = 'mayatovray'

    self.capacity = 1000
    self.deferred_capacity = 1000

    self.use_var_capacity = False
    self.capacity_coeff_min = 1.0 # block.setVariableCapacity( capmin, capmax )
    self.capacity_coeff_max = 1.0

    self.tasks_max_run_time = -1 #??
    self.max_running_tasks = -1
    self.max_running_tasks_per_host = -1

    self.hostsmask = ''
    self.hostsexcl = ''
    self.depmask = ''
    self.depglbl = ''
    self.need_os = ''

    # Pre Command - Command to execute on job registration.
    # Note, that this command is executed by server, and not from tasks working directory.
    # Use absolute paths here or even transfer paths if you server has another file system than renders.
    # Commands are executed in a special thread with commands queue.
    # This means if somebody executes 'sleep 1000', other commands execution (and jobs registration)
    # will be delayed on 1000 seconds (only delayed, not lost).
    # Try not use Pre Command at all. You always can create one more task(block) and make other tasks(blocks) depend on it.
    self.command_pre = ''

    # Post Command - Command executed on job deletion.
    # This commands are executed on render farm hosts by special system job.
    # Working directory of such system task will be the first block working folder.
    self.command_post = ''

    #self.consolidate_subtasks = False

    self.job = None
    self.gen_block = None
    self.once_block = None
    self.frame_block = None

    self.frame_subtasks = []
    self.frame_blocks = []
  #
  #
  def begin ( self ) :
    print '>> AfanasyJob begin...'

    self.job = af.Job ( self.name )
    self.job.setDescription ( self.description )
    self.job.setPriority ( self.priority )

    if self.hostsmask != '': self.job.setHostsMask ( self.hostsmask )
    if self.hostsexcl != '': self.job.setHostsMaskExclude ( self.hostsexcl )
    if self.depmask   != '': self.job.setDependMask ( self.depmask )
    if self.depglbl   != '': self.job.setDependMaskGlobal( self.depglbl )

    if self.need_os   != '': self.job.setNeedOS ( self.need_os )
  #
  #
  def frame_tasks ( self ) :
    print '>> AfanasyJob frame_tasks...'

    if self.use_gen_cmd :
      self.gen_block = af.Block ( 'generate_vrscene', self.deferred_service )
      self.gen_block.setWorkingDirectory ( self.work_dir )
      self.gen_block.setCapacity ( self.deferred_capacity )
      self.gen_block.setCommand ( self.gen_cmd + ' ' + self.gen_scene_name )
      self.gen_block.setNumeric ( self.start, self.stop, self.gen_task_size, self.step )
      if self.use_local_gen :
        self.gen_block.setHostsMask ( self.job.data['host_name'] ) 
        
    self.frame_block = af.Block ( 'render', self.service )
    self.frame_block.setWorkingDirectory ( self.work_dir )
    self.frame_block.setCapacity ( self.capacity )
    if self.use_var_capacity :
      self.frame_block.setVariableCapacity ( self.capacity_coeff_min, self.capacity_coeff_max )

    self.frame_block.setCommand ( self.cmd + ' -sceneFile="' + self.vrscene_files + '"' )
    self.frame_block.setFiles ( self.images )
    self.frame_block.setNumeric ( self.start, self.stop, self.task_size, self.step )
  #
  #
  def end ( self ) :
    print '>> AfanasyJob end...'
    gen_block_exists = False
    
    if self.gen_block is not None :
      gen_block_exists = True
      self.job.blocks.append ( self.gen_block )

    if self.frame_block is not None :
      if gen_block_exists :
        self.frame_block.setTasksDependMask ( 'generate_vrscene.*' )
      self.job.blocks.append ( self.frame_block )

    if self.paused : self.job.offline()

    self.job.output( 1 )
    self.job.send()
#
# VRayBackburnerJob class
# used with Backburner render manager
#
class VRayBackburnerJob ( VRayJob ) :
  #
  #
  def __init__ ( self, name='', description='' ):
    #
    VRayJob.__init__ ( self, name, description )
    self.dispatcher = 'backburner'

    self.deferredGen = False
    self.deferredBlock = 1

    self.cmdjob_path = '/usr/discreet/backburner/cmdjob'
    self.manager = ''
    self.server_list = ''
    self.server_group = ''
    self.server_count = 0

    self.port = 0
    self.use_workpath = False
    self.create_log = False
    self.log_dir = '/var/tmp'
    self.use_tasklist = False
    self.tasklist = 'tasklist.txt'
    self.use_job_param_file = False
    self.job_param_file = 'jobParamFile.txt'

    self.use_remote = False
    self.dirmap_from = ''
    self.dirmap_to = ''

  #
  #
  def begin ( self ) :
    print '>> BackburnerJob begin...'
  #
  #
  def once_per_job_tasks ( self ) :
    print '>> BackburnerJob once_per_job_tasks...'
  #
  #
  def frame_tasks ( self ) :
    print '>> BackburnerJob frame_tasks...'
  #
  #
  def end ( self ) :
    print '>> BackburnerJob end...'
