# -*- coding: utf-8 -*-
from services import service

import random

parser = 'generic'


class test(service.service):
    """Test service
    """

    def __init__(self, taskInfo, i_verbose):
        service.service.__init__(self, taskInfo, i_verbose)

        r = random.random()


        ver = .01
        if r  < ver:
            self.log = ('%f < %f') % (r, ver)
            self.taskInfo['command'] = ''

        ver = .03
        if r  < ver:
            self.skip_task = True
            self.log = ('%f < %f') % (r, ver)
