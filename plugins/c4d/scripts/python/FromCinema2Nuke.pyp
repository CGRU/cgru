#######################################################################################################
#  FromCinema2Nuke V1.0 - Date: 26.01.2017 - Created by Jan Oberhauser - jan.oberhauser@gmail.com     #
#  Exports Polygon-Objects, Cameras, Lights and Locators to Nuke                                      #
#######################################################################################################


#Imports the necessary stuff
import c4d
import os
import subprocess
import time
from math import *
from c4d import plugins, bitmaps

doc = c4d.documents.GetActiveDocument()


GROUP_ID1=1000
GROUP_ID2=1001
TEXTBOX_PATH=1005
TEXTBOX_SCRIPTNAME = 1021

BUTTON_CLOSE=1006
BUTTON_EXPORT=1007

CHECKBOX_CREATE_NUKE=1010  
CHECKBOX_OPEN_NUKE=1011
CHECKBOX_CREATE_DIFFUSE = 1012
CHECKBOX_DELETE_EXP_FILES = 1013
CHECKBOX_EXP_GEO_SEQ = 1014

STATIC_TEXT_PATH = 1101
STATIC_TEXT_SCRIPTNAME = 1102
STATIC_TEXT_STARTFRAME = 1103
STATIC_TEXT_ENDFRAME = 1104

EDITSLIDER_STARTFRAME = 1201
EDITSLIDER_ENDFRAME = 1202

WINDOW_WIDTH = 320


PLUGIN_ID = 1027904


class FromCinema2Nuke(plugins.CommandData):
    def __init__(self):

        #System Specific - Please change that it matches your System. Please use Slash instead of Backslash!!!
        self.nukePath = ''
#        self.nukePath = 'C:/Program Files/Nuke6.1v1/'
        self.nukeExec = 'nuke'
