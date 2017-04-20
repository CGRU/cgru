# -*- coding: utf-8 -*-

from parsers import parser

FRAME = "FumeFX: Frame: "

class mayafumefx(parser.parser):
    """Maya FumeFX simulation
    """

    def __init__(self):
        parser.parser.__init__(self)
        self.firstframe = True
        self.str_error = ['Error: FumeFX is not authorized']

    def do(self, data, mode):
        """Missing DocString

        :param data:
        :param mode:
        :return:
        """
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
