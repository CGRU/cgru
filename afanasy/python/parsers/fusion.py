# -*- coding: utf-8 -*-

import os
import re
import tempfile

from parsers import parser

re_match_frame = re.compile(r"(Rendered frame )(\d*)( \()(\d*)( of )(\d*)")
re_match_complete = re.compile("Render completed.*")


class fusion(parser.parser):
    """fusion parser"""

    def __init__(self):
        parser.parser.__init__(self)

        self.str_error = ["Error "]
        self.log_file = ("%s/fusion_render.log" % tempfile.gettempdir()).replace(
            "\\", "/"
        )
        self.str_finishedsuccess = ["Closing outstanding documents"]
        self.last_read_line = 0

        # delete the log file beforehand
        import os

        try:
            os.remove(self.log_file)
        except (OSError, IOError):
            pass

    def do(self, i_args):
        data = i_args["data"]

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
        self.log = "\n".join(data)
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

                try:
                    os.remove(self.log_file)
                except (OSError, IOError):
                    pass
