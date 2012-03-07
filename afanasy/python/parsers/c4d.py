from parsers import parser

import re

re_frame   = re.compile(r'Rendering frame \d+')
re_number  = re.compile(r'\d+')
Errors = ['Files cannot be written - please check output paths!','Error rendering project']

class c4d(parser.parser):
   'Cinema 4D'
   def __init__( self, frames = 1):
      parser.parser.__init__( self, frames)
      self.firstFrame = True
      self.firstFrameNumber = 0
      self.frames = frames

   def do( self, data):

    # Check if there are any Errors
    for error in Errors:
        if data.find(error) != -1:
            self.error = True
            break

    # Now update the some Informations like current frame & percent
    match = re_frame.search( data )  
    
    if match == None: return
    
    # Get the current frame-number
    frame = re_number.search( match.group(0))
    thisFrame = int(frame.group(0))
    
    # To know with what frame the task starts
    if self.firstFrame:
        self.firstFrameNumber = thisFrame
        self.firstFrame = False
    
    # Calculate the current frame (not the frame-number!)
    currentFrame = thisFrame - self.firstFrameNumber
    
    # Calculate
    taskPercent = int(float(currentFrame)/self.frames*100)
    
    # It does not output anything about the frame progress it only displays when it is totally finished
    self.percentframe = 100
    
    self.percent = taskPercent
    self.frame = currentFrame  
     
    self.calculate()