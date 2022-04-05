# -*- coding: utf-8 -*-

import json
import os
import sys
import traceback

import cgruutils

ACTIVITY = 'ACTIVITY: '
REPORT   = 'REPORT: '

# TODO: Class names should follow CamelCase naming convention
class parser(object):
    """This is base class, not to be instanced
    """

    def __init__(self):
        self.percent = 0
        self.frame = 0
        self.percentframe = 0
        self.error = False
        self.warning = False
        self.badresult = False
        self.finishedsuccess = False
        self.activity = ''
        self.resources = ''
        self.report = ''
        self.result = None
        self.log = None
        self.progress_changed = False
        self.numframes = 0
        self.taskInfo = {}
        self.pid = 0

        if not hasattr(self, 'str_warning'):
            self.str_warning = []
        if not hasattr(self, 'str_error'):
            self.str_error = []
        if not hasattr(self, 'str_badresult'):
            self.str_badresult = []
        if not hasattr(self, 'str_finishedsuccess'):
            self.str_finishedsuccess = []

        self.files = []
        self.files_onthefly = []
        self.files_all = []

        self.host_resources = None
        self.res_mem_peak_mb = None
        self.res_cpu_agv = None
        self.res_cpu_cur = []

    def setTaskInfo(self, taskInfo):
        """Missing DocString

        :param taskInfo:
        :return:
        """
        self.taskInfo = taskInfo
        self.numframes = taskInfo['frames_num']
        if self.numframes < 1:
            self.numframes = 1

    def appendFile(self, i_file, i_onthefly=False):
        """Missing DocString

        :param i_file:
        :return:
        """
        if i_file in self.files_all:
            return
        self.files_all.append(i_file)

        i_file = os.path.join(self.taskInfo['wdir'], i_file)
        if os.path.isfile(i_file):
            if i_onthefly:
                self.files_onthefly.append(i_file)
            else:
                self.files.append(i_file)

    def getFiles(self):
        """Missing DocString

        :return:
        """
        return self.files

    def getFilesOnTheFly(self):
        """Missing DocString

        :return:
        """
        files = self.files_onthefly
        self.files_onthefly = []
        return files

    def getLog(self):
        """
            This string will appear in a server task log
        """
        return self.log

    def do(self, i_args):
        """Missing DocString
        """
        pass

    def doBaseCheck(self, data):
        self.activity = ''
        self.report = ''
        self.warning = False
        self.error = False
        self.badresult = False
        self.result = None

        lower = data.lower()
        for string in self.str_warning:
            if lower.find(string.lower()) != -1:
                self.warning = True
        for string in self.str_error:
            if lower.find(string.lower()) != -1:
                self.error = True
        for string in self.str_badresult:
            if lower.find(string.lower()) != -1:
                self.badresult = True
        for string in self.str_finishedsuccess:
            if lower.find(string.lower()) != -1:
                self.finishedsuccess = True

        activity_pos = data.rfind(ACTIVITY)
        if activity_pos > -1:
            activity_pos += len(ACTIVITY)
            self.activity = data[activity_pos: data.find('\n', activity_pos)]

        report_pos = data.rfind(REPORT)
        if report_pos > -1:
            report_pos += len(REPORT)
            self.report = data[report_pos: data.find('\n', report_pos)]

        lines = data.split('\n')
        for line in lines:
            if line.find('@IMAGE@') != -1:  # Will be used in CGRU render scripts
                line = line[7:]
                self.appendFile(line.strip(), False)
            elif line.find('Image: ') == 0:  # ImageMagick
                line = line[7:]
                self.frame += 1
                self.calculate()
                # print(line)
                # print(self.frame,self.percent,self.numframes)
                self.appendFile(line.strip(), False)
            elif line.find('@IMAGE!@') != -1:  # Will be used in CGRU render scripts to generate thumb while task is still running
                line = line[8:]
                self.appendFile(line.strip(), True)

    def parse(self, i_args):
        """Missing DocString
        :return:
        """

        i_args['data'] = cgruutils.toStr(i_args['data'])
        if 'mode' in i_args:
            i_args['mode'] = cgruutils.toStr(i_args['mode'])
        if 'pid' in i_args:
            self.pid = i_args['pid']

        self.processResources(i_args)

        if len(i_args['data']):
            self.doBaseCheck(i_args['data'])

        try:
            self.result = self.do(i_args)
        except:  # TODO: too broad exception clause
            print('Error parsing output:')
            # print(str(sys.exc_info()[1]))
            traceback.print_exc(file=sys.stdout)

        self.progress_changed = self.hasProgressChanged(i_args)

    def hasProgressChanged(self, i_args):
        return len(i_args['data']) > 0

    def calculate(self):
        """Missing DocString
        """
        if self.frame < 0:
            self.frame = 0
        if self.frame > self.numframes:
            self.frame = self.numframes
        if self.percentframe < 0:
            self.percentframe = 0
        if self.percentframe > 100:
            self.percentframe = 100

        if self.numframes > 1:
            self.percent = \
                int((100.0 * self.frame + self.percentframe) / self.numframes)
        else:
            self.percent = self.percentframe

        if self.percent < 0:
            self.percent = 0
        if self.percent > 100:
            self.percent = 100


    def processResources(self, i_args):
        if not 'resources' in i_args:
            return

        resources = None
        try:
            resources = json.loads(cgruutils.toStr(i_args['resources']))
        except:
            print('Bad input resources json:')
            traceback.print_exc(file=sys.stdout)
            resources = None

        if 'host_resources' in resources:
            self.host_resources = resources['host_resources']

        if self.host_resources is None:
            return

        # Peak Memory
        if 'mem_total_mb' and 'mem_free_mb' in self.host_resources:
            mem_used_mb = self.host_resources['mem_total_mb'] - self.host_resources['mem_free_mb']
            if self.res_mem_peak_mb is None or self.res_mem_peak_mb < mem_used_mb:
                self.res_mem_peak_mb = mem_used_mb

        # Average CPU
        cpubusy = None
        for cpu in ['cpu_user', 'cpu_nice', 'cpu_system', 'cpu_iowait', 'cpu_irq', 'cpu_softirq']:
            if cpu in self.host_resources:
                if cpubusy is None:
                    cpubusy = self.host_resources[cpu]
                else:
                    cpubusy += self.host_resources[cpu]
        if cpubusy is not None:
            self.res_cpu_cur.append(cpubusy)
            self.res_cpu_agv = 0
            for cpu in self.res_cpu_cur:
                self.res_cpu_agv += cpu
            self.res_cpu_agv = int(round(self.res_cpu_agv / float(len(self.res_cpu_cur))))


    def toHTML(self, i_data):
        """ Convert data to HTML.
            Designed for GUIs for escape sequences, errors highlighting.
        :param i_data: input data
        :return: converted data
        """
        lines = cgruutils.toStr(i_data).replace('\r', '').split('\n')
        html = []

        for line in lines:
            html.append(self.toHTMLline(line))
        return '<br>\n'.join(html)

    def toHTMLline(self, i_line):
        """ Convert line to HTML.
            Designed for GUIs for escape sequences, errors highlighting.
        :param i_line: input line
        :return: converted line
        """

        self.parse({'data':i_line})

        if self.error:
            i_line = '<span style="background-color:#FF0000"><b>' + i_line + '</b></span>'
        if self.badresult:
            i_line = '<span style="background-color:#FF00FF"><b>' + i_line + '</b></span>'
        if self.warning:
            i_line = '<span style="background-color:#FFA500"><b>' + i_line + '</b></span>'
        if self.finishedsuccess:
            i_line = '<span style="color:#008800"><b>' + i_line + '</b></span>'
        if len(self.activity):
            i_line = '<span style="background-color:#00FF00"><b>' + i_line + '</b></span>'
        if len(self.report):
            i_line = '<i><b>' + i_line + '</b></i>'

        return self.tagHTML(i_line)

    def tagHTML(self, i_line):
        """ Convert line to HTML.
            Designed for GUIs for escape sequences, errors highlighting.
            Function designed to be implemented in child classes, if special highlinghting needed.
        :param i_line: input line
        :return: converted line
        """
        return i_line
