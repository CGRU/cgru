"""
  renderJob.py

  ver.0.1.0 (17 Jan 2013)

  Author:

  Yuri Meshalkin (aka mesh)
  mesh@kpp.kiev.ua

  (c) Kiev Post Production 2013

"""
#
# Abstract RenderBlock class
#
class RenderBlock ( object ) :
  #
  #
  def __init__( self, name = '', service = None ):
    #
    self.name = name
    self.service = service
    self.work_dir = ''
    
    self.start = 0
    self.stop = 10
    self.step = 1
    self.task_size = 1
    
    self.cmd = ''
    self.input_files = ''
    self.out_files = ''
    
    cleanup = False
  #
  # virtual methods
  #
  def setup ( self )   : print '>> RenderBlock setup...'
#
# Abstract RenderJob class
#
class RenderJob ( object ) :
  #
  #
  def __init__( self, name='', description='' ):
    #
    self.dispatcher = None
    self.name = name
    self.description = description
    self.work_dir = ''
    self.priority = 50
    self.paused = False
    self.padding = 1
    self.animation = True
    self.start = 0
    self.stop = 10
    self.step = 1
    self.task_size = 1
    self.num_tasks = 1
    self.cleanup_cmd = None
    
    self.gen_block = None
    self.frames_blocks = []
  #
  # virtual methods
  #
  def setup ( self )   : print '>> RenderJob setup...'
  def process ( self ) : print '>> RenderJob process...'
  #
  # setup_range
  #
  def setup_range ( self, currentTime = 0 ) :
    if not self.animation :
      self.start = currentTime
      self.stop = self.start
      self.step = 1
    self.num_tasks = (self.stop - self.start) / self.step + 1
    self.task_size = min( self.task_size, self.num_tasks ) 

