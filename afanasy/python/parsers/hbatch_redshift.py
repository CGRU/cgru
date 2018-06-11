# -*- coding: utf-8 -*-

from parsers import hbatch
from parsers import redshift


class hbatch_redshift(hbatch.hbatch, redshift.redshift):
    """Houdini batch + catch redshift node output
    """

    def __init__(self):
        hbatch.hbatch.__init__(self)

        # Saving the error list from hbatch
        temp_str_error = self.str_error
        temp_str_warning = self.str_warning
        temp_str_badresult = self.str_badresult

        redshift.redshift.__init__(self)

        # Restore the list of errors of batch
        self.str_error += temp_str_error
        self.str_warning += temp_str_warning
        self.str_badresult += temp_str_badresult

    def do(self, data, mode):
        """Missing DocString

        :param data:
        :param mode:
        :return:
        """
        # print data
        redshift.redshift.do(self, data, mode)
        hbatch.hbatch.do(self, data, mode)
