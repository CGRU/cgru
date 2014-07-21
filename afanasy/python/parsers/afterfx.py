# -*- coding: utf-8 -*-


from parsers import parser

import re

re_frame = re.compile(r'PROGRESS: .* (.*): .* Seconds')

Errors = ['aerender Error', 'After Effects error', 'Unable to Render']


class afterfx(parser.parser):
    """Adobe After Effects
    """

    def __init__(self):
        parser.parser.__init__(self)
        self.firstframe = True
        self.data_all = ''

    def do(self, data, mode):
        self.data_all += data

        for error in Errors:
            if data.find(error) != -1:
                self.error = True
                break

        # Check whether was any progress:
        if mode == 'finished':
            if self.data_all.find('PROGRESS') == -1:
                self.badresult = True

        match = re_frame.search(data)
        if match is None:
            return

        if not self.firstframe:
            self.frame += 1

        self.firstframe = False
        self.calculate()
