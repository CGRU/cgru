# -*- coding: utf-8 -*-

from parsers import hbatch
from parsers import prman


class hbatch_prman(hbatch.hbatch, prman.prman):
    """Houdini batch + catch rib node output
    """

    def __init__(self):
        hbatch.hbatch.__init__(self)
        prman.prman.__init__(self)

    def do(self, i_args):
        if prman.prman.do(self, i_args):
            self.calculate()
        hbatch.hbatch.do(self, i_args)
