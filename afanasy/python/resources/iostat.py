import resbase

import cgruconfig

import re
import os
import subprocess

ENV_KEY='af_render_customiostat_devices'
ENV_VAR='AF_CUSTOMIOSTAT_DEVICES'
DEFAULT_DEVICE='sda'

COL_rMBs=5
COL_wMBs=6
COL_util=-1
COL_avgrqsz=7
COL_avgqusz=8
COL_awaitsz=9
COL_svctm=-2

class iostat(resbase.resbase):
   'IO Stat - linux iostat command output'
   def __init__( self):
      resbase.resbase.__init__( self)
      self.value = 0

      self.device = DEFAULT_DEVICE
      if ENV_KEY in cgruconfig.VARS: self.device = cgruconfig.VARS[ENV_KEY]
      self.device = os.getenv('AF_IOSTAT_DEVICE', self.device)
      print 'Python IO Stat Device = "%s"' % self.device
      self.regexp = re.compile(self.device)

      self.process = None

      self.valuemax = 100
      self.labelsize = 10
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
            self.label += ' %s=%.2f' % (fields[COL_rMBs], rMBs)
            self.label += ' %s=%.2f' % (fields[COL_wMBs], wMBs)
            self.label += ' %s=%.2f' % (fields[COL_util], util)
            self.label += '\n'
            self.label += ' %s=%.2f' % (fields[COL_avgrqsz], avgrqsz)
            self.label += ' %s=%.2f' % (fields[COL_avgqusz], avgqusz)
            self.label += ' %s=%.2f' % (fields[COL_awaitsz], awaitsz)
            self.label += ' %s=%.2f' % (fields[COL_svctm],   svctm)
            self.value = int(util)
            if self.value > self.valuemax: self.value = self.valuemax
            if self.value < 0: self.value = 0
            self.graphr = int( 5 * self.value / 2 )
            self.graphg = 255 - int( 5 * self.value / 2 )
            self.graphb = 10
            self.bgcolorb = self.value
         else:
            self.label = 'No such devices founded: "%s"' % self.device

      self.process = subprocess.Popen(['iostat','-x','-d','-m','4','2'], stdout=subprocess.PIPE)
