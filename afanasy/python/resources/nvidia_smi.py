# -*- coding: utf-8 -*-

import json
import subprocess
import re

import cgruutils

from resources import resbase


class nvidia_smi(resbase.resbase):
    """nvidia-smi - NVIDIA System Management Interface program
    """

    def __init__(self):
        resbase.resbase.__init__(self)

        print('nvidia-smi - NVIDIA System Management Interface program')

        self.process = None

        self.tooltip = 'nvidia-smi'
        self.value = 0
        self.valuemax = 100
        self.height = 50
        self.graphr = 0
        self.graphg = 255
        self.graphb = 0
        self.labelsize = 10
        self.labelr = 255
        self.labelg = 255
        self.labelb = 0
        self.bgcolorr = 0
        self.bgcolorg = 0
        self.bgcolorb = 0
        self.width = 0

        self.valid = True

        self.mem_clr_min = 10
        self.mem_clr_max = 90
        self.tpr_clr_min = 70
        self.tpr_clr_max = 95

    def update(self):
        """Missing DocString
        :return:
        """

        if self.process is None:
            self.runProcess()
            return

        obj = dict()
        self.data, err = self.process.communicate()
        self.data = cgruutils.toStr(self.data)

        self.data = self.data.splitlines()
        self.line = 0
        while self.data[self.line].find('nvidia_smi_log') == -1: self.line += 1
        self.line += 2
        self.getLineObj(obj)

        #print(json.dumps(obj, sort_keys=True, indent=4))

        mem_total = 0
        mem_used  = 0
        mem_free  = 0
        tpr_val = 0
        tpr_max = 111

        label = ''
        tip = ''
        for gpu in obj['gpu']:
            if len(label): label += '\n'
            if len(tip): tip += '\n'

            label += gpu['product_name']
            tip += gpu['product_name']

            # Memory (used and total):
            mem = gpu['fb_memory_usage']
            total = int(mem['total'].split(' ')[0])
            used  = int(mem['used' ].split(' ')[0])
            free  = int(mem['free' ].split(' ')[0])
            label += ' %dG (%dM Used / %dM Free)' % (total/1000,used,free)
            tip += ' Memory: Total %dM, Used %dM, Free %dM' % (total,used,free)
            mem_total += total
            mem_used  += used
            mem_free  += free

            # Temperature (current and max):
            tpr = gpu['temperature']
            gpu_tpr = int(tpr['gpu_temp'].split(' ')[0])
            gpu_tpr_max = int(tpr['gpu_temp_slow_threshold'].split(' ')[0])
            label += ' %dC(%dC Max)' % (gpu_tpr, gpu_tpr_max)
            tip += ' Temperature: GPU %dC, Max %dC' % (gpu_tpr, gpu_tpr_max)
            if tpr_val < gpu_tpr: tpr_val = gpu_tpr
            if tpr_max > gpu_tpr_max: tpr_max = gpu_tpr_max

            # Collect processes and progs (processes with the same name)
            processes = []
            progs = {}
            for prc in gpu['processes']['process_info']:
                name = prc['process_name']
                mem = int(prc['used_memory'].split(' ')[0])
                # append processes:
                pc = dict()
                pc['name'] = name
                pc['mem'] = mem
                processes.append(pc)
                # Use base name for progs:
                name = name.split('/')[-1].split('\\')[-1]
                if name in progs:
                    progs[name] += mem
                else:
                    progs[name] = mem

            # constuct label string (sorted by mem):
            label += '\n'
            for name, mem in sorted(progs.items(), key=lambda kv: kv[1], reverse=True):
                label += ' ' + name
                label += ':%d' % mem

            # constuct tip strings (sorted by mem):
            processes.sort(key=lambda k: k['mem'], reverse=True)
            for prc in processes:
                tip += '\n' + prc['name']
                tip += ': %d' % prc['mem']


        self.label = 'v' + obj['driver_version'] + ' ' + label
        self.tooltip = 'NVIDIA Driver Verion: ' + obj['driver_version'] + '\n' + tip

        if mem_total:
            self.valuemax = mem_total
            self.value = mem_used
            clr = getClr(mem_used, mem_total, self.mem_clr_min, self.mem_clr_max)
            r = 2.0 * clr
            if r > 1.0: r = 1.0
            g = 2.0 * clr
            if g < 1.0: g = 1.0
            g = 2.0 - g
            self.graphr = int(255*r)
            self.graphg = int(255*g)

        if tpr_val:
            clr = getClr(tpr_val, tpr_max, self.tpr_clr_min, self.tpr_clr_max)
            self.bgcolorb = int(255*clr)

        self.runProcess()

    def getLineObj(self, obj):

        if self.line >= len(self.data):
            return

        #print('! %04d (%04d): %s' % (self.line, len(self.data), self.data[self.line]))

        if self.data[self.line].find('nvidia_smi_log') != -1:
            return

        while self.line < len(self.data):

            if len(self.data[self.line]) == 0:
                self.line += 1
                continue

            fields = re.findall('(<\w*>)(.*)(</\w*>)', self.data[self.line])
            if len(fields):
                fields = fields[0]
                if len(fields) == 3:
                    # simple string value '<name>value</name>':
                    #print('%04d (%04d): %s' % (self.line, len(self.data), self.data[self.line]))
                    obj[fields[0].strip('<>')] = fields[1].strip('<>')
            elif len(re.findall('</(\w*)>', self.data[self.line])) == 1:
                # end of object '</name>':
                #print('End of: ' + self.data[self.line])
                return
            else:
                # start of a new object '<name>':
                objname = re.findall('<(.*)>', self.data[self.line])
                if len(objname) == 1:
                    objname = objname[0]
                    if objname.find('gpu id=') == 0:
                        objname = 'gpu'
                        obj[objname] = []
                    self.line += 1
                    #print('Start of: ' + objname)
                    if objname in obj:
                        item = obj[objname]
                        if not isinstance(item, list):
                            obj[objname] = []
                            obj[objname].append(item)
                        item = dict()
                        obj[objname].append(item)
                        self.getLineObj(item)
                    else:
                        obj[objname] = dict()
                        self.getLineObj(obj[objname])

            self.line += 1


    def runProcess(self):
        self.process = subprocess.Popen(['nvidia-smi','-q','-x'], stdout=subprocess.PIPE)

def getClr(i_val, i_total, i_min, i_max):
    """Get a value from 0.0 to 1.0
    """
    clr = 100.0 * i_val / i_total
    clr = clr - i_min
    if clr < 0.0: clr = 0.0
    clr = clr / (i_max - i_min)
    if clr > 1.0: clr = 1.0
    return clr
