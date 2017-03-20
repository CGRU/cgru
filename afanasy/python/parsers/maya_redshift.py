# -*- coding: utf-8 -*-

from parsers import parser

import re

# INFO : [Redshift] 	Block 32/48 (7,4) rendered by GPU 0 in 2ms
re_percent = re.compile(
    r'(Block*)(\s*)(\d*)(\/)(\d*)(\s*)(\S*)(\s*)(rendered by GPU)'
)
re_frame = re.compile(r'Rendering.*frame [0-9]+')


class maya_redshift(parser.parser):
    """Maya Redshift
    """

    def __init__(self):
        parser.parser.__init__(self)
        self.firstframe = True
        self.data_all = ''

    def do(self, data, mode):
        """Missing DocString

        :param data:
        :param mode:
        :return:
        """

        if len(data) < 1:
            return

        frame = False
        block = 0
        blockCount = 0

        match = re_frame.search(data)
        if match is not None:
            frame = True

        # progress line?
        match = re_percent.findall(data)
        if len(match):
            # get current block
            block = float(match[0][2])
            # get blockCount
            blockCount = float(match[0][4])

            # calculate percentage
            percentframe = float(100 / (blockCount / block))
            self.percent = int(percentframe)
            self.percentframe = int(percentframe)

        self.calculate()

        if frame:
            if not self.firstframe:
                self.frame += 1

            self.firstframe = False