#        self.nukeExec = 'Nuke6.1.exe'

        self.showUI = True #Show UI-Window or just take default-Values

        #Sets Default-Values
        #self.defaultExportPath = '/Users/jober/Desktop/'
        #self.defaultExportPath = 'C:/Users/jober/Desktop/maya2nuke/2/'
        self.defaultExportPath = os.getenv('HOME', os.getenv('HOMEPATH', '')) + '/Desktop/ExportNuke/' 
        self.defaultNukeExportScriptName = 'NukeScene.nk'
        self.defaultExportTypeValue = 2 #Preset Export-Type - Values(1-> FBX, 2->OBJ)
        self.defaultExportGeometrySequence = False #Preset Export Geometry Sequence - Values(True,False) - Case-Sensetive
        self.defaultCreateDiffuseShader = False #Preset Create Diffuse-Shader for each exported Geometry - Values(True,False) - Case-Sensetive
        self.defaultCreateNukeScript = True #Preset Create Nuke-Script - Values(True,False) - Case-Sensetive
        self.defaultOpenNukeScript = True #Preset Open Nuke-Script - Values(True,False) - Case-Sensetive
        self.defaultDeleteExportFiles = False #Preset Delete Export-Files - Values(True,False) - Case-Sensetive
        self.defaultStartFrameValue = int(doc.GetMinTime().GetFrame(doc.GetFps()))
        self.defaultEndFrameValue = int(doc.GetMaxTime().GetFrame(doc.GetFps()))    
        
        self.doc = None



    def Execute(self, doc):
        # That it really takes the currenct scene
        self.defaultStartFrameValue = int(doc.GetMinTime().GetFrame(doc.GetFps()))
        self.defaultEndFrameValue = int(doc.GetMaxTime().GetFrame(doc.GetFps())) 
        
        #Init values with defaults
        self.startFrame = self.defaultStartFrameValue
        self.endFrame = self.defaultEndFrameValue
        self.createDiffuseShader = self.defaultCreateDiffuseShader
        self.exportGeometrySequence = self.defaultExportGeometrySequence
        self.createNukeScript = self.defaultCreateNukeScript
        self.openNukeScript = self.defaultOpenNukeScript
        self.deleteExport = self.defaultDeleteExportFiles
        self.nukeExportScriptName = self.defaultNukeExportScriptName
        self.exportPath = self.defaultExportPath
        self.exportType = self.defaultExportTypeValue 
        self.startExport = False
        
        self.doc = doc
        
        self.selected = self.doc.GetActiveObjects(False)
        if len(self.selected) == 0:
            c4d.gui.MessageDialog("Nothing is selected.\nPlease select the Objects, Cameras, Lights and Locators you want to export.\n")
        else:
            self.objects = []
            self.cameras = []
            self.lights = []
            self.locators = []
            self.newCameras = []
            if self.showUI == True:
                self.showParameterWindow() #Show Parameter Window
            
            if self.startExport:
                self.startExportProcedure()
        return True


    #STARTS THE EXPORT PROCEDURE
    def startExportProcedure(self, arg=None):
        #Saves Frame on which the Script starts
        timeScriptStart = self.doc.GetTime().Get()

        #Replaces Backslash with Slash
        self.exportPath = self.exportPath.replace('\\', '/')
        
        #Check if Slash is on the end
        if self.exportPath[-1] != '/':
            self.exportPath += '/'

        #Create export Folder if it does not exist
        if (os.path.isdir(self.exportPath) == 0):
            os.mkdir(self.exportPath)
        
        self.readObjects() #Reads in Objects in Lists
        
        self.exportObjects() #Exports the Objects
        self.exportCameras() #Exports the Cameras
        self.exportLights() #Exports the Lights
        self.exportLocators() #Exports the Locators
        
        if self.createNukeScript == True:
            self.createNukePythonFile() #Creates the Nuke-File
         
        if self.deleteExport == True:
            self.delteCameraFiles()    #Deletes the Camera-Files
            self.delteLightFiles()    #Deletes the Light-Files
            self.delteLocatorFile()    #Deletes the Locator-File
        
        if self.createNukeScript == True:
            self.delteNukePythonFile() #Deletes the Nuke-Python-File
                    
        if self.openNukeScript == True and self.createNukeScript == True:
            self.openNukeScriptAfterwards() #Open Nukescript
        
        #Goes back to the Frame it started, i think is not needed here because it runs the export in a seperate document
        self.doc.SetTime(c4d.BaseTime(timeScriptStart))



  #SHOW PARAMETER WINDOW
    def showParameterWindow(self):
        self.parWindow = self.fc2nExportWindow(self)
        self.parWindow.Open(c4d.DLG_TYPE_MODAL, defaultw=WINDOW_WIDTH, defaulth=120)
        

    
  #READ AND SORT OBJECTS
    def readObjects(self):
        #Go through all selected Objects
        for object in self.selected:
            #Get the Object-Type to sort
            objectType = c4d.GetObjectName(object.GetType())
            if objectType == "Camera":
                self.cameras[:0] = [object]
            elif objectType == "Null":
                self.locators[:0] = [object]
            elif objectType == "Light":
                self.lights[:0] = [object]
            else:
                self.objects[:0] = [object]


    #EXPORT OBJECTS
    def exportObjects(self):
        #Go through all selected Objects

        for object in self.objects:
            exportName = object.GetName().replace(':', '_').replace('.', '_')
            nameExtension = ''
            
            if bool(self.exportGeometrySequence) == bool(True):
                startF = self.startFrame
                endF = self.endFrame 
            else:
                startF = endF = self.doc.GetTime().GetFrame(self.doc.GetFps())

            tempDoc = c4d.documents.IsolateObjects(self.doc, [object])

            for timevalue in range(startF, (endF+1), 1):
                if bool(self.exportGeometrySequence) == bool(True):
                    nameExtension = '.%04d' % timevalue 

                if self.exportType == 1: #Export FBX
                    nameExtension += ".fbx" 
                else: #Export OBJ
                    nameExtension += ".obj" 

                self.setCurrentFrame(timevalue, tempDoc)
                c4d.documents.SaveDocument(tempDoc, self.exportPath+exportName+nameExtension, c4d.SAVEDOCUMENTFLAGS_0, c4d.FORMAT_OBJEXPORT )
                
            c4d.documents.KillDocument(tempDoc)

            print 'Exported: ' + exportName




    #EXPORT LOCACTOR-DATA IN FILE
    def exportLocators(self):
        channelMatch = {'c4d.ID_BASEOBJECT_REL_POSITION,c4d.VECTOR_X':'transform.tx', 'c4d.ID_BASEOBJECT_REL_POSITION,c4d.VECTOR_Y':'transform.ty', 'c4d.ID_BASEOBJECT_REL_POSITION,c4d.VECTOR_Z':'transform.tz', 'c4d.ID_BASEOBJECT_REL_SCALE,c4d.VECTOR_X':'transform.sx', 'c4d.ID_BASEOBJECT_REL_SCALE,c4d.VECTOR_Y':'transform.sy', 'c4d.ID_BASEOBJECT_REL_SCALE,c4d.VECTOR_Z':'transform.sz', 'c4d.ID_BASEOBJECT_REL_ROTATION,c4d.VECTOR_X':'transform.rx', 'c4d.ID_BASEOBJECT_REL_ROTATION,c4d.VECTOR_Y':'transform.ry', 'c4d.ID_BASEOBJECT_REL_ROTATION,c4d.VECTOR_Z':'transform.rz', 'c4d.ID_BASEOBJECT_ROTATION_ORDER':'transform.rotateOrder'}

        #Go through all selected Locators
        for locator in self.locators:
            #Export the Camera
            objectFileName = locator.GetName().replace(':', '_').replace('.', '_')
            self.exportData(locator, channelMatch, self.startFrame, self.endFrame, self.exportPath + objectFileName + '.fm2n')
            print "Exported: " + objectFileName



    #EXPORT CAMERA-DATA IN FILE
    def exportCameras(self):
