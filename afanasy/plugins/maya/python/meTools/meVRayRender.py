"""
  meVRayRender.py

  ver.0.3.8 (29 May 2013)
    - create missing folders for Render Layers 
    - add separator before frame number for multichannel exr (Vray4Maya bug?)
  
  ver.0.3.7 (28 May 2013)
    - fixed small bugs with deffered generation
    - check if vray is current renderer
    
  ver.0.3.6 (16 Feb 2013)
    - added image filename override option to render command
    - get rid of 'multichannel' suffix for 'exr' image format
  ver.0.3.5 (29 Jan 2013)
    - distributed render support
  ver.0.3.3 (28 Jan 2013)
    - minor bugs fixed
  ver.0.3.2 (23 Jan 2013)
    - fixed few small bugs
    - code cleanup
  ver.0.3.1 (23 Jan 2013)
    - fixed minor problems with render layers
  ver.0.3.0 (13 Jan 2013)
    - added render layers support
    - new RenderJob class
    - new AfanasyRenderJob class
    - some procedures moved to maya_ui_proc.py

  ver.0.2.0 (13 Jan 2013)
  ver.0.1.0 (10 Jan 2013)
  ver.0.0.1 (9 Jan 2013)

  Author:

  Yuri Meshalkin (aka mesh)
  mesh@kpp.kiev.ua

  (c) Kiev Post Production 2013

  Description:

  This UI script generates .vrscene files from open Maya scene
  and submits them to VRay Standalone by creating a job for Afanasy.

  Usage:
  You can add this code as shelf button :

  import meTools.meVRayRender as meVRayRender
  reload( meVRayRender ) # for debugging purposes
  meVRayRender = meVRayRender.meVRayRender()

  Important !!!
  Do not use another object name.
  Only meVRayRender.renderLayerSelected() will work with script job.

  For using with Afanasy, add %AF_ROOT%\plugins\maya\python to %PYTHONPATH%

"""
import sys, os, string
import maya.OpenMaya as OpenMaya
import maya.cmds as cmds
import maya.mel as mel
from functools import partial

from maya_ui_proc import *
from afanasyRenderJob import *

self_prefix = 'meVRayRender_'
meVRayRenderVer = '0.3.8'
meVRayRenderMainWnd = self_prefix + 'MainWnd'

