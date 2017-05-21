# -*- coding: utf-8 -*-

from parsers import parser

import re

# INFO : [Redshift] Block 32/48 (7,4) rendered by GPU 0 in 2ms
#        [Redshift] Block 126/135 (14,0) rendered by GPU 1 in 12ms
re_percent = re.compile(
    r'(Block*)(\s*)(\d*)(\/)(\d*)(\s*)(\S*)(\s*)(rendered by GPU.*)'
)
re_frame = re.compile(r'Rendering.*frame [0-9]+')


class maya_redshift(parser.parser):
    """Maya Redshift parser
    """

    def __init__(self):
        parser.parser.__init__(self)
        self.data_all = ''
        self.first_frame = True

        self.str_error = ['Frame rendering aborted']

        self.block = 0
        self.block_count = 0

    def do(self, data, mode):
        """Missing DocString

        :param data:
        :param mode:
        :return:
        """

        if len(data) < 1:
            return

        match = re_frame.search(data)
        if match is not None:
            if not self.first_frame:
                self.frame += 1
            else:
                self.first_frame = False
            self.block = 0
            self.block_count = 0
            self.percentframe = 0

        match = re_percent.findall(data)
        if match:
            # get current block
            matched_block = float(match[0][2])
            self.block = max(self.block, matched_block)

            # get block_count
            found_block_count = float(match[0][4])
            self.block_count = max(self.block_count, found_block_count)

            # calculate percentage
            self.percentframe = \
                int(100.0 * self.block / float(self.block_count))

        self.calculate()
