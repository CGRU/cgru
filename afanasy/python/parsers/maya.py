# -*- coding: utf-8 -*-

from parsers import parser


class maya(parser.parser):
    """Generic Maya render output parser
    """

    def __init__(self):
        parser.parser.__init__(self)

        self.str_error += [
            'Frame aborted',
            'Frame rendering aborted',
            'Fatal Error',
            'Maya exited with status 210',
            'failed to render',
            '(kFailure)',
        ]
