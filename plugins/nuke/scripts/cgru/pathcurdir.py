import nuke

import os

knobnames = ['file','proxy']

def do( tocut):
   nodes = []
   nodes.extend( nuke.allNodes('Read'))
   nodes.extend( nuke.allNodes('Write'))
   for node in nodes:
      for knobname in knobnames:
         knob = node.knob( knobname)
         if knob is not None:
            value = knob.value()
            if value is not None and value != '':
               if tocut:
                  knob.setValue( value.replace( os.getcwd()+'/',''))
               elif value[0] != '/':
                  knob.setValue( os.getcwd()+'/'+value)

def cut(): do( True)
def add(): do( False)
