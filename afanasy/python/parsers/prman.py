from parsers import parser

class prman(parser.parser):
   'PIXAR\'s RenderMan'
   def __init__( self, frames = 1):
      parser.parser.__init__( self, frames)

   def do( self, data):
#      print 'Parsing "prman":'
#      print data
      data_len = len(data)
#      print 'data_len='+str(data_len)
      if data_len > 6:
         if data[data_len-2] == '%':
#            print '% founed:'
#            print data[data_len-5:data_len-2]
            percent = int(data[data_len-5:data_len-2])
            if percent >= -1 and percent <= 100:
               self.percentframe = percent
               self.percent = percent
#               print 'PERCENT='+str(self.percent)
               return True
      return False
