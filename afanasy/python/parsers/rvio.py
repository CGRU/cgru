from parsers import parser

import re

re_percent = re.compile(
    r'Writing frame [0-9]+ \(([0-9]+)\.[0-9]+% done\)'
)


class rvio(parser.parser):
    """VRay Standalone
    """

    def __init__(self):
        parser.parser.__init__(self)

    def do(self, i_args):
        data = i_args['data']

        if len(data) < 1:
            return

        match = re_percent.findall(data)
        if len(match):
            print("", match[-1])
            self.percentframe = int(match[-1])
            self.percent = int(match[-1])
