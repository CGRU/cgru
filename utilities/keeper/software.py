import os, sys, time

import cgruconfig

from PyQt4 import QtCore, QtGui

Names = ['Blender','Houdini','Maya','Nuke']
if sys.platform.find('win') == 0:
   Names.extend(['3DSMax','Softimage','AfterFX'])

def startDetached( command):
   if sys.platform.find('win') == 0:
      QtCore.QProcess.startDetached( 'cmd.exe', ['/c', command])
   else:
      QtCore.QProcess.startDetached( '/bin/bash', ['-c', command])

def startBlender():   startDetached('blender')
def startHoudini():   startDetached('houdini')
def startMaya():      startDetached('maya')
def startNuke():      startDetached('nuke')
def start3DSMax():    startDetached('3dsmax')
def startSoftimage(): startDetached('xsi')
def startAfterFX():   startDetached('afterfx')

def getIcon( soft):
   iconpath = os.path.join( cgruconfig.VARS['icons_dir'], os.path.join('software', soft.lower() + '.png'))
   if os.path.isfile( iconpath): return QtGui.QIcon( iconpath)

def exampleSoftware( folder, script):
   cmd = os.environ['CGRU_LOCATION']
   cmd = os.path.join( cmd, 'examples')
   cmd = os.path.join( cmd, folder)
   cmd = os.path.join( cmd, script)
   if sys.platform.find('win') == 0:
      cmd += '.cmd'
   else:
      cmd += '.sh'
   print(cmd)
   QtCore.QProcess.startDetached( cmd)

def exampleBlender():   exampleSoftware('blender','start_blender')
def exampleHoudini():   exampleSoftware('houdini','start_houdini')
def exampleMaya():      exampleSoftware('maya','start_maya')
def exampleNuke():      exampleSoftware('nuke','start_nuke')
def example3DSMax():    exampleSoftware('3D Studio Max','start_3dsmax')
def exampleSoftimage(): exampleSoftware('softimage','start_softimage')
def exampleAfterFX():   exampleSoftware('After FX','start_afterfx')

def browse():
   cmd = "browse"
   if sys.platform.find('win') == 0: cmd += '.cmd'
   else: cmd += '.sh'
   cmd = os.path.join('utilities', cmd)
   cmd = os.path.join( os.environ['CGRU_LOCATION'], cmd)
   folder = os.path.join( os.environ['CGRU_LOCATION'],'software_setup')
   cmd = '"%s" "%s"' % ( cmd, folder)
   startDetached( cmd)

def locateSoftware( soft):
   filename = QtGui.QFileDialog.getOpenFileName( None, 'Select %s executabe:' % soft)
   if filename is None: return
   filename = str( filename)
   if filename == '': return
   filename = os.path.normpath( filename)
   setup_folder = os.path.join( os.environ['CGRU_LOCATION'], 'software_setup')
   setup_file = 'locate_' + soft.lower()
   setup_file = os.path.join( setup_folder, setup_file)
   app_dir = os.path.dirname( filename)
   app_exe = os.path.basename( filename)
   if os.path.basename( app_dir) == 'bin':
      app_dir = os.path.dirname( app_dir)
      app_exe = os.path.join('bin', app_exe)
   lines = ['Created by Keeper at ' + time.ctime()]
   if sys.platform.find('win') == 0:
      setup_file += '.cmd'
      lines[0] = 'rem ' + lines[0]
      lines.append('set APP_DIR=%s' % app_dir)
      lines.append('set APP_EXE=%%APP_DIR%%\\%s' % app_exe)
   else:
      setup_file += '.sh'
      lines[0] = '# ' + lines[0]
      lines.append('export APP_DIR="%s"' % app_dir)
      lines.append('export APP_EXE="${APP_DIR}/%s"' % app_exe)
   file = open( setup_file,'w')
   for line in lines: file.write( line + '\n')
   file.close()

def locateBlender():    locateSoftware('Blender')
def locateHoudini():    locateSoftware('Houdini')
def locateMaya():       locateSoftware('Maya')
def locateNuke():       locateSoftware('Nuke')
def locate3DSMax():     locateSoftware('3DSMax')
def locateSoftimage():  locateSoftware('Softimage')
def locateAfterFX():    locateSoftware('AfterFX')