vray_transfer_assets_list = [ 'none', 'transfer', 'use_cache' ]
vray_verbosity_list = [ 'none', 'errors', 'warning', 'info', 'details' ]
#
# meVRayRender
#
class meVRayRender ( object ) :
  #
  # __init__
  #
  def __init__ ( self, selection = '' ) :
    #
    self.selection = selection
    self.winMain = ''

    self.os = sys.platform
    if self.os.startswith ( 'linux' ) : self.os = 'linux'
    elif self.os == 'darwin'          : self.os = 'mac'
    elif self.os == 'win32'           : self.os = 'win'

    print 'sys.platform = %s self.os = %s' % ( sys.platform, self.os )
    
    if cmds.getAttr ( 'defaultRenderGlobals.currentRenderer' ) != 'vray' :
      cmds.warning ( 'V-Ray is not current renderer!' )
      return
    
    self.rootDir = cmds.workspace ( q = True, rootDirectory = True )
    self.rootDir = self.rootDir[ :-1 ]
    self.layer   = cmds.editRenderLayerGlobals ( query = True, currentRenderLayer = True )

    self.job = None

    self.job_param     = {}
    self.vr_param      = {}
    self.vray_param    = {}
    self.img_param     = {}
    self.afanasy_param = {}
    self.bkburn_param  = {}

    self.vrgenCommand     = 'vrend'
    self.def_vrgenCommand = 'Render -r vray'
    self.def_scene_name   = '' # maya scene name used for deferred .vrscene generation

    self.save_frame_bgc = [ 0, 0, 0 ]
    self.def_frame_bgc = [ 0.75, 0.5, 0 ]
    
    self.exr_multichannel = False # Image Format Str = "exr (multichannel)"
    
    self.initParameters ()
    self.ui = self.setupUI ()
  #
  # initParameters
  #
  def initParameters ( self ) :
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
    self.job_param [ 'job_cleanup_vr' ]  = getDefaultIntValue ( self_prefix, 'job_cleanup_vr', 0 ) is 1
    self.job_param [ 'job_cleanup_script' ] = getDefaultIntValue ( self_prefix, 'job_cleanup_script', 0 ) is 1
    self.job_param [ 'job_padding' ]     = cmds.getAttr ( 'vraySettings.fileNamePadding' )
    #
    # .vrscene generation parameters
    #
    self.vr_param [ 'vr_reuse' ]         = getDefaultIntValue ( self_prefix, 'vr_reuse', 1 ) is 1
    #self.vr_param [ 'vr_plugin_prefix'] = cmds.getAttr( 'vraySettings.misc_pluginsPrefix' )
    scene_name = cmds.getAttr ( 'vraySettings.vrscene_filename' )
    if scene_name == None or scene_name == '' : scene_name = 'vrscenes/' + getMayaSceneName ()
    self.vr_param [ 'vr_filename' ]      = scene_name
    self.vr_param [ 'vr_padding' ]       = self.job_param [ 'job_padding' ]
    self.vr_param [ 'vr_perframe' ]      = getDefaultIntValue ( self_prefix, 'vr_perframe', 1 ) is 1
    self.vr_param [ 'vr_separate' ]      = getDefaultIntValue ( self_prefix, 'vr_separate', 0 ) is 1
    self.vr_param [ 'vr_export_lights' ] = getDefaultIntValue ( self_prefix, 'vr_export_lights', 0 ) is 1
    self.vr_param [ 'vr_export_nodes' ]  = getDefaultIntValue ( self_prefix, 'vr_export_nodes', 0 ) is 1
    self.vr_param [ 'vr_export_geometry' ] = getDefaultIntValue ( self_prefix, 'vr_export_geometry', 0 ) is 1
    self.vr_param [ 'vr_export_materials'] = getDefaultIntValue ( self_prefix, 'vr_export_materials', 0 ) is 1
    self.vr_param [ 'vr_export_textures'] = getDefaultIntValue ( self_prefix, 'vr_export_textures', 0 ) is 1
    self.vr_param [ 'vr_export_bitmaps' ] = getDefaultIntValue ( self_prefix, 'vr_export_textures', 0 ) is 1
    self.vr_param [ 'vr_hex_mesh' ]       = getDefaultIntValue ( self_prefix, 'vr_hex_mesh', 0 ) is 1
    self.vr_param [ 'vr_hex_transform' ]  = getDefaultIntValue ( self_prefix, 'vr_hex_transform', 0 ) is 1
    self.vr_param [ 'vr_compression' ]    = getDefaultIntValue ( self_prefix, 'vr_compression', 0 ) is 1
    
    self.vr_param [ 'vr_deferred' ]       = getDefaultIntValue ( self_prefix, 'vr_deferred', 0 ) is 1
    self.vr_param [ 'vr_local_vrgen' ]    = getDefaultIntValue ( self_prefix, 'vr_local_vrgen', 1 ) is 1
    self.vr_param [ 'vr_def_task_size' ]  = getDefaultIntValue ( self_prefix, 'vr_def_task_size', 4 )
    self.vr_param [ 'vr_export_all_layers' ] = getDefaultIntValue ( self_prefix, 'vr_export_all_layers', 0 ) is 1
    #
    # VRay parameters
    #
    self.vray_param [ 'vray_options' ]    = getDefaultStrValue ( self_prefix, 'vray_options', '' )
    self.vray_param [ 'vray_verbosity' ]  = getDefaultStrValue ( self_prefix, 'vray_verbosity', 'none' )
    self.vray_param [ 'vray_threads' ]    = getDefaultIntValue ( self_prefix, 'vray_threads', 4 )
    self.vray_param [ 'vray_clearRVOn' ]  = getDefaultIntValue ( self_prefix, 'vr_clearRVOn', 0 ) is 1
    self.vray_param [ 'vray_progress_frequency' ]  = getDefaultIntValue ( self_prefix, 'vray_progress_frequency', 1 )
    self.vray_param [ 'vray_low_thread_priority' ] = getDefaultIntValue ( self_prefix, 'vray_low_thread_priority', 0 ) is 1
    
    self.vray_param [ 'vray_distributed' ]  = getDefaultIntValue ( self_prefix, 'vray_distributed', 0 ) is 1
    self.vray_param [ 'vray_nomaster' ]      = getDefaultIntValue ( self_prefix, 'vray_nomaster', 0 ) is 1
    self.vray_param [ 'vray_hosts' ]         = getDefaultStrValue ( self_prefix, 'vray_hosts', '' ) # @AF_HOSTS@
    self.vray_param [ 'vray_port' ]          = getDefaultIntValue ( self_prefix, 'vray_port', 20204 )
    self.vray_param [ 'vray_hosts_min' ]     = getDefaultIntValue ( self_prefix, 'vray_hosts_min', 1 )
    self.vray_param [ 'vray_hosts_max' ]     = getDefaultIntValue ( self_prefix, 'vray_hosts_max', 4 )
    self.vray_param [ 'vray_threads_limit' ] = getDefaultIntValue ( self_prefix, 'vray_threads_limit', 4 )
    self.vray_param [ 'vray_transfer_assets' ] = getDefaultStrValue ( self_prefix, 'vray_transfer_assets', 'none' )
    #
    # image parameters
    #
    self.img_param [ 'img_filename_prfix' ]  = self.getImageFileNamePrefix ()
    self.img_param [ 'img_format' ]          = self.getImageFormat ()
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
    self.afanasy_param [ 'af_service' ]            = 'vray' #getDefaultStrValue ( self_prefix, 'af_service', 'vray' )
    self.afanasy_param [ 'af_deferred_service' ]   = 'mayatovray' #getDefaultStrValue ( self_prefix, 'af_deferred_service', 'mayatovray' )
    self.afanasy_param [ 'af_os' ]                 = getDefaultStrValue ( self_prefix, 'af_os', '' ) #linux mac windows
    self.afanasy_param [ 'af_hostsmask' ]          = getDefaultStrValue ( self_prefix, 'af_hostsmask', '.*' )
    self.afanasy_param [ 'af_hostsexcl' ]          = getDefaultStrValue ( self_prefix, 'af_hostsexcl', '' )
    self.afanasy_param [ 'af_depmask' ]            = getDefaultStrValue ( self_prefix, 'af_depmask', '' )
    self.afanasy_param [ 'af_depglbl' ]            = getDefaultStrValue ( self_prefix, 'af_depglbl', '' )
  #
  # getImageFileNamePrefix
  #
  def getImageFileNamePrefix ( self ) :
    #
    fileNamePrefix = cmds.getAttr ( 'vraySettings.fileNamePrefix' )
    if fileNamePrefix is None or fileNamePrefix == '' or fileNamePrefix == 'None':
      fileNamePrefix = getMayaSceneName ()
    if self.exr_multichannel :
      # add separator before frame number for multichannel exr (Vray4Maya bug?)
      fileNamePrefix += '.'  
    return fileNamePrefix
  #
  # getImageFormat
  #
  def getImageFormat ( self ) :
    #
    imageFormatStr = cmds.getAttr ( 'vraySettings.imageFormatStr' )
    if imageFormatStr is None : # Sometimes it happens ...
      imageFormatStr = 'png'
    else :
      if 'exr' in imageFormatStr : 
        # !!! get rid of '(multichannel)'
        if '(multichannel)' in imageFormatStr :
          self.exr_multichannel = True 
        imageFormatStr = 'exr'  
      
    return imageFormatStr
  #
  # getDeferredCmd
  #
  def getDeferredCmd ( self, layer = None  ) :
    #
    gen_cmd = self.def_vrgenCommand + ' '
    gen_cmd += '-proj ' + '"' + self.rootDir + '"' + ' '
    #gen_cmd += '-rd ' + '"' + self.rootDir + '"' + ' '
    #gen_cmd += '-im ' + '"images/' + self.getImageFileNamePrefix() + '"' + ' '
    #gen_cmd += '-of ' + self.getImageFormat() + ' '
    if layer is not None : gen_cmd += '-rl ' + layer + ' '
    else                 : gen_cmd += '-rl 1 ' #'-perlayer 1 '
    gen_cmd += '-exportFileName "' + self.get_vr_file_names ( False, layer ) + '" '
    gen_cmd += '-noRender '
    gen_cmd += self.get_vrgen_options ( None )
    return gen_cmd
  #
  # getRenderCmd
  #
  def getRenderCmd ( self, layer = None  ) :
    #
    vray_verbosity_level    = vray_verbosity_list.index ( self.vray_param [ 'vray_verbosity' ] )
    options                 = str ( self.vray_param [ 'vray_options' ] ).strip ()
    vray_threads            = self.vray_param [ 'vray_threads' ]
    vray_progress_frequency = self.vray_param [ 'vray_progress_frequency' ]
    vray_distributed        = self.vray_param [ 'vray_distributed' ]

    if vray_verbosity_level < 3     : vray_verbosity_level = 3
    if vray_progress_frequency == 0 : vray_progress_frequency = 1

    cmd = 'vray '
    cmd += '-display=0 '
    cmd += '-showProgress=1 '
    if vray_verbosity_level != 0    : cmd += '-verboseLevel=' + str ( vray_verbosity_level ) + ' '
    if vray_progress_frequency != 0 : cmd += '-progressIncrement=' + str ( vray_progress_frequency ) + ' '
    cmd += '-progressUseCR=0 '
    if vray_distributed :
      #  -distributed=1 -portNumber=20207 -renderhost="burn02.kpp;burn03.kpp;burn04.kpp"
      cmd += '-distributed=1 '
    if vray_threads != 0 : cmd += '-numThreads=' + str ( vray_threads ) + ' '
    cmd += '-imgFile=' + self.get_image_names ( False, layer ) + ' '

    cmd += options
    
    return cmd
  #
  # get_vr_file_names
  #
  def get_vr_file_names ( self, frame_number = True, layer = None  ) :
    #
    filename = cmds.workspace ( expandName = self.vr_param [ 'vr_filename' ] )
    ( name, ext ) = os.path.splitext ( filename )
    pad_str = getPadStr ( self.vr_param [ 'vr_padding' ], self.vr_param [ 'vr_perframe' ] )
    # add layer to filename if there are some render layers
    # besides the masterLayer
    if len ( getRenderLayersList ( False ) ) > 1 and layer is not None:
      name += '_' + layer
    if frame_number and pad_str != '' :
      name += '_' + ('@' + pad_str + '@') # hardcoded in vrend ???
    return ( name + '.vrscene' )
  #
  # get_image_names
  #
  def get_image_names ( self, frame_number = True, layer = None ) :
    #
    pad_str = getPadStr ( self.vr_param [ 'vr_padding' ], self.vr_param [ 'vr_perframe' ] )
    fileNamePrefix = self.getImageFileNamePrefix ()
    ext = self.getImageFormat ()
    images_rule = cmds.workspace ( fileRuleEntry = 'images' )
    images_dir = cmds.workspace ( expandName = images_rule )
    #images = cmds.renderSettings( fullPath = True, genericFrameImageName = ('@' + pad_str + '@')  )
    # add layer to filename if there are some render layers
    # besides the masterLayer
    if len ( getRenderLayersList ( False ) ) > 1 and layer is not None :
      images_dir += '/' + layer
    imageFileName = images_dir + '/' + fileNamePrefix
    if frame_number and pad_str != '' :
      imageFileName += '.' + ('@' + pad_str + '@')
    imageFileName += '.' + ext
    return fromNativePath ( imageFileName )
  #
  # get_vrgen_options
  #
  def get_vrgen_options ( self, layer = None ) :
    #
    renderCamList = getRenderCamerasList ()
    currentCam = renderCamList [ 0 ]
    print 'Using camera : %s' % currentCam
    vrgen_cmd = ( ' -cam ' + currentCam )
    if layer is not None : vrgen_cmd += ( ' -layer ' + layer + ' ' )
    return vrgen_cmd
  #
  # generate_vrscene
  #
  def generate_vrscene ( self, isSubmitingJob = False ):
    #
    skipExport = False
    exportAllRenderLayers = self.vr_param [ 'vr_export_all_layers' ]

    animation    = self.job_param [ 'job_animation' ]
    start        = self.job_param [ 'job_start' ]
    stop         = self.job_param [ 'job_end' ]
    step         = self.job_param [ 'job_step' ]
    vr_reuse     = self.vr_param [ 'vr_reuse' ]
    vr_filename  = self.vr_param [ 'vr_filename' ]
    vr_padding   = self.vr_param [ 'vr_padding' ]
    vr_perframe  = self.vr_param [ 'vr_perframe' ]
    vr_separate  = self.vr_param [ 'vr_separate' ]
    vr_export_lights    = self.vr_param [ 'vr_export_lights' ]
    vr_export_nodes     = self.vr_param [ 'vr_export_nodes' ]
    vr_export_geometry  = self.vr_param [ 'vr_export_geometry' ]
    vr_export_materials = self.vr_param [ 'vr_export_materials' ]
    vr_export_textures  = self.vr_param [ 'vr_export_textures' ]
    vr_export_bitmaps   = self.vr_param [ 'vr_export_bitmaps' ]
    vr_hex_mesh         = self.vr_param [ 'vr_hex_mesh' ]
    vr_hex_transform    = self.vr_param [ 'vr_hex_transform' ]
    vr_compression      = self.vr_param [ 'vr_compression' ]
    vr_deferred         = self.vr_param [ 'vr_deferred' ]
    vray_distributed    = self.vray_param [ 'vray_distributed' ]

    full_filename = cmds.workspace( expandName = vr_filename )
    ( filename, ext ) = os.path.splitext ( full_filename )

    dirname = os.path.dirname ( filename )
    if not os.path.exists ( dirname ):
      print ( "path %s not exists... let's create it!" ) % dirname
      os.mkdir ( dirname )

    # use full_filename for vrscene if there are some render layers
    # besides the masterLayer
    fileName = fromNativePath ( full_filename )
    if len ( getRenderLayersList ( False ) ) == 1 :
      fileName = cmds.workspace ( projectPath = fileName )

    # TODO!!! check if files are exist and have to be overriden
    if isSubmitingJob and vr_reuse:
      skipExport = True
      print 'Skipping .vrscene files generation ...'

    if not skipExport:
      if not animation :
        start = stop = int ( cmds.currentTime ( q = True ) )
        step = 1
      #
      # save RenderGlobals
      #
      defGlobals = 'defaultRenderGlobals'
      vraySettings = 'vraySettings'
      saveMayaGlobals = {}
      saveVrayGlobals = {}
      saveMayaGlobals [ 'animation' ]  = cmds.getAttr ( defGlobals + '.animation' )
      saveMayaGlobals [ 'startFrame' ] = cmds.getAttr ( defGlobals + '.startFrame' )
      saveMayaGlobals [ 'endFrame' ]   = cmds.getAttr ( defGlobals + '.endFrame' )
      saveMayaGlobals [ 'byFrameStep'] = cmds.getAttr ( defGlobals + '.byFrameStep' )

      name = cmds.getAttr ( vraySettings + '.fileNamePrefix' )
      if name is None : name = ''
      saveVrayGlobals [ 'fileNamePrefix' ]    = str ( name )
      saveVrayGlobals [ 'fileNamePadding' ]   = cmds.getAttr ( vraySettings + '.fileNamePadding' )
      saveVrayGlobals [ 'animBatchOnly' ]     = cmds.getAttr ( vraySettings + '.animBatchOnly' )
      saveVrayGlobals [ 'vrscene_render_on' ] = cmds.getAttr ( vraySettings + '.vrscene_render_on' )
      saveVrayGlobals [ 'vrscene_on' ]        = cmds.getAttr ( vraySettings + '.vrscene_on' )

      name = cmds.getAttr ( vraySettings + '.vrscene_filename' )
      if name is None : name = ''
      saveVrayGlobals [ 'vrscene_filename' ]     = str ( name )
      saveVrayGlobals [ 'misc_eachFrameInFile' ] = cmds.getAttr ( vraySettings + '.misc_eachFrameInFile' )
      saveVrayGlobals [ 'misc_separateFiles' ]   = cmds.getAttr ( vraySettings + '.misc_separateFiles' )
      saveVrayGlobals [ 'misc_exportLights' ]    = cmds.getAttr ( vraySettings + '.misc_exportLights' )
      saveVrayGlobals [ 'misc_exportNodes' ]     = cmds.getAttr ( vraySettings + '.misc_exportNodes' )
      saveVrayGlobals [ 'misc_exportGeometry' ]  = cmds.getAttr ( vraySettings + '.misc_exportGeometry' )
      saveVrayGlobals [ 'misc_exportMaterials' ] = cmds.getAttr ( vraySettings + '.misc_exportMaterials' )
      saveVrayGlobals [ 'misc_exportTextures' ]  = cmds.getAttr ( vraySettings + '.misc_exportTextures' )
      saveVrayGlobals [ 'misc_exportBitmaps' ]   = cmds.getAttr ( vraySettings + '.misc_exportBitmaps' )
      saveVrayGlobals [ 'misc_meshAsHex' ]       = cmds.getAttr ( vraySettings + '.misc_meshAsHex' )
      saveVrayGlobals [ 'misc_transformAsHex' ]  = cmds.getAttr ( vraySettings + '.misc_transformAsHex' )
      saveVrayGlobals [ 'misc_compressedVrscene' ] = cmds.getAttr ( vraySettings + '.misc_compressedVrscene' )
      saveVrayGlobals [ 'sys_distributed_rendering_on' ] = cmds.getAttr ( vraySettings + '.sys_distributed_rendering_on' )
      #
      # override RenderGlobals
      #
      cmds.setAttr ( defGlobals + '.animation', True ) # even for single frame, for proper image name
      cmds.setAttr ( defGlobals + '.startFrame', start )
      cmds.setAttr ( defGlobals + '.endFrame', stop )
      cmds.setAttr ( defGlobals + '.byFrameStep', step )
      cmds.setAttr ( vraySettings + '.fileNamePrefix', self.getImageFileNamePrefix (), type = 'string' ) # will use MayaSceneName if empty
      cmds.setAttr ( vraySettings + '.fileNamePadding', vr_padding )
      cmds.setAttr ( vraySettings + '.animBatchOnly', False )
      cmds.setAttr ( vraySettings + '.vrscene_render_on', False )
      cmds.setAttr ( vraySettings + '.vrscene_on', True )
      cmds.setAttr ( vraySettings + '.vrscene_filename', fileName, type = 'string' )
      cmds.setAttr ( vraySettings + '.misc_eachFrameInFile', vr_perframe )
      cmds.setAttr ( vraySettings + '.misc_separateFiles', vr_separate )
      if vr_separate :
        cmds.setAttr ( vraySettings + '.misc_exportLights', vr_export_lights )
        cmds.setAttr ( vraySettings + '.misc_exportNodes', vr_export_nodes )
        cmds.setAttr ( vraySettings + '.misc_exportGeometry', vr_export_geometry )
        cmds.setAttr ( vraySettings + '.misc_exportMaterials', vr_export_materials )
        cmds.setAttr ( vraySettings + '.misc_exportTextures', vr_export_textures )
        cmds.setAttr ( vraySettings + '.misc_exportBitmaps', vr_export_bitmaps )
      cmds.setAttr ( vraySettings + '.misc_meshAsHex', vr_hex_mesh )
      cmds.setAttr ( vraySettings + '.misc_transformAsHex', vr_hex_transform )
      cmds.setAttr ( vraySettings + '.misc_compressedVrscene', vr_compression )
      cmds.setAttr ( vraySettings + '.sys_distributed_rendering_on', vray_distributed )

      if vr_deferred :
      # generate uniquie maya scene name and save it
      # with current render and .vrscene generation settings
        print 'Use deferred .vrscene generation'
        #cmds.setAttr( defGlobals + '.imageFilePrefix', ( 'images/' + image_name ), type='string' )
        scene_name = getMayaSceneName ( withoutSubdir = False ) # get scene name without extension
        def_scene_name = scene_name + '_deferred'

        cmds.file ( rename=def_scene_name )
        self.def_scene_name = cmds.file ( save = True, de = True ) # save it with default extension
        cmds.file ( rename = scene_name ) # rename scene back

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
          saveMayaGlobals [ 'renderableLayer' ] = cmds.getAttr ( layer + '.renderable' )
          cmds.setAttr ( layer + '.renderable', True )
          # print 'set current layer renderable (%s)' % layer

          cmds.editRenderLayerGlobals ( currentRenderLayer = layer )

          vrgen_cmd = self.vrgenCommand  + ' ' + self.get_vrgen_options ( layer )
          print 'vrgen_cmd = %s' % vrgen_cmd
          mel.eval ( vrgen_cmd )

          cmds.setAttr ( layer + '.renderable', saveMayaGlobals [ 'renderableLayer' ] )

        if exportAllRenderLayers :
          # restore current layer
          cmds.editRenderLayerGlobals ( currentRenderLayer = current_layer )
      #
      # restore RenderGlobals
      #
      cmds.setAttr ( defGlobals + '.animation',   saveMayaGlobals [ 'animation' ] )
      cmds.setAttr ( defGlobals + '.startFrame',  saveMayaGlobals [ 'startFrame' ] )
      cmds.setAttr ( defGlobals + '.endFrame',    saveMayaGlobals [ 'endFrame' ] )
      cmds.setAttr ( defGlobals + '.byFrameStep', saveMayaGlobals [ 'byFrameStep' ] )
      name = saveVrayGlobals ['fileNamePrefix' ]
      if name is None : name = ''
      cmds.setAttr ( vraySettings + '.fileNamePrefix',    str ( name ), type = 'string' )
      cmds.setAttr ( vraySettings + '.fileNamePadding',   saveVrayGlobals [ 'fileNamePadding' ] )
      cmds.setAttr ( vraySettings + '.animBatchOnly',     saveVrayGlobals [ 'animBatchOnly' ] )
      cmds.setAttr ( vraySettings + '.vrscene_render_on', saveVrayGlobals [ 'vrscene_render_on' ] )
      cmds.setAttr ( vraySettings + '.vrscene_on',        saveVrayGlobals [ 'vrscene_on' ] )
      name = saveVrayGlobals [ 'vrscene_filename' ]
      if name is None : name = ''
      cmds.setAttr ( vraySettings + '.vrscene_filename',       str ( name ), type = 'string' )
      cmds.setAttr ( vraySettings + '.misc_eachFrameInFile',   saveVrayGlobals [ 'misc_eachFrameInFile' ] )
      cmds.setAttr ( vraySettings + '.misc_separateFiles',     saveVrayGlobals [ 'misc_separateFiles' ] )
      cmds.setAttr ( vraySettings + '.misc_exportLights',      saveVrayGlobals [ 'misc_exportLights' ] )
      cmds.setAttr ( vraySettings + '.misc_exportNodes',       saveVrayGlobals [ 'misc_exportNodes' ] )
      cmds.setAttr ( vraySettings + '.misc_exportGeometry',    saveVrayGlobals [ 'misc_exportGeometry' ] )
      cmds.setAttr ( vraySettings + '.misc_exportMaterials',   saveVrayGlobals ['misc_exportMaterials' ] )
      cmds.setAttr ( vraySettings + '.misc_exportTextures',    saveVrayGlobals ['misc_exportTextures' ] )
      cmds.setAttr ( vraySettings + '.misc_exportBitmaps',     saveVrayGlobals [ 'misc_exportBitmaps' ] )
      cmds.setAttr ( vraySettings + '.misc_meshAsHex',         saveVrayGlobals [ 'misc_meshAsHex' ] )
      cmds.setAttr ( vraySettings + '.misc_transformAsHex',    saveVrayGlobals [ 'misc_transformAsHex' ] )
      cmds.setAttr ( vraySettings + '.misc_compressedVrscene', saveVrayGlobals [ 'misc_compressedVrscene' ] )
      cmds.setAttr ( vraySettings + '.sys_distributed_rendering_on', saveVrayGlobals [ 'sys_distributed_rendering_on' ] )
  #
  # submitJob
  #
  def submitJob( self, param = None ) :
    #
    job_dispatcher   = self.job_param [ 'job_dispatcher' ]
    job_description  = self.job_param [ 'job_description' ]
    job_name         = str ( self.job_param [ 'job_name' ] ).strip ()
    if job_name == '' : job_name = getMayaSceneName ()

    vr_deferred      = self.vr_param [ 'vr_deferred' ]
    vr_local_vrgen   = self.vr_param [ 'vr_local_vrgen' ]
    vr_def_task_size = self.vr_param [ 'vr_def_task_size' ]
    vr_reuse         = self.vr_param [ 'vr_reuse' ]
    exportAllRenderLayers = self.vr_param [ 'vr_export_all_layers' ]

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
      print 'backburner is not supported in this version'
      #self.job = MentalRayBackburnerJob ( job_name, job_description )
      return
    else :
      vr_deferred = False
      self.job = RenderJob ( job_name, job_description )

    self.job.work_dir  = self.rootDir
    self.job.padding   = self.vr_param [ 'vr_padding' ]
    self.job.priority  = self.job_param [ 'job_priority' ]
    self.job.paused    = self.job_param [ 'job_paused' ]
    self.job.task_size = self.job_param [ 'job_size' ]
    self.job.animation = self.job_param [ 'job_animation' ]
    self.job.start     = self.job_param [ 'job_start' ]
    self.job.stop      = self.job_param [ 'job_end' ]
    self.job.step      = self.job_param [ 'job_step' ]

    self.generate_vrscene ( True ) # isSubmitingJob=True

    self.job.setup_range ( int( cmds.currentTime ( q = True ) ) )
    self.job.setup ()

    # save current layer
    current_layer = cmds.editRenderLayerGlobals ( q = True, currentRenderLayer = True )

    if job_dispatcher == 'afanasy' :
      if vr_deferred and not vr_reuse:
        if exportAllRenderLayers :
          gen_cmd = self.getDeferredCmd ( None )
        else :
          layer_name = current_layer
          if current_layer == 'defaultRenderLayer' : layer_name = 'masterLayer'
          gen_cmd = self.getDeferredCmd ( layer_name )
        gen_cmd += ( ' -s @#@' )
        gen_cmd += ( ' -e @#@' )
        gen_cmd += ( ' -b ' + str ( self.job.step ) )
        print 'gen_cmd = %s %s' % ( gen_cmd, self.def_scene_name )

        self.job.gen_block = AfanasyRenderBlock ( 'generate_vrscene', deferred_service, self.job, vr_local_vrgen )
        self.job.gen_block.capacity = deferred_capacity
        self.job.gen_block.cmd = gen_cmd
        self.job.gen_block.input_files = self.def_scene_name
        self.job.gen_block.task_size = min ( vr_def_task_size, self.job.num_tasks )
        self.job.gen_block.setup ()

      renderLayers = []
      if exportAllRenderLayers :
        renderLayers = getRenderLayersList ( True ) # renderable only
      else :
        # use only current layer
        renderLayers.append ( current_layer )
      
      createLayersFolders = len ( getRenderLayersList ( False ) ) > 1
      
      for layer in renderLayers :
        cmds.editRenderLayerGlobals ( currentRenderLayer = layer )
        layer_name = layer
        if layer == 'defaultRenderLayer' : layer_name = 'masterLayer'
        
        if createLayersFolders :  
          # create folders for Render Layers
          images_rule = cmds.workspace ( fileRuleEntry = 'images' )
          images_dir = cmds.workspace ( expandName = images_rule )
          layer_dir = os.path.join ( images_dir, layer_name )
          if not os.path.exists ( layer_dir ) : 
            print '-> Create missing dir %s' % layer_dir    
            os.makedirs ( layer_dir )

        frame_block = AfanasyRenderBlock ( ('render_' + layer_name ), service, self.job )
        frame_block.capacity = capacity
        frame_block.input_files = self.get_vr_file_names ( True, layer_name )
        frame_block.out_files = self.get_image_names ( True, layer_name )
        
        render_cmd = self.getRenderCmd ( layer_name )
        
        if self.vray_param [ 'vray_distributed' ] :
          frame_block.distributed = True
          frame_block.hosts_min = self.vray_param [ 'vray_hosts_min' ]
          frame_block.hosts_max = self.vray_param [ 'vray_hosts_max' ]
          if frame_block.hosts_max <= 0 : frame_block.hosts_max = 1 
          if frame_block.hosts_min > frame_block.hosts_max : frame_block.hosts_min = 1

          render_cmd += ' -distributed=1 '
          transferAssets = vray_transfer_assets_list.index ( self.vray_param [ 'vray_transfer_assets' ] )
          render_cmd += ( ' -transferAssets=%d ' % transferAssets )

          hosts_str = str ( self.vray_param [ 'vray_hosts' ] ).strip ()
          if hosts_str != '' :
            hosts = ' -renderhost='
            hosts_list = hosts_str.split ( ' ' )
            hosts_str = (';').join ( hosts_list )
            hosts += ( '"' + hosts_str + '"' )
            render_cmd += ( hosts + ' ') 
          else :
            render_cmd += ' @AF_HOSTS@ '  
        
        render_cmd += ' -sceneFile='
        
        frame_block.cmd = render_cmd
        
        frame_block.setup ()
        self.job.frames_blocks.append ( frame_block )

      self.job.process ()

    if exportAllRenderLayers :
      # restore current layer
      cmds.editRenderLayerGlobals ( currentRenderLayer = current_layer )
  #
  # vrFileNameChanged
  #
  def vrFileNameChanged ( self, name, value ) :
    #
    if name == 'vr_filename' :
      setDefaultStrValue ( self_prefix, name, self.vr_param, value )
    else:
      setDefaultIntValue ( self_prefix, name, self.vr_param, value )
    self.setResolvedPath ()
  #
  # setResolvedPath
  #
  def setResolvedPath ( self ) :
    #
    filename = cmds.workspace ( expandName = self.vr_param [ 'vr_filename' ] )
    ( filename, ext ) = os.path.splitext ( filename )

    # add layer to filename if there are some render layers
    # besides the masterLayer
    if len ( getRenderLayersList ( False ) ) > 1 :
      filename += '_' + str ( self.layer )

    if self.vr_param[ 'vr_padding' ] > 0 and self.vr_param[ 'vr_perframe' ] == True :
      filename += '_' # hardcoded in vrend ???
      pad_str = getPadStr ( self.vr_param [ 'vr_padding' ], self.vr_param [ 'vr_perframe' ] )
      filename += pad_str

    ext = '.vrscene'
    filename += ext
    cmds.textFieldGrp ( self.winMain + '|f0|t0|tc1|fr2|fc2|' + 'vr_resolved_path', edit = True, text = filename )
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
  # enable_separate
  #
  def enable_separate ( self, arg ) :
    #
    vr = self.winMain + '|f0|t0|tc1|fr4|fc4|'
    setDefaultIntValue ( self_prefix, 'vr_separate', self.vr_param, arg )
    cmds.checkBoxGrp ( vr + 'vr_export_lights', edit = True, enable = arg )
    cmds.checkBoxGrp ( vr + 'vr_export_nodes', edit = True, enable = arg )
    cmds.checkBoxGrp ( vr + 'vr_export_geometry', edit = True, enable = arg )
    cmds.checkBoxGrp ( vr + 'vr_export_materials', edit = True, enable = arg )
    cmds.checkBoxGrp ( vr + 'vr_export_textures', edit = True, enable = arg )
    cmds.checkBoxGrp ( vr + 'vr_export_bitmaps', edit = True, enable = arg )
  #
  # enable_deferred
  #
  def enable_deferred ( self, arg ) :
    #
    vr_def_frame = self.winMain + '|f0|t0|tc1|fr3'
    vr_def       = vr_def_frame + '|fc3|'
    setDefaultIntValue ( self_prefix, 'vr_deferred', self.vr_param, arg )
    cmds.checkBoxGrp ( vr_def + 'vr_local_vrgen', edit = True, enable = arg )
    cmds.intFieldGrp ( vr_def + 'vr_def_task_size', edit = True, enable = arg )
    bg_color = self.save_frame_bgc
    if arg : bg_color = self.def_frame_bgc
    cmds.frameLayout ( vr_def_frame, edit = True, bgc = bg_color ) # , enableBackground=False
  #
  # enable_distributed
  #
  def enable_distributed ( self, arg ) :
    vray_distr_frame = self.winMain + '|f0|t0|tc2|fr2'
    vray_distr = vray_distr_frame + '|fc2|'
    setDefaultIntValue ( self_prefix, 'vray_distributed', self.vray_param, arg )
    cmds.checkBoxGrp ( vray_distr + 'vray_nomaster', edit = True, enable = arg )
    #cmds.intFieldGrp ( vray_distr + 'vray_port', edit = True, enable = arg )
    cmds.intFieldGrp ( vray_distr + 'vray_hosts_min', edit = True, enable = arg )
    cmds.intFieldGrp ( vray_distr + 'vray_hosts_max', edit = True, enable = arg )
    #cmds.intFieldGrp ( vray_distr + 'vray_threads_limit', edit = True, enable = arg )
    cmds.optionMenuGrp ( vray_distr + 'vray_transfer_assets', edit = True, enable = arg )  
    cmds.textFieldGrp ( vray_distr + 'vray_hosts', edit = True, enable = arg )
    bg_color = self.save_frame_bgc 
    if arg : bg_color = self.def_frame_bgc
    cmds.frameLayout ( vray_distr_frame, edit = True, bgc = bg_color ) # , enableBackground=False
  #
  # onRenderLayerSelected
  #
  def onRenderLayerSelected ( self, arg ) :
    #
    self.layer = arg
    if self.layer == 'masterLayer' : arg = 'defaultRenderLayer'
    #print '* onRenderLayerSelected %s' % self.layer
    cmds.evalDeferred ( 'import maya.OpenMaya; maya.cmds.editRenderLayerGlobals( currentRenderLayer = "' + str ( arg ) + '" )', lowestPriority=True  )
  #
  # renderLayerSelected
  #
  def renderLayerSelected ( self ) :
    #
    self.layer = cmds.editRenderLayerGlobals ( query = True, currentRenderLayer = True )
    if self.layer == 'defaultRenderLayer' : self.layer = 'masterLayer'
    print '* renderLayerSelected %s' % self.layer
    cmds.optionMenuGrp ( self.winMain + '|f0|c0|r0|' + 'layer_selector', e = True, value = self.layer )
    self.setResolvedPath ()
  #
  # renderLayerRenamed
  #
  def renderLayerRenamed ( self ) :
    #
    self.layer = cmds.editRenderLayerGlobals ( query = True, currentRenderLayer = True )
    #print '* renderLayerRenamed %s' % self.layer
    # self.updateRenderLayerMenu()
    cmds.evalDeferred ( partial ( self.updateRenderLayerMenu ), lowestPriority=True )
  #
  # renderLayerChanged
  #
  def renderLayerChanged ( self ) :
    #
    self.layer = cmds.editRenderLayerGlobals ( query = True, currentRenderLayer = True )
    #print '* renderLayerChanged %s' % self.layer
    #self.updateRenderLayerMenu()
    cmds.evalDeferred ( partial ( self.updateRenderLayerMenu ), lowestPriority=True )
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
    cmds.optionMenuGrp ( self.winMain + '|f0|c0|r0|' + 'layer_selector', e = True, value = self.layer )

    #self.renderLayersSetup( renderLayers )
    cmds.evalDeferred ( partial ( self.renderLayersSetup, renderLayers ), lowestPriority = True )
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
        cmds.scriptJob ( nodeNameChanged = [ layer, 'import maya.OpenMaya; maya.cmds.evalDeferred( "meVRayRender.renderLayerRenamed()" , lowestPriority = True )' ],
                         parent = selector,
                         replacePrevious = firstRun )
        firstRun = False
  #
  # setupUI
  #
  def setupUI ( self ) :
    #
    # Main window setup
    #
    self.deleteUI ( True )
    self.winMain = cmds.window ( meVRayRenderMainWnd,
                                title = 'meVRayRender ver.' + meVRayRenderVer + ' (' + self.os + ')' ,
                                menuBar = True,
                                retain = False,
                                widthHeight = ( 420, 460 ) )

    self.mainMenu = cmds.menu ( label = 'Commands', tearOff = False )
    cmds.menuItem ( label = 'Render Globals ...',      command = maya_render_globals )
    cmds.menuItem ( label = 'Check Texture Paths ...', command = checkTextures )
    cmds.menuItem ( label = 'Generate .vrscene',       command = self.generate_vrscene )
    cmds.menuItem ( label = 'Submit Job',              command = self.submitJob )
    cmds.menuItem ( divider = True )
    cmds.menuItem ( label = 'Close',                   command = self.deleteUI )
    #
    # setup render layers script jobs
    #
    cmds.scriptJob ( attributeChange = [ 'renderLayerManager.currentRenderLayer',
                                         'import maya.OpenMaya; maya.cmds.evalDeferred( "meVRayRender.renderLayerSelected()" , lowestPriority = True )' ],
                                          parent = self.winMain  )
    cmds.scriptJob (           event = [ 'renderLayerChange',
                                         'import maya.OpenMaya; maya.cmds.evalDeferred( "meVRayRender.renderLayerChanged()" , lowestPriority = True )' ],
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

    cmds.checkBoxGrp ( 'vr_export_all_layers', cw = ( ( 1, cw1 ), ( 2, cw1 * 2 ) ),
                       label = 'Export All Renderable ',
                       value1 = self.vr_param [ 'vr_export_all_layers' ],
                       cc = partial ( setDefaultIntValue, self_prefix, 'vr_export_all_layers', self.vr_param ) )
    cmds.setParent ( '..' )
    cmds.setParent ( '..' )
    #
    # setup tabs
    #
    tab = cmds.tabLayout ( 't0', scrollable = True, childResizable = True )  # tabLayout -scr true -cr true  tabs; //
    #
    # Job tab
    #
    tab_job = cmds.columnLayout ( 'tc0', columnAttach = ('left',0), rowSpacing = 0, adjustableColumn = True )
    cmds.frameLayout ( 'fr1', label =' Parameters ', borderVisible = True, borderStyle = 'etchedIn', marginHeight = mr_hi  )
    cmds.columnLayout ( 'fc1', columnAttach = ( 'left', 0 ), rowSpacing = 0, adjustableColumn = True )
    #
    job_dispatcher = cmds.optionMenuGrp ( 'job_dispatcher', cw = ( ( 1, cw1 ), ), 
                        cal = ( 1, 'right' ),
                        label = 'Job Dispatcher ',
                        cc = partial ( setDefaultStrValue, self_prefix, 'job_dispatcher', self.job_param ) )
    for name in ( 'none', 'afanasy' ) : cmds.menuItem ( label = name ) # 'backburner',
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
                       cc = ( partial ( setDefaultIntValue3, self_prefix, ( 'job_start', 'job_end', 'job_step' ), self.job_param ) ) )
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
                       marginHeight = mr_hi, cll = True, cl = True  )
    cmds.columnLayout( 'fc2', columnAttach = ( 'left', 0 ), rowSpacing = 0, adjustableColumn = True )
    cmds.checkBoxGrp ( 'job_cleanup_vr', cw = ( ( 1, cw1 ), ( 2, cw1 * 2 ) ),
                       label = '',
                       label1 = ' .vrscene files',
                       value1 = self.job_param [ 'job_cleanup_vr' ],
                       enable = False,
                       cc = partial ( setDefaultIntValue, self_prefix, 'job_cleanup_vr', self.job_param ) )
    cmds.checkBoxGrp ( 'job_cleanup_script', cw = ( ( 1, cw1 ), ( 2, cw1 * 2 ) ),
                       label = '',
                       label1 = ' script file',
                       value1 = self.job_param [ 'job_cleanup_script' ],
                       enable = False,
                       cc = partial ( setDefaultIntValue, self_prefix, 'job_cleanup_script', self.job_param ) )
    cmds.setParent ( '..' )
    cmds.setParent ( '..' )
    cmds.setParent ( '..' )
    #
    # .vrscene files generation tab
    #
    tab_vrparam = cmds.columnLayout ( 'tc1', columnAttach = ( 'left',0 ), rowSpacing = 0, adjustableColumn = True )
    # Deferred generation frame
    fr3 = cmds.frameLayout ( 'fr3', label = ' Deferred .vrscene generation ',
                       borderVisible = True,
                       borderStyle = 'etchedIn',
                       marginHeight = mr_hi,
                       cll = True, cl = True  )
    cmds.columnLayout ( 'fc3', columnAttach = ( 'left', 0 ), rowSpacing = 0, adjustableColumn = True )
    cmds.checkBoxGrp ( 'vr_deferred', cw = ( ( 1, cw1 ), ( 2, cw1 * 2 ) ),
                      label = 'Use deferred ',
                      ann = 'Generate .vrscene files in background process',
                      value1 = self.vr_param [ 'vr_deferred' ],
                      cc = partial ( self.enable_deferred )) # self.setDefaultIntValue, 'rib_deferred_ribgen' ) )
    cmds.checkBoxGrp ( 'vr_local_vrgen', cw = ( ( 1, cw1 ), ( 2, cw1 * 2 ) ),
                      label = '',
                      label1 = ' Only on localhost ',
                      ann = 'Do not use remote hosts',
                      value1 = self.vr_param [ 'vr_local_vrgen' ],
                      enable = self.vr_param [ 'vr_deferred' ],
                      cc = partial ( setDefaultIntValue, self_prefix, 'vr_local_vrgen', self.vr_param ) )
    cmds.intFieldGrp ( 'vr_def_task_size', cw = ( ( 1, cw1 ), ( 2, cw2 ) ),
                      label = 'Task Size ',
                      value1 = self.vr_param [ 'vr_def_task_size' ],
                      enable = self.vr_param [ 'vr_deferred' ],
                      cc = partial ( setDefaultIntValue, self_prefix, 'vr_def_task_size', self.vr_param ) )
    bg_color = self.save_frame_bgc
    if self.vr_param [ 'vr_deferred' ] : bg_color = self.def_frame_bgc
    cmds.frameLayout ( fr3, edit = True, bgc = bg_color ) # , enableBackground=False
    cmds.setParent ( '..' )
    cmds.setParent ( '..' )
    # .vrscene Export Settings frame
    cmds.frameLayout ( 'fr1', label = ' Export Settings ',
                       borderVisible = True,
                       borderStyle = 'etchedIn',
                       marginHeight = mr_hi  )
    cmds.columnLayout ( 'fc1', columnAttach = ( 'left', 0 ), rowSpacing = 0, adjustableColumn = True )
    cmds.checkBoxGrp ( 'vr_reuse', cw = ( ( 1, cw1 ), ( 2, cw1 * 2 ) ),
                      label = 'Use existing .vrscene ',
                      ann = 'Do not generate .vrscene files if they are exist',
                      value1 = self.vr_param [ 'vr_reuse' ],
                      cc = partial ( setDefaultIntValue, self_prefix, 'vr_reuse', self.vr_param ) )
    cmds.text( label = '' )
    vr_filename = cmds.textFieldButtonGrp ( 'vr_filename', cw = ( 1, cw1 ), adj = 2,
                      label = 'File Name ',
                      buttonLabel = '...',
                      text = self.vr_param [ 'vr_filename' ],
                      cc = partial ( self.vrFileNameChanged, 'vr_filename' ) )
    cmds.textFieldButtonGrp ( vr_filename,
                      edit = True,
                      bc = partial ( browseFile, self.rootDir, vr_filename, 'vray scene files (*.vrscene)' ) )
    cmds.intFieldGrp ( 'vr_padding', cw=( ( 1, cw1 ), ( 2, cw2 ) ),
                      label = 'Frame Padding ',
                      value1 = self.vr_param [ 'vr_padding' ],
                      cc = partial ( self.vrFileNameChanged, 'vr_padding' ) )
    cmds.checkBoxGrp ( 'vr_perframe', cw = ( ( 1, cw1 ), ( 2, cw1 * 2 ) ),
                       label = '',
                       label1 = ' File Per Frame ',
                       value1 = self.vr_param ['vr_perframe' ],
                       cc = partial ( self.vrFileNameChanged, 'vr_perframe' ) )
    cmds.checkBoxGrp ( 'vr_hex_mesh', cw = ( ( 1, cw1 ), ( 2, cw1 * 2 ) ),
                       label = 'Write in hex format ',
                       label1 = ' mesh data',
                       value1 = self.vr_param [ 'vr_hex_mesh' ],
                       cc = partial ( setDefaultIntValue, self_prefix, 'vr_hex_mesh', self.vr_param ) )
    cmds.checkBoxGrp ( 'vr_hex_transform', cw = ( ( 1, cw1 ), ( 2, cw1 * 2 ) ),
                       label = '',
                       label1 = ' transform data',
                       value1 = self.vr_param [ 'vr_hex_transform' ],
                       cc = partial ( setDefaultIntValue, self_prefix, 'vr_hex_transform', self.vr_param ) )
    cmds.checkBoxGrp ( 'vr_compression', cw = ( ( 1, cw1 ), ( 2, cw1 * 2 ) ),
                       label = 'Compressed ',
                       label1 = '',
                       value1 = self.vr_param [ 'vr_compression' ],
                       cc = partial ( setDefaultIntValue, self_prefix, 'vr_compression', self.vr_param ) )
    cmds.setParent ( '..' )
    cmds.setParent ( '..' )
    # .vrscene Separate Files frame
    cmds.frameLayout ( 'fr4', label = ' Separate Files ',
                       borderVisible = True,
                       borderStyle = 'etchedIn',
                       marginHeight = mr_hi,
                       cll = True, cl = True  )
    cmds.columnLayout ( 'fc4', columnAttach = ( 'left', 0 ), rowSpacing = 0, adjustableColumn = True )
    cmds.checkBoxGrp ( 'vr_separate', cw=( (1, cw1), (2, cw1 * 2 ) ),
                       label = 'Use Separate ',
                       label1 = '',
                       value1 = self.vr_param [ 'vr_separate' ],
                       cc = partial ( self.enable_separate )) #partial( self.setDefaultIntValue, 'vr_separate' ) )
    cmds.checkBoxGrp ( 'vr_export_lights', cw = ( (1, cw1 ), ( 2, cw1 * 2 ) ),
                       label = '',
                       label1 = ' Lights',
                       value1 = self.vr_param [ 'vr_export_lights' ],
                       enable = self.vr_param [ 'vr_separate' ],
                       cc = partial ( setDefaultIntValue, self_prefix, 'vr_export_lights', self.vr_param ) )
    cmds.checkBoxGrp ( 'vr_export_nodes', cw = ( ( 1, cw1 ), ( 2, cw1 * 2 ) ),
                       label = '',
                       label1 = ' Nodes',
                       value1 = self.vr_param [ 'vr_export_nodes' ],
                       enable = self.vr_param [ 'vr_separate' ],
                       cc = partial ( setDefaultIntValue, self_prefix, 'vr_export_nodes', self.vr_param ) )
    cmds.checkBoxGrp ( 'vr_export_geometry', cw = ( ( 1, cw1 ), ( 2, cw1 * 2 ) ),
                       label = '',
                       label1 = ' Geometry',
                       value1 = self.vr_param [ 'vr_export_geometry' ],
                       enable = self.vr_param [ 'vr_separate' ],
                       cc = partial ( setDefaultIntValue, self_prefix, 'vr_export_geometry', self.vr_param ) )
    cmds.checkBoxGrp ( 'vr_export_materials', cw = ( ( 1, cw1 ), ( 2, cw1 * 2 ) ),
                       label = '',
                       label1 = ' Materials',
                       value1 = self.vr_param [ 'vr_export_materials' ],
                       enable = self.vr_param [ 'vr_separate' ],
                       cc = partial ( setDefaultIntValue, self_prefix, 'vr_export_materials', self.vr_param ) )
    cmds.checkBoxGrp ( 'vr_export_textures', cw = ( ( 1, cw1 ), ( 2, cw1 * 2 ) ),
                       label = '',
                       label1 = ' Textures',
                       value1 = self.vr_param ['vr_export_textures' ],
                       enable = self.vr_param [ 'vr_separate' ],
                       cc = partial ( setDefaultIntValue, self_prefix, 'vr_export_textures', self.vr_param ) )
    cmds.checkBoxGrp ( 'vr_export_bitmaps', cw = ( ( 1, cw1 ), ( 2, cw1 * 2 ) ),
                       label = '',
                       label1 = ' Bitmaps',
                       value1 = self.vr_param [ 'vr_export_bitmaps' ],
                       enable = self.vr_param [ 'vr_separate' ],
                       cc = partial ( setDefaultIntValue, self_prefix, 'vr_export_bitmaps', self.vr_param  ) )
    cmds.setParent ( '..' )
    cmds.setParent ( '..' )
    # Resolved Path frame
    cmds.frameLayout ( 'fr2', label = ' Resolved Path ',
                       borderVisible = True,
                       borderStyle = 'etchedIn',
                       marginHeight = mr_hi  )
    cmds.columnLayout ( 'fc2', columnAttach = ( 'left', 0 ), rowSpacing = 0, adjustableColumn = True )
    cmds.textFieldGrp ( 'vr_resolved_path', cw = ( 1, 0 ), adj = 2,
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
    cmds.frameLayout ( 'fr2', label = ' Distributed render ', 
                       borderVisible = True, 
                       borderStyle = 'etchedIn', 
                       marginHeight = mr_hi, 
                       cll = True, cl = True )
    cmds.columnLayout ( 'fc2', columnAttach = ( 'left', 0 ), rowSpacing = 0, adjustableColumn = True )
    cmds.checkBoxGrp ( 'vray_distributed', cw = ( ( 1, cw1 ), ( 2, cw1 * 2 ) ),
                       label = 'Use distributed ',
                       ann = 'Use slave hosts for rendering',
                       value1 = self.vray_param [ 'vray_distributed' ],
                       cc = partial ( self.enable_distributed ) ) 
    cmds.checkBoxGrp ( 'vray_nomaster', cw = ( ( 1, cw1 ), ( 2, cw1 * 2 ) ),
                       label = '', 
                       label1 = " No Master ",
                       ann = "When rendering with multiple hosts, schedule all jobs on slaves only, if possible",
                       value1 = self.vray_param [ 'vray_nomaster' ],
                       enable = self.vray_param [ 'vray_distributed' ],
                       cc = partial ( setDefaultIntValue, self_prefix, 'vray_nomaster', self.vray_param ) )
    #cmds.intFieldGrp ( 'vray_port', cw = ( ( 1, cw1 ), ( 2, cw2 ) ),
    #                   label = 'Port ',
    #                   value1 = self.vray_param [ 'vray_port' ],
    #                   enable = self.vray_param [ 'vray_distributed' ],
    #                   cc = partial ( setDefaultIntValue, self_prefix, 'vray_port', self.vray_param ) )
    cmds.intFieldGrp ( 'vray_hosts_min', cw = ( ( 1, cw1 ), ( 2, cw2 ) ),
                       label = 'Min Hosts ',
                       value1 = self.vray_param [ 'vray_hosts_min' ],
                       enable = self.vray_param [ 'vray_distributed' ],
                       cc = partial ( setDefaultIntValue, self_prefix, 'vray_hosts_min', self.vray_param ) )
    cmds.intFieldGrp ( 'vray_hosts_max', cw = ( ( 1, cw1 ), ( 2, cw2 ) ),
                       label = 'Max Hosts ',
                       value1 = self.vray_param [ 'vray_hosts_max' ],
                       enable = self.vray_param [ 'vray_distributed' ],
                       cc = partial ( setDefaultIntValue, self_prefix, 'vray_hosts_max', self.vray_param ) )
    vray_transfer = cmds.optionMenuGrp ( 'vray_transfer_assets', cw = ( ( 1, cw1 ), ), cal = ( 1, 'right' ),
                        label = 'Transfer Assets ',
                        cc = partial ( setDefaultStrValue, self_prefix, 'vray_transfer_assets', self.vray_param ) )
    for name in vray_transfer_assets_list : cmds.menuItem ( label = name )
    cmds.optionMenuGrp ( vray_transfer, e = True, value = self.vray_param [ 'vray_transfer_assets' ] )                       
    #cmds.intFieldGrp ( 'vray_threads_limit', cw = ( ( 1, cw1 ), ( 2, cw2 ) ),
    #                   label = 'Threads Limit ',
    #                   ann = 'Max threads per host. All available threads will be used if 0',
    #                   value1 = self.vray_param [ 'vray_threads_limit' ],
    #                   enable = self.vray_param [ 'vray_distributed' ],
    #                   cc = partial ( setDefaultIntValue, self_prefix, 'vray_threads_limit', self.vray_param ) )
    cmds.textFieldGrp ( 'vray_hosts', cw = ( 1, cw1 ), adj = 2,
                       label = 'Remote Hosts ',
                       ann = 'Remote hosts names (if empty, will be filled by Render Manager)',
                       text = self.vray_param [ 'vray_hosts' ],
                       enable = self.vray_param [ 'vray_distributed' ],
                       cc = partial ( setDefaultStrValue, self_prefix, 'vray_hosts', self.vray_param ) )                       
    bg_color = self.save_frame_bgc 
    if self.vray_param [ 'vray_distributed' ] : bg_color = self.def_frame_bgc
    cmds.frameLayout ( self.winMain + '|f0|t0|tc2|fr2', edit = True, bgc = bg_color ) # , enableBackground=False
    cmds.setParent ( '..' )
    cmds.setParent ( '..' )
    cmds.frameLayout ( 'fr1', label = ' VRay command line options ',
                        borderVisible = True,
                        borderStyle = 'etchedIn',
                        marginHeight = mr_hi  )
    cmds.columnLayout ( 'fc1', columnAttach = ( 'left', 0 ),
                        rowSpacing = 0,
                        adjustableColumn = True )
    cmds.textFieldGrp ( 'vray_options', cw = ( 1, cw1 ), adj = 2,
                        label = 'Additional Options ',
                        text = self.vray_param [ 'vray_options' ],
                        cc = partial ( setDefaultStrValue, self_prefix, 'vray_options', self.vray_param ) )
    cmds.text ( label = '' )
    vray_verbosity = cmds.optionMenuGrp ( 'vray_verbosity', cw = ( ( 1, cw1 ), ), cal = ( 1, 'right' ),
                        label = 'Verbosity ',
                        cc = partial ( setDefaultStrValue, self_prefix, 'vray_verbosity', self.vray_param ) )
    for name in vray_verbosity_list : cmds.menuItem ( label = name )
    cmds.optionMenuGrp ( vray_verbosity, e = True, value = self.vray_param [ 'vray_verbosity' ] )
    cmds.intFieldGrp ( 'vray_progress_frequency', cw= ( ( 1, cw1 ), ( 2, cw2 ) ),
                        label = 'Progress frequency ',
                        ann = 'Progress information should be emitted only when this percentage of the whole render time has passed.',
                        value1 = self.vray_param [ 'vray_progress_frequency' ],
                        cc = partial ( setDefaultIntValue, self_prefix, 'vray_progress_frequency', self.vray_param ) )
    cmds.intFieldGrp ( 'vray_threads', cw = ( ( 1, cw1 ), ( 2, cw2 ) ),
                        label = 'Threads ',
                        ann = 'The number of threads.',
                        value1 = self.vray_param [ 'vray_threads' ],
                        cc = partial ( setDefaultIntValue, self_prefix, 'vray_threads', self.vray_param ) )
    cmds.checkBoxGrp ( 'vray_low_thread_priority', cw = ( ( 1, cw1 ), ( 2, cw1 * 2 ) ),
                        label = '',
                        label1 = ' Low Thread Priority',
                        value1 = self.vray_param [ 'vray_low_thread_priority' ],
                        cc = partial ( setDefaultIntValue, self_prefix, 'vray_low_thread_priority', self.vray_param ) )
    cmds.checkBoxGrp ( 'vray_clearRVOn', cw = ( ( 1, cw1 ), ( 2, cw1 * 2 ) ),
                        label = '',
                        label1 = ' Clear Render View',
                        value1 = self.vray_param [ 'vray_clearRVOn' ],
                        cc = partial ( setDefaultIntValue, self_prefix, 'vray_clearRVOn', self.vray_param ) )
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
                    tabLabel = ( ( tab_job, 'Job' ),
                               ( tab_vrparam, '.vrscene' ),
                               ( tab_render, 'Renderer' ),
                               ( tab_afanasy, 'Afanasy' )
                             )
                  )
    cmds.setParent ( form )
    btn_sbm = cmds.button ( label = 'Submit', command = self.submitJob, ann = 'Generate .vrscene files and submit to dispatcher' )
    btn_gen = cmds.button ( label = 'Generate .vrscene', command = self.generate_vrscene, ann = 'Force .vrscene files generation' )
    btn_cls = cmds.button ( label = 'Close', command = self.deleteUI )

    cmds.formLayout( form, edit = True,
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
                      attachControl = ( ( tab, 'top', 0, proj ),
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
  def deleteUI ( self, param ) :
    #
    winMain = meVRayRenderMainWnd
    if cmds.window ( winMain, exists = True )    : cmds.deleteUI ( winMain, window = True )
    if cmds.windowPref ( winMain, exists = True ): cmds.windowPref ( winMain, remove = True )
#
#
#
print 'meVRayRender sourced ...'
