# -*- coding: utf-8 -*-

from parsers import parser

import re

import cgruutils

PERCENT = 'ALF_PROGRESS '
PERCENT_len = len(PERCENT)

IMAGE_RE = re.compile(r'.*Image: (.+)')

PDG_IMG_RE = re.compile(r'PDG_RESULT:.*;\s*(.*)\s*;.*;.*')

ErrorsRE = [re.compile(r'Error loading geometry .* from stdin')]

PeakMem_RE = re.compile('.*Peak Memory Usage: *(.*)')

class karma(parser.parser):
    """Houdini karma with "Alfred Style Progress"
    """

    def __init__(self):
        parser.parser.__init__(self)
        self.str_warning = ['Unable to access file', 'Unable to load texture',
                            'Recompress failed', 'WARNING: A procedural of',
                            ' Unknown parameter: ', ' is rendering as a packed '
                            'procedural but has a mix of packed and '
                            'non-packed primitives', 'karma: No velocity '
                            'attribute found for motion blur',
                            " primitives which may not be rendered directly",
                            'Procedural generated out of bounds geometry',
                            'karma: WARNING:']
        self.str_error = [
            'No licenses could be found to run this application',
            'Please check for a valid license server host',
            'Failed to create file']

        self.karma_mem = None

    def do(self, i_args):
        data = i_args['data']
        res = ''

        for errorRE in ErrorsRE:
            if errorRE.search(data) is not None:
                self.error = True
                break

        lines = data.split('\n')
        for line in lines:
            m = IMAGE_RE.match(line)
            if m:
                self.appendFile(m.group(1))
            m = PDG_IMG_RE.search(line)
            if m:
                img = m.group(1)
                if cgruutils.isImageExt(img):
                    self.appendFile(self.expandEnvVars(img))
                

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

        # Search for karma pear memory usage
        for line in lines:
            match = PeakMem_RE.match(line)
            if match:
                mem = cgruutils.memStrToBytes(match.group(1))
                if mem:
                    mem = 'gb:{:.2f}'.format(float(mem) / (2**30))
                    self.karma_mem = 'karma_' + mem.strip('.0')

        # Resources and host peak memory
        if self.res_mem_peak_mb:
            res += ' mem_peak_mb:%d' % self.res_mem_peak_mb

        # Resources and karma peak memory
        if self.karma_mem:
            res += ' %s' % self.karma_mem

        # Resources and averarge CPU
        if self.res_cpu_agv:
            res += ' cpu_avg:%d' % self.res_cpu_agv

        # Add resources to parser object if any
        res = res.strip()
        if len(res):
            self.resources = res


    def expandEnvVars(self, i_str):

        if not 'environment' in self.taskInfo:
            return i_str

        for name in self.taskInfo['environment']:
            i_str = i_str.replace('__%s__' % name, cgruutils.toStr(self.taskInfo['environment'][name]))

        return i_str
