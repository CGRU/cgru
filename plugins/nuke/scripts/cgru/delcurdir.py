import nuke

import os

def call():
   knobFile = nuke.thisNode().knob('file')
   if knobFile is not None:
      knobFile.setValue( knobFile.value().replace( os.getcwd()+'/',''))
   knobProxy = nuke.thisNode().knob('proxy')
   if knobFile is not None:
      knobProxy.setValue( knobProxy.value().replace( os.getcwd()+'/',''))
