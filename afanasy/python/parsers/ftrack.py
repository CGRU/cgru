# -*- coding: utf-8 -*-

from parsers import parser


class Ftrack(parser.parser):
    """Simple Ftrack  parser
    """
    def __init__(self):
        parser.parser.__init__(self)
        self.firstframe = True
        self.str_error = ["FTRACK ERROR"]

    def do(self, i_args):
        data = i_args['data']

        data_len = len(data)
        if data_len < 1:
            return
