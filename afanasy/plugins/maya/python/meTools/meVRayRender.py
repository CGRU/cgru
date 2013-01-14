"""
  meVRayRender.py

  ver.0.2.0 13 Jan 2013
  ver.0.1.0 10 Jan 2013
  ver.0.0.1 9 Jan 2013

  Author:

  Yuri Meshalkin (aka mesh)
  mesh@kpp.kiev.ua

  (c) Kiev Post Production 2013

  Description:

  This UI script generates .vrscene files from open Maya scene
  and submits them to VRay Standalone directly or by
  creating a job for Afanasy.

  Usage:

  import meTools.meVRayRender as vr
  reload( vr ) # for debugging purposes
  vr = vr.meVRayRender()

  For using with Afanasy, add %AF_ROOT%\plugins\maya\python to %PYTHONPATH%
"""

import sys, os, string
import maya.OpenMaya as OpenMaya
from functools import partial
import maya.cmds as cmds
import maya.mel as mel

from vrayJob import VRayJob, VRayAfanasyJob  #, MentalRayBackburnerJob

self_prefix = 'meVRayRender_'
meVRayRenderVer = '0.2.0'
meVRayRenderMainWnd = self_prefix + 'MainWnd'
#
# meVRayRender
#
class meVRayRender( object ):
  #
  #
  #
  def __init__( self, selection='' ):
    #print ">> meVRayRender: Class created"
    self.selection = selection
    self.winMain = ''

    self.os = sys.platform
    if self.os.startswith('linux') : self.os = 'linux'
    elif self.os == 'darwin' : self.os = 'mac'
    elif self.os == 'win32' : self.os = 'win'

    print 'sys.platform = %s self.os = %s' % ( sys.platform, self.os )
    self.rootDir = cmds.workspace( q=True, rootDirectory=True )
    self.rootDir = self.rootDir[:-1]

    self.job = None
    self.job_param = {}
    self.vr_param = {}
    self.vray_param = {}
    self.img_param = {}
    self.afanasy_param = {}
    self.bkburn_param = {}

    self.vrgenCommand = 'vrend'
    self.def_vrgenCommand = 'Render -r vray'
    self.def_scene_name = '' # maya scene name used for deferred .vrscene generation

    self.initParameters()
    self.ui=self.setupUI()
  #
  #
  #def __del__( self ): print( ">> meVRayRender: Class deleted" )
  #
  #
  def initParameters( self ):
    #
    # Job parameters
    #
    self.job_param['job_dispatcher'] = self.getDefaultStrValue( 'job_dispatcher', 'afanasy' )

    self.job_param['job_name'] = self.getMayaSceneName()
    self.job_param['job_description'] = self.getDefaultStrValue( 'job_description', '' )

    self.job_param['job_animation'] = self.getDefaultIntValue( 'job_animation', 1 ) is 1
    self.job_param['job_start'] = self.getDefaultIntValue( 'job_start', 1 )
    self.job_param['job_end'] = self.getDefaultIntValue( 'job_end', 100 )
    self.job_param['job_step'] = self.getDefaultIntValue( 'job_step', 1 )
    self.job_param['job_size'] = self.getDefaultIntValue( 'job_size', 1 )
    self.job_param['job_paused'] = self.getDefaultIntValue( 'job_paused', 1 ) is 1
    self.job_param['job_priority'] = self.getDefaultIntValue( 'job_priority', 50 )

    self.job_param['job_cleanup_vr'] = self.getDefaultIntValue( 'job_cleanup_vr', 0 ) is 1
    self.job_param['job_cleanup_script'] = self.getDefaultIntValue( 'job_cleanup_script', 0 ) is 1

    self.job_param['job_padding'] = cmds.getAttr( 'vraySettings.fileNamePadding' )
    #
    # .vrscene generation parameters
    #
    self.vr_param['vr_reuse'] = self.getDefaultIntValue( 'vr_reuse', 1 ) is 1

    #self.vr_param['vr_plugin_prefix'] = cmds.getAttr( 'vraySettings.misc_pluginsPrefix' )
    #'vrscenes/' + self.getMayaSceneName() + '.vrscene'
    scene_name = cmds.getAttr( 'vraySettings.vrscene_filename' )

    if scene_name == None or scene_name == '' :
      scene_name = 'vrscenes/' + self.getMayaSceneName()
    #print 'vr_filename = %s' % scene_name
    self.vr_param['vr_filename'] = scene_name
    self.vr_param['vr_padding'] = self.job_param['job_padding'] # self.getDefaultIntValue( 'vr_padding', 4 )
    self.vr_param['vr_perframe'] = self.getDefaultIntValue( 'vr_perframe', 1 ) is 1 # vraySettings.misc_eachFrameInFile

    self.vr_param['vr_separate'] = self.getDefaultIntValue( 'vr_separate', 0 ) is 1 # vraySettings.misc_separateFiles

    self.vr_param['vr_export_lights'] = self.getDefaultIntValue( 'vr_export_lights', 0 ) is 1 # vraySettings.misc_exportLights
    self.vr_param['vr_export_nodes'] = self.getDefaultIntValue( 'vr_export_nodes', 0 ) is 1 # vraySettings.misc_exportNodes
    self.vr_param['vr_export_geometry'] = self.getDefaultIntValue( 'vr_export_geometry', 0 ) is 1 # vraySettings.misc_exportGeometry
    self.vr_param['vr_export_materials'] = self.getDefaultIntValue( 'vr_export_materials', 0 ) is 1 # vraySettings.misc_exportMaterials
    self.vr_param['vr_export_textures'] = self.getDefaultIntValue( 'vr_export_textures', 0 ) is 1 # vraySettings.misc_exportTextures
    self.vr_param['vr_export_bitmaps'] = self.getDefaultIntValue( 'vr_export_textures', 0 ) is 1 # vraySettings.misc_exportBitmaps

    self.vr_param['vr_hex_mesh'] = self.getDefaultIntValue( 'vr_hex_mesh', 0 ) is 1 # vraySettings.misc_meshAsHex
    self.vr_param['vr_hex_transform'] = self.getDefaultIntValue( 'vr_hex_transform', 0 ) is 1 # vraySettings.misc_transformAsHex
    self.vr_param['vr_compression'] = self.getDefaultIntValue( 'vr_compression', 0 ) is 1 # vraySettings.misc_compressedVrscene

    self.vr_param['vr_deferred'] = self.getDefaultIntValue( 'vr_deferred', 0 ) is 1
    self.vr_param['vr_local_vrgen'] = self.getDefaultIntValue( 'vr_local_migen', 1 ) is 1
    self.vr_param['vr_def_task_size'] = self.getDefaultIntValue( 'vr_def_task_size', 4 )
    #
    # VRay parameters
    #
    self.vray_param['vray_options'] = self.getDefaultStrValue( 'vray_options', '' )
    self.vray_param['vray_verbosity'] = self.getDefaultStrValue( 'vray_verbosity', 'none' )

    self.vray_param['vray_progress_frequency'] = self.getDefaultIntValue( 'vray_progress_frequency', 1 )
    self.vray_param['vray_threads'] = self.getDefaultIntValue( 'vray_threads', 4 )
    self.vray_param['vray_low_thread_priority'] = self.getDefaultIntValue( 'vr_low_thread_priority', 0 ) is 1 # vraySettings.sys_low_thread_priority
    self.vray_param['vray_clearRVOn'] = self.getDefaultIntValue( 'vr_clearRVOn', 0 ) is 1 # vraySettings.clearRVOn
    #
    # image parameters
    #
    self.img_param['img_filename_prfix'] = self.getImageFileNamePrefix()
    self.img_param['img_format'] = self.getImageFormat()
    #
    # Afanasy parameters
    #
    self.afanasy_param['af_capacity'] = self.getDefaultIntValue( 'af_capacity', 1000 )
    self.afanasy_param['af_deferred_capacity'] = self.getDefaultIntValue( 'af_deferred_capacity', 1000 )
    self.afanasy_param['af_use_var_capacity'] = self.getDefaultIntValue( 'af_use_var_capacity', 0 ) is 1
    self.afanasy_param['af_cap_min'] = self.getDefaultFloatValue( 'af_cap_min', 1.0 )
    self.afanasy_param['af_cap_max'] = self.getDefaultFloatValue( 'af_cap_max', 1.0 )
    self.afanasy_param['af_max_running_tasks'] = self.getDefaultIntValue( 'af_max_running_tasks', -1 )
    self.afanasy_param['af_max_tasks_per_host'] = self.getDefaultIntValue( 'af_max_tasks_per_host', -1 )
    self.afanasy_param['af_service'] = self.getDefaultStrValue( 'af_service', 'vray' )
    self.afanasy_param['af_deferred_service'] = self.getDefaultStrValue( 'af_deferred_service', 'mayatovray' )
    self.afanasy_param['af_os'] = self.getDefaultStrValue( 'af_os', '' ) #linux mac windows
    # Hosts Mask - Job run only on renders which host name matches this mask.
    self.afanasy_param['af_hostsmask'] = self.getDefaultStrValue( 'af_hostsmask', '.*' )
    # Exclude Hosts Mask - Job can not run on renders which host name matches this mask.
    self.afanasy_param['af_hostsexcl'] = self.getDefaultStrValue( 'af_hostsexcl', '' )
    # Depend Mask - Job will wait other user jobs which name matches this mask.
    self.afanasy_param['af_depmask'] = self.getDefaultStrValue( 'af_depmask', '' )
    # Global Depend Mask - Job will wait other jobs from any user which name matches this mask.
    self.afanasy_param['af_depglbl'] = self.getDefaultStrValue( 'af_depglbl', '' )
    #self.afanasy_param['af_consolidate_subtasks'] = self.getDefaultIntValue( 'af_consolidate_subtasks', 1 ) is 1

    #
    # backburner parameters
    #
    """
    cmdjob_path = '/usr/discreet/backburner/cmdjob'

    if self.os == 'win': cmdjob_path = 'C:/Program Files (x86)/Autodesk/Backburner/cmdjob.exe'

    self.bkburn_param['bkburn_cmdjob_path'] = self.getDefaultStrValue( 'bkburn_cmdjob_path', cmdjob_path )

    self.bkburn_param['bkburn_manager'] = self.getDefaultStrValue( 'bkburn_manager', 'burn01' )
    self.bkburn_param['bkburn_server_list'] = self.getDefaultStrValue( 'bkburn_server_list', '' )
    self.bkburn_param['bkburn_server_group'] = self.getDefaultStrValue( 'bkburn_server_group', 'MRS_391_LIN' )
    self.bkburn_param['bkburn_server_count'] = self.getDefaultIntValue( 'bkburn_server_count', 0 )

  # UNUSED START
    #self.bkburn_param['bkburn_port'] = self.getDefaultIntValue( 'bkburn_port', 0 )
    #self.bkburn_param['bkburn_workpath'] = self.getDefaultIntValue( 'bkburn_workpath', 0 ) is 1
    #self.bkburn_param['bkburn_create_log'] = self.getDefaultIntValue( 'bkburn_create_log', 0 ) is 1
    #tmp_path = '/var/tmp'
    #if self.os == 'win': tmp_path = 'C:/TEMP'
    #self.bkburn_param['bkburn_log_dir'] = self.getDefaultStrValue( 'bkburn_log_dir', tmp_path )
    #self.bkburn_param['bkburn_create_tasklist'] = self.getDefaultIntValue( 'bkburn_create_tasklist', 0 ) is 1
    #self.bkburn_param['bkburn_tasklist'] = self.getDefaultStrValue( 'bkburn_tasklist', tmp_path + '/tasklist.txt' )

    #self.bkburn_param['bkburn_use_jobParamFile'] = self.getDefaultIntValue( 'bkburn_use_jobParamFile', 0 ) is 1
    #self.bkburn_param['bkburn_jobParamFile'] = self.getDefaultStrValue( 'bkburn_jobParamFile', tmp_path + '/jobParamFile.txt' )
  # UNUSED START
    self.job_param['job_use_remote'] = self.getDefaultIntValue( 'job_use_remote', 0 ) is 1
    self.job_param['job_dirmap_from'] = self.getDefaultStrValue( 'job_dirmap_from', '//openfiler/fc.raid.PROJECTS' )
    self.job_param['job_dirmap_to'] = self.getDefaultStrValue( 'job_dirmap_to', '/hosts/OPENFILER/mnt/fc/raid/PROJECTS' )
    """
