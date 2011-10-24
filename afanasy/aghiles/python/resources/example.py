import resbase

import os
import subprocess

class example(resbase.resbase):
   'Example of custom resource meter class'
   def __init__( self):
      resbase.resbase.__init__( self)
      self.value = 0

      self.value = 0
      self.valuemax = 100
      self.height = 50
      self.graphr = 0
      self.graphg = 255
      self.graphb = 0
      self.labelsize = 10
      self.labelr = 255
      self.labelg = 255
      self.labelb = 0
      self.bgcolorr = 10
      self.bgcolorg = 20
      self.bgcolorb = 5
      self.width = 0

      self.valid = True

   def update( self):
      self.value += 3
      if self.value > self.valuemax: self.value = 0
      self.valuemax = 100
      self.height = 15 + self.value
      self.graphr = 2 * self.value
      self.label = 'Example\nValue = %d' % self.value
      self.labelsize = 5 + self.value/5
      self.labelr = 255
      self.labelg = 255
      self.labelb = 2 * self.value
      self.bgcolorr = 10
      self.bgcolorg = 20
      self.bgcolorb = 2 * self.value
      self.tooltip = 'Custom resources meter example.'
