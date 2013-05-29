"""
  maya_ui_proc.py
  
  ver.0.4.1 (10 May 2013)
    getMayaSceneName ( withoutSubdir = True )
  ver.0.4.0 (13 Mar 2013)
    getRenderLayersList:
    - check for referenced render layers
  ver.0.3.0 (28 Jan 2013)
  ver.0.2.0 (18 Jan 2013)
  ver.0.1.0 (17 Jan 2013)

  Author:

  Yuri Meshalkin (aka mesh)
  mesh@kpp.kiev.ua

  (c) Kiev Post Production 2013

  Description:
  
  Collection of procedures for working
  with Maya UI

"""
import sys, os, string
import maya.OpenMaya as OpenMaya
import maya.cmds as cmds
import maya.mel as mel  
#
# getDefaultStrValue
#
def getDefaultStrValue ( self_prefix, name, value ) :
  #
  var_name = self_prefix + name
  if cmds.optionVar ( exists = var_name ) == 1:
    ret = cmds.optionVar ( q = var_name )
  else:
    cmds.optionVar ( sv = ( var_name, value) )
    ret = value
  return ret
#
# getDefaultIntValue
#
def getDefaultIntValue ( self_prefix, name, value ) :
  #
  var_name = self_prefix + name
  if cmds.optionVar ( exists = var_name ) == 1:
    ret = cmds.optionVar ( q = var_name )
  else:
    cmds.optionVar ( iv = ( var_name, value) )
    ret = value
  return ret
#
# getDefaultFloatValue
#
def getDefaultFloatValue ( self_prefix, name, value ) :
  #
  var_name = self_prefix + name
  if cmds.optionVar ( exists = var_name ) == 1:
    ret = cmds.optionVar ( q = var_name )
  else:
    cmds.optionVar ( fv = ( var_name, value) )
    ret = value
  return ret
#
# setDefaultIntValue
#
def setDefaultIntValue ( self_prefix, name, section = None, value = None ) :
  #
  var_name = self_prefix + name
  cmds.optionVar ( iv = ( var_name, value ) )
  if section is not None : section [ name ] = value
  return value
#
# setDefaultIntValue2
#
def setDefaultIntValue2 ( self_prefix, names, section = None, value1 = None, value2 = None ) :
  #
  var_name = self_prefix + names [0]
  cmds.optionVar ( iv = ( var_name, value1 ) )
  var_name = self_prefix + names [1]
  cmds.optionVar( iv = ( var_name, value2 ) )
  if section is not None : 
    section [ names [0] ] = value1
    section [ names [1] ] = value2
  return ( value1, value2 )
#
# setDefaultIntValue3
#
def setDefaultIntValue3 ( self_prefix, names, section = None, value1=None, value2=None, value3=None ) :
  #
  var_name = self_prefix + names [0]
  cmds.optionVar ( iv = ( var_name, value1 ) )
  var_name = self_prefix + names [1]
  cmds.optionVar ( iv = ( var_name, value2 ) )
  var_name = self_prefix + names [2]
  cmds.optionVar( iv = ( var_name, value3) )
  if section is not None : 
    section [ names [0] ] = value1
    section [ names [1] ] = value2
    section [ names [2] ] = value3
  return ( value1, value2, value3 )
#
# setDefaultFloatValue
#
def setDefaultFloatValue ( self_prefix, name, section = None, value = None ) :
  #
  var_name = self_prefix + name
  cmds.optionVar( fv = ( var_name, value) )
  if section is not None : section [ name ] = value
  return value
#
# setDefaultFloatValue2
#
def setDefaultFloatValue2 ( self_prefix, names, section = None, value1 = None, value2 = None ) :
  #
  var_name = self_prefix + names [0]
  cmds.optionVar ( fv = ( var_name, value1 ) )
  var_name = self_prefix + names [1]
  cmds.optionVar ( fv = ( var_name, value2 ) )
  if section is not None : 
    section [ names [0] ] = value1
    section [ names [1] ] = value2
  return ( value1, value2 )
#
# setDefaultStrValue
#
def setDefaultStrValue ( self_prefix, name, section = None, value = None ) :
  #
  var_name = self_prefix + name
  cmds.optionVar ( sv = ( var_name, value ) )
  if section is not None : section [ name ] = value
  return value
