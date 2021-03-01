# -*- coding: utf-8 -*-

from parsers import maya

import re

# re.findall(r'(\|\s*)(\d*)(%\s*done)','12:34:56      32MB   |   45% done - 23 rays/pixel')
# |     0% done - 37 rays/pixel
re_percent = re.compile(r'(\s*)(\d*)(\s*% done)')


class maya_arnold(maya.maya):
    """mtoa parser
    """

    def __init__(self):
        maya.maya.__init__(self)
        self.firstframe = True
        self.data_all = ''

        self.str_error += [
            '[mtoa] Failed batch render'
        ]

    def do(self, i_args):
        data = i_args['data']

        if len(data) < 1:
            return

        match = re_percent.findall(data)
        if match:
            percentframe = float(match[-1][1])
            self.percentframe = int(percentframe)
            if self.percentframe == 100 and self.numframes > 1:
                self.frame += 1
                self.percentframe = 0
            self.calculate()
