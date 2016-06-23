# -*- coding: utf-8 -*-

from parsers import parser

import re

PERCENT = 'ALF_PROGRESS '
PERCENT_len = len(PERCENT)

IMAGE = 'Generating Image:'
IMAGE_len = len(IMAGE)

ErrorsRE = [re.compile(r'Error loading geometry .* from stdin')]


class mantra(parser.parser):
    """Houdini mantra with "Alfred Style Progress"
    """

    def __init__(self):
        parser.parser.__init__(self)
        self.str_warning = ['Unable to access file', 'Unable to load texture']
        self.str_error = [
            'No licenses could be found to run this application',
            'Please check for a valid license server host',
            'Failed to create file']

    def do(self, data, mode):
        """Missing DocString

        :param data:
        :param mode:
        :return:
        """

        for errorRE in ErrorsRE:
            if errorRE.search(data) is not None:
                self.error = True
                break

        lines = data.split('\n')
        for line in lines:
            if line.find(IMAGE) != -1:
                line = line[IMAGE_len:]
                line = line[:line.find('(')]
                self.appendFile(line.strip())

        percent_pos = data.find(PERCENT)
        if percent_pos > -1:
            ppos = data.find('%')
            if ppos > -1:
                try:
                    percent = int(data[percent_pos + PERCENT_len:ppos])
                except:  # TODO: too broad exception clause
                    pass
                if -1 <= percent <= 100:
                    self.percentframe = percent
                    self.calculate()
