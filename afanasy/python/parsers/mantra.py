# -*- coding: utf-8 -*-

from parsers import parser

import re

PERCENT = 'ALF_PROGRESS '
PERCENT_len = len(PERCENT)

IMAGE_RE = re.compile(r'.*Generating Image: (.+) \(\d+x\d+\)')

ErrorsRE = [re.compile(r'Error loading geometry .* from stdin')]

# Will be used in 'mantrafilter.py' to ask parser to stop a process:
ParserErrorStr = '[ PARSER ERROR ]'

class mantra(parser.parser):
    """Houdini mantra with "Alfred Style Progress"
    """

    def __init__(self):
        parser.parser.__init__(self)
        self.str_warning = ['Unable to access file', 'Unable to load texture',
                            'Recompress failed', 'WARNING: A procedural of',
                            ' Unknown parameter: ', ' is rendering as a packed '
                            'procedural but has a mix of packed and '
                            'non-packed primitives', 'mantra: No velocity '
                            'attribute found for motion blur',
                            " primitives which may not be rendered directly",
                            'Procedural generated out of bounds geometry',
                            'mantra: WARNING:']
        self.str_error = [
            'No licenses could be found to run this application',
            'Please check for a valid license server host',
            'Failed to create file',
            ParserErrorStr]

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
            m = IMAGE_RE.match(line)
            if m:
                self.appendFile(m.group(1))

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
