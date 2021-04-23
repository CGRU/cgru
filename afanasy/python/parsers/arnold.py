# -*- coding: utf-8 -*-

from parsers import parser

import re

import cgruutils


re_percent = re.compile(r'(\s*)(\d*)(\s*% done)')

PeakMem_RE = re.compile('.*peak CPU memory used *(.*)')

class arnold(parser.parser):
    """Arnold
    """

    def __init__(self):
        parser.parser.__init__(self)
        self.firstframe = True
        self.data_all = ''
        

        self.str_warning += [
            'WARNING | rendering with watermarks because of failed authorization:',
        ]


        self.arnold_mem = None        

    def do(self, i_args):
        """Missing DocString
        """
        data = i_args['data']
        res = ''

        if len(data) < 1:
            return

        lines = data.split('\n')

        match = re_percent.findall(data)
        if len(match):
            percentframe = float(match[-1][1])
            self.percent = int(percentframe)

        for line in lines:
            match = PeakMem_RE.match(line)
            if match:
                mem = cgruutils.memStrToBytes(match.group(1))
                if mem:
                    mem = 'gb:{:.2f}'.format(float(mem) / (2**30))
                    self.arnold_mem = 'arnold_' + mem.strip('.0')

        # Resources and host peak memory
        if self.res_mem_peak_mb:
            res += ' mem_peak_mb:%d' % self.res_mem_peak_mb

        # Resources and arnold peak memory
        if self.arnold_mem:
            res += ' %s' % self.arnold_mem

        # Resources and averarge CPU
        if self.res_cpu_agv:
            res += ' cpu_avg:%d' % self.res_cpu_agv

        # Add resources to parser object if any
        res = res.strip()
        if len(res):
            self.resources = res
