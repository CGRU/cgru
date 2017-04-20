# -*- coding: utf-8 -*-

from parsers import parser

import re

re_percent = re.compile(r'Progress for .*: \d+%')
re_percent_number = re.compile(r'\d+')
    
re_frame = re.compile(r'Saving Image:')

re_image = re.compile(r'File \'(.*)\'')


class clarisse(parser.parser):
    """Isotropix Clarisse
    """

    def __init__(self):
        parser.parser.__init__(self)

    def do(self, data, mode):
        """Missing DocString

        :param data:
        :param mode:
        :return:
        """
        need_calc = False

        # Search for frame percent:
        match = re_percent.search(data)
        if match is not None:
            self.percentframe = int( re_percent_number.findall(match.group(0))[-1])
            need_calc = True

        # Search for frame number:
        match = re_frame.search(data)
        if match is not None:
            self.frame += 1
            need_calc = True

        # Search for an image for thumbnail:
        match = re_image.search(data)
        if match is not None:
            self.appendFile( match.group(1))

        if need_calc:
            self.calculate()

