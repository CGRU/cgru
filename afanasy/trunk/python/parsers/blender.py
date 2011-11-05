from parsers import parser

keyframe = 'Fra:'
keypart = 'Part '

class blender(parser.parser):
   'Blender batch'
   def __init__( self, frames = 1):
      parser.parser.__init__( self, frames)
      self.firstframe = True
      self.framestring = keyframe
      self.partstring = keypart

   def do( self, data):
      lines = data.split('\n')
      for line in lines:
#         print line
         if line.find( keyframe) > -1:
            frmpos = line.find(' ')
            if frmpos < 0: continue

            need_calc = False

            # Increment frame if new:
            if line[0:frmpos] != self.framestring:
               self.framestring = line[0:frmpos]
               need_calc = True
               if self.firstframe:
                  self.firstframe = False
               else:
                  self.frame += 1
                  self.percentframe = 0

            # Try to calculate part:
            ptpos = line.find( keypart)
            if ptpos > 0:
               parts = line[ptpos+5:].split('-')
               if len(parts) == 2:
                  ok = True
                  try:
                     part0 = int(parts[0])
                     part1 = int(parts[1])
                  except:
                     ok = False
                  if ok:
                     if part1 > 0:
                        self.percentframe = int( 100 * part0 / part1)
                        need_calc = True

            if need_calc: self.calculate()
