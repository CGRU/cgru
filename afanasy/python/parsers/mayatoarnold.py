# -*- coding: utf-8 -*-

from parsers import maya, arnold

FRAME = 'writing scene to'


class mayatoarnold(maya.maya, arnold.arnold):
    """Maya to Arnold
    """
    def __init__(self):
        maya.maya.__init__(self)
        arnold.arnold.__init__(self)

    def do(self, i_args):
        data = i_args['data']

        # TODO: This method may need to be inherited directly from the arnold.arnold.do() method
        needcalc = False

        if data.find(FRAME) > -1:
            self.frame += 1
            needcalc = True

        if needcalc:
            self.calculate()
