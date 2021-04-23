# -*- coding: utf-8 -*-

from parsers import hbatch
from parsers import arnold


class houdinitoarnold(hbatch.hbatch, arnold.arnold):
    """Houdini batch + catch arnold node output
    """

    def __init__(self):
        hbatch.hbatch.__init__(self)
        arnold.arnold.__init__(self)

    def do(self, i_args):
        arnold.arnold.do(self, i_args)
        hbatch.hbatch.do(self, i_args)
