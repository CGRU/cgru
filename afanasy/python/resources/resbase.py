import sys

class resbase:
   'This is base class, not to be instanced'
   def __init__( self):

      self.value = 0
      self.valuemax = 100
      self.height = 50
      self.graphr = 0
      self.graphg = 255
      self.graphb = 0
      self.label = 'Custom\nMeter'
      self.labelsize = 10
      self.labelr = 255
      self.labelg = 255
      self.labelb = 0
      self.bgcolorr = 10
      self.bgcolorg = 20
      self.bgcolorb = 5
      self.width = 0
      self.tooltip = 'Custom resources meter.'

      self.valid = False

   def update( self):
      print 'Erorr: resbase.calculate: Invalid call, this method must be implemented.'

   def do( self):
      if not self.valid:
         print 'Error: Resources meter is not valid.'
         return None
      try:
         self.update()
      except:
         print 'Error updating resources:'
         print str(sys.exc_info()[1])
         self.label = 'Error'
      return self.value, self.valuemax, self.width, self.height, self.graphr, self.graphg, self.graphb, self.label, self.labelsize, self.labelr, self.labelg, self.labelb, self.bgcolorr, self.bgcolorg, self.bgcolorb, self.tooltip

   def output( self):
      print self.label
      print '%d (of %d) wh(%d,%d) l%d gc(%d,%d,%d) lc(%d,%d,%d) bc(%d,%d,%d)' % (self.value, self.valuemax, self.width, self.height, self.labelsize, self.graphr, self.graphg, self.graphb, self.labelr, self.labelg, self.labelb, self.bgcolorr, self.bgcolorg, self.bgcolorb)
      if self.tooltip != '': print self.tooltip
