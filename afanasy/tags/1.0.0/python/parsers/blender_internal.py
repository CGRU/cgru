#import parser
import blender

keyframe = 'Part '
keyframe_len = len(keyframe)

class blender_internal( blender.blender):
   'Blender with internal render engine'
   def __init__( self, frames = 1):
      blender.blender.__init__( self, frames)

   def do( self, data):
      blender.blender.do( self, data)
      lines = data.split('\n')
      framepercent_founded = False
      for line in lines:
#         print line
         #  looking for keyframe:
         partpos = line.find( keyframe)
         if partpos < 0: continue
         #  looking for parts separator:
         numpartspos = line[partpos+keyframe_len:].find('-')
         if numpartspos < 1: continue
         part_cur_str = line[partpos+keyframe_len:partpos+keyframe_len+numpartspos]
         #  getting first part number:
         if part_cur_str.isdigit() != True: continue
         part_cur = int(part_cur_str)
         #  looking for parts quantity:
         parts_str = line[partpos+keyframe_len+numpartspos+1:]
         parts_str_len = len(parts_str)
         for c in range(parts_str_len):
            if parts_str[c].isdigit(): continue
            numpartspos = c
            break
         #  getting parts quantity number:
         parts_str = parts_str[0:numpartspos]
         if parts_str.isdigit() != True: continue
         parts = int(parts_str)

         # checking values validness:
         if parts < 1: continue
         if part_cur > parts: continue
         framepercent_founded = True

         # calcuating percent
         self.percentframe = 100 * part_cur / parts

      if framepercent_founded: self.calculate()
