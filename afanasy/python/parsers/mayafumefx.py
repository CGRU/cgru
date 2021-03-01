# -*- coding: utf-8 -*-

from parsers import maya

FRAME = "FumeFX: Frame: "


class mayafumefx(maya.maya):
    """Maya FumeFX simulation
    """

    def __init__(self):
        maya.maya.__init__(self)
        self.firstframe = True
        self.str_error += ['Error: FumeFX is not authorized']

    def do(self, i_args):
        data = i_args['data']

        needcalc = False
        index = 0
        while index < len(data):
            index = data.find(FRAME, index)
            if index == -1:
                break
            if self.firstframe:
                self.firstframe = False
            else:
                self.frame += 1
                needcalc = True
            index += len(FRAME)
        if needcalc:
            self.calculate()