#
# getMayaSceneName
#
# If 'withoutSubdir' is 'False', then return scene relative path
# to current project (include subdirectory in scene dir)
#
def getMayaSceneName ( withoutSubdir = True ) :
  #
  fullName = cmds.file ( q = True, sceneName = True )
  if withoutSubdir :
    sceneName = os.path.basename ( fullName )
  else :
    sceneName = cmds.workspace ( projectPath = fullName )
  ( sceneName, ext ) = os.path.splitext ( sceneName )
  if sceneName == '' : sceneName = 'untitled'
  return sceneName
#
# isRelative
#
def isRelative ( fileName ) :
  #
  ret = True
  if fileName != '' :
    if string.find ( fileName, ':' ) == 1 or string.find ( fileName, '/' ) == 0 or string.find ( fileName, '\\' ) == 0:
      ret = False
  return ret
#
# fromNativePath
#
def fromNativePath ( nativePath ) : return str ( nativePath ).replace ( '\\', '/')  
#
# getDirName
#
def getDirName ( image_name ) :
  #
  filename = cmds.workspace ( expandName = image_name )
  image_dir = os.path.dirname ( filename )
  return image_dir  
#
# getPadStr
#
def getPadStr ( padding, perframe ) :
  #
  pad_str = ''
  #print 'padding = %d perframe = %s' % ( padding, str ( perframe ) )
  if padding > 0 and perframe == True :
    pad_str = '#'
    for i in range ( 1, padding ): pad_str += '#'  
  return pad_str
#
# browseDirectory
#
def browseDirectory ( rootDir, control ) :
  #
  path = cmds.textFieldButtonGrp ( control, q = True, text = True )
  startDir = path
  if isRelative ( path ) :
    startDir = os.path.join ( rootDir, path )
  dirNames = cmds.fileDialog2 ( fileMode = 3, startingDirectory = startDir, dialogStyle = 1 )
  if dirNames is not None :
    dirName = cmds.workspace ( projectPath = fromNativePath( dirNames [0] ) )
    cmds.textFieldButtonGrp ( control, e = True, text = dirName, forceChangeCommand = True )
#
# browseFile
#
def browseFile ( rootDir, control, extFilter ) :
  #
  path = cmds.textFieldButtonGrp ( control, q = True, text = True )
  startDir = path
  if isRelative ( path ) :
    startDir = os.path.join ( rootDir, path )
  fileNames = cmds.fileDialog2 ( fileMode = 1, startingDirectory = os.path.dirname ( startDir ), dialogStyle = 1, fileFilter = extFilter )
  if fileNames is not None :
    fileName = cmds.workspace ( projectPath = fromNativePath ( fileNames [0] ) )
    cmds.textFieldButtonGrp ( control, e = True, text = fileName, forceChangeCommand = True )  
#
# getRenderCamerasList (renderable)
#
def getRenderCamerasList ( ) :
  #
  renderCamerasList = []
  cameras = cmds.listCameras ()
  for cam in cameras :
    camShape = cam
    if cmds.objectType ( cam ) == 'transform' : # != 'camera' :
      camShapes = cmds.listRelatives ( str ( cam ), shapes = True  )
      camShape = camShapes [ 0 ]
    if cmds.getAttr ( camShape + '.renderable' ) :
      renderCamerasList.append ( str ( cam ) )
  return renderCamerasList
#
# getRenderLayersList
# Returns list of render layers sorted as in RenderLayer Editor
#
def getRenderLayersList ( renderable = False ) :
  #
  renderLayers = {}
  layers = cmds.ls ( type = 'renderLayer' )
  for layer in layers :
    if not cmds.referenceQuery ( layer, isNodeReferenced = True ) :
      id = cmds.getAttr ( layer + '.displayOrder' )
      #print '* setup : %s (%d)' % ( layer, id )
      while id in renderLayers : id += 1 # try to find unique id
      if renderable and ( cmds.getAttr ( layer + '.renderable' ) == False ) :
        continue
      renderLayers [ id ] = layer    
  # get sorted by id list 
  labels = renderLayers.values ()
  labels.reverse ()
  return  labels   
#
# Open Maya Render Settings window
#
def maya_render_globals ( arg ) : mel.eval ( "unifiedRenderGlobalsWindow" )  
#
# mrShaderManager
#
def mrShaderManager ( arg ): mel.eval ( "mrShaderManager" ) 
#
# checkTextures
#
def checkTextures ( arg ) :
  #
  import meTools.meCheckTexturePaths as tx
  #reload( tx )
  tx.meCheckTexturePaths () 