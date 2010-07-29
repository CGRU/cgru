import resbase

import os
import subprocess

class iostat(resbase.resbase):
   'IO Stat - linux iostat command output'
   def __init__( self):
      resbase.resbase.__init__( self)
      self.value = 0

      self.device = os.getenv('AF_IOSTAT_DEVICE','sda')
      self.process = None

      self.valuemax = 100
      self.labelsize = 8
      self.labelr = 250
      self.labelg = 250
      self.labelb = 50
      self.height = 30
      self.bgcolorr = 10
      self.bgcolorg = 20
      self.bgcolorb = 5

      self.valid = True

   def _del__( self):
      print 'Goodbye !!!!!!'
      self.process.kill()

   def update( self):
      if self.process is not None:
         data = self.process.stdout.readlines()
         if len(data) > 3:
            fieldsline = data[-3]
            valiesline = data[-2]
            fields = fieldsline.split()
            values = valiesline.split()
            if len(fields) == 12 and len(values) == 12:
               self.label = values[0] + ':'
               self.label += ' %s=%s' % (fields[5],values[5])
               self.label += ' %s=%s' % (fields[6],values[6])
               self.label += ' %s=%s' % (fields[11],values[11])
               self.label += '\n'
               self.label += ' %s=%s' % (fields[7],values[7])
               self.label += ' %s=%s' % (fields[8],values[8])
               self.label += ' %s=%s' % (fields[9],values[9])
               self.label += ' %s=%s' % (fields[10],values[10])
               
               self.value = int(float(values[11]))
               if self.value > self.valuemax: self.value = self.valuemax
               if self.value < 0: self.value = 0

               self.graphr = int( 5 * self.value / 2 )
               self.graphg = 255 - int( 5 * self.value / 2 )
               self.graphb = 10
               self.bgcolorb = self.value

      self.process = subprocess.Popen(['iostat','-x','-d','-m',self.device,'4','2'], stdout=subprocess.PIPE)
