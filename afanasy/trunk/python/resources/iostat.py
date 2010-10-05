import resbase

import afenv

import re
import os
import subprocess

ENV_KEY='render_customiostat_devices'
ENV_VAR='AF_CUSTOMIOSTAT_DEVICES'
DEFAULT_DEVICE='sda'

COL_rMBs=5
COL_wMBs=6
COL_util=11
COL_avgrqsz=7
COL_avgqusz=8
COL_awaitsz=9
COL_svctm=10

class iostat(resbase.resbase):
   'IO Stat - linux iostat command output'
   def __init__( self):
      resbase.resbase.__init__( self)
      self.env = afenv.Env()
      if self.env.valid == False: print 'ERROR: iostat: Invalid environment, may be some problems.'
      self.value = 0

      self.device = DEFAULT_DEVICE
      if ENV_KEY in self.env.Vars: self.device = self.env.Vars[ENV_KEY]
      self.device = os.getenv('AF_IOSTAT_DEVICE', self.device)
      print 'Python IO Stat Device = "%s"' % self.device
      self.regexp = re.compile(self.device)

      self.process = None

      self.valuemax = 100
      self.labelsize = 8
      self.labelr = 250
      self.labelg = 250
      self.labelb = 50
      self.height = 40
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

         fields_pos = -1
         for i in range(1,len(data)):
            i = len(data) - i
            if data[i][:6] == 'Device':
               fields_pos = i
               break
         if fields_pos == -1:
            print 'ERROR: iostat: Can not find "Device" in output lines.'
            return
         if fields_pos >= (len(data)-1):
            print 'ERROR: iostat: Can not find any devices.'
            return
         fieldsline = data[fields_pos]
         fields = fieldsline.split()
         if len(fields) != 12:
            print 'ERROR: iostat: Unexpected number of fields.'
            return

         matcheddevices = ''
         rMBs     = 0.0
         wMBs     = 0.0
         util     = 0.0
         avgrqsz  = 0.0
         avgqusz  = 0.0
         awaitsz  = 0.0
         svctm    = 0.0

         for i in range( fields_pos+1, len(data)-1):
            values = data[i].split()
            if len(values) < 1: continue
            device = values[0]
            match = self.regexp.match( device)
            if match is None: continue
            if match.group(0) != device: continue
            if len(values) != 12:
               print 'WARNING: iostat: Unexpected number of values.'
               continue
            matcheddevices += ' '+device
            try:
               f_rMBs    = float(values[COL_rMBs])
               f_wMBs    = float(values[COL_wMBs])
               f_util    = float(values[COL_util])
               f_avgrqsz = float(values[COL_avgrqsz])
               f_avgqusz = float(values[COL_avgqusz])
               f_awaitsz = float(values[COL_awaitsz])
               f_svctm   = float(values[COL_svctm])
            except:
               print 'ERROR: iostat: Invalid columns values formatting.'
               continue
            rMBs += f_rMBs
            wMBs += f_wMBs
            if f_util    > util:    util    = f_util
            if f_avgrqsz > avgrqsz: avgrqsz = f_avgrqsz
            if f_avgqusz > avgqusz: avgqusz = f_avgqusz
            if f_awaitsz > awaitsz: awaitsz = f_awaitsz
            if f_svctm   > svctm:   svctm   = f_svctm

         if matcheddevices != '':
            self.label = matcheddevices+':'
            self.label += ' %s=%s' % (fields[5],values[5])
            self.label += ' %s=%s' % (fields[6],values[6])
            self.label += ' %s=%s' % (fields[11],values[11])
            self.label += '\n'
            self.label += ' %s=%s' % (fields[7],values[7])
            self.label += ' %s=%s' % (fields[8],values[8])
            self.label += '\n'
            self.label += ' %s=%s' % (fields[9],values[9])
            self.label += ' %s=%s' % (fields[10],values[10])
         else:
            self.label = 'No such devices founded: "%s"' % self.device

         self.value = int(float(values[11]))
         if self.value > self.valuemax: self.value = self.valuemax
         if self.value < 0: self.value = 0

         self.graphr = int( 5 * self.value / 2 )
         self.graphg = 255 - int( 5 * self.value / 2 )
         self.graphb = 10
         self.bgcolorb = self.value

      self.process = subprocess.Popen(['iostat','-x','-d','-m','4','2'], stdout=subprocess.PIPE)
