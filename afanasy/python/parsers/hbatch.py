# -*- coding: utf-8 -*-

from parsers import parser

keyframe = 'RopNode.render: frame '


class hbatch(parser.parser):
    """Houdini9.x batch
    """

    def __init__(self):
        parser.parser.__init__(self)
        self.firstframe = True

    def do(self, data, mode):
        """Missing DocString

        :param data:
        :param mode:
        :return:
        """

        self.str_error = [
            'No licenses could be found to run this application',
            'Please check for a valid license server host']  

        self.str_warning = [
            'OpenCL Exception',
            'Failed to create']

        count = data.count(keyframe)
        if count > 0:
            if self.firstframe:
                self.firstframe = False
                count -= 1

            if count > 0:
                self.frame += count
                self.percentframe = 0
                self.calculate()