#        channelMatch = {'c4d.ID_BASEOBJECT_REL_POSITION,c4d.VECTOR_X':'transform.tx', 'c4d.ID_BASEOBJECT_REL_POSITION,c4d.VECTOR_Y':'transform.ty', 'c4d.ID_BASEOBJECT_REL_POSITION,c4d.VECTOR_Z':'transform.tz', 'c4d.ID_BASEOBJECT_REL_SCALE,c4d.VECTOR_X':'transform.sx', 'c4d.ID_BASEOBJECT_REL_SCALE,c4d.VECTOR_Y':'transform.sy', 'c4d.ID_BASEOBJECT_REL_SCALE,c4d.VECTOR_Z':'transform.sz', 'c4d.ID_BASEOBJECT_REL_ROTATION,c4d.VECTOR_X':'transform.rx', 'c4d.ID_BASEOBJECT_REL_ROTATION,c4d.VECTOR_Y':'transform.ry', 'c4d.ID_BASEOBJECT_REL_ROTATION,c4d.VECTOR_Z':'transform.rz', 'c4d.ID_BASEOBJECT_ROTATION_ORDER':'transform.rotateOrder', 'c4d.CAMERA_FOCUS':'camera.fl', 'c4d.CAMERAOBJECT_FOV':'camera.horizontalFilmAperture', 'c4d.CAMERAOBJECT_FOV_VERTICAL':'camera.verticalFilmAperture'}
#        channelMatch = {'transform.tx':'tx', 'transform.ty':'ty', 'transform.tz':'tz', 'transform.rx':'rx', 'transform.ry':'ry', 'transform.rz':'rz', 'transform.rotateOrder':'rOrd', 'camera.fl':'focal', 'camera.horizontalFilmAperture':'aperture'}
        channelMatch = {'c4d.ID_BASEOBJECT_REL_POSITION,c4d.VECTOR_X':'transform.tx', 'c4d.ID_BASEOBJECT_REL_POSITION,c4d.VECTOR_Y':'transform.ty', 'c4d.ID_BASEOBJECT_REL_POSITION,c4d.VECTOR_Z':'transform.tz', 'c4d.ID_BASEOBJECT_REL_SCALE,c4d.VECTOR_X':'transform.sx', 'c4d.ID_BASEOBJECT_REL_SCALE,c4d.VECTOR_Y':'transform.sy', 'c4d.ID_BASEOBJECT_REL_SCALE,c4d.VECTOR_Z':'transform.sz', 'c4d.ID_BASEOBJECT_REL_ROTATION,c4d.VECTOR_X':'transform.rx', 'c4d.ID_BASEOBJECT_REL_ROTATION,c4d.VECTOR_Y':'transform.ry', 'c4d.ID_BASEOBJECT_REL_ROTATION,c4d.VECTOR_Z':'transform.rz', 'c4d.ID_BASEOBJECT_ROTATION_ORDER':'transform.rotateOrder', 'c4d.CAMERA_FOCUS':'camera.fl', 'c4d.CAMERAOBJECT_APERTURE':'camera.horizontalFilmAperture', 'c4d.CAMERAOBJECT_TARGETDISTANCE':'camera.focal_point', 'c4d.CAMERAOBJECT_TARGETDISTANCE':'camera.focal_point', 'c4d.CAMERAOBJECT_FNUMBER_VALUE':'camera.fstop'}
        
        #Go through all selected Cameras
        for camera in self.cameras:
            #Export the Camera 
            objectFileName = camera.GetName().replace(':', '_').replace('.', '_')
            self.exportData(camera, channelMatch, self.startFrame, self.endFrame, self.exportPath + objectFileName + '.fm2n')
            print "Exported: " + objectFileName


    #EXPORT LIGHT-DATA IN FILE
    def exportLights(self):
        #Go through all Lights
        channelMatch = {'c4d.ID_BASEOBJECT_REL_POSITION,c4d.VECTOR_X':'transform.tx', 'c4d.ID_BASEOBJECT_REL_POSITION,c4d.VECTOR_Y':'transform.ty', 'c4d.ID_BASEOBJECT_REL_POSITION,c4d.VECTOR_Z':'transform.tz', 'c4d.ID_BASEOBJECT_REL_SCALE,c4d.VECTOR_X':'transform.sx', 'c4d.ID_BASEOBJECT_REL_SCALE,c4d.VECTOR_Y':'transform.sy', 'c4d.ID_BASEOBJECT_REL_SCALE,c4d.VECTOR_Z':'transform.sz', 'c4d.ID_BASEOBJECT_REL_ROTATION,c4d.VECTOR_X':'transform.rx', 'c4d.ID_BASEOBJECT_REL_ROTATION,c4d.VECTOR_Y':'transform.ry', 'c4d.ID_BASEOBJECT_REL_ROTATION,c4d.VECTOR_Z':'transform.rz', 'c4d.ID_BASEOBJECT_ROTATION_ORDER':'transform.rotateOrder', 'c4d.LIGHT_BRIGHTNESS':'transform.intensity', 'c4d.LIGHT_COLOR,c4d.VECTOR_X':'transform.cr', 'c4d.LIGHT_COLOR,c4d.VECTOR_Y':'transform.cg', 'c4d.LIGHT_COLOR,c4d.VECTOR_Z':'transform.cb', 'c4d.LIGHT_DETAILS_OUTERANGLE':'transform.coneAngle', 'c4d.LIGHT_DETAILS_INNERANGLE':'transform.penumbraAngle', 'c4d.LIGHT_DETAILS_FALLOFF':'transform.dropoff'}
        
        for light in self.lights:            
            #Export Lights
            
            if light[c4d.LIGHT_TYPE] in [0,1,3]:
                objectFileName = light.GetName().replace(':', '_').replace('.', '_')
                self.exportData(light, channelMatch, self.startFrame, self.endFrame, self.exportPath + objectFileName + '.fm2n')
                print "Exported: " + objectFileName
            else:
                c4d.gui.MessageDialog(light.GetName() + " is not supported in Nuke. You can just use Point-(Omni), Distance-(Infinite) and Spot-Lights!")
    
    
    
    #DELETE CAMERA FILES
    def delteCameraFiles(self):
        for camera in self.cameras:
            objectFileName = camera.GetName().replace(':', '_').replace('.', '_')
            os.remove(self.exportPath + objectFileName + '.fm2n')

    #DELETE LIGHT FILES
    def delteLightFiles(self):
        for light in self.lights:
            objectFileName = light.GetName().replace(':', '_').replace('.', '_')
            os.remove(self.exportPath + objectFileName + '.fm2n')

    #DELETE LOCATOR FILE
    def delteLocatorFile(self):
        for locator in self.locators:
            objectFileName = locator.GetName().replace(':', '_').replace('.', '_')
            os.remove(self.exportPath + objectFileName + '.fm2n')


    #CREATE NUKE PYTHON FILE
    def createNukePythonFile(self):

        #Writes out the Import-Function
        nukePyFile = "def importData(importFile, channelMatch):\n\
    #Open File\n\
    file = open(importFile, 'r')\n\
    \n\
    #Read first Line\n\
    line = file.readline()\n\
    \n\
    values = line.split('\\t')\n\
    objectName = values[0]\n\
    objectNodeType = values[1]\n\
    if objectNodeType != 'camera' and objectNodeType != 'locator':\n\
        eval(objectName)['light_type'].setValue (objectNodeType)\n\
    \n\
    \n\
    ###########################################################################################\n\
    # START - Read all the static channel values                                              #\n\
    ###########################################################################################\n\
    line = file.readline()\n\
    while line[:18] != \'+++++Animated+++++':\n\
        #Split apart into channel and value\n\
        values = line.split('\\t')\n\
        \n\
        #Get the DestinationChannel - 0-> Channel, 1-> ChannelIndex\n\
        destinationChannel = channelMatch[values[0]].split(':')\n\
        \n\
        if destinationChannel[0] == 'rot_order':\n\
            eval(objectName)[destinationChannel[0]].setValue(values[1])\n\
        elif len(destinationChannel) == 2:\n\
            eval(objectName)[destinationChannel[0]].setValue(float(values[1]), int(destinationChannel[1]))\n\
        else:\n\
            eval(objectName)[destinationChannel[0]].setValue(float(values[1]))\n\
        \n\
        #Read next Line\n\
        line = file.readline()\n\
    \n\
    ###########################################################################################\n\
    # START - Read all the ANIMATED channel values                                            #\n\
    ###########################################################################################\n\
    line = file.readline()\n\
    #Reads in the Channelorder\n\
    destinationChannels = line.split('\\t')\n\
    #Remove the NewLine from the last Element\n\
    destinationChannels[len(destinationChannels)-1] = destinationChannels[len(destinationChannels)-1].replace('\\n', '')\n\
    \n\
    #Go through all Channels\n\
    row = 0\n\
    for channel in destinationChannels:\n\
        if row > 0:\n\
            #Get the DestinationChannel - 0-> Channel, 1-> ChannelIndex\n\
            destinationChannel = channelMatch[channel].split(':')\n\
            if len(destinationChannel) == 1:\n\
                #Makes the Channel Animated for normal Channels\n\
                eval(objectName)[destinationChannel[0]].setAnimated()\n\
            else:\n\
                #Makes the Channel Animated for Vector Channels\n\
                eval(objectName)[destinationChannel[0]].setAnimated(int(destinationChannel[1]))\n\
        row += 1\n\
    \n\
    line = file.readline()\n\
    while True:\n\
        #Split into the different Values\n\
        values = line.split('\\t')\n\
    \n\
        row = 0\n\
        for value in values:\n\
            if row == 0:\n\
                timevalue = value\n\
            else:\n\
                #Get the DestinationChannel - 0-> Channel, 1-> ChannelIndex\n\
                destinationChannel = channelMatch[destinationChannels[row]].split(':')\n\
                \n\
                if len(destinationChannel) == 2:\n\
                    eval(objectName)[destinationChannel[0]].setValueAt(float(value), int(timevalue), int(destinationChannel[1]))\n\
                else:\n\
                    eval(objectName)[destinationChannel[0]].setValueAt(float(value), int(timevalue))\n\
            \n\
            row += 1\n\
        \n\
        #Read next Line\n\
        line = file.readline()\n\
        #Stop while when the end of the file is reached\n\
        if line == '':\n\
            file.close()\n\
            break\n\n\n"


        #Create Scene and Background-Constant
        sceneInput = -1
        nukePyFile += '#Create Scene\n\
nuke.root().knob(\'first_frame\').setValue(' + str(self.startFrame) + ')\n\
nuke.root().knob(\'last_frame\').setValue(' + str(self.endFrame) + ')\n\
\n\
scene = nuke.nodes.Scene()\n\
\n\
#Creates Background-Constant\n\
background1 = nuke.nodes.Constant()\n\
background1[\'format\'].setValue(\'HD\')\n\
\n'

        #Create Objects         
        if self.exportType == 1: #Export FBX
            fileExtension = 'fbx'
        else: #Export OBJ
          fileExtension = 'obj'
        for object in self.objects:
            importName = object.GetName().replace(':', '_').replace('.', '_')
            sceneInput += 1
            if bool(self.exportGeometrySequence) == bool(True):
                nameExtension = '.%04d'
            else:
                nameExtension = ''
                
            nukePyFile += '#Creates Geometry-Objects and connects them to Scene\n\
' + importName + ' = nuke.nodes.ReadGeo(file=\'' + self.exportPath + importName + nameExtension + '.' + fileExtension + '\')\n\
\n'

            if bool(self.createDiffuseShader) == bool(True):
                nukePyFile += importName + 'ApplyMat = nuke.nodes.ApplyMaterial()\n\
' + importName + 'ApplyMat.setInput(0, ' + importName + ')\n\
' + importName + 'Diffuse = nuke.nodes.Diffuse()\n\
' + importName + 'ApplyMat.setInput(1, ' + importName + 'Diffuse)\n\
scene.setInput(' + str(sceneInput) + ',' + importName + 'ApplyMat)\n\n'
            else:
                nukePyFile += importName + 'background = nuke.nodes.Constant()\n\
' + importName + 'background[\'color\'].setValue(1)\n\
' + importName + 'background[\'format\'].setValue(\'HD\')\n\
' + importName + '.setInput(0, ' + importName + 'background)\n\
scene.setInput(' + str(sceneInput) + ',' + importName + ')\n\n'         


        #Create Lights
        nukePyFile += 'channelMatch = {\'transform.tx\':\'translate:0\', \'transform.ty\':\'translate:1\', \'transform.tz\':\'translate:2\', \'transform.rx\':\'rotate:0\', \'transform.ry\':\'rotate:1\', \'transform.rz\':\'rotate:2\', \'transform.sx\':\'scaling:0\', \'transform.sy\':\'scaling:1\', \'transform.sz\':\'scaling:2\', \'transform.rotateOrder\':\'rot_order\', \'transform.intensity\':\'intensity\', \'transform.cr\':\'color:0\', \'transform.cg\':\'color:1\', \'transform.cb\':\'color:2\', \'transform.coneAngle\':\'cone_angle\', \'transform.penumbraAngle\':\'cone_penumbra_angle\', \'transform.dropoff\':\'cone_falloff\'}\n'
        for light in self.lights:
            importName = light.GetName().replace(':', '_').replace('.', '_')
            sceneInput += 1
            nukePyFile += '#Creates Lights and connects them with ScanlineRender\n\
' + importName + ' = nuke.nodes.Light2()\n\
' + importName + '[\'name\'].setValue("' + importName + '")\n\
scene.setInput(' + str(sceneInput) + ',' + importName + ')\n\n\
\n\
#IMPORT LIGHT-VALUES\n\
importData(\'' + self.exportPath + importName + '.fm2n\', channelMatch)\n\n'        


        #Create Cameras
        for camera in self.cameras:
            importName = camera.GetName().replace(':', '_').replace('.', '_')
            nukePyFile += '#Creates Cameras and connects them with ScanlineRender\n\
channelMatch = {\'transform.tx\':\'translate:0\', \'transform.ty\':\'translate:1\', \'transform.tz\':\'translate:2\', \'transform.rx\':\'rotate:0\', \'transform.ry\':\'rotate:1\', \'transform.rz\':\'rotate:2\', \'transform.sx\':\'scaling:0\', \'transform.sy\':\'scaling:1\', \'transform.sz\':\'scaling:2\', \'transform.rotateOrder\':\'rot_order\', \'camera.fl\':\'focal\', \'camera.horizontalFilmAperture\':\'haperture\', \'camera.verticalFilmAperture\':\'vaperture\', \'camera.focal_point\':\'focal_point\', \'camera.fstop\':\'fstop\'}\n\
\n\
' + importName + ' = nuke.nodes.Camera()\n\
' + importName + '[\'name\'].setValue("' + importName + '")\n\
\n\
#Creates ScanlineRender and connects it with Scene\n\
' + importName + 'Render = nuke.nodes.ScanlineRender()\n\
#' + importName + 'Render.setXYpos(' + importName + 'Render[\'xpos\'].getValue(), ' + importName + 'Render[\'ypos\'].getValue()+400)\n\
' + importName + 'Render.setInput(1,scene)\n\
' + importName + 'Render.setInput(0,background1)\n\n\
' + importName + 'Render.setInput(2,' + importName + ')\n\
\n\
\n\
#IMPORT CAMERA-VALUES\n\
filename = \'' + self.exportPath + importName + '.fm2n\'\n\
importData(filename, channelMatch)\n\n'


        #Create Locators
        if len(self.locators) > 0:
            sceneInput += 1
            nukePyFile += '#Create Null to connect all Locators\n\
sceneLocatorInput = -1\n\
sceneLocator = nuke.nodes.Scene()\n\
channelMatch = {\'transform.tx\':\'translate:0\', \'transform.ty\':\'translate:1\', \'transform.tz\':\'translate:2\', \'transform.rx\':\'rotate:0\', \'transform.ry\':\'rotate:1\', \'transform.rz\':\'rotate:2\', \'transform.sx\':\'scaling:0\', \'transform.sy\':\'scaling:1\', \'transform.sz\':\'scaling:2\', \'transform.rotateOrder\':\'rot_order\'}\n\n'

            for locator in self.locators:
