"""
  meMentalRayRender
  
  ver.0.3.7 
    22 Aug 2012
    - backburner specific control fields are moved
      to corresponded tab, some unused parameters removed
    
  ver.0.3.6 22 Aug 2012
    9 Jul 2012
    - attempt to add Afanasy support
     
  ver.0.3.5 21 Mar 2012
  ver.0.3.4 16 Mar 2012
  ver.0.3.3 18 Nov 2011
  ver.0.3.1 14 Nov 2011

  Author:

  Yuri Meshalkin (aka mesh)
  mesh@kpp.kiev.ua
  
  (c) Kiev Post Production 2011
  
  Description:

  This UI script generates .mi files from open Maya scene
  and submits them to mentalray Standalone directly or by
  creating a job for backburner.
  
  "Use Remote Render" mode allows you to submit backburner job
  from your local OS to render-farm with other OS (e.g. linux)
  by using Directory Mapping.
 

  Usage:

  import meTools.meMentalRayRender as mr
  reload( mr ) # for debugging purposes
  mr = mr.meMentalRayRender()
  
  For using with Afanasy, add %AF_ROOT%\plugins\maya\python to %PYTHONPATH%


"""
import sys, os, string
import maya.OpenMaya as OpenMaya
from functools import partial
import maya.cmds as cmds
import maya.mel as mel

