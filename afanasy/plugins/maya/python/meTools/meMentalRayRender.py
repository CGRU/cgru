"""
  meMentalRayRender
  
  ver.0.4.2 (23 Jan 2013) 
    - fixed minor problems with render layers
  ver.0.4.1 (18 Jan 2013)
  ver.0.4.0 (17 Jan 2013)
    - added render layers support
    - new RenderJob class
    - new AfanasyRenderJob class
    - some procedures moved to maya_ui_proc.py
  
  ver.0.3.9 (13 Jan 2013)
    - added support for deferred .mi generation
    - fixed handling of image names and padding

  ver.0.3.8 (27 Dec 2012)
    - job specific functions moved to mrayJob class
    - backburner support dropped (temporary)
    - enchanced Afanasy jobs support
    - prepared support for deferred .mi generation

  ver.0.3.7 (22 Aug 2012)
    - backburner specific control fields are moved
      to corresponded tab, some unused parameters removed

  ver.0.3.6 (9 Jul 2012)
    - attempt to add Afanasy support

  ver.0.3.5 (21 Mar 2012)
  ver.0.3.4 (16 Mar 2012)
  ver.0.3.3 (18 Nov 2011)
  ver.0.3.1 (14 Nov 2011)

  Author:

  Yuri Meshalkin (aka mesh)
  mesh@kpp.kiev.ua

  (c) Kiev Post Production 2011, 2012, 2013

  Description:

  This UI script generates .mi files from open Maya scene
  and submits them to mentalray Standalone directly or by
  creating a job for backburner or Afanasy.

  "Use Remote Render" mode allows you to submit backburner job
  from your local OS to render-farm with other OS (e.g. linux)
  by using Directory Mapping.


  Usage:
  You can add this code as shelf button :
   
  import meTools.meMentalRayRender as meMentalRayRender
  reload( meMentalRayRender )
  meMentalRayRender = meMentalRayRender.meMentalRayRender()
  
  Important!!! 
  Do not use another object name. 
  Only meMentalRayRender.renderLayerSelected() will work with script job.

  For using with Afanasy, add %AF_ROOT%\plugins\maya\python to %PYTHONPATH%
  
"""
import sys, os, string
import maya.OpenMaya as OpenMaya
import maya.cmds as cmds
import maya.mel as mel
from functools import partial

from maya_ui_proc import *
from afanasyRenderJob import *

