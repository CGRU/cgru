# -*- coding: utf-8 -*-

from parsers import parser

FRAME    = 'FRAME: '
PERCENT  = 'PROGRESS: '

class generic(parser.parser):
    """Simple generic parser
    """

    def __init__(self):
        parser.parser.__init__(self)

        self.str_warning =         ['[ PARSER WARNING ]']
        self.str_error =           ['[ PARSER ERROR ]']
        self.str_badresult =       ['[ PARSER BAD RESULT ]']
        self.str_finishedsuccess = ['[ PARSER FINISHED SUCCESS ]']

        self.firstframe = True

    def do(self, data, mode):
        """Missing DocString

        :param data:
        :param mode:
        :return:
        """
        needcalc = False

        if data.rfind(FRAME) > -1:
            if self.firstframe:
                self.firstframe = False
            else:
                self.frame += 1
                needcalc = True

        percent_pos = data.rfind(PERCENT)
        if percent_pos > -1:
            percent_pos += len(PERCENT)
            ppos = data.find('%', percent_pos)
            if ppos > -1:
                needcalc = True
                self.percentframe = int(data[percent_pos:ppos])

        if needcalc:
            self.calculate()
