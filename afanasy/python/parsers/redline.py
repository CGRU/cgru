# -*- coding: utf-8 -*-

from parsers import parser

import re


class redline(parser.parser):
    """redline command parser
    """

    def __init__(self):
        parser.parser.__init__(self)

    # print('redline parser instanced')

    def do(self, i_args):
        data = i_args['data']

        data = re.findall(r'\b 0\.\d\d', data)
        if len(data) == 0:
            return
        data = data[0]
        print('output = ' + data)
#        if data.find('%') <= 0:
#            return

        data = data.replace('0.', '')
        self.percentframe = int(data)

        # print('percent = ' + data)

        self.calculate()
