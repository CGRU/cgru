# -*- coding: utf-8 -*-

from services import service

parser = 'generic'


class htracker(service.service):
    """ Houdini distributed simulations tracker
    """
    def getCommand(self):
        """ Add job ID to command.
            It needed to send job started tracker parameters (host, port).
        """
        cmd = self.taskInfo['command']
        cmd += " -J %d" % self.taskInfo['job_id']
        return cmd

    def checkExitStatus(self, i_status):

        if i_status == 0:
            return True
        if i_status == 1:
            return True

        return False

