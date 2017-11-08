# -*- coding: utf-8 -*-

from parsers import parser

import re

# INFO : [Redshift] Block 32/48 (7,4) rendered by GPU 0 in 2ms
#        [Redshift] Block 126/135 (14,0) rendered by GPU 1 in 12ms
re_percent = re.compile(
    r'(Block*)(\s*)(\d*)(\/)(\d*)(\s*)(\S*)(\s*)(rendered by GPU.*)'
)

class redshift(parser.parser):

    def __init__(self):
        parser.parser.__init__(self)

        self.str_error = [
            '[Redshift]No devices found. Aborting rendering.',
            'Frame aborted',
            'Bad node type found: Redshift_ROP',
            'Bad node type found: redshift_vopnet']

        self.str_warning = []

        self.str_badresult = [
            'Frame rendering aborted']
            

    def do(self, data, mode):
        """Missing DocString

        :param data:
        :param mode:
        :return:
        """

        percent = 0.0

        lines = data.split('\n')
        for line in lines:
            m = re_percent.search(line)
            if m:
                # get current block
                block = float(m.group(3))
                # get blockCount
                blockCount = float(m.group(5))
                # calculate percentage
                percent = max(percent, block / blockCount * 100)

        if self.percentframe < percent:
            self.percentframe = int(percent)
            self.calculate()
