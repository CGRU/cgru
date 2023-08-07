# -*- coding: utf-8 -*-

from parsers import parser

import re

#
# EXAMPLE OUTPUT
# INFO : [Redshift] Block 32/48 (7,4) rendered by GPU 0 in 2ms
#        [Redshift] Block 126/135 (14,0) rendered by GPU 1 in 12ms
# Error: [Redshift] License error: (RLM) All licenses in use (-22) //
#
# Resources: [Redshift]              Total triangles:                        18290255
#

re_percent = re.compile(
    r'(Block*)(\s*)(\d*)(\/)(\d*)(\s*)(\S*)(\s*)(rendered by GPU.*)'
)

re_triangles = re.compile(r'(.*)(Total triangles:)(\s*)(\d*)')

re_frame_start = re.compile(r'.*Rendering.*frame [0-9]+\.\.\.')
re_frame_skip = re.compile(r'.*Skipping frame.*')
re_frame_done = re.compile(r'.*Frame.*done.*')
re_rendering_done = re.compile(r'.*Rendering done.*')


class redshift(parser.parser):

    def __init__(self):
        parser.parser.__init__(self)
        self.data_all = ''

        self.str_error += [
            'No devices found. Aborting rendering.',
            'Rendering was internally aborted',
            'Bad node type found: Redshift_ROP',
            'Bad node type found: redshift_vopnet',
            'Error an illegal memory access was encountered',
            'License error',
        ]

        self.block = 0
        self.block_count = 0

        self.triangles = None

    def do(self, i_args):
        data = i_args['data']

        if len(data) < 1:
            return

        # catch new frame start
        match = re_frame_start.search(data)
        if match is not None:
            self.block = 0
            self.block_count = 0
            self.percentframe = 0

        # catch frame block progress
        percent = 0.0
        lines = data.split('\n')
        for line in lines:
            match = re_percent.search(line)
            if match:
                # get current block
                matched_block = float(match.group(3))
                self.block = max(self.block, matched_block)

                # get block_count
                found_block_count = float(match.group(5))
                self.block_count = max(self.block_count, found_block_count)

                # calculate percentage
                percent = int(100.0 * self.block / float(self.block_count))

            triangles = None
            search = re_triangles.search(line)
            if search:
                try:
                    triangles = int(search.group(4))
                except:
                    triangles = None

            if triangles is not None:
                if self.triangles is None:
                    self.triangles = triangles
                elif self.triangles < triangles:
                    self.triangles = triangles

        if self.triangles:
            self.resources = 'triangles:{}'.format(self.triangles)

        if self.percentframe < percent:
            self.percentframe = int(percent)

        match = re_frame_done.findall(data)
        if match:
            self.percentframe = 0
            self.frame += len(match)
        else:
            # try to match skip frame
            match = re_frame_skip.findall(data)
            if match:
                self.percentframe = 0
                self.frame += len(match)

        match = re_rendering_done.findall(data)
        if match:
            if self.numframes > 1:
                self.percentframe = 0
                self.frame = self.numframes
            else:
                self.percentframe = 100

        self.calculate()