self_prefix = 'meMentalRayRender_'
meMentalRayRenderVer = '0.4.2'
meMentalRayRenderMainWnd = self_prefix + 'MainWnd'
#
# meMentalRayRender
#
class meMentalRayRender ( object ):
  #
  #
  #
  def __init__ ( self, selection = '' ):
    #
    self.selection = selection
    self.winMain = ''

    self.os = sys.platform
    if self.os.startswith ( 'linux' ) : self.os = 'linux'
    elif self.os == 'darwin'          : self.os = 'mac'
    elif self.os == 'win32'           : self.os = 'win'

    print 'sys.platform = %s self.os = %s' % ( sys.platform, self.os )

    self.rootDir = cmds.workspace ( q = True, rootDirectory = True )
    self.rootDir = self.rootDir[ :-1 ]
    self.layer = cmds.editRenderLayerGlobals ( query = True, currentRenderLayer = True )

    self.job = None

    self.job_param     = {}
    self.mi_param      = {}
    self.mr_param      = {}
    self.img_param     = {}
    self.afanasy_param = {}
    self.bkburn_param  = {}

    self.migenCommand     = 'Mayatomr -miStream'
    self.def_migenCommand = 'Render -r mi'
    self.def_scene_name   = '' # maya scene name used for deferred .mi generation

    self.initParameters ()
    self.ui = self.setupUI ()
  #
  #
  #def __del__( self ): print( ">> meMentalRayRender: Class deleted" )
  #
  #
  def initParameters ( self ): 
    #
    # Job parameters
    #
    self.job_param [ 'job_dispatcher' ]  = getDefaultStrValue ( self_prefix, 'job_dispatcher', 'afanasy' )
    self.job_param [ 'job_name' ]        = getMayaSceneName ()
    self.job_param [ 'job_description' ] = getDefaultStrValue ( self_prefix, 'job_description', '' )
    self.job_param [ 'job_animation' ]   = getDefaultIntValue ( self_prefix, 'job_animation', 1 ) is 1
    self.job_param [ 'job_start' ]       = getDefaultIntValue ( self_prefix, 'job_start', 1 )
    self.job_param [ 'job_end' ]         = getDefaultIntValue ( self_prefix, 'job_end', 100 )
    self.job_param [ 'job_step' ]        = getDefaultIntValue ( self_prefix, 'job_step', 1 )
    self.job_param [ 'job_size' ]        = getDefaultIntValue ( self_prefix, 'job_size', 1 )
    self.job_param [ 'job_paused' ]      = getDefaultIntValue ( self_prefix, 'job_paused', 1 ) is 1
    self.job_param [ 'job_priority' ]    = getDefaultIntValue ( self_prefix, 'job_priority', 50 )
    self.job_param [ 'job_cleanup_mi' ]  = getDefaultIntValue ( self_prefix, 'job_cleanup_mi', 0 ) is 1
    self.job_param [ 'job_cleanup_script' ] = getDefaultIntValue ( self_prefix, 'job_cleanup_script', 0 ) is 1
    #
    # .mi generation parameters
    #
    self.mi_param [ 'mi_reuse' ]         = getDefaultIntValue ( self_prefix, 'mi_reuse', 1 ) is 1
    self.mi_param [ 'mi_filename' ]      = 'mi/' + getMayaSceneName () + '.mi' # self.getDefaultStrValue( 'mi_filename',
    self.mi_param [ 'mi_padding' ]       = getDefaultIntValue ( self_prefix, 'mi_padding', 4 )
    self.mi_param [ 'mi_perframe' ]      = getDefaultIntValue ( self_prefix, 'mi_perframe', 1 ) is 1
    self.mi_param [ 'mi_selection' ]     = getDefaultIntValue ( self_prefix, 'mi_selection', 0 ) is 1
    self.mi_param [ 'mi_filepaths' ]     = getDefaultStrValue ( self_prefix, 'mi_filepaths', 'Relative' )
    self.mi_param [ 'mi_binary' ]        = getDefaultIntValue ( self_prefix, 'mi_binary', 0 ) is 1
    self.mi_param [ 'mi_tabstop' ]       = getDefaultIntValue ( self_prefix, 'mi_tabstop', 2 )
    self.mi_param [ 'mi_verbosity' ]     = getDefaultStrValue ( self_prefix, 'mi_verbosity', 'none' )
    self.mi_param [ 'mi_deferred' ]      = getDefaultIntValue ( self_prefix, 'mi_deferred', 0 ) is 1
    self.mi_param [ 'mi_local_migen' ]   = getDefaultIntValue ( self_prefix, 'mi_local_migen', 1 ) is 1
    self.mi_param [ 'mi_def_task_size' ] = getDefaultIntValue ( self_prefix, 'mi_def_task_size', 4 )
    self.mi_param [ 'mi_export_all_layers' ] = getDefaultIntValue ( self_prefix, 'mi_export_all_layers', 0 ) is 1
    #self.mi_param['mi_compression'] = self.getDefaultStrValue( 'mi_compression', 'Off' )
    #
    # mentalray parameters
    #
    self.mr_param [ 'mr_options' ]            = getDefaultStrValue ( self_prefix, 'mr_options', '' )
    self.mr_param [ 'mr_verbosity' ]          = getDefaultStrValue ( self_prefix, 'mr_verbosity', 'none' )
    self.mr_param [ 'mr_progress_frequency' ] = getDefaultIntValue ( self_prefix, 'mr_progress_frequency', 1 )
    self.mr_param [ 'mr_threads' ]            = getDefaultIntValue ( self_prefix, 'mr_threads', 4 )
    self.mr_param [ 'mr_texture_continue' ]   = getDefaultIntValue ( self_prefix, 'mr_texture_continue', 1 )
    #
    # image parameters
    #
    self.img_param [ 'img_filename' ] = self.getImageFileNamePrefix ()
    self.img_param [ 'img_format' ]   = self.getImageFormat ()
    #
    # Afanasy parameters
    #
    self.afanasy_param [ 'af_capacity' ]           = getDefaultIntValue ( self_prefix, 'af_capacity', 1000 )
    self.afanasy_param [ 'af_deferred_capacity' ]  = getDefaultIntValue ( self_prefix, 'af_deferred_capacity', 1000 )
    self.afanasy_param [ 'af_use_var_capacity' ]   = getDefaultIntValue ( self_prefix, 'af_use_var_capacity', 0 ) is 1
    self.afanasy_param [ 'af_cap_min' ]            = getDefaultFloatValue ( self_prefix, 'af_cap_min', 1.0 )
    self.afanasy_param [ 'af_cap_max' ]            = getDefaultFloatValue ( self_prefix, 'af_cap_max', 1.0 )
    self.afanasy_param [ 'af_max_running_tasks' ]  = getDefaultIntValue ( self_prefix, 'af_max_running_tasks', -1 )
    self.afanasy_param [ 'af_max_tasks_per_host' ] = getDefaultIntValue ( self_prefix, 'af_max_tasks_per_host', -1 )
    self.afanasy_param [ 'af_service' ]            = getDefaultStrValue ( self_prefix, 'af_service', 'mentalray' )
    self.afanasy_param [ 'af_deferred_service' ]   = getDefaultStrValue ( self_prefix, 'af_deferred_service', 'mayatomr' )
    self.afanasy_param [ 'af_os' ]                 = getDefaultStrValue ( self_prefix, 'af_os', '' ) #linux mac windows
    self.afanasy_param [ 'af_hostsmask' ]          = getDefaultStrValue ( self_prefix, 'af_hostsmask', '.*' )
    self.afanasy_param [ 'af_hostsexcl' ]          = getDefaultStrValue ( self_prefix, 'af_hostsexcl', '' )
    self.afanasy_param [ 'af_depmask' ]            = getDefaultStrValue ( self_prefix, 'af_depmask', '' )
    self.afanasy_param [ 'af_depglbl' ]            = getDefaultStrValue ( self_prefix, 'af_depglbl', '' )
  #
  # getImageFileNamePrefix
  #
  def getImageFileNamePrefix ( self ):
    fileNamePrefix = cmds.getAttr ( 'defaultRenderGlobals.imageFilePrefix' )
    if fileNamePrefix == None or fileNamePrefix == '' :
      fileNamePrefix = getMayaSceneName ()
    return fileNamePrefix
  #
  # getImageFormat
  #
  def getImageFormat ( self ):
    imageFormatStr = ''
    format_idx = cmds.getAttr ( 'defaultRenderGlobals.imageFormat' )
    if format_idx == 0 : # Gif
      imageFormatStr = 'iff'
    elif format_idx == 1: # Softimage (pic)
      imageFormatStr = 'pic'
    elif format_idx == 2: # Wavefront (rla)
      imageFormatStr = 'rla'
    elif format_idx == 3 : # Tiff
      imageFormatStr = 'tif'
    elif format_idx == 4 : # Tiff16
      imageFormatStr = 'iff'
    elif format_idx == 5 : # SGI (rgb)
      imageFormatStr = 'rgb'
    elif format_idx == 6 : # Alias (pix)
      imageFormatStr = 'pix'
    elif format_idx == 7 : # Maya IFF (iff)
      imageFormatStr = 'iff'
    elif format_idx == 8 : # JPEG (jpg)
      imageFormatStr = 'jpg'
    elif format_idx == 9 : # EPS (eps)
      imageFormatStr = 'eps'
    elif format_idx == 10 : # Maya 16 IFF (iff)
      imageFormatStr = 'iff'
    elif format_idx == 11 : # Cineon
      imageFormatStr = 'iff'
    elif format_idx == 12 : # Quantel PAL (yuv)
      imageFormatStr = 'yuv'
    elif format_idx == 13 : # SGI 16
      imageFormatStr = 'iff'
    elif format_idx == 19 : # Targa (tga)
      imageFormatStr = 'tga'
    elif format_idx == 20 : # Windows Bitmap (bmp)
      imageFormatStr = 'bmp'
    elif format_idx == 21 : # SGI Movie
      imageFormatStr = 'iff'
    elif format_idx == 22 : # Quicktime
      imageFormatStr = 'iff'
    elif format_idx == 23 : # AVI
      imageFormatStr = 'iff'
    elif format_idx == 30 : # MacPaint
      imageFormatStr = 'iff'
    elif format_idx == 31 : # PSD
      imageFormatStr = 'psd'
    elif format_idx == 32 : # PNG
      imageFormatStr = 'png'
    elif format_idx == 33 : # QuickDraw
      imageFormatStr = 'iff'
    elif format_idx == 34 : # QuickTime Image
      imageFormatStr = 'iff'
    elif format_idx == 35 : # DDS
      imageFormatStr = 'iff'
    elif format_idx == 36 : # PSD Layered
      imageFormatStr = 'psd'
    elif format_idx == 50 : # IMF plugin
      imageFormatStr = 'iff'
    elif format_idx == 51 : # Custom Image Format
      imageFormatStr = 'tif'
    elif format_idx == 60 : # Macromedia SWF (swf)
      imageFormatStr = 'iff'
    elif format_idx == 61 : # Adobe Illustrator (ai)
      imageFormatStr = 'iff'
    elif format_idx == 62 : # SVG (svg)
      imageFormatStr = 'iff'
    elif format_idx == 63 : # Swift3DImporter (swft)
      imageFormatStr = 'iff'
    return imageFormatStr
  #
  # getDeferredCmd
  #
  def getDeferredCmd ( self, layer = None  ) :
    #
    gen_cmd = self.def_migenCommand + ' '
    gen_cmd += '-proj ' + '"' + self.rootDir + '"' + ' '
    if layer is not None : gen_cmd += '-rl ' + layer + ' '
    else                 : gen_cmd += '-perlayer 1 '  
    gen_cmd += self.get_migen_options ( layer )
    return gen_cmd
  #
  # getRenderCmd
  #
  def getRenderCmd ( self, layer = None  ) :
    #
    verbosity = [ 'none', 'fatal', 'error', 'warning', 'info', 'progress', 'debug', 'details' ]
    mr_verbosity_level    = verbosity.index ( self.mr_param [ 'mr_verbosity' ] )
    options               = str ( self.mr_param [ 'mr_options' ] ).strip ()
    mr_threads            = self.mr_param [ 'mr_threads' ]
    mr_progress_frequency = self.mr_param [ 'mr_progress_frequency' ]
    mr_texture_continue   = self.mr_param [ 'mr_texture_continue' ] 

    if mr_verbosity_level < 5     : mr_verbosity_level = 5
    if mr_progress_frequency == 0 : mr_progress_frequency = 1
 
    cmd = 'ray '
    if mr_verbosity_level    != 0 : cmd += '-v ' + str ( mr_verbosity_level ) + ' '
    if mr_progress_frequency != 0 : cmd += '-progress_frequency ' + str ( mr_progress_frequency ) + ' '
    if mr_threads            != 0 : cmd += '-threads ' + str ( mr_threads ) + ' '
    if mr_texture_continue   != 0 : cmd += '-texture_continue on'  + ' '
    
    cmd += '-file_dir "' + getDirName ( self.get_image_names () ) + '" '
    #cmd += '-file_name "' + self.getImageFileNamePrefix() + '" ' 
    #cmd += '-file_type ' + self.getImageFormat() + ' '

    cmd += options
    return cmd
  #
  # get_mi_file_names
  #
  def get_mi_file_names ( self, layer = None ) :
    #
    filename = cmds.workspace ( expandName = self.mi_param [ 'mi_filename' ] )
    ( name, ext ) = os.path.splitext ( filename )
    if layer is not None : name += '_' + layer
    #dot_pos = name.rfind('.')
    #miFileName = name[0:dot_pos] + '.' + ('@' + pad_str + '@') + '.mi'
    pad_str = getPadStr ( self.mi_param [ 'mi_padding' ], self.mi_param [ 'mi_perframe' ] )
    return ( name + '.' + ('@' + pad_str + '@') + '.mi' )
  #
  # get_image_names
  #
  def get_image_names ( self ) :
    #
    pad_str = getPadStr ( self.mi_param [ 'mi_padding' ], self.mi_param [ 'mi_perframe' ] )
    images = cmds.renderSettings( fullPath = True, genericFrameImageName = ('@' + pad_str + '@')  )
    #imageFileName = ';'.join ( images )
    imageFileName = str ( images[0] )
    return fromNativePath ( imageFileName )
  #
  # get_migen_options
  #
  def get_migen_options ( self, layer = None ) :
    #
    animation    = self.job_param [ 'job_animation' ]
    start        = self.job_param [ 'job_start' ]
    stop         = self.job_param [ 'job_end' ]
    step         = self.job_param [ 'job_step' ]
    mi_reuse     = self.mi_param [ 'mi_reuse' ]
    mi_selection = self.mi_param [ 'mi_selection' ]
    mi_filename  = self.mi_param [ 'mi_filename' ]
    mi_padding   = self.mi_param [ 'mi_padding' ]
    mi_perframe  = self.mi_param [ 'mi_perframe' ]
    mi_tabstop   = self.mi_param [ 'mi_tabstop' ]
    mi_binary    = self.mi_param [ 'mi_binary' ]
    mi_filepaths = self.mi_param [ 'mi_filepaths' ]
    mi_deferred  = self.mi_param [ 'mi_deferred' ]
    # mi_compression = cmds.optionMenuGrp( mi + 'mi_compression', q=True, value=True )
    # mi_verbosity_level = cmds.optionMenuGrp( mi + 'mi_verbosity', q=True, sl=True ) - 1
    migen_cmd = ''
    filename = cmds.workspace( expandName = mi_filename )
    ( filename, ext ) = os.path.splitext ( filename )
    if ext == '' or ext == '.' : ext = '.mi'
    if layer is not None :
      if not mi_deferred :
        # do not add layer name to filename for deferred generation 
        # this will be done by Maya
        layer_in_filename = layer
        if layer == 'defaultRenderLayer' : layer_in_filename = 'masterLayer'
        filename += '_' + layer_in_filename
        # this flag doesn't work with deferred generation
        migen_cmd += '-layer ' + layer + ' '  
    filename += ext
    migen_cmd += '-file \"' + filename + '\" '
    #if mi_verbosity_level < 5 : mi_verbosity_level = 5
    #migen_cmd += '-v ' + str( mi_verbosity_level ) + ' '
    migen_cmd += '-perframe '
    if mi_perframe :
      migen_cmd += '2 ' # (name.#.ext)
      migen_cmd += '-padframe ' + str ( mi_padding ) + ' '
    else:
      migen_cmd += '0 ' # (single .mi file)

    if mi_binary :
      if mi_deferred : migen_cmd += '-binary 1 '
      else           : migen_cmd += '-binary '
    else:
      migen_cmd += '-tabstop ' + str ( mi_tabstop ) + ' '
    #migen_cmd += '-pcm ' # export pass contribition maps
    #migen_cmd += '-pud ' # export pass user data
    if mi_deferred :
      migen_cmd += '-exportPathNames "' 
      if mi_filepaths == 'NoChange'   : migen_cmd += '3300000000'  
      elif mi_filepaths == 'Absolute' : migen_cmd += '3311111111'
      elif mi_filepaths == 'Relative' : migen_cmd += '3322222222'
      else                            : migen_cmd += '3333333333'
      migen_cmd += '" '
    else :
      migen_cmd += '-exportPathNames '
      if mi_filepaths == 'NoChange'   : migen_cmd += '0000000000'  
      elif mi_filepaths == 'Absolute' : migen_cmd += 'nnaaaaaaaa'
      elif mi_filepaths == 'Relative' : migen_cmd += 'nnrrrrrrrr'
      else                            : migen_cmd += 'nnnnnnnnnn'
      migen_cmd += ' '
      if mi_selection : migen_cmd += '-active '
  
    return migen_cmd
  #
  # generate_mi
  #
  def generate_mi ( self, isSubmitingJob = False ):
    #
    skipExport = False
    exportAllRenderLayers = self.mi_param [ 'mi_export_all_layers' ]

    animation    = self.job_param [ 'job_animation' ]
    start        = self.job_param [ 'job_start' ]
    stop         = self.job_param [ 'job_end' ]
    step         = self.job_param [ 'job_step' ]
    mi_reuse     = self.mi_param [ 'mi_reuse' ]
    mi_selection = self.mi_param [ 'mi_selection' ]
    mi_filename  = self.mi_param [ 'mi_filename' ]
    mi_padding   = self.mi_param [ 'mi_padding' ]
    mi_perframe  = self.mi_param [ 'mi_perframe' ]
    mi_tabstop   = self.mi_param [ 'mi_tabstop' ]
    mi_binary    = self.mi_param [ 'mi_binary' ]
    mi_filepaths = self.mi_param [ 'mi_filepaths' ]
    mi_deferred  = self.mi_param [ 'mi_deferred' ]
    # mi_compression = cmds.optionMenuGrp( mi + 'mi_compression', q=True, value=True )
    # mi_verbosity_level = cmds.optionMenuGrp( mi + 'mi_verbosity', q=True, sl=True ) - 1

    filename = cmds.workspace ( expandName = mi_filename )
    ( filename, ext ) = os.path.splitext ( filename )
    if ext == '' or ext == '.' : ext = '.mi'
    filename += ext

    dirname = os.path.dirname ( filename )
    if not os.path.exists ( dirname ):
      print ( "path %s not exists" ) % dirname
      os.mkdir ( dirname )

    # TODO!!! check if files are exist and have to be overriden
    if isSubmitingJob and mi_reuse:
      skipExport = True
      print "Skipping .mi files generation ..."

    if not skipExport:
      if not animation :
        start = stop = int ( cmds.currentTime ( q=True ) )
        step = 1
      #
      # save RenderGlobals
      #
      defGlobals = 'defaultRenderGlobals'
      saveGlobals = {}
      saveGlobals [ 'animation' ]       = cmds.getAttr( defGlobals + '.animation' )
      saveGlobals [ 'startFrame' ]      = cmds.getAttr( defGlobals + '.startFrame' )
      saveGlobals [ 'endFrame' ]        = cmds.getAttr( defGlobals + '.endFrame' )
      saveGlobals [ 'byFrameStep' ]     = cmds.getAttr( defGlobals + '.byFrameStep' )
      saveGlobals [ 'extensionPadding'] = cmds.getAttr( defGlobals + '.extensionPadding' )
      
      #
      # override RenderGlobals
      #
      cmds.setAttr( defGlobals + '.animation', True ) # True even for single frame, for proper image name
      cmds.setAttr( defGlobals + '.startFrame', start )
      cmds.setAttr( defGlobals + '.endFrame', stop )
      cmds.setAttr( defGlobals + '.byFrameStep', step )
      cmds.setAttr( defGlobals + '.extensionPadding', mi_padding ) # set images padding same as .mi
      
      image_name = self.getImageFileNamePrefix()

      if mi_deferred :
      # generate uniquie maya scene name and save it
      # with current render and .mi generation settings
        print 'Use deferred .mi generation'
        saveGlobals['imageFilePrefix'] = str ( cmds.getAttr ( defGlobals + '.imageFilePrefix' ) )

        cmds.setAttr( defGlobals + '.imageFilePrefix', image_name, type = 'string' )
        scene_name     = getMayaSceneName () # get scene name without extension
        def_scene_name = scene_name + '_deferred'
        cmds.file ( rename = def_scene_name )
        self.def_scene_name = cmds.file ( save = True, de = True ) # save it with default extension
        cmds.file ( rename = scene_name ) # rename scene back

        cmds.setAttr ( defGlobals + '.imageFilePrefix', saveGlobals [ 'imageFilePrefix' ], type = 'string' )
      else :
        renderLayers = []
        # save current layer
        current_layer = cmds.editRenderLayerGlobals ( q = True, currentRenderLayer = True )
        if exportAllRenderLayers :
          renderLayers = getRenderLayersList ( True ) # renderable only 
        else :
          # use only current layer
          renderLayers.append ( current_layer )
        
        for layer in renderLayers :   
        
          #if layer == 'masterLayer' : layer = 'defaultRenderLayer' 
          saveGlobals ['renderableLayer'] = cmds.getAttr ( layer + '.renderable' ) 
          cmds.setAttr ( layer + '.renderable', True )
          # print 'set current layer renderable (%s)' % layer

          cmds.editRenderLayerGlobals ( currentRenderLayer = layer )  
          
          migen_cmd = self.migenCommand + ' ' + self.get_migen_options ( layer )
          migen_cmd += '-pcm ' # export pass contribition maps
          migen_cmd += '-pud ' # export pass user data
          
          #cmds.setAttr( defGlobals + '.imageFilePrefix', image_name, type='string' ) # will use MayaSceneName if empty 
          
          print "migen_cmd = %s" % migen_cmd
          mel.eval ( migen_cmd )
          
          cmds.setAttr ( layer + '.renderable', saveGlobals [ 'renderableLayer' ] )
        
        if exportAllRenderLayers :
          # restore current layer
          cmds.editRenderLayerGlobals ( currentRenderLayer = current_layer ) 
      #
      # restore RenderGlobals
      #
      cmds.setAttr ( defGlobals + '.animation',        saveGlobals [ 'animation' ] )
      cmds.setAttr ( defGlobals + '.startFrame',       saveGlobals [ 'startFrame' ] )
      cmds.setAttr ( defGlobals + '.endFrame',         saveGlobals [ 'endFrame' ] )
      cmds.setAttr ( defGlobals + '.byFrameStep',      saveGlobals [ 'byFrameStep' ] )
      cmds.setAttr ( defGlobals + '.extensionPadding', saveGlobals [ 'extensionPadding' ] )
  #
  # submitJob
  #
  def submitJob( self, param=None ):
    #
    job_dispatcher   = self.job_param [ 'job_dispatcher' ]
    job_description  = self.job_param [ 'job_description' ]
    job_name         = str( self.job_param [ 'job_name' ] ).strip()
    if job_name == '' : job_name = getMayaSceneName ()
    
    mi_deferred      = self.mi_param [ 'mi_deferred' ]
    mi_local_migen   = self.mi_param [ 'mi_local_migen' ]
    mi_def_task_size = self.mi_param [ 'mi_def_task_size' ]
    mi_reuse         = self.mi_param [ 'mi_reuse' ]
    exportAllRenderLayers = self.mi_param [ 'mi_export_all_layers' ]

    if job_dispatcher == 'afanasy' :
      self.job = AfanasyRenderJob ( job_name, job_description )
      
      self.job.use_var_capacity   = self.afanasy_param [ 'af_use_var_capacity' ]
      self.job.capacity_coeff_min = self.afanasy_param [ 'af_cap_min' ]
      self.job.capacity_coeff_max = self.afanasy_param [ 'af_cap_max' ]
      self.job.max_running_tasks  = self.afanasy_param [ 'af_max_running_tasks' ]
      self.job.max_tasks_per_host = self.afanasy_param [ 'af_max_tasks_per_host' ]
      
      self.job.hostsmask          = str ( self.afanasy_param [ 'af_hostsmask' ] ).strip ()
      self.job.hostsexcl          = str ( self.afanasy_param [ 'af_hostsexcl' ] ).strip ()
      self.job.depmask            = str ( self.afanasy_param [ 'af_depmask' ] ).strip ()
      self.job.depglbl            = str ( self.afanasy_param [ 'af_depglbl' ] ).strip ()
      self.job.need_os            = str ( self.afanasy_param [ 'af_os' ] ).strip ()
      
      service            = str ( self.afanasy_param [ 'af_service'] ).strip ()
      deferred_service   = str ( self.afanasy_param [ 'af_deferred_service' ] ).strip ()
      
      capacity           = self.afanasy_param [ 'af_capacity' ]
      deferred_capacity  = self.afanasy_param [ 'af_deferred_capacity' ]
      
    elif job_dispatcher == 'backburner' :
      print 'backburner not supported in this version'
      #self.job = MentalRayBackburnerJob ( job_name, job_description )
      return
    else :
      mi_deferred = False
      self.job = RenderJob ( job_name, job_description )
    
    self.job.work_dir  = self.rootDir
    self.job.padding   = self.mi_param [ 'mi_padding' ]
    self.job.priority  = self.job_param [ 'job_priority' ]
    self.job.paused    = self.job_param [ 'job_paused' ]
    self.job.task_size = self.job_param [ 'job_size' ]
    self.job.animation = self.job_param [ 'job_animation' ]
    self.job.start     = self.job_param [ 'job_start' ]
    self.job.stop      = self.job_param [ 'job_end' ]
    self.job.step      = self.job_param [ 'job_step' ]
    
    self.generate_mi ( True ) # isSubmitingJob=True
    
    self.job.setup_range ( int ( cmds.currentTime ( q = True ) ) )
    self.job.setup()
    
    # save current layer
    current_layer = cmds.editRenderLayerGlobals ( q = True, currentRenderLayer = True )
    
    if job_dispatcher == 'afanasy' :
      if mi_deferred and not mi_reuse:
        if exportAllRenderLayers : 
          gen_cmd = self.getDeferredCmd ( None )
        else :
          gen_cmd = self.getDeferredCmd ( current_layer )
        gen_cmd += ( ' -s @#@' ) 
        gen_cmd += ( ' -e @#@' ) 
        gen_cmd += ( ' -b ' + str ( self.job.step ) )
        print 'gen_cmd = %s %s' % ( gen_cmd, self.def_scene_name ) 
          
        self.job.gen_block = AfanasyRenderBlock ( 'generate_mi', deferred_service, self.job, mi_local_migen )
        self.job.gen_block.capacity = deferred_capacity 
        self.job.gen_block.cmd = gen_cmd
        self.job.gen_block.input_files = self.def_scene_name 
        self.job.gen_block.task_size = min ( mi_def_task_size, self.job.num_tasks )
        self.job.gen_block.setup ()
      
      renderLayers = []
      if exportAllRenderLayers :
        renderLayers = getRenderLayersList ( True ) # renderable only 
      else :
        # use only current layer
        renderLayers.append ( current_layer )
      
      for layer in renderLayers :
        cmds.editRenderLayerGlobals ( currentRenderLayer = layer )  
        layer_name = layer
        if layer == 'defaultRenderLayer' : layer_name = 'masterLayer'
        
        frame_block = AfanasyRenderBlock ( ('render_' + layer_name ), service, self.job )
        frame_block.capacity = capacity     
        frame_block.cmd = self.getRenderCmd ( layer )
        frame_block.input_files = self.get_mi_file_names ( layer_name )
        frame_block.out_files = self.get_image_names ()
        frame_block.setup ()
        self.job.frames_blocks.append ( frame_block )

      self.job.process ()
    
    if exportAllRenderLayers :
      # restore current layer
      cmds.editRenderLayerGlobals ( currentRenderLayer = current_layer ) 
  #
  # miFileNameChanged
  #
  def miFileNameChanged ( self, name, value ):
    #
    if name == 'mi_filename' :
      setDefaultStrValue ( self_prefix, name, self.mi_param, value )
    else:
      self.setDefaultIntValue ( self_prefix, name, self.mi_param, value )
    self.setResolvedPath ()
  #
  # setResolvedPath
  #
  def setResolvedPath ( self ):
    #
    filename = cmds.workspace ( expandName = self.mi_param[ 'mi_filename' ] )
    ( filename, ext ) = os.path.splitext ( filename )

    filename += '_' + str ( self.layer )
    if self.mi_param[ 'mi_padding' ] > 0 and self.mi_param[ 'mi_perframe' ] == True :
      filename += '.'
      pad_str = getPadStr ( self.mi_param [ 'mi_padding' ], self.mi_param [ 'mi_perframe' ] )
    
    if ext == '' or ext == '.' : ext = '.mi'
    filename += ext
    cmds.textFieldGrp ( self.winMain + '|f0|t0|tc1|fr2|fc2|' + 'mi_resolved_path', edit = True, text = filename )
  #
  # enable_range
  #
  def enable_range ( self, arg ) :
    #
    setDefaultIntValue ( self_prefix, 'job_animation', self.job_param, arg )
    cmds.intFieldGrp ( self.winMain + '|f0|t0|tc0|fr1|fc1|' + 'job_range', edit = True, enable = arg )
  #
  # enable_var_capacity
  #
  def enable_var_capacity ( self, arg ) :
    #
    setDefaultIntValue ( self_prefix, 'af_use_var_capacity', self.afanasy_param, arg )
    cmds.floatFieldGrp ( self.winMain + '|f0|t0|tc3|fr1|fc1|' + 'af_var_capacity', edit = True, enable = arg )
  #
  # enable_deferred
  #
  def enable_deferred ( self, arg ) :
    #
    mi_def_frame = self.winMain + '|f0|t0|tc1|fr3'
    mi_def       = mi_def_frame + '|fc3|'
    setDefaultIntValue ( self_prefix, 'mi_deferred', self.mi_param, arg )
    cmds.checkBoxGrp ( mi_def + 'mi_local_migen', edit = True, enable = arg )
    cmds.intFieldGrp ( mi_def + 'mi_def_task_size', edit = True, enable = arg )
    bg_color = self.save_frame_bgc 
    if arg : bg_color = self.def_frame_bgc
    cmds.frameLayout ( mi_def_frame, edit = True, bgc = bg_color ) # , enableBackground=False
  #
  # onRenderLayerSelected
  #
  def onRenderLayerSelected ( self, arg ) :
    #
    self.layer = arg 
    if self.layer == 'masterLayer' : arg = 'defaultRenderLayer' 
    #print '* onRenderLayerSelected %s' % self.layer
    cmds.evalDeferred ( 'import maya.OpenMaya; maya.cmds.editRenderLayerGlobals ( currentRenderLayer = "' + str ( arg ) + '" )', lowestPriority = True  )  
  #
  # renderLayerSelected
  #
  def renderLayerSelected ( self ) :
    #
    self.layer = cmds.editRenderLayerGlobals ( query = True, currentRenderLayer = True )
    if self.layer == 'defaultRenderLayer' : self.layer = 'masterLayer'
    print '* renderLayerSelected %s' % self.layer
    cmds.optionMenuGrp ( self.winMain + '|f0|c0|r0|' + 'layer_selector', e = True, value = self.layer )
    self.setResolvedPath()
  #
  # renderLayerRenamed
  #
  def renderLayerRenamed ( self ) :
    #
    self.layer = cmds.editRenderLayerGlobals ( query = True, currentRenderLayer = True )
    #print '* renderLayerRenamed %s' % self.layer
    # self.updateRenderLayerMenu()
    cmds.evalDeferred ( partial ( self.updateRenderLayerMenu ), lowestPriority = True )
  #
  # renderLayerChanged
  #
  def renderLayerChanged ( self ) :
    #
    self.layer = cmds.editRenderLayerGlobals ( query = True, currentRenderLayer = True )
    #print '* renderLayerChanged %s' % self.layer
    #self.updateRenderLayerMenu()
    cmds.evalDeferred ( partial ( self.updateRenderLayerMenu ), lowestPriority = True )
    return True
  #
  # updateRenderLayerMenu
  #
  def updateRenderLayerMenu ( self ) :
    #
    list_items = cmds.optionMenuGrp ( self.winMain + '|f0|c0|r0|' + 'layer_selector', q = True, itemListLong = True )
    if list_items is not None :
      # clear OptionMenu
      for item in list_items : cmds.deleteUI ( item )
    renderLayers = getRenderLayersList ( False )
    for layer in renderLayers: 
      if layer == 'defaultRenderLayer' : layer = 'masterLayer'
      cmds.menuItem ( label = layer, parent = ( self.winMain + '|f0|c0|r0|' + 'layer_selector|OptionMenu' ) ) 
    self.layer = cmds.editRenderLayerGlobals ( query = True, currentRenderLayer = True )
    if self.layer == 'defaultRenderLayer' : self.layer = 'masterLayer'
    cmds.optionMenuGrp( self.winMain + '|f0|c0|r0|' + 'layer_selector', e = True, value = self.layer ) 
    cmds.evalDeferred ( partial( self.renderLayersSetup, renderLayers ), lowestPriority = True )
  #
  # renderLayersSetup
  # add script job for renaming render layer
  #
  def renderLayersSetup ( self, layers ) :
    #
    #print '* renderLayersSetup'
    selector = self.winMain + '|f0|c0|r0|layer_selector'
    firstRun = True
    for layer in layers :
      if layer != 'defaultRenderLayer' : 
        #cmds.scriptJob( nodeNameChanged=[ layer, partial( self.renderLayerRenamed ) ], parent=top, replacePrevious=firstRun )
        cmds.scriptJob ( nodeNameChanged = [ layer, 'import maya.OpenMaya; maya.cmds.evalDeferred ( "meMentalRayRender.renderLayerRenamed()" , lowestPriority=True )' ], 
                         parent = selector, 
                         replacePrevious = firstRun )
        firstRun = False  
  #
  # setupUI
  #
  def setupUI ( self ):
    #
    # Main window setup
    #
    self.deleteUI ( True )
    self.winMain = cmds.window ( meMentalRayRenderMainWnd,
                                title = 'meMentalRayRender ver.' + meMentalRayRenderVer + ' (' + self.os + ')' ,
                                menuBar = True,
                                retain = False,
                                widthHeight = ( 420, 460 ) )

    self.mainMenu = cmds.menu ( label = 'Commands', tearOff = False )
    cmds.menuItem ( label = 'Render Globals ...',      command = maya_render_globals )
    cmds.menuItem ( label = 'Check Shaders ...',       command = mrShaderManager )
    cmds.menuItem ( label = 'Check Texture Paths ...', command = checkTextures )
    cmds.menuItem ( label = 'Generate .mi',            command = self.generate_mi )
    cmds.menuItem ( label = 'Submit Job',              command = self.submitJob )
    cmds.menuItem ( divider = True )
    cmds.menuItem ( label = 'Close',                   command = self.deleteUI )
    #
    # setup render layers script jobs
    #
    cmds.scriptJob ( attributeChange = [ 'renderLayerManager.currentRenderLayer', 
                                         'import maya.OpenMaya; maya.cmds.evalDeferred( "meMentalRayRender.renderLayerSelected()" , lowestPriority = True )' ], 
                                          parent = self.winMain  )
    cmds.scriptJob (           event = [ 'renderLayerChange', 
                                         'import maya.OpenMaya; maya.cmds.evalDeferred( "meMentalRayRender.renderLayerChanged()" , lowestPriority = True )' ], 
                                          parent = self.winMain ); 
    cw1 = 120
    cw2 = 60
    cw3 = 20
    mr_hi = 8
    
    form = cmds.formLayout ( 'f0', numberOfDivisions = 100 )
    proj = cmds.columnLayout ( 'c0', columnAttach = ( 'left',0 ), rowSpacing = 2, adjustableColumn = True, height = 50 )
    cmds.textFieldGrp ( cw = ( 1, 70 ), adj = 2, label = 'Project Root ', text = self.rootDir, editable = False ) # , bgc=(0,0,0)
    
    cmds.rowLayout ( 'r0', numberOfColumns = 2 )
    layer_selector = cmds.optionMenuGrp ( 'layer_selector', cw = ( ( 1, 70 ), ), cal = ( 1, 'right' ),
                                           label = 'Render Layer ',
                                           cc = partial( self.onRenderLayerSelected ) )
    self.updateRenderLayerMenu()
    
    cmds.checkBoxGrp ( 'mi_export_all_layers', cw = ( ( 1, cw1 ), ( 2, cw1 * 2 ) ),
                       label = 'Export All Renderable ',
                       value1 = self.mi_param [ 'mi_export_all_layers' ],
                       cc = partial( setDefaultIntValue, self_prefix, 'mi_export_all_layers', self.mi_param ) )
    cmds.setParent( '..' )
    cmds.setParent( '..' )
    #
    # setup tabs
    #
    tab = cmds.tabLayout ( 't0', scrollable = True, childResizable = True )  # tabLayout -scr true -cr true  tabs; //
    #
    # Job tab
    #
    tab_job = cmds.columnLayout ( 'tc0', columnAttach = ( 'left', 0 ), rowSpacing = 0, adjustableColumn = True ) 
    cmds.frameLayout ( 'fr1', label = ' Parameters ', borderVisible = True, borderStyle = 'etchedIn', marginHeight = mr_hi  )
    cmds.columnLayout ( 'fc1', columnAttach = ( 'left', 0 ), rowSpacing = 0, adjustableColumn = True )
    #
    job_dispatcher = cmds.optionMenuGrp ( 'job_dispatcher', cw = ( ( 1, cw1 ), ), cal = ( 1, 'right' ),
                                          label = 'Job Dispatcher ',  
                                          cc = partial ( setDefaultStrValue, self_prefix, 'job_dispatcher',self.job_param ) )
    for name in ( 'none', 'afanasy' ): cmds.menuItem ( label = name ) # 'backburner',
    cmds.optionMenuGrp ( job_dispatcher, e = True, value = self.job_param [ 'job_dispatcher' ] )
    cmds.text ( label = '' )
    cmds.textFieldGrp ( 'job_name', cw = ( 1, cw1 ), adj = 2, 
                        label = 'Job Name ', 
                        text = self.job_param [ 'job_name' ],
                        cc = partial ( setDefaultStrValue, self_prefix, 'job_name', self.job_param ) )
    cmds.textFieldGrp ( 'job_description', cw = ( 1, cw1 ), adj = 2,
                        label = 'Description ',
                        text = self.job_param [ 'job_description' ],
                        cc = partial ( setDefaultStrValue, self_prefix, 'job_description', self.job_param ) )
    cmds.checkBoxGrp ( 'job_paused', cw = ( ( 1, cw1 ), ( 2, cw1 * 2 ) ),
                       label = 'Start Paused ',
                       value1 = self.job_param [ 'job_paused' ],
                       cc = partial ( setDefaultIntValue, self_prefix, 'job_paused', self.job_param ) )
    cmds.text ( label = '' )
    cmds.checkBoxGrp ( 'job_animation', cw = ( ( 1, cw1 ), ( 2, cw1 * 2 ) ),
                       label = 'Animation ',
                       value1 = self.job_param [ 'job_animation' ],
                       cc = partial ( self.enable_range ) )
    cmds.intFieldGrp ( 'job_range', cw = ( ( 1, cw1 ), ( 2, cw2 ), ( 3, cw2 ), ( 4, cw2 ) ), nf = 3, 
                       label = 'Start/Stop/By ',
                       value1 = self.job_param [ 'job_start' ],
                       value2 = self.job_param [ 'job_end' ],
                       value3 = self.job_param [ 'job_step' ],
                       enable = self.job_param [ 'job_animation' ],
                       cc = partial ( setDefaultIntValue3, self_prefix, ( 'job_start', 'job_end', 'job_step' ), self.job_param ) )
    cmds.intFieldGrp ( 'job_size', cw = ( ( 1, cw1 ), ( 2, cw2 ) ),
                       label = 'Task Size ',
                       ann = 'Should be smaller then number of frames to render',
                       value1 = self.job_param [ 'job_size' ],
                       cc = partial ( setDefaultIntValue, self_prefix, 'job_size', self.job_param ) )
    cmds.intFieldGrp ( 'job_priority', cw = ( ( 1, cw1 ), ( 2, cw2 ) ),
                       label = 'Priority ',
                       value1 = self.job_param [ 'job_priority' ],
                       cc = partial ( setDefaultIntValue, self_prefix, 'job_priority', self.job_param ) )
    cmds.setParent ( '..' )
    cmds.setParent ( '..' )
    cmds.frameLayout ( 'fr2', label = ' Cleanup ', borderVisible = True, borderStyle = 'etchedIn', 
                       marginHeight = mr_hi, 
                       cll = True, cl = True  )
    cmds.columnLayout ( 'fc2', columnAttach = ( 'left', 0 ), rowSpacing = 0, adjustableColumn = True )
    cmds.checkBoxGrp ( 'job_cleanup_mi', cw = ( ( 1, cw1 ), ( 2, cw1 * 2 ) ),
                       label = '', 
                       label1 = ' .mi files',
                       value1 = self.job_param [ 'job_cleanup_mi' ],
                       enable = False,
                       cc = partial ( setDefaultIntValue, self_prefix, 'job_cleanup_mi', self.job_param ) )
    cmds.checkBoxGrp( 'job_cleanup_script', cw = ( ( 1, cw1 ), ( 2, cw1 * 2 ) ),
                      label = '', 
                      label1 = ' script file',
                      value1 = self.job_param [ 'job_cleanup_script' ],
                      enable = False,
                      cc = partial ( setDefaultIntValue, self_prefix, 'job_cleanup_script', self.job_param ) )
    cmds.setParent ( '..' )
    cmds.setParent ( '..' )
    cmds.setParent ( '..' )
    #
    # .mi files generation tab
    #
    tab_miparam = cmds.columnLayout ( 'tc1', columnAttach = ( 'left', 0 ), rowSpacing = 0, adjustableColumn = True )
    cmds.frameLayout ( 'fr3', label = ' Deferred .mi generation ', 
                       borderVisible = True, 
                       borderStyle = 'etchedIn', 
                       marginHeight = mr_hi, 
                       cll = True, cl = True )
    cmds.columnLayout ( 'fc3', columnAttach = ( 'left', 0 ), rowSpacing = 0, adjustableColumn = True )
    cmds.checkBoxGrp ( 'mi_deferred', cw = ( ( 1, cw1 ), ( 2, cw1 * 2 ) ),
                       label = 'Use deferred ',
                       ann = 'Generate .mi files in background process',
                       value1 = self.mi_param [ 'mi_deferred' ],
                       cc = partial ( self.enable_deferred ) ) 
    cmds.checkBoxGrp ( 'mi_local_migen', cw = ( ( 1, cw1 ), ( 2, cw1 * 2 ) ),
                       label = '', 
                       label1 = " Only on localhost ",
                       ann = "Do not use remote hosts",
                       value1 = self.mi_param [ 'mi_local_migen' ],
                       enable = self.mi_param [ 'mi_deferred' ],
                       cc = partial ( setDefaultIntValue, self_prefix, 'mi_local_migen', self.mi_param ) )
    cmds.intFieldGrp ( 'mi_def_task_size', cw = ( ( 1, cw1 ), ( 2, cw2 ) ),
                       label = 'Task Size ',
                       value1 = self.mi_param [ 'mi_def_task_size' ],
                       enable = self.mi_param [ 'mi_deferred' ],
                       cc = partial ( setDefaultIntValue, self_prefix, 'mi_def_task_size', self.mi_param ) )
    self.save_frame_bgc = cmds.frameLayout ( 'fr3', query = True, bgc = True )
    self.def_frame_bgc = [ 0.75, 0.5, 0 ]
    bg_color = self.save_frame_bgc 
    if self.mi_param [ 'mi_deferred' ] : bg_color = self.def_frame_bgc
    cmds.frameLayout ( 'fr3', edit = True, bgc = bg_color ) # , enableBackground=False
    cmds.setParent ( '..' )
    cmds.setParent ( '..' )
    cmds.frameLayout ( 'fr1', label = ' Export Settings ', 
                       borderVisible = True, 
                       borderStyle = 'etchedIn', 
                       marginHeight = mr_hi  )
    cmds.columnLayout ( 'fc1', columnAttach = ( 'left', 0 ), rowSpacing = 0, adjustableColumn = True )
    cmds.checkBoxGrp ( 'mi_reuse', cw = ( ( 1, cw1 ), ( 2, cw1 * 2 ) ),
                       label = 'Use existing .mi files ',
                       ann = 'Do not generate .mi files if they are exist',
                       value1 = self.mi_param [ 'mi_reuse' ],
                       cc = partial ( setDefaultIntValue, self_prefix, 'mi_reuse', self.mi_param ) )
    cmds.text ( label = '' )
    #mi_dir = cmds.textFieldButtonGrp( cw=( 1, cw1 ), adj=2, label="Directory ", buttonLabel="...", text=self.mi_param['mi_dir'] )
    #cmds.textFieldButtonGrp( mi_dir, edit=True, bc=partial( browseDirectory, self.rootDir, mi_dir ), cc=partial( self.setDefaultStrValue, 'mi_dir' ) )
    mi_filename = cmds.textFieldButtonGrp ( 'mi_filename', cw = ( 1, cw1 ), adj = 2,
                                           label = 'File Name ', 
                                           buttonLabel = '...',
                                           text = self.mi_param [ 'mi_filename' ],
                                           cc = partial ( self.miFileNameChanged, 'mi_filename' ) )
    cmds.textFieldButtonGrp ( mi_filename, edit = True, bc = partial ( browseFile, self.rootDir, mi_filename, 'mentalray files (*.mi)' ) )
    cmds.intFieldGrp ( 'mi_padding', cw = ( ( 1, cw1 ), ( 2, cw2 ) ),
                       label = 'Frame Padding ',
                       value1 = self.mi_param [ 'mi_padding' ],
                       cc = partial ( self.miFileNameChanged, 'mi_padding' ) )
    cmds.checkBoxGrp ( 'mi_perframe', cw = ( ( 1, cw1 ), ( 2, cw1 * 2 ) ),
                       label = '', 
                       label1 = ' File Per Frame ',
                       value1 = self.mi_param [ 'mi_perframe' ],
                       cc = partial( self.miFileNameChanged, 'mi_perframe' ) )
    cmds.checkBoxGrp ( 'mi_selection', cw = ( ( 1, cw1 ), ( 2, cw1 * 2 ) ),
                       label = '', 
                       label1 = ' Export Only Selected Objects',
                       value1 = self.mi_param [ 'mi_selection' ],
                       cc = partial ( setDefaultIntValue, self_prefix, 'mi_selection', self.mi_param ) )
    cmds.checkBoxGrp ( 'mi_binary', cw = ( ( 1, cw1 ), ( 2, cw1 * 2 ) ),
                       label = '', 
                       label1 = ' Binary',
                       value1 = self.mi_param [ 'mi_binary' ],
                       cc = partial ( setDefaultIntValue, self_prefix, 'mi_binary', self.mi_param ) )
    cmds.intFieldGrp ( 'mi_tabstop', cw = ( ( 1, cw1 ), ( 2, cw2 ) ),
                       label = "Tab stop (ASCII) ",
                       value1 = self.mi_param [ 'mi_tabstop' ],
                       cc = partial ( setDefaultIntValue, self_prefix, 'mi_tabstop', self.mi_param ) )
    #mi_compression = cmds.optionMenuGrp( 'mi_compression', cw=( (1, cw1), ), label="Compression ", cal=(1, 'right'), cc=partial( self.setDefaultStrValue, 'mi_compression' ) )
    #cmds.menuItem( label='Off' )
    #cmds.menuItem( label='GzipBestSpeed' )
    #cmds.menuItem( label='GzipDefault' )
    #cmds.menuItem( label='GzipBest' )
    #cmds.optionMenuGrp( mi_compression, e=True, value=self.mi_param['mi_compression'] )
    mi_filepaths = cmds.optionMenuGrp ( 'mi_filepaths', cw = ( ( 1, cw1 ), ), cal = ( 1, 'right' ),
                                        label = 'Export File Paths ',
                                        cc = partial ( setDefaultStrValue, self_prefix, 'mi_filepaths', self.mi_param ) )
    for name in ( 'NoChange', 'NoPath', 'Relative', 'Absolute' ): cmds.menuItem ( label = name )
    cmds.optionMenuGrp( mi_filepaths, e = True, value = self.mi_param [ 'mi_filepaths' ] )
    # mi_verbosity = none, fatal, error, warning, info, progress, and details
    #mi_verbosity = cmds.optionMenuGrp( 'mi_verbosity', cw=( (1, cw1), ), cal=(1, 'right'),
    #                                   label="Verbosity ",
    #                                   cc=partial( self.setDefaultStrValue, 'mi_verbosity' ) )
    #for name in ('none', 'fatal', 'error', 'warning', 'info', 'progress', 'details'):
    #  cmds.menuItem( label=name )
    #cmds.optionMenuGrp( mi_verbosity, e=True, value=self.mi_param['mi_verbosity'] )
    #
    cmds.setParent ( '..' )
    cmds.setParent ( '..' )
    cmds.frameLayout ( 'fr2', label = ' Resolved Path ', 
                       borderVisible = True, 
                       borderStyle = 'etchedIn', 
                       marginHeight = mr_hi  )
    cmds.columnLayout ( 'fc2', columnAttach = ( 'left', 0 ), rowSpacing = 0, adjustableColumn = True )
    cmds.textFieldGrp ( 'mi_resolved_path', cw = ( 1, 0 ), adj = 2, 
                        label = '', 
                        text = '', 
                        editable = False )
    self.setResolvedPath ()
    cmds.setParent ( '..' )
    cmds.setParent ( '..' )
    cmds.setParent ( '..' )
    #
    # Renderer tab
    #
    tab_render = cmds.columnLayout ( 'tc2', columnAttach = ( 'left', 0 ), 
                                      rowSpacing = 0, 
                                      adjustableColumn = True )
    cmds.frameLayout ( 'fr1', label = ' MentalRay options ', 
                       borderVisible = True, 
                       borderStyle = 'etchedIn', 
                       marginHeight = mr_hi  )
    cmds.columnLayout ( 'fc1', columnAttach = ( 'left', 0 ), 
                        rowSpacing = 0, 
                        adjustableColumn = True )
    cmds.textFieldGrp ( 'mr_options', cw = ( 1, cw1 ), adj = 2,
                       label = 'Additional Options ',
                       text = self.mr_param [ 'mr_options' ],
                       cc = partial ( setDefaultStrValue, self_prefix, 'mr_options', self.mr_param ) )
    cmds.text ( label = '' )
    mr_verbosity = cmds.optionMenuGrp ( 'mr_verbosity', cw = ( ( 1, cw1 ), ), cal = ( 1, 'right' ),
                       label = 'Verbosity ',
                       cc = partial ( setDefaultStrValue, self_prefix, 'mr_verbosity', self.mr_param ) )
    for name in ( 'none', 'fatal', 'error', 'warning', 'info', 'progress', 'debug', 'details' ): cmds.menuItem ( label = name )
    cmds.optionMenuGrp ( mr_verbosity, e = True, value = self.mr_param [ 'mr_verbosity' ] )
    cmds.intFieldGrp ( 'mr_progress_frequency', cw = ( ( 1, cw1 ), ( 2, cw2 ) ),
                       label = 'Progress frequency ',
                       ann = 'Progress information should be emitted only when this percentage of the whole render time has passed.',
                       value1 = self.mr_param [ 'mr_progress_frequency' ],
                       cc = partial ( setDefaultIntValue, self_prefix, 'mr_progress_frequency', self.mr_param ) )
    cmds.intFieldGrp ( 'mr_threads', cw = ( ( 1, cw1 ), ( 2, cw2 ) ),
                       label = 'Threads ',
                       ann = 'The number of threads',
                       value1 = self.mr_param [ 'mr_threads' ],
                       cc = partial ( setDefaultIntValue, self_prefix, 'mr_threads', self.mr_param ) )
    cmds.checkBoxGrp ( 'mr_texture_continue', cw = ( ( 1, cw1 ), ( 2, cw1 * 2 ) ),
                       label = '', 
                       label1 = ' Skip missing textures',
                       ann = 'If this option is specified, mental ray will continue for missing texture files',
                       value1 = self.mr_param [ 'mr_texture_continue' ],
                       cc = partial ( setDefaultIntValue, self_prefix, 'mr_texture_continue', self.mr_param ) )
    cmds.setParent ( '..' )
    cmds.setParent ( '..' )
    cmds.setParent ( '..' )
    #
    # Afanasy tab
    #
    tab_afanasy = cmds.columnLayout ( 'tc3', columnAttach = ( 'left',0 ), 
                        rowSpacing = 0, 
                        adjustableColumn = True )
    cmds.frameLayout ( 'fr1', label = ' Parameters ', 
                        borderVisible = True, 
                        borderStyle = 'etchedIn', 
                        marginHeight = mr_hi )
    cmds.columnLayout ( 'fc1', columnAttach = ( 'left', 4 ), 
                        rowSpacing = 0, 
                        adjustableColumn = True )
    cmds.intFieldGrp ( 'af_capacity', cw = ( ( 1, cw1 ), ( 2, cw2 ) ),
                        label = 'Task Capacity ',
                        value1 = self.afanasy_param [ 'af_capacity' ],
                        cc = partial ( setDefaultIntValue, self_prefix, 'af_capacity', self.afanasy_param ) )
    cmds.intFieldGrp ( 'af_deferred_capacity', cw =( ( 1, cw1 ), ( 2, cw2 ) ),
                        label = 'Deferred Capacity ',
                        value1 = self.afanasy_param [ 'af_deferred_capacity' ],
                        cc = partial ( setDefaultIntValue, self_prefix, 'af_deferred_capacity', self.afanasy_param ) )                      
    cmds.checkBoxGrp ( 'af_use_var_capacity', cw = ( ( 1, cw1 ), ( 2, cw1 * 2 ) ),
                        label = 'Use Variable Capacity ',
                        ann = 'Block can generate tasks with capacity*coefficient to fit free render capacity',
                        value1 = self.afanasy_param [ 'af_use_var_capacity' ],
                        cc = partial ( self.enable_var_capacity ) )
    cmds.floatFieldGrp ( 'af_var_capacity', cw = ( ( 1, cw1 ), ( 2, cw2 ), ( 3, cw2 ), ( 4, cw2 ) ), nf = 2, pre = 2,
                        label = 'Min/Max coefficient ',
                        value1 = self.afanasy_param [ 'af_cap_min' ],
                        value2 = self.afanasy_param [ 'af_cap_max' ],
                        enable = self.afanasy_param [ 'af_use_var_capacity' ],
                        cc = partial ( setDefaultFloatValue2, self_prefix, ('af_cap_min', 'af_cap_max'), self.afanasy_param ) )
    cmds.intFieldGrp ( 'af_max_running_tasks', cw = ( ( 1, cw1 ), ( 2, cw2 ) ),
                        label = 'Max Running Tasks ',
                        value1 = self.afanasy_param [ 'af_max_running_tasks' ],
                        cc = partial ( setDefaultIntValue, self_prefix, 'af_max_running_tasks', self.afanasy_param ) )
    cmds.intFieldGrp ( 'af_max_tasks_per_host', cw = ( ( 1, cw1 ), ( 2, cw2 ) ),
                        label = 'Max Tasks Per Host ',
                        value1 = self.afanasy_param [ 'af_max_tasks_per_host' ],
                        cc = partial ( setDefaultIntValue, self_prefix, 'af_max_tasks_per_host', self.afanasy_param ) )
    cmds.textFieldGrp ( 'af_service', cw = ( 1, cw1 ), adj = 2,
                        label = 'Service ',
                        text = self.afanasy_param [ 'af_service' ],
                        enable = False,
                        cc = partial ( setDefaultStrValue, self_prefix, 'af_service', self.afanasy_param ) )
    cmds.textFieldGrp ( 'af_deferred_service', cw = ( 1, cw1 ), adj = 2,
                        label = 'Deferred Service ',
                        text = self.afanasy_param [ 'af_deferred_service' ],
                        enable = False,
                        cc = partial ( setDefaultStrValue, self_prefix, 'af_deferred_service', self.afanasy_param ) )
    cmds.textFieldGrp ( 'af_hostsmask', cw = ( 1, cw1 ), adj = 2,
                        label = 'Hosts Mask ',
                        ann = 'Job run only on renders which host name matches this mask\n e.g.  .* or host.*',
                        text = self.afanasy_param [ 'af_hostsmask' ],
                        cc = partial ( setDefaultStrValue, self_prefix, 'af_hostsmask', self.afanasy_param ) )
    cmds.textFieldGrp ( 'af_hostsexcl', cw = ( 1, cw1 ), adj = 2,
                        label = 'Exclude Hosts Mask ',
                        ann = 'Job can not run on renders which host name matches this mask\n e.g.  host.* or host01|host02',
                        text = self.afanasy_param [ 'af_hostsexcl' ],
                        cc = partial ( setDefaultStrValue, self_prefix, 'af_hostsexcl', self.afanasy_param ) )
    cmds.textFieldGrp ( 'af_depmask', cw = ( 1, cw1 ), adj = 2,
                        label = 'Depend Mask ',
                        ann = 'Job will wait other user jobs which name matches this mask',
                        text = self.afanasy_param [ 'af_depmask' ],
                        cc = partial ( setDefaultStrValue, self_prefix, 'af_depmask', self.afanasy_param ) )
    cmds.textFieldGrp ( 'af_depglbl', cw = ( 1, cw1 ), adj = 2,
                        label = 'Global Depend Mask ',
                        ann = 'Job will wait other jobs from any user which name matches this mask',
                        text = self.afanasy_param [ 'af_depglbl' ],
                        cc = partial ( setDefaultStrValue, self_prefix, 'af_depglbl', self.afanasy_param ) )
    cmds.textFieldGrp ( 'af_os', cw = ( 1, cw1 ), adj = 2,
                        label = 'Needed OS ',
                        ann = 'windows linux mac',
                        text = self.afanasy_param [ 'af_os' ],
                        cc = partial ( setDefaultStrValue, self_prefix, 'af_os', self.afanasy_param ) )
    cmds.setParent ( '..' )
    cmds.setParent ( '..' )
    cmds.setParent ( '..' )
    cmds.tabLayout ( tab, edit = True,
                    tabLabel = ( ( tab_job, "Job" ),
                               ( tab_miparam, ".mi" ),
                               ( tab_render, "Renderer" ),
                               ( tab_afanasy, "Afanasy" )
                             )
                  )
    cmds.setParent ( form )
    btn_sbm = cmds.button ( label = 'Submit', command = self.submitJob, ann = 'Generate .mi files and submit to dispatcher' )
    btn_gen = cmds.button ( label = 'Generate .mi', command = self.generate_mi, ann = 'Force .mi files generation' )
    btn_cls = cmds.button ( label = 'Close', command = self.deleteUI )
    cmds.formLayout (  form, edit = True,
                      attachForm = ( ( proj, 'top',    0 ),
                                   ( proj, 'left',    0 ),
                                   ( proj, 'right',    0 ),
                                   ( tab, 'left',    0 ),
                                   ( tab, 'right',    0 ),
                                   ( btn_cls,   'bottom', 0 ),
                                   ( btn_gen,   'bottom', 0 ),
                                   ( btn_sbm,   'bottom', 0 ),
                                   ( btn_sbm,   'left',   0 ),
                                   ( btn_cls,   'right',  0 )
                                 ),
                      attachControl = ( ( tab, 'top', 0, proj ), # 
                                      ( tab, 'bottom', 0, btn_sbm ),
                                      ( btn_gen, 'left', 0, btn_sbm ),
                                      ( btn_gen, 'right', 0, btn_cls )
                                    ),
                      attachPosition = ( ( btn_sbm, 'right', 0, 33 ),
                                       ( btn_gen, 'right', 0, 66 ),
                                       ( btn_cls, 'left', 0, 66 )
                                      )
                   )
    cmds.showWindow ( self.winMain )
    return form
  #
  # deleteUI
  #
  def deleteUI ( self, param ):
    #
    winMain = meMentalRayRenderMainWnd
    if cmds.window ( winMain, exists = True )    : cmds.deleteUI ( winMain, window = True )
    if cmds.windowPref ( winMain, exists = True ): cmds.windowPref ( winMain, remove = True )
#
#
#
print 'meMentalRayRender sourced ...'