meMentalRayRenderVer = "0.3.7 beta"
meMentalRayRenderMainWnd = "meMentalRayRenderMainWnd"
#
# meMentalRayRender
#
class meMentalRayRender( object ):
  #
  #
  #
  def __init__( self, selection='' ):
    print ">> meMentalRayRender: Class created"
    self.selection = selection
    self.winMain = ''
    
    self.os = sys.platform
    if self.os.startswith('linux') : self.os = 'linux'
    elif self.os == 'darwin' : self.os = 'mac'
    elif self.os == 'win32' : self.os = 'win'
    
    print 'sys.platform = %s self.os = %s' % ( sys.platform, self.os )
    self.rootDir = cmds.workspace( q=True, rootDirectory=True )
    self.rootDir = self.rootDir[:-1]
      
    self.job_param = {}
    self.mi_param = {}
    self.mr_param = {}
    self.img_param = {}
    self.af_param = {}
    self.bkburn_param = {}
    
    self.initParameters()
    self.ui=self.setupUI()
  #
  #
  #  
  def __del__( self ):
    print( ">> meMentalRayRender: Class deleted" )
    pass
  #
  #
  #  
  def initParameters( self ):
    #
    # Job parameters
    #
    self.job_param['job_name'] = self.getMayaSceneName()
    self.job_param['job_description'] = self.getDefaultStrValue( 'job_description', '' )
    
    self.job_param['job_animation'] = self.getDefaultIntValue( 'job_animation', 1 ) is 1 
    self.job_param['job_start'] = self.getDefaultIntValue( 'job_start', 1 )
    self.job_param['job_end'] = self.getDefaultIntValue( 'job_end', 100 )
    self.job_param['job_step'] = self.getDefaultIntValue( 'job_step', 1 )
    self.job_param['job_size'] = self.getDefaultIntValue( 'job_size', 1 )
    self.job_param['job_paused'] = self.getDefaultIntValue( 'job_paused', 1 ) is 1 
    
    self.job_param['job_priority'] = self.getDefaultIntValue( 'job_priority', 50 )

    self.job_param['job_dispatcher'] = self.getDefaultStrValue( 'job_dispatcher', 'backburner' )
    #
    # .mi generation parameters
    #
    #self.mi_param['mi_dir'] = self.getDefaultStrValue( 'mi_dir', 'mi' )
    self.mi_param['mi_filename'] = self.getDefaultStrValue( 'mi_filename', 'mi/' + self.getMayaSceneName() + '.mi' ) 
    self.mi_param['mi_padding'] = self.getDefaultIntValue( 'mi_padding', 4 )
    
    self.mi_param['mi_perframe'] = self.getDefaultIntValue( 'mi_perframe', 1 ) is 1
    self.mi_param['mi_selection'] = self.getDefaultIntValue( 'mi_selection', 0 ) is 1
    self.mi_param['mi_filepaths'] = self.getDefaultStrValue( 'mi_filepaths', 'Relative' )
    self.mi_param['mi_fileformat'] = self.getDefaultStrValue( 'mi_fileformat', 'ASCII' )
    self.mi_param['mi_tabstop'] = self.getDefaultIntValue( 'mi_tabstop', 2 )
    self.mi_param['mi_verbosity'] = self.getDefaultStrValue( 'mi_verbosity', 'none' )
        
    self.mi_param['mi_reuse'] = self.getDefaultIntValue( 'mi_reuse', 1 ) is 1
    #self.mi_param['mi_compression'] = self.getDefaultStrValue( 'mi_compression', 'Off' )
    #
    # mentalray parameters
    #
    self.mr_param['mr_options'] = self.getDefaultStrValue( 'mr_options', '' )
    self.mr_param['mr_verbosity'] = self.getDefaultStrValue( 'mr_verbosity', 'none' )
    
    self.mr_param['mr_progress_frequency'] = self.getDefaultIntValue( 'mr_progress_frequency', 1 )
    self.mr_param['mr_threads'] = self.getDefaultIntValue( 'mr_threads', 4 )
    self.mr_param['mr_texture_continue'] = self.getDefaultIntValue( 'mr_texture_continue', 1 )
    #
    # image parameters
    #
    self.img_param['img_filename'] = self.getImageFileName()
    self.img_param['img_format'] = self.getImageFormat()
    #
    # Afanasy parameters
    #
    self.af_param['af_capacity'] = self.getDefaultIntValue( 'af_capacity', 1000 )
    #
    # backburner parameters
    #
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
    
    mr_renderer_path = '/usr/autodesk/mrstand3.9.1-adsk2012-x64/bin/ray'
    
    if self.os == 'win': mr_renderer_path = 'C:/Autodesk/mrstand3.9.1-adsk2012/bin/ray.exe'
    elif self.os == 'mac': mr_renderer_path = '/Applications/Autodesk/mrstand3.9.1-adsk2012/bin/ray'
    
    self.mr_param['mr_renderer_local'] = self.getDefaultStrValue( 'mr_renderer_local', mr_renderer_path )
    self.mr_param['mr_renderer_remote'] = self.getDefaultStrValue( 'mr_renderer_remote', '/usr/autodesk/mrstand3.9.1-adsk2012-x64/bin/cmdray.sh' )
    self.mr_param['mr_root_as_param'] = self.getDefaultIntValue( 'mr_root_as_param', 0 ) is 1 
  
  #
  # getDefaultStrValue
  #  
  def getDefaultStrValue( self, name, value ):
    name = 'meMentalRayRender_' + name
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
    name = 'meMentalRayRender_' + name
    if cmds.optionVar( exists=name ) == 1:
      ret = cmds.optionVar( q=name )
    else:
      cmds.optionVar( iv=( name, value) )
      ret = value
    return ret
  #
  # setDefaultIntValue
  #  
  def setDefaultIntValue( self, name, value=None ):
    print ( "setDefaultIntValue name :%s value : %s" ) % ( name, value )
    name = 'meMentalRayRender_' + name
    cmds.optionVar( iv=( name, value) )   
    return value 
  #
  # setDefaultIntValue3
  #  
  def setDefaultIntValue3( self, names, value1=None, value2=None, value3=None ):
    print ( "setDefaultIntValue3 names :%s values : %d %d %d" ) % ( names, value1, value2, value3 )
    
    name = 'meMentalRayRender_' + names[0]
    cmds.optionVar( iv=( name, value1) ) 
    name = 'meMentalRayRender_' + names[1]
    cmds.optionVar( iv=( name, value2) ) 
    name = 'meMentalRayRender_' + names[2]
    cmds.optionVar( iv=( name, value3) ) 
    return (value1, value2, value3)       
  #
  # setDefaultStrValue
  #    
  def setDefaultStrValue( self, name, value ):
    print ( "setDefaultStrValue name :%s value : %s" ) % ( name, value )
    name = 'meMentalRayRender_' + name
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
  def getImageFileName( self ):
    pass    
  #
  # 
  #  
  def getImageFormat( self ):
    pass       
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
  # checkShaders
  #    
  def checkShaders( self, value ):
    #print ">> meMentalRayRender: checkShaders()"
    mel.eval("mrShaderManager")
  #
  # checkTextures
  #    
  def checkTextures( self, value ):
    #print ">> meMentalRayRender: checkTextures()"
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
  # validate_mi_FileName
  #  
  def validate_mi_FileName( self, control, extFilter ):
    pass
  #
  # setup_dirmaps
  # 
  def setupDirmaps( self, enableDirmap=True, fromPath='', toPath='' ):
    print ">> meMentalRayRender: setup_dirmaps"
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
  # generate_mi
  #      
  def generate_mi( self, isSubmitingJob=False ):
    print ">> meMentalRayRender: generate_mi() (isSubmitingJob=%d)" % isSubmitingJob
    job = self.winMain + '|f0|t0|tc0|fr1|fc1|'
    mi = self.winMain + '|f0|t0|tc1|fr1|fc1|'
    skipExport = False
    
    job_animation = cmds.checkBoxGrp( job + 'job_animation', q=True, value1=True ) 
    job_start = cmds.intFieldGrp( job + 'job_range', q=True, value1=True )
    job_end = cmds.intFieldGrp( job + 'job_range', q=True, value2=True )
    job_step = cmds.intFieldGrp( job + 'job_range', q=True, value3=True )
    
    
    mi_filename = cmds.textFieldGrp( mi + 'mi_filename', q=True, text=True )
    mi_padding = cmds.intFieldGrp( mi + 'mi_padding', q=True, value1=True )
    mi_perframe = cmds.checkBoxGrp( mi + 'mi_perframe', q=True, value1=True )
    mi_tabstop = cmds.intFieldGrp( mi + 'mi_tabstop', q=True, value1=True )
    mi_fileformat = cmds.optionMenuGrp( mi + 'mi_fileformat', q=True, value=True )
    mi_filepaths = cmds.optionMenuGrp( mi + 'mi_filepaths', q=True, value=True )
    mi_selection = cmds.checkBoxGrp( mi + 'mi_selection', q=True, value1=True )
    # mi_compression = cmds.optionMenuGrp( mi + 'mi_compression', q=True, value=True )
    mi_verbosity_level = cmds.optionMenuGrp( mi + 'mi_verbosity', q=True, sl=True ) - 1
    
    mi_reuse = cmds.checkBoxGrp( mi + 'mi_reuse', q=True, value1=True )
       
    filename = cmds.workspace( expandName=mi_filename )
    (filename, ext) = os.path.splitext( filename )
    
    if ext == '' or ext == '.' :
      print 'added .mi extension to path...' 
      ext = '.mi' 
      
    filename += ext   
      
    dirname = os.path.dirname( filename )
    if not os.path.exists( dirname ):
      print ( "path %s not exists" ) % dirname
      os.mkdir( dirname )
    else:
      # TODO!!! check if files are exist and have to be overriden
      if isSubmitingJob and mi_reuse:
        skipExport = True
        print "Skipping .mi files generation ..."
    
    if not skipExport: 
      infoStr = 'Generating .mi files for '
      if not job_animation:
        infoStr += ('current frame (%d) ...' % cmds.currentTime( q=True ) )
      else:
        infoStr += ('range %d - %d by %d ...' % (job_start, job_end, job_step) ) 
      print infoStr
        
      cmd = 'Mayatomr -miStream '
      cmd += '-file \"' + filename + '\" '
      if mi_verbosity_level !=0 : 
        cmd += '-v ' + str(mi_verbosity_level) + ' '
      cmd += '-perframe '
      if mi_perframe :
        cmd += '2 ' # (name.#.ext)
        cmd += '-padframe ' + str(mi_padding) + ' '
      else:
        cmd += '0 ' # (single .mi file)  
        
      if mi_fileformat == 'Binary' :
        cmd += '-binary '
      else:
        cmd += '-tabstop ' + str(mi_tabstop) + ' '  
      
      cmd += '-pcm ' # export pass contribition maps
      cmd += '-pud ' # export pass user data
      
      # link library    -none
      # include mi file -none 
      
      # texture file, 
      # lightmap, 
      # light profile, 
      # output image, 
      # shadow map, 
      # finalgather map, 
      # photon map, 
      # demand load object mi file.

      cmd += '-exportPathNames nn' 
      if mi_filepaths == 'Absolute' :
        cmd += 'aaaaaaaa '
      elif mi_filepaths == 'Relative':
        cmd += 'rrrrrrrr '
      else:
        cmd += 'nnnnnnnn '
  
      if mi_selection :
        cmd += '-active '
          
      print "cmd = %s" % cmd
    
      #
      # save RenderGlobals
      #
      defGlobals = 'defaultRenderGlobals'
      saveGlobals = {}
      saveGlobals['animation'] = cmds.getAttr( defGlobals + '.animation' )
      saveGlobals['startFrame'] = cmds.getAttr( defGlobals + '.startFrame' )
      saveGlobals['endFrame'] = cmds.getAttr( defGlobals + '.endFrame' )
      saveGlobals['byFrameStep'] = cmds.getAttr( defGlobals + '.byFrameStep' )
      #
      # override RenderGlobals
      #
      cmds.setAttr( defGlobals + '.animation', job_animation )
      cmds.setAttr( defGlobals + '.startFrame', job_start )
      cmds.setAttr( defGlobals + '.endFrame', job_end )
      cmds.setAttr( defGlobals + '.byFrameStep', job_step )
      
      mel.eval( cmd )
      #
      # restore RenderGlobals
      #
      cmds.setAttr( defGlobals + '.animation', saveGlobals['animation'] )
      cmds.setAttr( defGlobals + '.startFrame', saveGlobals['startFrame'] )
      cmds.setAttr( defGlobals + '.endFrame', saveGlobals['endFrame'] )
      cmds.setAttr( defGlobals + '.byFrameStep', saveGlobals['byFrameStep'] )
  #
  # submitJob
  #    
  def submitJob( self, param=None ):
    print ">> meMentalRayRender: submitJob()"
    
    job = self.winMain + '|f0|t0|tc0|fr1|fc1|'
    #job2 = self.winMain + '|f0|t0|tc0|fr2|fc2|'
    mi = self.winMain + '|f0|t0|tc1|fr1|fc1|'
    mray = self.winMain + '|f0|t0|tc2|fr1|fc1|'
    af = self.winMain + '|f0|t0|tc3|fr1|fc1|'
    bkburn = self.winMain + '|f0|t0|tc4|fr1|fc1|'
    bkburn2 = self.winMain + '|f0|t0|tc4|fr2|fc2|'
    
    job_name = cmds.textFieldGrp( job + 'job_name', q=True, text=True )
    job_description = cmds.textFieldGrp( job + 'job_description', q=True, text=True )
    
    job_animation = cmds.checkBoxGrp( job + 'job_animation', q=True, value1=True ) 
    job_start = cmds.intFieldGrp( job + 'job_range', q=True, value1=True )
    job_end = cmds.intFieldGrp( job + 'job_range', q=True, value2=True )
    job_step = cmds.intFieldGrp( job + 'job_range', q=True, value3=True )
    job_size = cmds.intFieldGrp( job + 'job_size', q=True, value1=True )
    job_priority = cmds.intFieldGrp( job + 'job_priority', q=True, value1=True )
    
    job_paused = cmds.checkBoxGrp( job + 'job_paused', q=True, value1=True )

    job_dispatcher = cmds.optionMenuGrp( job + 'job_dispatcher', q=True, value=True )
    
    mi_filename = cmds.textFieldGrp( mi + 'mi_filename', q=True, text=True )
    mi_padding = cmds.intFieldGrp( mi + 'mi_padding', q=True, value1=True )
    mi_perframe = cmds.checkBoxGrp( mi + 'mi_perframe', q=True, value1=True )
    mi_reuse = cmds.checkBoxGrp( mi + 'mi_reuse', q=True, value1=True )
    
   
    mr_options = cmds.textFieldGrp( mray + 'mr_options', q=True, text=True )
    mr_verbosity_level = cmds.optionMenuGrp( mray + 'mr_verbosity', q=True, sl=True ) - 1
    
    mr_progress_frequency = cmds.intFieldGrp( mray + 'mr_progress_frequency', q=True, value1=True )
    mr_threads = cmds.intFieldGrp( mray + 'mr_threads', q=True, value1=True )
    mr_texture_continue = cmds.checkBoxGrp( mray + 'mr_texture_continue', q=True, value1=True )
    
    af_capacity = cmds.intFieldGrp( af + 'af_capacity', q=True, value1=True )
        
    bkburn_cmdjob_path = cmds.textFieldButtonGrp( bkburn + 'bkburn_cmdjob_path', q=True, text=True )
    bkburn_manager = cmds.textFieldGrp( bkburn + 'bkburn_manager', q=True, text=True )
    bkburn_server_list = cmds.textFieldGrp( bkburn + 'bkburn_server_list', q=True, text=True )
    bkburn_server_group = cmds.textFieldGrp( bkburn + 'bkburn_server_group', q=True, text=True )
    bkburn_server_count = cmds.intFieldGrp( bkburn + 'bkburn_server_count', q=True, value1=True )
    #bkburn_port = cmds.intFieldGrp( bkburn + 'bkburn_port', q=True, value1=True )
    
    job_use_remote = cmds.checkBoxGrp( bkburn2 + 'job_use_remote', q=True, value1=True )
    job_dirmap_from = cmds.textFieldButtonGrp( bkburn2 + 'job_dirmap_from', q=True, text=True )
    job_dirmap_to = cmds.textFieldGrp( bkburn2 + 'job_dirmap_to', q=True, text=True )
    
    mr_renderer_local = cmds.textFieldButtonGrp( bkburn2 + 'mr_renderer_local', q=True, text=True )
    mr_renderer_remote = cmds.textFieldGrp( bkburn2 + 'mr_renderer_remote', q=True, text=True )
    mr_root_as_param = cmds.checkBoxGrp( bkburn2 + 'mr_root_as_param', q=True, value1=True )
    
    # UNUSED START  
    #bkburn_workpath = cmds.checkBoxGrp( bkburn + 'bkburn_workpath', q=True, value1=True )
    
    #bkburn_create_log = cmds.checkBox( bkburn + 'r1|bkburn_create_log', q=True, value=True )
    #bkburn_create_tasklist = cmds.checkBox( bkburn + 'r2|bkburn_create_tasklist', q=True, value=True )
    #bkburn_use_jobParamFile = cmds.checkBox( bkburn + 'r3|bkburn_use_jobParamFile', q=True, value=True )
    #bkburn_log_dir = cmds.textFieldButtonGrp( bkburn + 'r1|bkburn_log_dir', q=True, text=True )
    #bkburn_tasklist = cmds.textFieldButtonGrp( bkburn + 'r2|bkburn_tasklist', q=True, text=True )
    #bkburn_jobParamFile = cmds.textFieldButtonGrp( bkburn + 'r3|bkburn_jobParamFile', q=True, text=True )
    # UNUSED START  
    
    self.setupDirmaps( job_use_remote, job_dirmap_from, job_dirmap_to )
    
    self.generate_mi( True ) # isSubmitingJob=True
    
    if not job_animation: 
      job_start = cmds.currentTime( q=True )
      job_end = job_start
      job_step = 0
      print '* currentTime = %f' % job_start
      
    tp_start = job_start # Specify the starting value of an internally generated table used as a task list file. (Ignored if -taskList is used)
    tp_jump = job_step   # Specify the increment of the internally generated table used as a task list file. (Ignored if -taskList is used)
    numTasks = 1         # Number of tasks to perform. (Ignored if -taskList is used)
    
    if job_end < job_start : 
      job_end = job_start
    if job_animation : 
      numTasks = ( job_end - job_start ) / job_step + 1
    
    renderOptions = '' 
    if job_dispatcher == 'afanasy' :
      if mr_verbosity_level < 5: mr_verbosity_level = 5
      if mr_progress_frequency == 0 : mr_progress_frequency = 1
        
    if mr_verbosity_level != 0 : renderOptions += ' -v ' + str( mr_verbosity_level )
    if mr_progress_frequency != 0 : renderOptions += ' -progress_frequency ' + str( mr_progress_frequency )
    if mr_threads != 0 : renderOptions += ' -threads ' + str( mr_threads )  
    if mr_texture_continue != 0 : renderOptions += ' -texture_continue on' 
    renderOptions += ' ' + mr_options 
        
    job_dispatcher_name = job_dispatcher
    if job_dispatcher == 'none' : job_dispatcher_name = 'local renderer'
    print 'Submiting .mi files to %s ... ' % ( job_dispatcher_name )
    #
    # backburner
    #
    if job_dispatcher == 'backburner' :
      cmd = '\"' + bkburn_cmdjob_path + '\" '
      cmd += '-jobName \"' + job_name + '\" '
      if job_description != '' : cmd += '-description \"' + job_description + '\" '
      cmd += '-manager ' + bkburn_manager + ' ' 
      #if bkburn_port != 0 : cmd += '-port ' + str(bkburn_port) + ' '
      if job_priority != 50 : cmd += '-priority ' + str(job_priority) + ' '
      if bkburn_server_count != 0 : cmd += '-serverCount ' + str(bkburn_server_count) + ' '
      if bkburn_server_list != '' : cmd += '-servers \"' + bkburn_server_list + '\" ' 
      if bkburn_server_group != '' : cmd += '-group \"' + bkburn_server_group + '\" '
      
      #if bkburn_workpath : 
      #  cmd += '-workPath '
      #  if job_use_remote: 
      #    cmd += '\"' + cmds.dirmap( convertDirectory=self.rootDir ) + '\" '    
      #  else:
      #    cmd += '\"' + self.rootDir + '\" '   
      
      #if bkburn_create_log : cmd += '-logPath \"' + bkburn_log_dir + '\" '
      #if bkburn_create_tasklist : pass
      #if bkburn_use_jobParamFile : pass
      
      cmd += '-tp_start ' + str(tp_start) + ' '
      cmd += '-numTasks ' + str(numTasks) + ' '  
      if job_paused: cmd += '-suspended '
        
      renderCmd = '\"' + mr_renderer_local + '\" '
      if job_use_remote:
        renderCmd = '\"' + mr_renderer_remote + '\" '
        if mr_root_as_param :
          renderCmd += '\"' + cmds.dirmap( convertDirectory=self.rootDir ) + '\" '  
      
      renderCmd += renderOptions
              
      filename = cmds.workspace( expandName=mi_filename )
      if job_use_remote: 
        filename = cmds.dirmap( convertDirectory=filename )
      (name, ext) = os.path.splitext( filename )
      filename = name + '.%' + str(mi_padding) + 'tp1.mi' 
      renderCmd += '\"' + filename + '\" '
      cmd += renderCmd
      print "cmd = %s" % cmd
      cmd = str(cmd).replace( '"', '\\\"')
      print "cmd = %s" % cmd
      out = mel.eval( 'system "\\"' + str( cmd ) + '\\" "'  )
      print out
      #os.system( cmd )
      #import subprocess
      #subprocess.call( cmd, shell=True )
    #
    # Afanasy
    #
    elif job_dispatcher == 'afanasy' :
      import af  
      
      job = af.Job( job_name )
      job.setDescription ( job_description )
      job.setPriority ( job_priority )

      block1 = af.Block( 'render', 'mentalray' )
      block1.setWorkingDirectory( self.rootDir )
      
      pad_str = '#'
      if mi_padding > 0 and mi_perframe == True :
        for i in range( 1, mi_padding ): 
          pad_str += '#'  
      # get rid of occasionaly added 
      (name, ext) = os.path.splitext( mi_filename )
      miFileName = self.rootDir + '/' + name + '.@' + pad_str + '@.mi'
      imageFileName = ''
      images = cmds.renderSettings( fullPath = True, genericFrameImageName = ('@' + pad_str + '@')  )  # ('%0' + str(mi_padding) + 'd')
      
      for img in range( 0, len( images ) ) :
        # use relative pathnames if possible
        imageFileName += self.fromNativePath ( cmds.workspace( projectPath = images[ img ] ) )
        #imageFileName += images[ img ]
        if img < ( len( images ) - 1 ) : 
           imageFileName += ';'
      
      cmd = 'ray ' + renderOptions + ' ' + miFileName
      
      block1.setCommand( cmd )
      block1.setNumeric( int(job_start), int(job_end), job_size, job_step )
      #block1.setTasksDependMask( 'final')
      block1.setFiles( imageFileName )
      
      job.blocks.append( block1 )
      
      if job_paused : job.offline()
      
      job.output( 1 )
      job.send()
      
    else: # job_dispatcher == 'none'
      print 'Submiting .mi files to local renderer (Use Remote ignored) ...'
      renderCmd = '\"' + mr_renderer_local + '\" ' 
      #if mr_root_as_param : renderCmd += '\"' + self.rootDir + '\" '
      
      renderCmd += renderOptions
         
      filename = cmds.workspace( expandName=mi_filename )
      renderCmd += '\"' + filename + '\" ' 
      cmd = renderCmd
      print "cmd = %s" % cmd
      cmd = str(cmd).replace( '"', '\\\"')
      print "cmd = %s" % cmd
      os.chdir( self.rootDir )
      out = mel.eval( 'system "\\"' + str( cmd ) + '\\" "'  )
      print out   
  #
  #
  #
  def miFileNameChanged( self, name, value ):
    self.setDefaultStrValue( name, value ) 
    self.setResolvedPath() 
  #
  #
  #  
  def setResolvedPath( self ):
    mi = self.winMain + '|f0|t0|tc1|fr1|fc1|'
    mi2 = self.winMain + '|f0|t0|tc1|fr2|fc2|'
    mi_filename = cmds.textFieldGrp( mi + 'mi_filename', q=True, text=True )
    mi_padding = cmds.intFieldGrp( mi + 'mi_padding', q=True, value1=True )
    mi_perframe = cmds.checkBoxGrp( mi + 'mi_perframe', q=True, value1=True )
    
    filename = cmds.workspace( expandName=mi_filename )
    (filename, ext) = os.path.splitext( filename )
    if ext == '' or ext == '.' : ext = '.mi'
      
    if mi_padding > 0 and mi_perframe == True :
      filename += '.'
      pad_str = '#'
      for i in range( 1, mi_padding ): pad_str += '#'
      filename += pad_str
    filename += ext 
    cmds.textFieldGrp( mi2 + 'mi_resolved_path', edit=True, text=filename )
    
  #
  # setupUI
  #  
  def setupUI( self ):
    #print ">> meMentalRayRender: setupUI()"
    self.deleteUI( True )
    #
    # Main window setup
    #
    self.winMain = cmds.window( meMentalRayRenderMainWnd, 
                                title='meMentalRayRender ver.' + meMentalRayRenderVer + ' (' + self.os + ')' , 
                                menuBar=True,
                                retain=False,
                                widthHeight=(420, 460) )
                                
    self.mainMenu = cmds.menu( label="Commands", tearOff=False )
    cmds.menuItem( label='Check Shaders ...', command=self.checkShaders )
    cmds.menuItem( label='Check Texture Paths ...', command=self.checkTextures )
    cmds.menuItem( label='Generate .mi', command=self.generate_mi )
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
    # Job tab
    #
    tab = cmds.tabLayout( 't0', scrollable=True, childResizable=True )  # tabLayout -scr true -cr true  tabs; //  
    tab_setup = cmds.columnLayout( 'tc0', columnAttach=('left',0), rowSpacing=0, adjustableColumn=True ) # string $displayColumn = `columnLayout -cat left 0 -rs 0 -adj true displayTab`;
    cmds.frameLayout( 'fr1', label=' Parameters ', borderVisible=True, borderStyle='etchedIn', marginHeight=mr_hi  )
    cmds.columnLayout( 'fc1', columnAttach=('left',0), rowSpacing=0, adjustableColumn=True )
    #
    job_dispatcher = cmds.optionMenuGrp( 'job_dispatcher', cw=( (1, cw1), ), cal=(1, 'right'), 
                        label="Job Dispatcher ",  enable=True,
                        cc=partial( self.setDefaultStrValue, 'job_dispatcher' ) )
    for name in ('none', 'backburner', 'afanasy' ): cmds.menuItem( label=name )
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
                      cc=partial( self.setDefaultIntValue, 'job_animation' ) )
    
    cmds.intFieldGrp( 'job_range', cw=( (1, cw1), (2, cw2), (3, cw2), (4, cw2) ), nf=3, label="Start/Stop/By ", 
                       value1=self.job_param['job_start'], 
                       value2=self.job_param['job_end'], 
                       value3=self.job_param['job_step'], 
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
    
    cmds.setParent( '..' )
    #
    # .mi files tab
    #
    tab_miparam = cmds.columnLayout( 'tc1', columnAttach=('left',0), rowSpacing=0, adjustableColumn=True )
    cmds.frameLayout( 'fr1', label=' Export Settings ', borderVisible=True, borderStyle='etchedIn', marginHeight=mr_hi  )
    cmds.columnLayout( 'fc1', columnAttach=('left',0), rowSpacing=0, adjustableColumn=True )
    #
    cmds.checkBoxGrp( 'mi_reuse', cw=( (1, cw1), (2, cw1 * 2 ) ), 
                      label="Use Existing .mi Files ", 
                      value1=self.mi_param['mi_reuse'], 
                      cc=partial( self.setDefaultIntValue, 'mi_reuse' ) )  
    cmds.text( label='' )     
    
    #mi_dir = cmds.textFieldButtonGrp( cw=( 1, cw1 ), adj=2, label="Directory ", buttonLabel="...", text=self.mi_param['mi_dir'] )
    #cmds.textFieldButtonGrp( mi_dir, edit=True, bc=partial( self.browseDirectory, mi_dir ), cc=partial( self.setDefaultStrValue, 'mi_dir' ) )
    mi_filename = cmds.textFieldButtonGrp( 'mi_filename', cw=( 1, cw1 ), adj=2, 
                                           label="File Name ", buttonLabel="...", 
                                           text=self.mi_param['mi_filename'], 
                                           cc=partial( self.miFileNameChanged, 'mi_filename' ) ) 
    cmds.textFieldButtonGrp( mi_filename, edit=True, bc=partial( self.browseFile, mi_filename, 'mentalray files (*.mi)' ) )
    #cmds.text( label='' )
    # Resolved Path
    #
    cmds.intFieldGrp( 'mi_padding', cw=( (1, cw1), (2, cw2) ), 
                      label="Frame Padding ", 
                      value1=self.mi_param['mi_padding'], 
                      cc=partial( self.miFileNameChanged, 'mi_padding' ) )                         
    cmds.checkBoxGrp( 'mi_perframe', cw=( (1, cw1), (2, cw1 * 2 ) ), 
                      label="File Per Frame ", 
                      value1=self.mi_param['mi_perframe'], 
                      cc=partial( self.miFileNameChanged, 'mi_perframe' ) )  
   
    cmds.text( label='' )
    cmds.checkBoxGrp( 'mi_selection', cw=( (1, cw1), (2, cw1 * 2 ) ), 
                      label="Export Selection ", 
                      value1=self.mi_param['mi_selection'], 
                      cc=partial( self.setDefaultIntValue, 'mi_selection' ) ) 
    
    cmds.text( label='' )
    mi_filepaths = cmds.optionMenuGrp( 'mi_filepaths', cw=( (1, cw1), ), cal=(1, 'right'),
                                       label="Export File Paths ",  
                                       cc=partial( self.setDefaultStrValue, 'mi_filepaths' ) )
    for name in ( 'NoPath', 'Relative', 'Absolute' ):
      cmds.menuItem( label=name )
    cmds.optionMenuGrp( mi_filepaths, e=True, value=self.mi_param['mi_filepaths'] )
    
    mi_fileformat = cmds.optionMenuGrp( 'mi_fileformat', cw=( (1, cw1), ), cal=(1, 'right'), 
                                        label="Format .mi ",  
                                        cc=partial( self.setDefaultStrValue, 'mi_fileformat' ) )
    for name in ('Binary', 'ASCII'):
      cmds.menuItem( label=name )
    cmds.optionMenuGrp( mi_fileformat, e=True, value=self.mi_param['mi_fileformat'] )
    
    cmds.intFieldGrp( 'mi_tabstop', cw=( (1, cw1), (2, cw2) ), 
                      label="Tab stop (ASCII) ", 
                      value1=self.mi_param['mi_tabstop'], 
                      cc=partial( self.setDefaultIntValue, 'mi_tabstop' ) )   
    #mi_compression = cmds.optionMenuGrp( 'mi_compression', cw=( (1, cw1), ), label="Compression ", cal=(1, 'right'), cc=partial( self.setDefaultStrValue, 'mi_compression' ) )
    #cmds.menuItem( label='Off' )
    #cmds.menuItem( label='GzipBestSpeed' )
    #cmds.menuItem( label='GzipDefault' )
    #cmds.menuItem( label='GzipBest' )
    #cmds.optionMenuGrp( mi_compression, e=True, value=self.mi_param['mi_compression'] )
    
    # none, fatal, error, warning, info, progress, and details
    mi_verbosity = cmds.optionMenuGrp( 'mi_verbosity', cw=( (1, cw1), ), cal=(1, 'right'),  
                                       label="Verbosity ", 
                                       cc=partial( self.setDefaultStrValue, 'mi_verbosity' ) )
    for name in ('none', 'fatal', 'error', 'warning', 'info', 'progress', 'details'):
      cmds.menuItem( label=name )

    cmds.optionMenuGrp( mi_verbosity, e=True, value=self.mi_param['mi_verbosity'] )
    #                                      
    cmds.setParent( '..' )                                                     
    cmds.setParent( '..' )
    
    cmds.frameLayout( 'fr2', label=' Resolved Path ', borderVisible=True, borderStyle='etchedIn', marginHeight=mr_hi  )
    cmds.columnLayout( 'fc2', columnAttach=('left',0), rowSpacing=0, adjustableColumn=True )
    #
    cmds.textFieldGrp( 'mi_resolved_path', cw=( 1, 0 ), adj=2, label='', text='', editable=False )
    self.setResolvedPath()
    #                                      
    cmds.setParent( '..' )                                                     
    cmds.setParent( '..' )
    
    cmds.setParent( '..' )
    #
    # MentalRay tab
    #
    tab_mray = cmds.columnLayout( 'tc2', columnAttach=('left',0), rowSpacing=0, adjustableColumn=True )
    #cmds.text( label='' )
    cmds.frameLayout( 'fr1', label=' ray command line options ', borderVisible=True, borderStyle='etchedIn', marginHeight=mr_hi  )
    cmds.columnLayout( 'fc1', columnAttach=('left',0), rowSpacing=0, adjustableColumn=True )
                        
    cmds.textFieldGrp( 'mr_options', cw=( 1, cw1 ), adj=2, 
                       label="Additional Options ", 
                       text=self.mr_param['mr_options'], 
                       cc=partial( self.setDefaultStrValue, 'mr_options' ) )
    cmds.text( label='' ) 
    mr_verbosity = cmds.optionMenuGrp( 'mr_verbosity', cw=( (1, cw1), ), cal=(1, 'right'),  
                                       label="Verbosity ", 
                                       cc=partial( self.setDefaultStrValue, 'mr_verbosity' ) )
    for name in ('none', 'fatal', 'error', 'warning', 'info', 'progress', 'debug', 'details'):
      cmds.menuItem( label=name )

    cmds.optionMenuGrp( mr_verbosity, e=True, value=self.mr_param['mr_verbosity'] )  
    
    cmds.intFieldGrp( 'mr_progress_frequency', cw=( (1, cw1), (2, cw2) ), 
                      label="Progress frequency ", 
                      ann = 'Progress information should be emitted only when this percentage of the whole render time has passed.',
                      value1=self.mr_param['mr_progress_frequency'], 
                      cc=partial( self.setDefaultIntValue, 'mr_progress_frequency' ) )  
    cmds.intFieldGrp( 'mr_threads', cw=( (1, cw1), (2, cw2) ), 
                      label="Threads ", 
                      ann = 'The number of threads.',
                      value1=self.mr_param['mr_threads'], 
                      cc=partial( self.setDefaultIntValue, 'mr_threads' ) )                      
    cmds.checkBoxGrp( 'mr_texture_continue', cw=( (1, cw1), (2, cw1 * 2 ) ), 
                      label='', label1=" Skip missing textures", 
                      ann = 'If this option is specified, mental ray will continue for missing texture files',
                      value1=self.mr_param['mr_texture_continue'], 
                      cc=partial( self.setDefaultIntValue, 'mr_texture_continue' ) ) 
                      
                                      
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
                      value1=self.af_param['af_capacity'], 
                      cc=partial( self.setDefaultIntValue, 'af_capacity' ) )
    
    cmds.setParent( '..' )
    cmds.setParent( '..' )
            
    cmds.setParent( '..' )
    #
    # Backburner tab
    #
    tab_backburner = cmds.columnLayout( 'tc4', columnAttach=('left',0), rowSpacing=0, adjustableColumn=True )
    cmds.frameLayout( 'fr1', label=' Parameters ', borderVisible=True, borderStyle='etchedIn', marginHeight=mr_hi )
    cmds.columnLayout( 'fc1', columnAttach=('left',4), rowSpacing=0, adjustableColumn=True )
    #cmds.text( label='' )
    cmds.textFieldButtonGrp( 'bkburn_cmdjob_path', cw=( 1, cw1 ), adj=2, 
                            label="Cmdjob Path", buttonLabel="...", 
                            text=self.bkburn_param['bkburn_cmdjob_path'] )
    cmds.textFieldGrp( 'bkburn_manager', cw=( 1, cw1 ), adj=2, 
                       label="Manager Name ", 
                       text=self.bkburn_param['bkburn_manager'], 
                       cc=partial( self.setDefaultStrValue, 'bkburn_manager' ) )
    cmds.textFieldGrp( 'bkburn_server_list', cw=( 1, cw1 ), adj=2, 
                       label="Server List ", 
                       ann="Semi colon seperated list of servers.\n(Ignored if a group is used)",
                       text=self.bkburn_param['bkburn_server_list'], 
                       cc=partial( self.setDefaultStrValue, 'bkburn_server_list' ) )
    cmds.textFieldGrp( 'bkburn_server_group', cw=( 1, cw1 ), adj=2, 
                       label="Server Group ", 
                       text=self.bkburn_param['bkburn_server_group'], 
                       cc=partial( self.setDefaultStrValue, 'bkburn_server_group' ) )
    cmds.intFieldGrp( 'bkburn_server_count', cw=( (1, cw1), (2, cw2) ), 
                      label="Server Count ", 
                      ann="Max number of servers that can work on this job at any point in time.",
                      value1=self.bkburn_param['bkburn_server_count'], 
                      cc=partial( self.setDefaultIntValue, 'bkburn_server_count' ) )

    cmds.setParent( '..' )
    cmds.setParent( '..' )                      
    """ 
    cmds.intFieldGrp( 'bkburn_port', cw=( (1, cw1), (2, cw2) ), 
                      label="Port ", 
                      value1=self.bkburn_param['bkburn_port'], 
                      cc=partial( self.setDefaultIntValue, 'bkburn_port' ) )
                                       
    cmds.checkBoxGrp( 'bkburn_workpath', cw=( (1, cw1), (2, cw1 * 2 ) ), 
                      label='', label1=" -workPath = Project Root", 
                      ann = 'Working folder for cmdjob.exe and servers.\nUsed to resolve relative paths for running\nthe executable.',
                      value1=self.bkburn_param['bkburn_workpath'], 
                      cc=partial( self.setDefaultIntValue, 'bkburn_workpath' ) )
    cmds.text( label='' )
    cmds.rowLayout( 'r1', numberOfColumns=2, columnWidth1=cw2, adjustableColumn2=2, enable=True )
    cmds.checkBox( 'bkburn_create_log', width=cw3, 
                      label='', #"Create Log Files ", 
                      value=self.bkburn_param['bkburn_create_log'], 
                      cc=partial( self.setDefaultIntValue, 'bkburn_create_log' ) ) 
    bkburn_log_dir = cmds.textFieldButtonGrp( 'bkburn_log_dir', cw=( 1, cw1 - cw3 ), adj=2, 
                      label="Save Log To Path ", buttonLabel="...", 
                      text=self.bkburn_param['bkburn_log_dir'], 
                      cc=partial( self.setDefaultStrValue, 'bkburn_log_dir' ) )
    cmds.textFieldButtonGrp( bkburn_log_dir, edit=True, bc=partial( self.browseDirectory, bkburn_log_dir ) )
    cmds.setParent( '..' ) 
    cmds.rowLayout( 'r2', numberOfColumns=2, columnWidth1=cw2, adjustableColumn2=2, enable=False )
    cmds.checkBox( 'bkburn_create_tasklist', width=cw3, 
                      label='', #"Create Tasklist File ", 
                      value=self.bkburn_param['bkburn_create_tasklist'], 
                      cc=partial( self.setDefaultIntValue, 'bkburn_create_tasklist' ) ) 
    bkburn_tasklist = cmds.textFieldButtonGrp( 'bkburn_tasklist', cw=( 1, cw1 - cw3 ), adj=2, 
                      label="Use Tasklist File ", buttonLabel="...",
                      ann="File contains a tab seperated table.\nUse fill-in tokens to reference the table.", 
                      text=self.bkburn_param['bkburn_tasklist'], 
                      cc=partial( self.setDefaultStrValue, 'bkburn_tasklist' ) )
    cmds.textFieldButtonGrp( bkburn_tasklist, e=True, bc=partial( self.browseFile, bkburn_tasklist, 'Text files (*.txt)' ) )   
    cmds.setParent( '..' ) 
    cmds.rowLayout( 'r3', numberOfColumns=2, columnWidth1=cw2, adjustableColumn2=2, enable=False )
    cmds.checkBox( 'bkburn_use_jobParamFile', width=cw3,
                      label='', #"Use Job Param File ", 
                      value=self.bkburn_param['bkburn_use_jobParamFile'], 
                      cc=partial( self.setDefaultIntValue, 'bkburn_create_tasklist' ) ) 
    bkburn_jobParamFile = cmds.textFieldButtonGrp( 'bkburn_jobParamFile', cw=( 1, cw1 - cw3 ), adj=2, 
                     label="Use Job Param File ", buttonLabel="...",
                     ann="File with two tab-seperated column used to add custom data to the job.\nThe first colum is the parameter's name.\nThe second column is the parameter's value.",
                     text=self.bkburn_param['bkburn_jobParamFile'], 
                     cc=partial( self.setDefaultStrValue, 'bkburn_jobParamFile' ) )
    cmds.textFieldButtonGrp( bkburn_jobParamFile, e=True, bc=partial( self.browseFile, bkburn_jobParamFile, 'Text files (*.txt)' ) )                                                        
    """
    

    
    cmds.frameLayout( 'fr2', label=' Remote Directory Mapping ', borderVisible=True, borderStyle='etchedIn', marginHeight=mr_hi  )
    cmds.columnLayout( 'fc2', columnAttach=('left',0), rowSpacing=0, adjustableColumn=True )
    #
    cmds.checkBoxGrp( 'job_use_remote', cw=( (1, cw1), (2, cw1 * 2 ) ), 
                      label="Use Remote Render ", 
                      value1=self.job_param['job_use_remote'], 
                      cc=partial( self.setDefaultIntValue, 'job_use_remote' ) )
                       
    job_dirmap_from = cmds.textFieldButtonGrp( 'job_dirmap_from', cw=( 1, cw1 ), adj=2, 
                                               label='Dirmap "From Dir" ', buttonLabel="...", 
                                               text=self.job_param['job_dirmap_from'] )
                                               
    cmds.textFieldButtonGrp( job_dirmap_from, edit=True, 
                             cc=partial( self.setDefaultStrValue, 'job_dirmap_from' ), 
                             bc=partial( self.browseDirectory, job_dirmap_from ) )
                             
    cmds.textFieldGrp( 'job_dirmap_to', cw=( 1, cw1 ), adj=2, label='Dirmap "To Dir" ', 
                       text=self.job_param['job_dirmap_to'], 
                       cc=partial( self.setDefaultStrValue, 'job_dirmap_to' ) )
                       
    cmds.text( label='' ) 
    
    mr_renderer_local = cmds.textFieldButtonGrp( 'mr_renderer_local', cw=( 1, cw1 ), adj=2, 
                                                 label="Local Renderer ", buttonLabel="...", 
                                                 text=self.mr_param['mr_renderer_local'], 
                                                 cc=partial( self.setDefaultStrValue, 'mr_renderer_local' ) )
                                                    
    cmds.textFieldButtonGrp( mr_renderer_local, e=True, bc=partial( self.browseFile, mr_renderer_local, '*.*' ) )
    cmds.textFieldGrp( 'mr_renderer_remote', cw=( 1, cw1 ), adj=2, 
                       label="Remote Renderer ", 
                       text=self.mr_param['mr_renderer_remote'], 
                       cc=partial( self.setDefaultStrValue, 'mr_renderer_remote' )  )
    cmds.checkBoxGrp( 'mr_root_as_param', cw=( (1, cw1), (2, cw1 * 2 ) ), 
                      label='', label1=" 1-st parameter = Project Root", 
                      ann = 'Set 1-st parameter to Project Root, if render script,\nthat can handle this, is used as RemoteRenderer\nfor changing work path to network directory from windows',
                      value1=self.mr_param['mr_root_as_param'], 
                      cc=partial( self.setDefaultIntValue, 'mr_root_as_param' ) )                       
                                
    # 
    cmds.setParent( '..' )                                                     
    cmds.setParent( '..' )
    #cmds.text( label='' )
    cmds.setParent( '..' ) 
    
    cmds.tabLayout( tab, edit=True, 
                    tabLabel=( ( tab_setup, "Job" ), 
                               ( tab_miparam, ".mi" ),
                               ( tab_mray, "MentalRay" ),
                               ( tab_afanasy, "Afanasy" ),
                               ( tab_backburner, "Backburner" ) 
                             )             
                  )
    
    cmds.setParent( form )
    btn_sbm = cmds.button( label="Submit", command=self.submitJob, ann='Generate .mi files and submit to dispatcher' )
    btn_gen = cmds.button( label="Generate .mi", command=self.generate_mi, ann='Force .mi files generation' )
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
    #print (">> meMentalRayRender: deleteUI() "  )
    winMain = meMentalRayRenderMainWnd
    
    if cmds.window( winMain, exists=True ): cmds.deleteUI( winMain, window=True )
    if cmds.windowPref( winMain, exists=True ): cmds.windowPref( winMain, remove=True )
#
#
#
print 'meMentalRayRender sourced ...'
