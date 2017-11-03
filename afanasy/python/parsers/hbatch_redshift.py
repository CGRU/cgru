# -*- coding: utf-8 -*-

from parsers import hbatch


class hbatch_redshift(hbatch.hbatch):

    def __init__(self):
        hbatch.hbatch.__init__(self)

        self.str_error += [
            '[Redshift]No devices found. Aborting rendering.',
            'Frame aborted',
            'Bad node type found: Redshift_ROP',
            'Bad node type found: redshift_vopnet']

        self.str_warning += []

        self.str_badresult += [
            'Frame rendering aborted']
            

    def do(self, data, mode):
        """Missing DocString

        :param data:
        :param mode:
        :return:
        """

        hbatch.hbatch.do(self, data, mode)
