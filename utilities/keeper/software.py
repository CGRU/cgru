import os, sys

from PyQt4 import QtCore

def startBlender(): QtCore.QProcess.startDetached('blender')
def startHoudini(): QtCore.QProcess.startDetached('houdini')
def startMaya(): QtCore.QProcess.startDetached('maya')
def startNuke(): QtCore.QProcess.startDetached('nuke')
