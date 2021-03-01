# -*- coding: utf-8 -*-

from parsers import blender
from parsers import yafray


class blender_yafray(blender.blender, yafray.yafray):
    """Blender batch + catch yafray output
    """

    def __init__(self):
        blender.blender.__init__(self)
        yafray.yafray.__init__(self)

    def do(self, i_args):

        if yafray.yafray.do(self, i_args):
            self.calculate()

        blender.blender.do(self, i_args)
