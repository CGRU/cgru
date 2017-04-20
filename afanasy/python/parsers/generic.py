# -*- coding: utf-8 -*-

from parsers import parser

FRAME    = 'FRAME: '
PERCENT  = 'PROGRESS: '
ACTIVITY = 'ACTIVITY: '
REPORT   = 'REPORT: '

PERCENT_len  = len(PERCENT)
ACTIVITY_len = len(ACTIVITY)
REPORT_len   = len(REPORT)


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
            percent_pos += PERCENT_len
            ppos = data.find('%', percent_pos)
            if ppos > -1:
                needcalc = True
                self.percentframe = int(data[percent_pos:ppos])

        activity_pos = data.rfind(ACTIVITY)
        if activity_pos > -1:
            activity_pos += ACTIVITY_len
            self.activity = data[activity_pos: data.find('\n', activity_pos)]

        report_pos = data.rfind(REPORT)
        if report_pos > -1:
            report_pos += REPORT_len
            self.report = data[report_pos: data.find('\n', report_pos)]

        if needcalc:
            self.calculate()