#                importName = locator.replace(':', '_')
                importName = locator.GetName().replace(':', '_').replace('.', '_')
                nukePyFile += '#Creates Locators and connects them with Scene-Node\n\
\n\
' + importName + ' = nuke.nodes.Axis()\n\
' + importName + '[\'name\'].setValue("' + importName + '")\n\
\n\
sceneLocatorInput += 1\n\
sceneLocator.setInput(sceneLocatorInput,' + importName + ')\n\n\
\n\
\n\
#IMPORT LOCATOR-VALUES\n\
filename = \'' + self.exportPath + importName + '.fm2n\'\n\
importData(filename, channelMatch)\n\n\
scene.setInput(' + str(sceneInput) + ',sceneLocator)\n\
\n'

        #Create Viewer-Node
        if len(self.cameras) > 0:
            importName = camera.GetName().replace(':', '_').replace('.', '_')
            nukePyFile += '#Create Viewer-Node\n\
viewer = nuke.nodes.Viewer()\n\
viewer.setInput(0, ' + importName + 'Render)\n'

  
        #Saves the Nuke-Script
        nukePyFile += '#Save Nuke-Script\n\
nuke.scriptSave( \'' + self.exportPath + self.nukeExportScriptName + '\')\n'

        nukePyFile += 'sys.exit(0)\n'


        #Save NukePythonFile to Disc
        f = open(self.exportPath + 'nukePyFile.py', 'w')
        f.write(nukePyFile.encode('ascii', 'xmlcharrefreplace') )
        f.close()
        
        #Execute Python-File with Nuke
        os.system('"' + self.nukePath + self.nukeExec + '" -t < ' + self.exportPath + 'nukePyFile.py' )

        
    
    def convertValuesCinema2Maya(self, parameterName, parameterValue):
        radianParameters = ['transform.rx', 'transform.ry', 'transform.rz', 'transform.coneAngle', 'transform.penumbraAngle']
        
        #Convert from Radians in Degree
        if parameterName in radianParameters:
            return (180*parameterValue)/pi

        #No idear why i have to flip Z but i do it
        if parameterName == 'transform.tz':
            return parameterValue * -1


        if parameterName == 'transform.rotateOrder':
            if parameterValue == 6: #If it is HPB
                return 'ZXY' #HPB is the same like ZXY only with the order changed | H = Y, P = X, B = Z
            else:
                rotateOrders = ['YXZ', 'YZX', 'ZYX', 'ZXY', 'XZY', 'XYZ']
                return rotateOrders[parameterValue]
        
        return parameterValue



    def getObjectParameterValue(self, object, channel, channels):
        parameterName = channels[channel]
           
            #Transformation-Values
        if parameterName == 'transform.tx':
            thisValue = object.GetMg().off.x

        elif parameterName == 'transform.ty':
            thisValue = object.GetMg().off.y
            
        elif parameterName == 'transform.tz':
            thisValue = object.GetMg().off.z        

        #Scale - Values
        elif parameterName == 'transform.sx':
            thisValue = object.GetMg().v1.GetLength()

        elif parameterName == 'transform.sy':
            thisValue = object.GetMg().v2.GetLength()

        elif parameterName == 'transform.sz':
            thisValue = object.GetMg().v3.GetLength()

        #Rotation - Values
        elif parameterName == 'transform.rx':
            if object[c4d.ID_BASEOBJECT_ROTATION_ORDER] == 6: #When HPB
                thisValue = c4d.utils.MatrixToHPB(object.GetMg()).y
            else:
                thisValue = object[eval(channel)]
                
        elif parameterName == 'transform.ry':
            if object[c4d.ID_BASEOBJECT_ROTATION_ORDER] == 6: #When HPB
                thisValue = c4d.utils.MatrixToHPB(object.GetMg()).x
            else:
                thisValue = object[eval(channel)]
            
        elif parameterName == 'transform.rz':
            if object[c4d.ID_BASEOBJECT_ROTATION_ORDER] == 0 or object[c4d.ID_BASEOBJECT_ROTATION_ORDER] == 1 or object[c4d.ID_BASEOBJECT_ROTATION_ORDER] == 2:
                thisValue = object[eval(channel)] * -1
            elif object[c4d.ID_BASEOBJECT_ROTATION_ORDER] == 6: #When HPB
                thisValue = c4d.utils.MatrixToHPB(object.GetMg()).z
            else:
                thisValue = object[eval(channel)]
        else:
            #Any other stuff apart from Translaten, Rotation and Scale
            thisValue = object[eval(channel)]
            
        return self.convertValuesCinema2Maya(parameterName, thisValue)




    #Export the Data
    def exportData(self, object, channels, startF, endF, exportFile):
