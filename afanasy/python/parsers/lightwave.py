# -*- coding: utf-8 -*-

import re

from parsers import parser

# Example output
# Scene loaded.
# Beginning sequence: 1
# Beginning frame: 2
# Frame: 2


# Updating geometry
# Moving Sphere
# Preprocessing Frame...
# Beginning pre-process
# Updating pre-process
# Pre-process complete
# Rendering polygons
# Render Complete 7%
# Render Complete 16%
# Render Complete 25%
# Render Complete 34%
# Render Complete 41%
# Render Complete 52%
# Render Complete 61%
# Render Complete 70%
# Render Complete 81%
# Render Complete 90%
# Pass completed
# Updating frame: 2
# Finished frame: 2
# Render time: 11.0 seconds
# Writing RGB image to T:\TEST_LIGHTWAVE_OUTPUT\Final_Render_002.jpg

re_percent = re.compile(r"Render Complete (\d{1,3})%")
re_frame_start = re.compile(r"Beginning frame: (\d+)")
re_frame_done = re.compile(r"Finished frame: (\d+)")

class lightwave(parser.parser):
    def __init__(self):
        parser.parser.__init__(self)

    def do(self, i_args):
        data = i_args["data"]

        if len(data) < 1:
            return
        
        # catch new frame start
        match = re_frame_start.search(data)
        if match is not None:
            self.percentframe = 0

        # catch frame block progress
        percent = 0.0
        lines = data.split("\n")
        for line in lines:
            match = re_percent.search(line)
            if match:
                percent = float(match.group(1))

        if self.percentframe < percent:
            self.percentframe = int(percent)

        match = re_frame_done.findall(data)
        if match:
            self.percentframe = 0
            self.frame += len(match)

        self.calculate()
