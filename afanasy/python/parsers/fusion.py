# -*- coding: utf-8 -*-

from parsers import parser

import tempfile
import re

# Render started at Thu 3:24PM  (Range: 0 to 10)
# Rendered frame 0 (1 of 11), took 0.24 secs
# Rendered frame 1 (2 of 11), took 0.51 secs
# Rendered frame 2 (3 of 11), took 0.062 secs
# Rendered frame 3 (4 of 11), took 0.103 secs
# Rendered frame 4 (5 of 11), took 0.1 secs
# Rendered frame 5 (6 of 11), took 0.069 secs
# Rendered frame 6 (7 of 11), took 0.281 secs
# Rendered frame 7 (8 of 11), took 0.047 secs
# Rendered frame 8 (9 of 11), took 0.325 secs
# Rendered frame 9 (10 of 11), took 0.07 secs
# Rendered frame 10 (11 of 11), took 0.092 secs
# Render completed successfully at Thu 3:24PM - Total Time: 0h 0m 1.66s, Average: 6.03 frames/second
# Auto-exiting with errcode 0
# Closing outstanding documents

re_match_frame = re.compile(
    r'(Rendered frame )(\d*)( \()(\d*)( of )(\d*)'
)
re_match_complete = re.compile('Render completed.*')

class fusion(parser.parser):
    """fusion parser
    """

    def __init__(self):
        parser.parser.__init__(self)
        
        self.str_error = ['[mtoa] Failed batch render']
        self.log_file = \
            ('%s/fusion_render.log' % tempfile.gettempdir()).replace('\\', '/')
        self.str_finishedsuccess = ['Closing outstanding documents']
        self.last_read_line = 0

        # delete the log file beforehand
        import shutil
        import os
        try:
            os.remove(self.log_file)
        except (OSError, IOError):
            pass

    def do(self, data, mode):
        """Missing DocString

        :param data:
        :param mode:
        :return:
        """
        # the data is always empty
        # use the %TEMP%/fusion_render.log
        # file
        try:
            with open(self.log_file) as f:
                data = f.readlines()
        except (IOError, OSError) as e:
            pass

        current_line_count = len(data)
        if self.last_read_line == current_line_count - 1:
            # already read that line
            return

        # read new lines
        self.log = '\n'.join(data)
        for i in range(self.last_read_line, current_line_count):
            self.last_read_line = i
            line = data[i].strip()

            match = re_match_frame.findall(line)
            if match:
                self.frame = int(match[0][3])
                self.numframes = int(match[0][5])
                self.percentframe = 100
                self.calculate()

            match = re_match_complete.findall(line)
            if match:
                self.finishedsuccess = True
                # delete the log file

                import os
                try:
                    os.remove(self.log_file)
                except (OSError, IOError):
                    pass