#        objectPath = object.path()
        objectName = object.GetName().replace(':', '_').replace('.', '_')
        objectType = c4d.GetObjectName(object.GetType())
    
        channalsAnimated = []
        channalsNotAnimated = []
        
        objectNameWrite = objectName
        #Get the right Object-Type to write out
        if objectType == 'Camera':
            objectNodeTypeWrite = 'camera'
        elif objectType == 'Null':
            objectNodeTypeWrite = 'locator'
        elif objectType == 'Light':
            if object[c4d.LIGHT_TYPE] == 0:
                #Omni-Light
                objectNodeTypeWrite = 'point' 
            elif object[c4d.LIGHT_TYPE] == 1:
                #Spot-Light
                objectNodeTypeWrite = 'spot'
            elif object[c4d.LIGHT_TYPE] == 3:
                #Infinite-Light
                objectNodeTypeWrite = 'directional'

        
        #Get all the animated channels of that object
        allAnimatedChannels = object.GetCTracks()
        
        #Create an array with all the names of the animated channels
        allAnimatedChannelNames = []
        for aniChannel in allAnimatedChannels:
            allAnimatedChannelNames.append(aniChannel.GetName())


        self.aniChannelMatch = {'transform.tx':'Position . X', 'transform.ty':'Position . Y', 'transform.tz':'Position . Z', 'transform.sx':'Scale . X', 'transform.sy':'Scale . Y', 'transform.sz':'Scale . Z', 'transform.cr':'Color . R', 'transform.cg':'Color . G', 'transform.cb':'Color . B', 'transform.intensity':'Intensity'}

        if object[c4d.ID_BASEOBJECT_ROTATION_ORDER] == 6: #When HPB
            self.aniChannelMatch.update({'transform.rx':'Rotation . P', 'transform.ry':'Rotation . H', 'transform.rz':'Rotation . B'})
        else:
            self.aniChannelMatch.update({'transform.rx':'Rotation . X', 'transform.ry':'Rotation . Y', 'transform.rz':'Rotation . Z'})

  
        #Invert the aniChannelMatch to have acces to the channelNames
        channelMatch_inverted = dict( ((self.aniChannelMatch[k], k) for k in (self.aniChannelMatch) ) )
       
        #Now split all the channels which should get exported in an animated and not animated group
        for channel in channels:
            channelAnimated = False

            #Check if Channel is in MatchingDictionary (only animatable are in there), so if not there it will not be animated
            if channels[channel] in self.aniChannelMatch:
                #If it is in dictionary and also in the dictionary with the animated channels then it is animated
                if self.aniChannelMatch[channels[channel]] in allAnimatedChannelNames:
                    channelAnimated = True
                
            #Now depending on the set value it adds it to the matching group
            if channelAnimated:
                channalsAnimated.append(channel)
            else:    
                channalsNotAnimated.append(channel)

        
        #Output Values
        writeOut = objectNameWrite + '\t'  + objectNodeTypeWrite + '\t\n'
            
        #Write out not animated Channels  
        for channel in channalsNotAnimated:
            thisValue = self.getObjectParameterValue(object, channel, channels)

            writeOut += "%s\t%s\n" % (channels[channel], thisValue)
                
        writeOut += '+++++Animated+++++\n'
        #Write out animated Channels
    
        if len(channalsAnimated) > 0:
            #List all animated Channels
            writeOut += 'Frame'
            for channel in channalsAnimated:
                #channelValues = channel.split(':')
                writeOut += "\t%s" % channels[channel]
            writeOut += '\n'
                
            #Go through all Frames    
            for frame in range(startF, (endF+1)):
                #Write out Frame-Number
                writeOut += str(frame)
                
                #Set Frame - Maybe later differently
                self.setCurrentFrame(frame, self.doc)

                for channel in channalsAnimated:
                    writeOut += "\t%s" % self.getObjectParameterValue(object, channel, channels)
                writeOut += '\n'
        
        f = open(exportFile, 'w')
        f.write(writeOut)
        f.close()  
    

    def setCurrentFrame(self, frame, currentDocument):
        currentDocument.SetTime(c4d.BaseTime(float(frame)/currentDocument.GetFps()))
        currentDocument.ExecutePasses(None, True, True, True, 0);
        c4d.GeSyncMessage(c4d.EVMSG_TIMECHANGED)


    #DELETE NUKE PYTHON FILE
    def delteNukePythonFile(self):
        os.remove(self.exportPath + 'nukePyFile.py')
            
            
    #Open Nuke Script after export
    def openNukeScriptAfterwards(self):
        print '"' + self.nukePath + self.nukeExec + '" ' + self.exportPath + self.nukeExportScriptName
        subprocess.Popen([self.nukePath + self.nukeExec, self.exportPath + self.nukeExportScriptName])



    #Here starts the Class for the ParameterWindow
    class fc2nExportWindow(c4d.gui.GeDialog):
           
        def __init__(self, thisParent):
            #Get the parentClass and save it as an internal variable
            self.parentClass = thisParent
        
        def CreateLayout(self):
            #creat the layout of the dialog        
            self.SetTitle("FromCinema2Nuke - Export")
            
            self.GroupBegin(GROUP_ID1, c4d.BFH_SCALEFIT, cols=1, rows=17)
            
            self.AddStaticText(STATIC_TEXT_STARTFRAME, c4d.BFH_LEFT, name="Framerange:")
            self.AddEditSlider(EDITSLIDER_STARTFRAME, c4d.BFH_SCALEFIT)      
            self.AddEditSlider(EDITSLIDER_ENDFRAME, c4d.BFH_SCALEFIT)
    
            self.AddStaticText(STATIC_TEXT_PATH, c4d.BFH_LEFT, name="Export Path:")
            self.AddEditText(TEXTBOX_PATH, c4d.BFH_SCALEFIT)
    
            self.AddStaticText(STATIC_TEXT_SCRIPTNAME, c4d.BFH_LEFT, name="Export Script-Name:")
            self.AddEditText(TEXTBOX_SCRIPTNAME, c4d.BFH_SCALEFIT)         
            
            self.AddCheckbox(CHECKBOX_EXP_GEO_SEQ, c4d.BFH_FIT, name="Export Geometry animated (as Geometry-Sequence)", initw=WINDOW_WIDTH, inith=15)
            self.AddCheckbox(CHECKBOX_CREATE_NUKE, c4d.BFH_FIT, name="Create Nuke-Script", initw=WINDOW_WIDTH, inith=15)
            self.AddCheckbox(CHECKBOX_OPEN_NUKE, c4d.BFH_FIT, name="Open Nuke-Script after Export", initw=WINDOW_WIDTH, inith=15)
            self.AddCheckbox(CHECKBOX_CREATE_DIFFUSE, c4d.BFH_FIT, name="Create Diffuse-Shader for Geometry in Nuke-Script", initw=WINDOW_WIDTH, inith=15)
            self.AddCheckbox(CHECKBOX_DELETE_EXP_FILES, c4d.BFH_FIT, name="Delete Export-Files (.fm2n-Files) after Export", initw=WINDOW_WIDTH, inith=15) 
            
            self.GroupEnd()

            self.GroupBegin(GROUP_ID2, c4d.BFH_SCALEFIT, cols=2, rows=1)
            self.AddButton(BUTTON_CLOSE, c4d.BFH_SCALE, name="Close")
            self.AddButton(BUTTON_EXPORT, c4d.BFH_SCALE, name="Export")
            self.GroupEnd()

            return True



        def InitValues(self):
            #Get all the values from the parentClass and init everything with it
            self.SetReal(EDITSLIDER_STARTFRAME, self.parentClass.defaultStartFrameValue, min= self.parentClass.defaultStartFrameValue, max=self.parentClass.defaultEndFrameValue)
            self.SetReal(EDITSLIDER_ENDFRAME, self.parentClass.defaultEndFrameValue, min= self.parentClass.defaultStartFrameValue, max=self.parentClass.defaultEndFrameValue)
            
            self.SetString(TEXTBOX_PATH, self.parentClass.defaultExportPath)
            self.SetString(TEXTBOX_SCRIPTNAME, self.parentClass.defaultNukeExportScriptName)
            
            self.SetBool(CHECKBOX_EXP_GEO_SEQ, self.parentClass.defaultExportGeometrySequence)
            self.SetBool(CHECKBOX_CREATE_NUKE, self.parentClass.defaultCreateNukeScript)
            self.SetBool(CHECKBOX_OPEN_NUKE, self.parentClass.defaultOpenNukeScript)
            self.SetBool(CHECKBOX_CREATE_DIFFUSE, self.parentClass.defaultCreateDiffuseShader)
            self.SetBool(CHECKBOX_DELETE_EXP_FILES, self.parentClass.defaultDeleteExportFiles)

            return True
        
        def Command(self, id, msg):
            #handle user input
            if id==BUTTON_CLOSE:
                self.Close()
            elif id==BUTTON_EXPORT:
                
                self.parentClass.startFrame = self.GetReal(EDITSLIDER_STARTFRAME)
                self.parentClass.endFrame = self.GetReal(EDITSLIDER_ENDFRAME)
                
                self.parentClass.exportPath = self.GetString(TEXTBOX_PATH)
                self.parentClass.nukeExportScriptName = self.GetString(TEXTBOX_SCRIPTNAME)
                
                self.parentClass.exportType = self.parentClass.defaultExportTypeValue
                
                self.parentClass.exportGeometrySequence = self.GetBool(CHECKBOX_EXP_GEO_SEQ)
                self.parentClass.createNukeScript = self.GetBool(CHECKBOX_CREATE_NUKE)
                self.parentClass.openNukeScript = self.GetBool(CHECKBOX_OPEN_NUKE)
                self.parentClass.createDiffuseShader = self.GetBool(CHECKBOX_CREATE_DIFFUSE)
                self.parentClass.deleteExport = self.GetBool(CHECKBOX_DELETE_EXP_FILES)
                
                self.parentClass.startExport = True
                
                self.Close()
            return True



plugins.RegisterCommandPlugin(id=PLUGIN_ID, str="Export to Nuke",
                            info=c4d.OBJECT_GENERATOR, icon=None, help="Exports Objects from Cinema to Nuke",
                            dat=FromCinema2Nuke())