"""
  afanasyRenderJob.py

  ver.0.1.0 (28 Jan 2013)
  ver.0.0.1 (16 Jan 2013)

  Author:

  Yuri Meshalkin (aka mesh)
  mesh@kpp.kiev.ua

  (c) Kiev Post Production 2012

"""
import af
from renderJob import RenderJob, RenderBlock
#
# AfanasyRenderBlock class
# used with Afanasy render manager
#
class AfanasyRenderBlock ( RenderBlock ) :
  #
  # __init__
  #
  def __init__ ( self, name = '', service = None, parentJob = None, local = False ) :
    #
    RenderBlock.__init__ ( self, name, service )
    self.af_block = af.Block ( name, service )
    self.isNumeric = True
    self.capacity = 1000
    self.hostsmask = ''
    self.parentJob = parentJob
    
    self.distributed = False
    self.sameHostMaster = False
    self.hosts_min = 0
    self.hosts_max = 0
    
    if parentJob is not None :
      # setup general parameters from parent job
      self.start = parentJob.start
      self.stop = parentJob.stop
      self.step = parentJob.step
      self.task_size = parentJob.task_size
      self.af_block.setWorkingDirectory ( parentJob.work_dir )
      if local :
        self.af_block.setHostsMask ( parentJob.af_job.data [ 'host_name' ] ) 
  #
  # setup
  #
  def setup ( self ) :
    # 
    self.af_block.setCapacity ( self.capacity )
    if self.parentJob is not None :
      if self.parentJob.use_var_capacity :
        self.af_block.setVariableCapacity ( self.parentJob.capacity_coeff_min, self.parentJob.capacity_coeff_max )      
    if self.distributed :
      self.af_block.setMultiHost ( self.hosts_min, self.hosts_max, 0, self.sameHostMaster, '', 0 )  
    self.af_block.setCommand ( self.cmd + ' ' + self.input_files )
    self.af_block.setFiles ( self.out_files )
    if self.isNumeric :
      self.af_block.setNumeric ( self.start, self.stop, self.task_size, self.step )  
#
# AfanasyRenderJob class
# used with Afanasy render manager
#
class AfanasyRenderJob ( RenderJob ) :
  #
  #
  def __init__ ( self, name='', description='' ):
    #
    RenderJob.__init__ ( self, name, description )
    self.af_job = af.Job ( name )
    
    self.dispatcher = 'afanasy'
    
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
    
    self.distributed = False
    self.sameHostMaster = True
    self.hosts_min = 0
    self.hosts_max = 0
    self.threads_limit = 0
    self.port = 39010 # mentalray standalone slave port
    self.hosts = ''
  #    
  # setup
  #
  def setup ( self ) :
    print '>> AfanasyRenderJob setup...'
    self.af_job.setDescription ( self.description )
    self.af_job.setPriority ( self.priority )
    if self.hostsmask != '' : self.af_job.setHostsMask ( self.hostsmask )
    if self.hostsexcl != '' : self.af_job.setHostsMaskExclude ( self.hostsexcl )
    if self.depmask   != '' : self.af_job.setDependMask ( self.depmask )
    if self.depglbl   != '' : self.af_job.setDependMaskGlobal( self.depglbl )
    if self.need_os   != '' : self.af_job.setNeedOS ( self.need_os )
  #
  # process
  #
  def process ( self ) :
    print '>> AfanasyJob process...'
    gen_block_exists = False
    if self.gen_block is not None :
      gen_block_exists = True
      self.af_job.blocks.append ( self.gen_block.af_block )
      
    for frame_block in self.frames_blocks :
      if gen_block_exists :
        frame_block.af_block.setTasksDependMask ( self.gen_block.name )
      self.af_job.blocks.append ( frame_block.af_block )

    if self.paused : self.af_job.offline()

    self.af_job.output( 1 )
    self.af_job.send()