#
# Renderer params
#
    """
    mr_renderer_path = '/usr/autodesk/mrstand3.9.1-adsk2012-x64/bin/ray'

    if self.os == 'win': mr_renderer_path = 'C:/Autodesk/mrstand3.9.1-adsk2012/bin/ray.exe'
    elif self.os == 'mac': mr_renderer_path = '/Applications/Autodesk/mrstand3.9.1-adsk2012/bin/ray'

    self.mr_param['mr_renderer_local'] = self.getDefaultStrValue( 'mr_renderer_local', mr_renderer_path )
    self.mr_param['mr_renderer_remote'] = self.getDefaultStrValue( 'mr_renderer_remote', '/usr/autodesk/mrstand3.9.1-adsk2012-x64/bin/cmdray.sh' )
    self.mr_param['mr_root_as_param'] = self.getDefaultIntValue( 'mr_root_as_param', 0 ) is 1
    """

  #
  # getDefaultStrValue
  #
  def getDefaultStrValue( self, name, value ):
    name = self_prefix + name
    if cmds.optionVar( exists=name ) == 1:
      ret = cmds.optionVar( q=name )
    else:
      cmds.optionVar( sv=( name, value) )
      ret = value
    return ret
  #
  # getDefaultIntValue
  #
  def getDefaultIntValue( self, name, value ):
    name = self_prefix + name
    if cmds.optionVar( exists=name ) == 1:
      ret = cmds.optionVar( q=name )
    else:
      cmds.optionVar( iv=( name, value) )
      ret = value
    return ret
  #
  # getDefaultFloatValue
  #
  def getDefaultFloatValue( self, name, value ):
    name = self_prefix + name
    if cmds.optionVar( exists=name ) == 1:
      ret = cmds.optionVar( q=name )
    else:
      cmds.optionVar( fv=( name, value) )
      ret = value
    return ret
  #
  # setDefaultIntValue
  #
  def setDefaultIntValue( self, name, value=None ):
    name = self_prefix + name
    cmds.optionVar( iv=( name, value) )
    return value
  #
  # setDefaultIntValue2
  #
  def setDefaultIntValue2( self, names, value1=None, value2=None ):
    name = self_prefix + names[0]
    cmds.optionVar( iv=( name, value1) )
    name = self_prefix + names[1]
    cmds.optionVar( iv=( name, value2) )
    return (value1, value2)
  #
  # setDefaultIntValue3
  #
  def setDefaultIntValue3( self, names, value1=None, value2=None, value3=None ):
    name = self_prefix + names[0]
    cmds.optionVar( iv=( name, value1) )
    name = self_prefix + names[1]
    cmds.optionVar( iv=( name, value2) )
    name = self_prefix + names[2]
    cmds.optionVar( iv=( name, value3) )
    return (value1, value2, value3)
  #
  # setDefaultFloatValue
  #
  def setDefaultFloatValue( self, name, value=None ):
    name = self_prefix + name
    cmds.optionVar( fv=( name, value) )
    return value
  #
  # setDefaultFloatValue2
  #
  def setDefaultFloatValue2( self, names, value1=None, value2=None ):
    name = self_prefix + names[0]
    cmds.optionVar( fv=( name, value1) )
    name = self_prefix + names[1]
    cmds.optionVar( fv=( name, value2) )
    return (value1, value2)
  #
  # setDefaultStrValue
  #
  def setDefaultStrValue( self, name, value ):
    name = self_prefix + name
    cmds.optionVar( sv=( name, value) )
    return value
  #
  # getMayaSceneName
  #
  def getMayaSceneName( self ):
    fullName=cmds.file( q=True, sceneName=True )
    sceneName = os.path.basename( fullName )
    (sceneName, ext) = os.path.splitext( sceneName )
    if sceneName == '' : sceneName='untitled'
    return sceneName
  #
  #
  #
  def getImageFileNamePrefix( self ):
    fileNamePrefix = cmds.getAttr( 'vraySettings.fileNamePrefix' )
    if fileNamePrefix is None or fileNamePrefix == '' or fileNamePrefix == 'None':
      fileNamePrefix = self.getMayaSceneName()
    return fileNamePrefix
  #
  # getImageFormat
  #
  def getImageFormat( self ):
    imageFormatStr = cmds.getAttr( 'vraySettings.imageFormatStr' )
    if imageFormatStr is None : # Sometimes it happends ...
      imageFormatStr = 'png' 
      
    return imageFormatStr
  #
  # isRelative
  #
  def isRelative( self, fileName ):
    ret = True
    if fileName != '':
      if string.find( fileName, ':' ) == 1 or string.find( fileName, '/' ) == 0 or string.find( fileName, '\\' ) == 0:
        ret = False
    return ret
  #
  # checkTextures
  #
  def checkTextures( self, value ):
    import meTools.meCheckTexturePaths as tx
    #reload( tx )
    tx.meCheckTexturePaths()
  #
  # fromNativePath
  #
  def fromNativePath( self, nativePath ):
    return str(nativePath).replace( '\\', '/')
  #
  # browseDirectory
  #
  def browseDirectory( self, control ):
    path = cmds.textFieldButtonGrp( control, q=True, text=True )
    startDir = path
    if self.isRelative( path ) :
      startDir = os.path.join( self.rootDir, path )
    dirNames = cmds.fileDialog2( fileMode=3, startingDirectory=startDir, dialogStyle=1 ) #
    if dirNames is not None :
      dirName = cmds.workspace( projectPath=self.fromNativePath( dirNames[0] ) )
      #print ( "dirName = %s abspath = %s" ) % (dirNames[0], dirName)
      cmds.textFieldButtonGrp( control, e=True, text=dirName, forceChangeCommand=True )
  #
  # browseFile
  #
  def browseFile( self, control, extFilter ):
    path = cmds.textFieldButtonGrp( control, q=True, text=True )
    startDir = path
    if self.isRelative( path ) :
      startDir = os.path.join( self.rootDir, path )
    fileNames = cmds.fileDialog2( fileMode=1, startingDirectory=os.path.dirname( startDir ), dialogStyle=1, fileFilter=extFilter ) #
    if fileNames is not None :
      fileName = cmds.workspace( projectPath=self.fromNativePath( fileNames[0] ) )
      #print ( "dirName = %s abspath = %s" ) % (dirNames[0], dirName)
      cmds.textFieldButtonGrp( control, e=True, text=fileName, forceChangeCommand=True )
  #
  # setup_dirmaps
  #
  def setupDirmaps ( self, enableDirmap=True, fromPath='', toPath='' ):
    #
    job = self.winMain + '|f0|t0|tc0|'
    dirmaps = cmds.dirmap( getAllMappings=True )

    for i in range( 0, len( dirmaps ), 2 ):
      print "unmapDirectory: %s %s" % (dirmaps[i], dirmaps[i+1])
      cmds.dirmap( unmapDirectory=dirmaps[i] )

    if enableDirmap:
      if fromPath != '' and toPath != '' :
        print "mapDirectory: %s to %s" % ( fromPath, toPath )
        cmds.dirmap( mapDirectory=( fromPath, toPath ) )

    cmds.dirmap( enable=enableDirmap )
  #
  # getRenderCamerasList (renderable)
  #

  def getRenderCamerasList ( self  ) :
    renderCamerasList = []
    cameras = cmds.listCameras()
    for cam in cameras :
      camShapes = cmds.listRelatives( str(cam), shapes=True  )
      camShape = camShapes[0]
      renderable = cmds.getAttr( camShape + '.renderable' )
      if renderable == 1 :
        renderCamerasList.append( str(cam) )
    return renderCamerasList
  #
  # getDeferredCmd
  #
  def getDeferredCmd ( self  ) :
    gen_cmd = self.def_vrgenCommand + ' '
    gen_cmd += '-rd ' + '"' + self.rootDir + '"' + ' '
    #gen_cmd += '-im ' + '"images/' + self.getImageFileNamePrefix() + '"' + ' '
    #gen_cmd += '-of ' + self.getImageFormat() + ' '
    
    gen_cmd += '-exportFileName "' + self.get_vr_file_names( False ) + '" '

    gen_cmd += '-noRender '
    gen_cmd += self.get_vrgen_options()
    
    return gen_cmd
  #
  # getRenderCmd
  #
  def getRenderCmd ( self  ) :
    vray = self.winMain + '|f0|t0|tc2|fr1|fc1|'
    render_cmd = 'vray'
    options = str( cmds.textFieldGrp( vray + 'vray_options', q=True, text=True )).strip()
    vray_threads = cmds.intFieldGrp( vray + 'vray_threads', q=True, value1=True )

    vray_verbosity_level = cmds.optionMenuGrp( vray + 'vray_verbosity', q=True, sl=True ) - 1
    vray_progress_frequency = cmds.intFieldGrp( vray + 'vray_progress_frequency', q=True, value1=True )

    if vray_verbosity_level < 3: vray_verbosity_level = 3
    if vray_progress_frequency == 0 : vray_progress_frequency = 1

    cmd = render_cmd + ' '
    cmd += '-display=0 '
    cmd += '-showProgress=1 '
    if vray_verbosity_level != 0 : cmd += '-verboseLevel=' + str( vray_verbosity_level ) + ' '
    if vray_progress_frequency != 0 : cmd += '-progressIncrement=' + str( vray_progress_frequency ) + ' '
    cmd += '-progressUseCR=0 '
    if vray_threads != 0 : cmd += '-numThreads=' + str( vray_threads ) + ' '
    
    cmd += '-imgFile=' + self.get_image_names( False ) + ' ' 
    
    cmd += options
    return cmd
  #
  # get_vr_file_names
  #
  def get_vr_file_names ( self, frame_number = True ) :
    vr = self.winMain + '|f0|t0|tc1|fr1|fc1|'

    vr_filename = cmds.textFieldGrp( vr + 'vr_filename', q=True, text=True )
    vr_padding = cmds.intFieldGrp( vr + 'vr_padding', q=True, value1=True )
    vr_perframe = cmds.checkBoxGrp( vr + 'vr_perframe', q=True, value1=True )

    full_filename = cmds.workspace( expandName=vr_filename )
    ( filename, ext ) = os.path.splitext( full_filename )
    if ext == '' or ext == '.' : ext = '.vrscene'

    if vr_padding > 0 and vr_perframe == True :
      pad_str = '#'
      for i in range( 1, vr_padding ): pad_str += '#'

      if frame_number :
        filename += '_' + ('@' + pad_str + '@') # hardcoded in vrend ???
    
    filename += ext
    return filename
  
  #
  # get_image_names
  #
  def get_image_names ( self, frame_number = True ) :
    vr = self.winMain + '|f0|t0|tc1|fr1|fc1|'

    vr_perframe = cmds.checkBoxGrp( vr + 'vr_perframe', q=True, value1=True )
    vr_padding = cmds.intFieldGrp( vr + 'vr_padding', q=True, value1=True )
    
    pad_str = '#'
    if vr_padding > 0 and vr_perframe == True :
      for i in range( 1, vr_padding ):
        pad_str += '#'


    # setAttr -type "string" vraySettings.fileNamePrefix "BBB";
    # setAttr "vraySettings.imageFormatStr" 10;
    # setAttr -type "string" vraySettings.defaultDir "";
    # setAttr "vraySettings.fileNameFormat" 1; ??
    # setAttr -type "string" vraySettings.imageFormatStr "tif";
    # setAttr "vraySettings.dontSaveImage" 0;

    # setAttr "vraySettings.vfbOn" 1;
    # setAttr "vraySettings.vfbOffBatch" 1;
    # setAttr "vraySettings.clearRVOn" 0;
    # setAttr "vraySettings.hideRVOn" 1;

    fileNamePrefix = self.getImageFileNamePrefix()
    ext = self.getImageFormat()
    images_rule = cmds.workspace( fileRuleEntry='images' )
    images_dir = cmds.workspace( expandName=images_rule )
    
    
    #images = cmds.renderSettings( fullPath = True, genericFrameImageName = ('@' + pad_str + '@')  )
    imageFileName = images_dir + '/' + fileNamePrefix
    
    if frame_number :
      imageFileName += '.' + ('@' + pad_str + '@')
    
    imageFileName += '.' + ext
    
    return self.fromNativePath( imageFileName )
  #
  # get_vrgen_options
  #
  def get_vrgen_options ( self ) :
    job = self.winMain + '|f0|t0|tc0|fr1|fc1|'
    vr = self.winMain + '|f0|t0|tc1|fr1|fc1|'
    vr_def = self.winMain + '|f0|t0|tc1|fr3|fc3|'
    
    renderCamList = self.getRenderCamerasList()
    currentCam = renderCamList[0]
    print 'Using camera : %s' % currentCam
    vrgen_cmd = ( ' -cam ' + currentCam )
    
    return vrgen_cmd
  #
  # generate_vrscene
  #
  def generate_vrscene ( self, isSubmitingJob=False ):
    #
    job = self.winMain + '|f0|t0|tc0|fr1|fc1|'
    vr = self.winMain + '|f0|t0|tc1|fr1|fc1|'
    vr_def = self.winMain + '|f0|t0|tc1|fr3|fc3|'
    
    skipExport = False

    animation = cmds.checkBoxGrp( job + 'job_animation', q=True, value1=True )
    start = cmds.intFieldGrp( job + 'job_range', q=True, value1=True )
    stop = cmds.intFieldGrp( job + 'job_range', q=True, value2=True )
    step = cmds.intFieldGrp( job + 'job_range', q=True, value3=True )

    vr_reuse = cmds.checkBoxGrp( vr + 'vr_reuse', q=True, value1=True )

    vr_filename = cmds.textFieldGrp( vr + 'vr_filename', q=True, text=True )
    vr_padding = cmds.intFieldGrp( vr + 'vr_padding', q=True, value1=True )
    vr_perframe = cmds.checkBoxGrp( vr + 'vr_perframe', q=True, value1=True )

    vr_separate = cmds.checkBoxGrp( vr + 'vr_separate', q=True, value1=True )

    vr_export_lights = cmds.checkBoxGrp( vr + 'vr_export_lights', q=True, value1=True )
    vr_export_nodes = cmds.checkBoxGrp( vr + 'vr_export_nodes', q=True, value1=True )
    vr_export_geometry = cmds.checkBoxGrp( vr + 'vr_export_geometry', q=True, value1=True )
    vr_export_materials = cmds.checkBoxGrp( vr + 'vr_export_materials', q=True, value1=True )
    vr_export_textures = cmds.checkBoxGrp( vr + 'vr_export_textures', q=True, value1=True )
    vr_export_bitmaps = cmds.checkBoxGrp( vr + 'vr_export_bitmaps', q=True, value1=True )

    vr_hex_mesh = cmds.checkBoxGrp( vr + 'vr_hex_mesh', q=True, value1=True )
    vr_hex_transform = cmds.checkBoxGrp( vr + 'vr_hex_transform', q=True, value1=True )
    vr_compression = cmds.checkBoxGrp( vr + 'vr_compression', q=True, value1=True )
    
    vr_deferred = cmds.checkBoxGrp( vr_def + 'vr_deferred', q=True, value1=True )

    full_filename = cmds.workspace( expandName=vr_filename )
    (filename, ext) = os.path.splitext( full_filename )

    #if ext == '' or ext == '.' : ext = '.vrscene'
    #filename += ext
    dirname = os.path.dirname( filename )
    if not os.path.exists( dirname ):
      print ( "path %s not exists... let's create it!" ) % dirname
      os.mkdir( dirname )

    fileName = cmds.workspace( projectPath=self.fromNativePath( full_filename ) )

    # TODO!!! check if files are exist and have to be overriden
    if isSubmitingJob and vr_reuse:
      skipExport = True
      print "Skipping .vrscene files generation ..."

    if not skipExport:
      if not animation :
        start = stop = int( cmds.currentTime( q=True ) )
        step = 1
      #
      # save RenderGlobals
      #
      defGlobals = 'defaultRenderGlobals'
      vraySettings = 'vraySettings'
      saveMayaGlobals = {}
      saveVrayGlobals = {}
      saveMayaGlobals['animation'] = cmds.getAttr( defGlobals + '.animation' )
      saveMayaGlobals['startFrame'] = cmds.getAttr( defGlobals + '.startFrame' )
      saveMayaGlobals['endFrame'] = cmds.getAttr( defGlobals + '.endFrame' )
      saveMayaGlobals['byFrameStep'] = cmds.getAttr( defGlobals + '.byFrameStep' )

      name = cmds.getAttr( vraySettings + '.fileNamePrefix' )
      if name is None :
        name = '' 
      saveVrayGlobals['fileNamePrefix'] = str( name )

      saveVrayGlobals['fileNamePadding'] = cmds.getAttr( vraySettings + '.fileNamePadding' )
      saveVrayGlobals['animBatchOnly'] = cmds.getAttr( vraySettings + '.animBatchOnly' )
      saveVrayGlobals['vrscene_render_on'] = cmds.getAttr( vraySettings + '.vrscene_render_on' )
      saveVrayGlobals['vrscene_on'] = cmds.getAttr( vraySettings + '.vrscene_on' )
      name = cmds.getAttr( vraySettings + '.vrscene_filename' )
      if name is None :
        name = ''
      saveVrayGlobals['vrscene_filename'] = str( name )
      #print 'saveVrayGlobals[vrscene_filename] = %s' % saveVrayGlobals['vrscene_filename']
      saveVrayGlobals['misc_eachFrameInFile'] = cmds.getAttr( vraySettings + '.misc_eachFrameInFile' )
      saveVrayGlobals['misc_separateFiles'] = cmds.getAttr( vraySettings + '.misc_separateFiles' )

      saveVrayGlobals['misc_exportLights'] = cmds.getAttr( vraySettings + '.misc_exportLights' )
      saveVrayGlobals['misc_exportNodes'] = cmds.getAttr( vraySettings + '.misc_exportNodes' )
      saveVrayGlobals['misc_exportGeometry'] = cmds.getAttr( vraySettings + '.misc_exportGeometry' )
      saveVrayGlobals['misc_exportMaterials'] = cmds.getAttr( vraySettings + '.misc_exportMaterials' )
      saveVrayGlobals['misc_exportTextures'] = cmds.getAttr( vraySettings + '.misc_exportTextures' )
      saveVrayGlobals['misc_exportBitmaps'] = cmds.getAttr( vraySettings + '.misc_exportBitmaps' )

      saveVrayGlobals['misc_meshAsHex'] = cmds.getAttr( vraySettings + '.misc_meshAsHex' )
      saveVrayGlobals['misc_transformAsHex'] = cmds.getAttr( vraySettings + '.misc_transformAsHex' )
      saveVrayGlobals['misc_compressedVrscene'] = cmds.getAttr( vraySettings + '.misc_compressedVrscene' )

      #
      # override RenderGlobals
      #
      cmds.setAttr( defGlobals + '.animation', True ) # even for single frame, for proper image name
      cmds.setAttr( defGlobals + '.startFrame', start )
      cmds.setAttr( defGlobals + '.endFrame', stop )
      cmds.setAttr( defGlobals + '.byFrameStep', step )

      cmds.setAttr( vraySettings + '.fileNamePrefix', self.getImageFileNamePrefix(), type='string' ) # will use MayaSceneName if empty

      cmds.setAttr( vraySettings + '.fileNamePadding', vr_padding )
      cmds.setAttr( vraySettings + '.animBatchOnly', False )
      cmds.setAttr( vraySettings + '.vrscene_render_on', False )
      cmds.setAttr( vraySettings + '.vrscene_on', True )
      cmds.setAttr( vraySettings + '.vrscene_filename', fileName, type='string' )
      cmds.setAttr( vraySettings + '.misc_eachFrameInFile', vr_perframe )
      cmds.setAttr( vraySettings + '.misc_separateFiles', vr_separate )
      if vr_separate :
        print 'vr_export_lights = %d' % vr_export_lights
        print 'vr_export_nodes = %d' % vr_export_nodes
        print 'vr_export_geometry  = %d' % vr_export_geometry
        print 'vr_export_materials = %d' % vr_export_materials
        print 'vr_export_textures = %d' % vr_export_textures
        print 'vr_export_bitmaps = %d' % vr_export_bitmaps

        cmds.setAttr( vraySettings + '.misc_exportLights', vr_export_lights )
        cmds.setAttr( vraySettings + '.misc_exportNodes', vr_export_nodes )
        cmds.setAttr( vraySettings + '.misc_exportGeometry', vr_export_geometry )
        cmds.setAttr( vraySettings + '.misc_exportMaterials', vr_export_materials )
        cmds.setAttr( vraySettings + '.misc_exportTextures', vr_export_textures )
        cmds.setAttr( vraySettings + '.misc_exportBitmaps', vr_export_bitmaps )

      cmds.setAttr( vraySettings + '.misc_meshAsHex', vr_hex_mesh )
      cmds.setAttr( vraySettings + '.misc_transformAsHex', vr_hex_transform )
      cmds.setAttr( vraySettings + '.misc_compressedVrscene', vr_compression )

      vrgen_cmd = self.vrgenCommand  + ' ' + self.get_vrgen_options()
      
      print "vrgen_cmd = %s" % vrgen_cmd
      
      if vr_deferred :
      # generate uniquie maya scene name and save it
      # with current render and .vrscene generation settings
        print 'Use deferred .vrscene generation'
        #cmds.setAttr( defGlobals + '.imageFilePrefix', ( 'images/' + image_name ), type='string' )
        scene_name = self.getMayaSceneName() # get scene name without extension
        def_scene_name = scene_name + '_deferred'

        cmds.file( rename=def_scene_name )
        self.def_scene_name = cmds.file( save=True, de=True ) # save it with default extension
        cmds.file( rename=scene_name ) # rename scene back

      else :
        mel.eval( vrgen_cmd )
        
      
      #
      # restore RenderGlobals
      #
      cmds.setAttr( defGlobals + '.animation', saveMayaGlobals['animation'] )
      cmds.setAttr( defGlobals + '.startFrame', saveMayaGlobals['startFrame'] )
      cmds.setAttr( defGlobals + '.endFrame', saveMayaGlobals['endFrame'] )
      cmds.setAttr( defGlobals + '.byFrameStep', saveMayaGlobals['byFrameStep'] )
      name = saveVrayGlobals['fileNamePrefix']
      if name is None :
        name = ''
      cmds.setAttr( vraySettings + '.fileNamePrefix', str( name ), type='string' )

      cmds.setAttr( vraySettings + '.fileNamePadding', saveVrayGlobals['fileNamePadding'] )
      cmds.setAttr( vraySettings + '.animBatchOnly', saveVrayGlobals['animBatchOnly'] )
      cmds.setAttr( vraySettings + '.vrscene_render_on', saveVrayGlobals['vrscene_render_on'] )
      cmds.setAttr( vraySettings + '.vrscene_on', saveVrayGlobals['vrscene_on'] )
      #print 'saveVrayGlobals[vrscene_filename] = %s' % saveVrayGlobals['vrscene_filename']
      name = saveVrayGlobals['vrscene_filename']
      if name is None :
        name = ''
      cmds.setAttr( vraySettings + '.vrscene_filename', str( name ), type='string' )
      cmds.setAttr( vraySettings + '.misc_eachFrameInFile', saveVrayGlobals['misc_eachFrameInFile'] )
      cmds.setAttr( vraySettings + '.misc_separateFiles', saveVrayGlobals['misc_separateFiles'] )
      cmds.setAttr( vraySettings + '.misc_exportLights', saveVrayGlobals['misc_exportLights'] )
      cmds.setAttr( vraySettings + '.misc_exportNodes', saveVrayGlobals['misc_exportNodes'] )
      cmds.setAttr( vraySettings + '.misc_exportGeometry', saveVrayGlobals['misc_exportGeometry'] )
      cmds.setAttr( vraySettings + '.misc_exportMaterials', saveVrayGlobals['misc_exportMaterials'] )
      cmds.setAttr( vraySettings + '.misc_exportTextures', saveVrayGlobals['misc_exportTextures'] )
      cmds.setAttr( vraySettings + '.misc_exportBitmaps', saveVrayGlobals['misc_exportBitmaps'] )
      cmds.setAttr( vraySettings + '.misc_meshAsHex', saveVrayGlobals['misc_meshAsHex'] )
      cmds.setAttr( vraySettings + '.misc_transformAsHex', saveVrayGlobals['misc_transformAsHex'] )
      cmds.setAttr( vraySettings + '.misc_compressedVrscene', saveVrayGlobals['misc_compressedVrscene'] )
  #
  # submitJob
  #
  def submitJob( self, param=None ):
    # print ">> meMentalRayRender: submitJob()"

    job = self.winMain + '|f0|t0|tc0|fr1|fc1|'
    #job2 = self.winMain + '|f0|t0|tc0|fr2|fc2|'
    vr = self.winMain + '|f0|t0|tc1|fr1|fc1|'
    vr_def = self.winMain + '|f0|t0|tc1|fr3|fc3|'
    vray = self.winMain + '|f0|t0|tc2|fr1|fc1|'
    af = self.winMain + '|f0|t0|tc3|fr1|fc1|'
    bkburn = self.winMain + '|f0|t0|tc4|fr1|fc1|'
    bkburn2 = self.winMain + '|f0|t0|tc4|fr2|fc2|'

    job_name = cmds.textFieldGrp( job + 'job_name', q=True, text=True )
    if job_name == '' : job_name = self.getMayaSceneName()
    job_description = cmds.textFieldGrp( job + 'job_description', q=True, text=True )

    vr_deferred = cmds.checkBoxGrp( vr_def + 'vr_deferred', q=True, value1=True )
    vr_local_migen = cmds.checkBoxGrp( vr_def + 'vr_local_vrgen', q=True, value1=True )
    vr_def_task_size = cmds.intFieldGrp( vr_def + 'vr_def_task_size', q=True, value1=True )
    vr_reuse = cmds.checkBoxGrp( vr + 'vr_reuse', q=True, value1=True )
    
    job_dispatcher = cmds.optionMenuGrp( job + 'job_dispatcher', q=True, value=True )

    if job_dispatcher == 'afanasy' :
      self.job = VRayAfanasyJob ( job_name, job_description )
      self.job.capacity = cmds.intFieldGrp( af + 'af_capacity', q=True, value1=True )
      self.job.deferred_capacity = cmds.intFieldGrp( af + 'af_deferred_capacity', q=True, value1=True )


      self.job.use_var_capacity = cmds.checkBoxGrp( af + 'af_use_var_capacity', q=True, value1=True )
      self.job.capacity_coeff_min = cmds.floatFieldGrp( af + 'af_var_capacity', q=True, value1=True )
      self.job.capacity_coeff_max = cmds.floatFieldGrp( af + 'af_var_capacity', q=True, value2=True )

      self.job.max_running_tasks = cmds.intFieldGrp( af + 'af_max_running_tasks', q=True, value1=True )
      self.job.max_tasks_per_host = cmds.intFieldGrp( af + 'af_max_tasks_per_host', q=True, value1=True )

      self.job.service = str( cmds.textFieldGrp( af + 'af_service', q=True, text=True )).strip()
      self.job.deferred_service = str( cmds.textFieldGrp( af + 'af_deferred_service', q=True, text=True )).strip()

      self.job.hostsmask = str( cmds.textFieldGrp( af + 'af_hostsmask', q=True, text=True )).strip()
      self.job.hostsexcl = str( cmds.textFieldGrp( af + 'af_hostsexcl', q=True, text=True )).strip()
      self.job.depmask = str( cmds.textFieldGrp( af + 'af_depmask', q=True, text=True )).strip()
      self.job.depglbl = str( cmds.textFieldGrp( af + 'af_depglbl', q=True, text=True )).strip()
      self.job.need_os = str( cmds.textFieldGrp( af + 'af_os', q=True, text=True )).strip()
    elif job_dispatcher == 'backburner' :
      print 'backburner not supported in this version'
      #self.job = MentalRayBackburnerJob ( job_name, job_description )
      return
    else :
      vr_deferred = False
      self.job = VRayJob ( job_name, job_description )

    self.job.work_dir = self.rootDir

    self.job.priority = cmds.intFieldGrp( job + 'job_priority', q=True, value1=True )
    self.job.paused = cmds.checkBoxGrp( job + 'job_paused', q=True, value1=True )

    self.job.task_size = cmds.intFieldGrp( job + 'job_size', q=True, value1=True )
    self.job.padding = cmds.intFieldGrp( vr + 'vr_padding', q=True, value1=True )

    self.job.animation = cmds.checkBoxGrp( job + 'job_animation', q=True, value1=True )
    self.job.start = cmds.intFieldGrp( job + 'job_range', q=True, value1=True )
    self.job.stop = cmds.intFieldGrp( job + 'job_range', q=True, value2=True )
    self.job.step = cmds.intFieldGrp( job + 'job_range', q=True, value3=True )

    self.generate_vrscene( True ) # isSubmitingJob=True

    if not self.job.animation :
      self.job.start = int( cmds.currentTime( q=True ) )
      self.job.stop = self.job.start
      self.job.step = 1

    if vr_deferred and not vr_reuse:
      self.job.use_gen_cmd = True
      gen_cmd = self.getDeferredCmd ()
      gen_cmd += ( ' -s @#@' ) 
      gen_cmd += ( ' -e @#@' ) 
      gen_cmd += ( ' -b ' + str( self.job.step ) )
      
      self.job.gen_cmd = gen_cmd
      self.job.gen_scene_name = self.def_scene_name 
      self.job.gen_task_size = vr_def_task_size
      self.job.use_local_gen = vr_local_migen
      
      print 'gen_cmd = %s %s' % ( self.job.gen_cmd, self.job.gen_scene_name ) 
    
    self.job.cmd = self.getRenderCmd ()
    self.job.vrscene_files = self.get_vr_file_names ()
    self.job.images = self.get_image_names ()

    print 'self.get_vr_file_names = %s' % self.get_vr_file_names()
    print 'self.get_image_names = %s' % self.get_image_names()

    self.job.begin ()
    #self.job.once_per_job_tasks ()
    self.job.frame_tasks ()
    self.job.end ()
  #
  #
  #
  def vrFileNameChanged( self, name, value ):
    self.setDefaultStrValue( name, value )
    self.setResolvedPath()
  #
  #
  #
  def setResolvedPath( self ):
    vr = self.winMain + '|f0|t0|tc1|fr1|fc1|'
    vr2 = self.winMain + '|f0|t0|tc1|fr2|fc2|'

    vr_filename = cmds.textFieldGrp( vr + 'vr_filename', q=True, text=True )
    vr_padding = cmds.intFieldGrp( vr + 'vr_padding', q=True, value1=True )
    vr_perframe = cmds.checkBoxGrp( vr + 'vr_perframe', q=True, value1=True )

    full_filename = cmds.workspace( expandName=vr_filename )
    ( filename, ext ) = os.path.splitext( full_filename )
    if ext == '' or ext == '.' : ext = '.vrscene'

    if vr_padding > 0 and vr_perframe == True :
      filename += '_' # hardcoded in vrend ???
      pad_str = '#'
      for i in range( 1, vr_padding ): pad_str += '#'
      filename += pad_str
    filename += ext
    cmds.textFieldGrp( vr2 + 'vr_resolved_path', edit=True, text=filename )
  #
  # Open Maya Render Settings window
  #
  def maya_render_globals ( self, arg ) :
    mel.eval( "unifiedRenderGlobalsWindow" )
  #
  #
  #
  def enable_range ( self, arg ) :
    job = self.winMain + '|f0|t0|tc0|fr1|fc1|'
    self.setDefaultIntValue( 'job_animation', arg )
    cmds.intFieldGrp( job + 'job_range', edit = True, enable = arg )
  #
  #
  #
  def enable_var_capacity ( self, arg ) :
    af = self.winMain + '|f0|t0|tc3|fr1|fc1|'
    self.setDefaultIntValue( 'af_use_var_capacity', arg )
    cmds.floatFieldGrp( af + 'af_var_capacity', edit = True, enable = arg )
  #
  #
  #
  def enable_separate ( self, arg ) :
    vr = self.winMain + '|f0|t0|tc1|fr1|fc1|'
    self.setDefaultIntValue( 'vr_separate', arg )
    cmds.checkBoxGrp( vr + 'vr_export_lights', edit = True, enable = arg )
    cmds.checkBoxGrp( vr + 'vr_export_nodes', edit = True, enable = arg )
    cmds.checkBoxGrp( vr + 'vr_export_geometry', edit = True, enable = arg )
    cmds.checkBoxGrp( vr + 'vr_export_materials', edit = True, enable = arg )
    cmds.checkBoxGrp( vr + 'vr_export_textures', edit = True, enable = arg )
    cmds.checkBoxGrp( vr + 'vr_export_bitmaps', edit = True, enable = arg )
  #
  #
  #
  def enable_deferred ( self, arg ) :
    vr_def = self.winMain + '|f0|t0|tc1|fr3|fc3|'

    self.setDefaultIntValue( 'vr_deferred', arg )
    cmds.checkBoxGrp( vr_def + 'vr_local_vrgen', edit = True, enable = arg )
    cmds.intFieldGrp( vr_def + 'vr_def_task_size', edit = True, enable = arg )
  #
  # setupUI
  #
  def setupUI( self ):
    #print ">> meMentalRayRender: setupUI()"
    self.deleteUI( True )
    #
    # Main window setup
    #
    self.winMain = cmds.window( meVRayRenderMainWnd,
                                title='meVRayRender ver.' + meVRayRenderVer + ' (' + self.os + ')' ,
                                menuBar=True,
                                retain=False,
                                widthHeight=(420, 460) )

    self.mainMenu = cmds.menu( label="Commands", tearOff=False )
    cmds.menuItem( label='Render Globals ...', command = self.maya_render_globals )
    cmds.menuItem( label='Check Texture Paths ...', command=self.checkTextures )
    cmds.menuItem( label='Generate .vrscene', command=self.generate_vrscene )
    cmds.menuItem( label='Submit Job', command=self.submitJob )
    cmds.menuItem( divider=True )
    cmds.menuItem( label='Close', command=self.deleteUI )
    # cmds.menuItem( label='Render' )
    cw1 = 120
    cw2 = 60
    cw3 = 20
    mr_hi = 8

    form = cmds.formLayout( 'f0', numberOfDivisions=100 )
    proj = cmds.columnLayout( 'c0', columnAttach=('left',0), rowSpacing=2, adjustableColumn=True, height=32 )
    cmds.textFieldGrp( cw=( 1, 70 ), adj=2, label="Project Root ", text=self.rootDir, editable=False )
    cmds.setParent( '..' )
    #
    # setup tabs
    #
    tab = cmds.tabLayout( 't0', scrollable=True, childResizable=True )  # tabLayout -scr true -cr true  tabs; //
    #
    # Job tab
    #
    tab_job = cmds.columnLayout( 'tc0', columnAttach=('left',0), rowSpacing=0, adjustableColumn=True ) # string $displayColumn = `columnLayout -cat left 0 -rs 0 -adj true displayTab`;
    cmds.frameLayout( 'fr1', label=' Parameters ', borderVisible=True, borderStyle='etchedIn', marginHeight=mr_hi  )
    cmds.columnLayout( 'fc1', columnAttach=('left',0), rowSpacing=0, adjustableColumn=True )
    #
    job_dispatcher = cmds.optionMenuGrp( 'job_dispatcher', cw=( (1, cw1), ), cal=(1, 'right'),
                        label="Job Dispatcher ",  enable=True,
                        cc=partial( self.setDefaultStrValue, 'job_dispatcher' ) )
    for name in ('none', 'afanasy' ): cmds.menuItem( label=name ) # 'backburner',
    cmds.optionMenuGrp( job_dispatcher, e=True, value=self.job_param['job_dispatcher'] )
    cmds.text( label='' )
    cmds.textFieldGrp( 'job_name', cw=( 1, cw1 ), adj=2, label="Job Name ", text=self.job_param['job_name'] )
    cmds.textFieldGrp( 'job_description', cw=( 1, cw1 ), adj=2,
                        label="Description ",
                        text=self.job_param['job_description'],
                        cc=partial( self.setDefaultStrValue, 'job_description' ) )
    cmds.checkBoxGrp( 'job_paused', cw=( (1, cw1), (2, cw1 * 2 ) ),
                      label="Start Paused ",
                      value1=self.job_param['job_paused'],
                      cc=partial( self.setDefaultIntValue, 'job_paused' ) )

    cmds.text( label='' )

    cmds.checkBoxGrp( 'job_animation', cw=( (1, cw1), (2, cw1 * 2 ) ),
                      label="Animation ",
                      value1=self.job_param['job_animation'],
                      cc=partial( self.enable_range ) )

    cmds.intFieldGrp( 'job_range', cw=( (1, cw1), (2, cw2), (3, cw2), (4, cw2) ), nf=3, label="Start/Stop/By ",
                       value1=self.job_param['job_start'],
                       value2=self.job_param['job_end'],
                       value3=self.job_param['job_step'],
                        enable = self.job_param['job_animation'],
                       cc=( partial( self.setDefaultIntValue3, ('job_start', 'job_end', 'job_step') ) ) )


    cmds.intFieldGrp( 'job_size', cw=( (1, cw1), (2, cw2) ),
                      label="Task Size ",
                      ann="Should be smaller then number of frames to render",
                      value1=self.job_param['job_size'],
                      cc=partial( self.setDefaultIntValue, 'job_size' ) )

    cmds.intFieldGrp( 'job_priority', cw=( (1, cw1), (2, cw2) ),
                      label="Priority ",
                      value1=self.job_param['job_priority'],
                      cc=partial( self.setDefaultIntValue, 'job_priority' ) )
    #cmds.text( label='' )
    #
    cmds.setParent( '..' )
    cmds.setParent( '..' )

    cmds.frameLayout( 'fr2', label=' Cleanup ', borderVisible=True, borderStyle='etchedIn', marginHeight=mr_hi, cll=True, cl=True  )
    cmds.columnLayout( 'fc2', columnAttach=('left',0), rowSpacing=0, adjustableColumn=True )

    cmds.checkBoxGrp( 'job_cleanup_vr', cw=( (1, cw1), (2, cw1 * 2 ) ),
                      label='', label1=' .vrscene files',
                      value1=self.job_param['job_cleanup_vr'],
                      enable=False,
                      cc = partial( self.setDefaultIntValue, 'job_cleanup_mi' ) )
    cmds.checkBoxGrp( 'job_cleanup_script', cw=( (1, cw1), (2, cw1 * 2 ) ),
                      label='', label1=' script file',
                      value1=self.job_param['job_cleanup_script'],
                      enable=False,
                      cc = partial( self.setDefaultIntValue, 'job_cleanup_script' ) )

    cmds.setParent( '..' )
    cmds.setParent( '..' )

    cmds.setParent( '..' )
    #
    # .vrscene files generation tab
    #
    tab_vrparam = cmds.columnLayout( 'tc1', columnAttach=('left',0), rowSpacing=0, adjustableColumn=True )
    cmds.frameLayout( 'fr1', label=' Export Settings ', borderVisible=True, borderStyle='etchedIn', marginHeight=mr_hi  )
    cmds.columnLayout( 'fc1', columnAttach=('left',0), rowSpacing=0, adjustableColumn=True )
    #
    cmds.checkBoxGrp( 'vr_reuse', cw=( (1, cw1), (2, cw1 * 2 ) ),
                      label="Use existing .vrscene ",
                      ann = "Do not generate .vrscene files if they are exist",
                      value1=self.vr_param['vr_reuse'],
                      cc=partial( self.setDefaultIntValue, 'vr_reuse' ) )
    cmds.text( label='' )
    #mi_dir = cmds.textFieldButtonGrp( cw=( 1, cw1 ), adj=2, label="Directory ", buttonLabel="...", text=self.mi_param['mi_dir'] )

    vr_filename = cmds.textFieldButtonGrp( 'vr_filename', cw=( 1, cw1 ), adj=2,
                                           label="File Name ", buttonLabel="...",
                                           text=self.vr_param['vr_filename'],
                                           cc=partial( self.vrFileNameChanged, 'vr_filename' ) )
    cmds.textFieldButtonGrp( vr_filename, edit=True, bc=partial( self.browseFile, vr_filename, 'vray scene files (*.vrscene)' ) )

    #cmds.text( label='' )
    # Resolved Path
    #
    cmds.intFieldGrp( 'vr_padding', cw=( (1, cw1), (2, cw2) ),
                      label="Frame Padding ",
                      value1=self.vr_param['vr_padding'],
                      cc=partial( self.vrFileNameChanged, 'vr_padding' ) )
    cmds.checkBoxGrp( 'vr_perframe', cw=( (1, cw1), (2, cw1 * 2 ) ),
                      label = '', label1 = " File Per Frame ",
                      value1=self.vr_param['vr_perframe'],
                      cc=partial( self.vrFileNameChanged, 'vr_perframe' ) )
    #cmds.text( label='' )
    cmds.checkBoxGrp( 'vr_separate', cw=( (1, cw1), (2, cw1 * 2 ) ),
                      label = 'Separate Files ', label1 = "",
                      value1=self.vr_param['vr_separate'],
                      cc=partial( self.enable_separate )) #partial( self.setDefaultIntValue, 'vr_separate' ) )

    cmds.checkBoxGrp( 'vr_export_lights', cw=( (1, cw1), (2, cw1 * 2 ) ),
                      label = '', label1 = " Export Lights",
                      value1=self.vr_param['vr_export_lights'],
                      enable = self.vr_param[ 'vr_separate' ],
                      cc=partial( self.setDefaultIntValue, 'vr_export_lights' ) )
    cmds.checkBoxGrp( 'vr_export_nodes', cw=( (1, cw1), (2, cw1 * 2 ) ),
                      label = '', label1 = " Export Nodes",
                      value1=self.vr_param['vr_export_nodes'],
                      enable = self.vr_param[ 'vr_separate' ],
                      cc=partial( self.setDefaultIntValue, 'vr_export_nodes' ) )
    cmds.checkBoxGrp( 'vr_export_geometry', cw=( (1, cw1), (2, cw1 * 2 ) ),
                      label = '', label1 = " Export Geometry",
                      value1=self.vr_param['vr_export_geometry'],
                      enable = self.vr_param[ 'vr_separate' ],
                      cc=partial( self.setDefaultIntValue, 'vr_export_geometry' ) )
    cmds.checkBoxGrp( 'vr_export_materials', cw=( (1, cw1), (2, cw1 * 2 ) ),
                      label = '', label1 = " Export Materials",
                      value1=self.vr_param['vr_export_materials'],
                      enable = self.vr_param[ 'vr_separate' ],
                      cc=partial( self.setDefaultIntValue, 'vr_export_materials' ) )
    cmds.checkBoxGrp( 'vr_export_textures', cw=( (1, cw1), (2, cw1 * 2 ) ),
                      label = '', label1 = " Export Textures",
                      value1=self.vr_param['vr_export_textures'],
                      enable = self.vr_param[ 'vr_separate' ],
                      cc=partial( self.setDefaultIntValue, 'vr_export_textures' ) )
    cmds.checkBoxGrp( 'vr_export_bitmaps', cw=( (1, cw1), (2, cw1 * 2 ) ),
                      label = '', label1 = " Export Bitmaps",
                      value1=self.vr_param['vr_export_bitmaps'],
                      enable = self.vr_param[ 'vr_separate' ],
                      cc=partial( self.setDefaultIntValue, 'vr_export_bitmaps' ) )

    #cmds.text( label='' )

    cmds.checkBoxGrp( 'vr_hex_mesh', cw=( (1, cw1), (2, cw1 * 2 ) ),
                      label = 'Write in hex format ', label1 = " mesh data",
                      value1 = self.vr_param [ 'vr_hex_mesh' ],
                      cc = partial( self.setDefaultIntValue, 'vr_hex_mesh' ) )
    cmds.checkBoxGrp( 'vr_hex_transform', cw=( (1, cw1), (2, cw1 * 2 ) ),
                      label = '', label1 = " transform data",
                      value1 = self.vr_param [ 'vr_hex_transform' ],
                      cc = partial( self.setDefaultIntValue, 'vr_hex_transform' ) )

    cmds.checkBoxGrp( 'vr_compression', cw=( (1, cw1), (2, cw1 * 2 ) ),
                      label = 'Compressed ', label1 = "",
                      value1 = self.vr_param [ 'vr_compression' ],
                      cc = partial( self.setDefaultIntValue, 'vr_compression' ) )
    #
    cmds.setParent( '..' )
    cmds.setParent( '..' )

    cmds.frameLayout( 'fr2', label=' Resolved Path ', borderVisible=True, borderStyle='etchedIn', marginHeight=mr_hi  )
    cmds.columnLayout( 'fc2', columnAttach=('left',0), rowSpacing=0, adjustableColumn=True )
    #
    cmds.textFieldGrp( 'vr_resolved_path', cw=( 1, 0 ), adj=2, label='', text='', editable=False )
    self.setResolvedPath()
    #
    cmds.setParent( '..' )
    cmds.setParent( '..' )
    cmds.frameLayout( 'fr3', label=' Deferred .vrscene generation ', borderVisible=True, borderStyle='etchedIn', marginHeight=mr_hi, cll=True, cl=True  )
    cmds.columnLayout( 'fc3', columnAttach=('left',0), rowSpacing=0, adjustableColumn=True )
    cmds.checkBoxGrp( 'vr_deferred', cw=( (1, cw1), (2, cw1 * 2 ) ),
                      label = "Use deferred ",
                      ann = "Generate .vrscene files in background process",
                      value1 = self.vr_param[ 'vr_deferred' ],
                      cc = partial( self.enable_deferred )) # self.setDefaultIntValue, 'rib_deferred_ribgen' ) )

    cmds.checkBoxGrp( 'vr_local_vrgen', cw=( (1, cw1), (2, cw1 * 2 ) ),
                      label = '', label1 = " Only on localhost ",
                      ann = "Do not use remote hosts",
                      value1 = self.vr_param[ 'vr_local_vrgen' ],
                      enable = self.vr_param[ 'vr_deferred' ],
                      cc = partial( self.setDefaultIntValue, 'vr_local_vrgen' ) )

    cmds.intFieldGrp( 'vr_def_task_size', cw=( (1, cw1), (2, cw2) ),
                      label = "Task Size ",
                      value1 = self.vr_param [ 'vr_def_task_size' ],
                      enable = self.vr_param[ 'vr_deferred' ],
                      cc = partial( self.setDefaultIntValue, 'def_task_size' ) )
    cmds.setParent( '..' )
    cmds.setParent( '..' )

    cmds.setParent( '..' )
    #
    # Renderer tab
    #
    tab_vray = cmds.columnLayout( 'tc2', columnAttach=('left',0), rowSpacing=0, adjustableColumn=True )
    #cmds.text( label='' )
    cmds.frameLayout( 'fr1', label=' VRay command line options ', borderVisible=True, borderStyle='etchedIn', marginHeight=mr_hi  )
    cmds.columnLayout( 'fc1', columnAttach=('left',0), rowSpacing=0, adjustableColumn=True )

    cmds.textFieldGrp( 'vray_options', cw=( 1, cw1 ), adj=2,
                       label="Additional Options ",
                       text=self.vray_param['vray_options'],
                       cc=partial( self.setDefaultStrValue, 'vray_options' ) )
    cmds.text( label='' )
    vray_verbosity = cmds.optionMenuGrp( 'vray_verbosity', cw=( (1, cw1), ), cal=(1, 'right'),
                                       label="Verbosity ",
                                       cc=partial( self.setDefaultStrValue, 'vray_verbosity' ) )
    for name in ('none', 'errors', 'warning', 'info', 'details'):
      cmds.menuItem( label=name )

    cmds.optionMenuGrp( vray_verbosity, e=True, value=self.vray_param['vray_verbosity'] )

    cmds.intFieldGrp( 'vray_progress_frequency', cw=( (1, cw1), (2, cw2) ),
                      label="Progress frequency ",
                      ann = 'Progress information should be emitted only when this percentage of the whole render time has passed.',
                      value1=self.vray_param['vray_progress_frequency'],
                      cc=partial( self.setDefaultIntValue, 'vray_progress_frequency' ) )
    cmds.intFieldGrp( 'vray_threads', cw=( (1, cw1), (2, cw2) ),
                      label="Threads ",
                      ann = 'The number of threads.',
                      value1=self.vray_param['vray_threads'],
                      cc=partial( self.setDefaultIntValue, 'vray_threads' ) )
    cmds.checkBoxGrp( 'vray_low_thread_priority', cw=( (1, cw1), (2, cw1 * 2 ) ),
                      label='', label1=" Low Thread Priority",
                      value1=self.vray_param['vray_low_thread_priority'],
                      cc=partial( self.setDefaultIntValue, 'vray_low_thread_priority' ) )
    cmds.checkBoxGrp( 'vray_clearRVOn', cw=( (1, cw1), (2, cw1 * 2 ) ),
                      label='', label1=" Clear Render View",
                      value1=self.vray_param['vray_clearRVOn'],
                      cc=partial( self.setDefaultIntValue, 'vray_clearRVOn' ) )

    cmds.setParent( '..' )
    cmds.setParent( '..' )

    cmds.setParent( '..' )
    #
    # Afanasy tab
    #
    tab_afanasy = cmds.columnLayout( 'tc3', columnAttach=('left',0), rowSpacing=0, adjustableColumn=True )
    cmds.frameLayout( 'fr1', label=' Parameters ', borderVisible=True, borderStyle='etchedIn', marginHeight=mr_hi )
    cmds.columnLayout( 'fc1', columnAttach=('left',4), rowSpacing=0, adjustableColumn=True )

    cmds.intFieldGrp( 'af_capacity', cw=( (1, cw1), (2, cw2) ),
                      label="Task Capacity ",
                      value1=self.afanasy_param['af_capacity'],
                      cc=partial( self.setDefaultIntValue, 'af_capacity' ) )

    cmds.checkBoxGrp( 'af_use_var_capacity', cw=( (1, cw1), (2, cw1 * 2 ) ),
                      label = "Use Variable Capacity ",
                      ann = "Block can generate tasks with capacity*coefficient to fit free render capacity",
                      value1 = self.afanasy_param[ 'af_use_var_capacity' ],
                      cc = partial( self.enable_var_capacity ) )
    
    cmds.intFieldGrp( 'af_deferred_capacity', cw=( (1, cw1), (2, cw2) ),
                      label="Deferred Capacity ",
                      value1=self.afanasy_param['af_deferred_capacity'],
                      cc=partial( self.setDefaultIntValue, 'af_deferred_capacity' ) )

    cmds.floatFieldGrp( 'af_var_capacity', cw=( (1, cw1), (2, cw2), (3, cw2), (4, cw2) ), nf=2, pre=2,
                       label="Min/Max coefficient ",
                       value1 = self.afanasy_param['af_cap_min'],
                       value2 = self.afanasy_param['af_cap_max'],
                       enable = self.afanasy_param[ 'af_use_var_capacity' ],
                       cc = ( partial( self.setDefaultFloatValue2, ('af_cap_min', 'af_cap_max') ) ) )

    cmds.intFieldGrp( 'af_max_running_tasks', cw=( (1, cw1), (2, cw2) ),
                      label = "Max Running Tasks ",
                      value1 = self.afanasy_param[ 'af_max_running_tasks' ],
                      cc = partial( self.setDefaultIntValue, 'af_max_running_tasks' ) )

    cmds.intFieldGrp( 'af_max_tasks_per_host', cw=( (1, cw1), (2, cw2) ),
                      label = "Max Tasks Per Host ",
                      value1 = self.afanasy_param[ 'af_max_tasks_per_host' ],
                      cc = partial( self.setDefaultIntValue, 'af_max_tasks_per_host' ) )

    cmds.textFieldGrp( 'af_service', cw=( 1, cw1 ), adj=2,
                        label = "Service ",
                        text = self.afanasy_param [ 'af_service' ],
                        cc = partial( self.setDefaultStrValue, 'af_service' ) )
    cmds.textFieldGrp( 'af_deferred_service', cw=( 1, cw1 ), adj=2,
                        label = "Deferred Service ",
                        text = self.afanasy_param [ 'af_deferred_service' ],
                        cc = partial( self.setDefaultStrValue, 'af_deferred_service' ) )

    cmds.textFieldGrp( 'af_hostsmask', cw=( 1, cw1 ), adj=2,
                        label = "Hosts Mask ",
                        ann="Job run only on renders which host name matches this mask\n e.g.  .* or host.*",
                        text = self.afanasy_param [ 'af_hostsmask' ],
                        cc = partial( self.setDefaultStrValue, 'af_hostsmask' ) )

    cmds.textFieldGrp( 'af_hostsexcl', cw=( 1, cw1 ), adj=2,
                        label = "Exclude Hosts Mask ",
                        ann="Job can not run on renders which host name matches this mask\n e.g.  host.* or host01|host02",
                        text = self.afanasy_param [ 'af_hostsexcl' ],
                        cc = partial( self.setDefaultStrValue, 'af_hostsexcl' ) )

    cmds.textFieldGrp( 'af_depmask', cw=( 1, cw1 ), adj=2,
                        label = "Depend Mask ",
                        ann="Job will wait other user jobs which name matches this mask",
                        text = self.afanasy_param [ 'af_depmask' ],
                        cc = partial( self.setDefaultStrValue, 'af_depmask' ) )

    cmds.textFieldGrp( 'af_depglbl', cw=( 1, cw1 ), adj=2,
                        label = "Global Depend Mask ",
                        ann="Job will wait other jobs from any user which name matches this mask",
                        text = self.afanasy_param [ 'af_depglbl' ],
                        cc = partial( self.setDefaultStrValue, 'af_depglbl' ) )

    cmds.textFieldGrp( 'af_os', cw=( 1, cw1 ), adj=2,
                        label = "Needed OS ",
                        ann="windows linux mac",
                        text = self.afanasy_param [ 'af_os' ],
                        cc = partial( self.setDefaultStrValue, 'af_os' ) )

    #cmds.checkBoxGrp( 'af_consolidate_subtasks', cw=( (1, cw1), (2, cw1 * 2 ) ),
    #                  label="Consolidate subtasks ",
    #                  ann="Put perframe subtasks in block",
    #                  value1=self.afanasy_param ['af_consolidate_subtasks'],
    #                  cc=partial( self.setDefaultIntValue, 'af_consolidate_subtasks' ) )

    cmds.setParent( '..' )
    cmds.setParent( '..' )

    cmds.setParent( '..' )

    cmds.tabLayout( tab, edit=True,
                    tabLabel=( ( tab_job, "Job" ),
                               ( tab_vrparam, ".vrscene" ),
                               ( tab_vray, "Renderer" ),
                               ( tab_afanasy, "Afanasy" )
                             )
                  )

    cmds.setParent( form )
    btn_sbm = cmds.button( label="Submit", command=self.submitJob, ann='Generate .vrscene files and submit to dispatcher' )
    btn_gen = cmds.button( label="Generate .vrscene", command=self.generate_vrscene, ann='Force .vrscene files generation' )
    btn_cls = cmds.button( label="Close", command=self.deleteUI )

    cmds.formLayout(  form, edit=True,
                      attachForm=( ( proj, 'top',    0 ),
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
                      attachControl=( ( tab, 'top', 0, proj ),
                                      ( tab, 'bottom', 0, btn_sbm ),
                                      ( btn_gen, 'left', 0, btn_sbm ),
                                      ( btn_gen, 'right', 0, btn_cls )
                                    ),
                      attachPosition=( ( btn_sbm, 'right', 0, 33 ),
                                       ( btn_gen, 'right', 0, 66 ),
                                       ( btn_cls, 'left', 0, 66 )
                                      )
                   )

    cmds.showWindow( self.winMain )
    return form
  #
  # deleteUI
  #
  def deleteUI( self, param ):
    winMain = meVRayRenderMainWnd

    if cmds.window( winMain, exists=True ): cmds.deleteUI( winMain, window=True )
    if cmds.windowPref( winMain, exists=True ): cmds.windowPref( winMain, remove=True )
#
#
#
print 'meVRayRender sourced ...'

