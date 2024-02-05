# -*- coding: utf-8 -*-

from parsers import hbatch
from parsers import karma


class hbatch_karma(hbatch.hbatch, karma.karma):
    """Houdini batch + catch karma node output
    """

    def __init__(self):
        hbatch.hbatch.__init__(self)
        karma.karma.__init__(self)

    def do(self, i_args):
        karma.karma.do(self, i_args)
        hbatch.hbatch.do(self, i_args)
