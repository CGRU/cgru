# -*- coding: utf-8 -*-

from parsers import parser

import re


class awscli(parser.parser):
    """Rsync command parser
    """

    def __init__(self):
        parser.parser.__init__(self)

    # print('awscli parser instanced')

    def do(self, i_args):
        """Missing DocString
        """
        data = i_args['data']

        data = re.findall(r'\d+ of \d+ part', data)
        if len(data) == 0:
            return

        data = re.findall(r'(\d+) of (\d+) part', data[-1])
        if len(data) == 0:
            return

        self.percentframe = int(float(data[0][0]) / float(data[0][1]) * 100)

        # print('percent = ' + data)

        self.calculate()
