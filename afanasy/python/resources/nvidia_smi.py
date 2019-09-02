# -*- coding: utf-8 -*-

import json
import subprocess
import re
import traceback

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
        """ This function is launched periodically from a base class:
        """
        self.labelr = 100
        self.labelg = 200
        self.labelb = 0

        if self.process:
             self.readProcessOuput()

        self.runProcess()


    def readProcessOuput(self):
        """ Read process stdout
        """
        self.data = None
        obj = dict()

        # Communicate process
        try:
            self.data, err = self.process.communicate()
            if err and len(err):
                print(err)
        except:
            self.setError('Can`t communicate process.', traceback.format_exc())
            return

        # Validate and prepare output data    
        self.data = cgruutils.toStr(self.data)
        self.data = self.data.splitlines()
        self.line = 0
        found = False
        while self.line < len(self.data):
            if self.data[self.line].count('<nvidia_smi_log>'):
                found = True
                self.line += 1
                break
            self.line += 1
        if not found:
            self.setError('Ouput does not contain <nvidia_smi_log>.', '\n'.join(self.data))
            return

        # Recursive function to parse XML data and construct dict object:
        self.getLineObj(obj)
        #print(json.dumps(obj, sort_keys=True, indent=4))

        # Process dict object
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
            label += ' %.0fG (%dM Used / %dM Free)' % (float(total)/1000.0,used,free)
            tip += ' Memory: Total %dM, Used %dM, Free %dM' % (total,used,free)
            mem_total += total
            mem_used  += used
            mem_free  += free

            # Temperature (current and max):
            tpr = gpu['temperature']
            gpu_tpr = int(tpr['gpu_temp'].split(' ')[0])
            gpu_tpr_max = int(tpr['gpu_temp_slow_threshold'].split(' ')[0])
            label += ' %dC(%dC Max)' % (gpu_tpr, gpu_tpr_max)
            tip += '; Temperature: GPU %dC, Max %dC' % (gpu_tpr, gpu_tpr_max)
            if tpr_val < gpu_tpr: tpr_val = gpu_tpr
            if tpr_max > gpu_tpr_max: tpr_max = gpu_tpr_max

            # Utilization:
            util = gpu['utilization']
            util_gpu = int(util['gpu_util'].split(' ')[0])
            util_mem = int(util['memory_util'].split(' ')[0])
            util_enc = int(util['encoder_util'].split(' ')[0])
            util_dec = int(util['decoder_util'].split(' ')[0])
            if util_gpu: label += ' G%d%%' % util_gpu
            if util_mem: label += ' M%d%%' % util_mem
            if util_enc: label += ' E%d%%' % util_enc
            if util_dec: label += ' D%d%%' % util_dec
            tip += '; Utilization: GPU:%d%% MEM:%d%% Encoder:%d%% Decoder:%d%%' % (util_gpu, util_mem, util_enc, util_dec)

            # Collect processes and progs (processes with the same name)
            if not 'process_info' in gpu['processes']: continue
            self.labelg = 255
            processes = []
            progs = {}

            for prc in gpu['processes']['process_info']:
                iterate = False
                try:
                    if isinstance(prc, dict):
                        name = prc['process_name']
                        mem = int(prc['used_memory'].split(' ')[0])
                        iterate = True
                    else:
                        name = gpu['processes']['process_info']['process_name']
                        mem = int(gpu['processes']['process_info']['used_memory'].split(' ')[0])
                except:
                    print(str(gpu['processes']['process_info']))
                    print(traceback.format_exc())
                    name = str(prc)
                    mem = 0
                    self.labelr = 255
                    self.labelg = 0
                    self.labelb = 0

                # append processes:
                pc = dict()
                pc['name'] = name
                pc['mem'] = mem
                processes.append(pc)
                # Cut command arguments:
                name = name.strip().split(' ')[0]
                # Use base name for progs:
                name = name.split('/')[-1].split('\\')[-1]
                if name in progs:
                    progs[name] += mem
                else:
                    progs[name] = mem

                if not iterate:
                    break

            # constuct label string (sorted by mem):
            label += '\n'
            for name, mem in sorted(progs.items(), key=lambda kv: kv[1], reverse=True):
                label += ' ' + name
                label += ':%dM' % mem

            # constuct tip strings (sorted by mem):
            processes.sort(key=lambda k: k['mem'], reverse=True)
            for prc in processes:
                tip += '\n' + prc['name']
                tip += ': %d MiB' % prc['mem']


        self.label = 'v' + obj['driver_version'] + ' ' + label
        self.tooltip = 'NVIDIA Driver Verion: ' + obj['driver_version'] + '\n' + tip

        if mem_used:
            self.labelr = 255

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


    def getLineObj(self, obj):
        """ Parse XML line.
            Function assumes that each object is in a new line.
            This significally simplifies parsing.
        """
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
        try:
            self.process = subprocess.Popen(['nvidia-smi','-q','-x'], stdout=subprocess.PIPE)
            #self.process = subprocess.Popen(['bash','-c','nvidia-smi -q -x'], stdout=subprocess.PIPE)
        except:
            self.process = None
            self.setError('Failed to launch nvidia-smi', traceback.format_exc())
            return


    def setError(self, i_msg, i_tip = None):
        """ Set error label and tooltip
        """
        self.label = 'ERROR: ' + i_msg
        self.labelr = 255
        self.labelg = 0
        self.labelb = 0
        if i_tip is None:
            i_tip = self.label
        self.tooltip = i_tip
        print(self.tooltip)


def getClr(i_val, i_total, i_min, i_max):
    """ Get a value from 0.0 to 1.0 from inputs
    """
    clr = 100.0 * i_val / i_total
    clr = clr - i_min
    if clr < 0.0: clr = 0.0
    clr = clr / (i_max - i_min)
    if clr > 1.0: clr = 1.0
    return clr
