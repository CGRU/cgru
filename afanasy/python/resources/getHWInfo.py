#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import re
import os
import subprocess
import sys
import traceback

'''
AMD Ryzen Threadripper 1950X 16-Core Processor
AMD Ryzen Threadripper 2970WX 24-Core Processor
Intel(R) Core(TM) i9-9900KF CPU @ 3.60GHz
Intel(R) Core(TM) i7-4930K CPU @ 3.40GHz
Intel(R) Core(TM) i5-4590 CPU @ 3.30GHz
Intel(R) Xeon(R) CPU E5-2670 0 @ 2.60GHz
Intel(R) Xeon(R) CPU           E5530  @ 2.40GHz
11th Gen Intel(R) Core(TM) i5-11300H @ 3.10GHz
12th Gen Intel(R) Core(TM) i9-12900K
'''

RegExps = [
    [re.compile(r'.*Intel.*Xeon.* (\w+\d+-?\d+\s?v?\d?).*', re.I),r'X:\1'],
    [re.compile(r'.*Intel.*Core.* (i\d+-?\d+\S+).*', re.I),r'I:\1'],
    [re.compile(r'AMD.* (\w*\d+\w*) .*', re.I),r'A:\1'],
    [re.compile(r'AMD (\w*) .*', re.I),r'A:\1']
]

HwInfo = None

def outError(i_msg):
    print('Error getting hardware info: ' + i_msg)

def getCPUInfo_Linux():
    info = ''
    try:
        cpuinfo = open('/proc/cpuinfo')
    except:
        print(traceback.format_exc())
        return info
    for line in cpuinfo.readlines():
        if line.find('model name') == 0:
            info = line.split(':')[1].strip()
            break
    if len(info) == 0:
        return info
    for regexp in RegExps:
        info = re.sub(regexp[0], regexp[1], info)
    return info.strip()

def getHostNameCtl_Linux():
    pref, suff = '', ''
    try:
        proc = subprocess.run(['hostnamectl'], timeout=1, check=True, encoding='utf-8', stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    except:
        print(traceback.format_exc())
        return pref, suff
    for line in proc.stdout.split('\n'):
        words = line.split(':',1)
        if len(words) != 2:
            continue
        if words[0].lower().find('chassis') != -1:
            pref = words[1].strip().upper()[0] + ' '
        if words[0].lower().find('hardware') != -1:
            suff += ' ' + words[1].strip()
        if words[0].lower().find('virtual') != -1:
            suff += ' ' + words[1].strip()
    return pref, suff

def getHWInfo_Linux():
    cpu = getCPUInfo_Linux()
    pref, suff = getHostNameCtl_Linux()
    #print('"%s" "%s" "%s"' % (pref, cpu, suff))
    return '%s%s%s' % (pref, cpu, suff)

def getCPUInfo_Windows():
    info = ''
    try:
        proc = subprocess.run(['wmic','cpu','get','name'], timeout=1, check=True, encoding='utf-8', stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    except:
        print(traceback.format_exc())
        return info
    info = proc.stdout.strip()
    if len(info) == 0:
        return info
    info = info.split("\n")[-1]
    for regexp in RegExps:
        info = re.sub(regexp[0], regexp[1], info)
    return info.strip()

def getModel_Windows():
    pref, suff = '', ''
    try:
        proc = subprocess.run(['wmic','computersystem','get','model,manufacturer,hypervisorpresent','/format:list'], timeout=1, check=True, encoding='utf-8', stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    except:
        print(traceback.format_exc())
        return pref, suff
    model = ''
    virtual = False
    for line in proc.stdout.split('\n'):
        words = line.split('=',1)
        if len(words) != 2:
            continue
        elif words[0].lower().find('model') != -1:
            pref = words[1].strip().upper()[0] + ' '
            model = ' ' + words[1].strip()
        elif words[0].lower().find('manufacturer') != -1:
            suff += ' ' + words[1].strip()
        elif words[0].lower().find('hypervisorpresent') != -1:
            if words[1].lower().find('true') != -1:
                virtual = True
    if virtual:
        pref = 'V '
    return pref, model + suff

def getHWInfo_Windows():
    cpu = getCPUInfo_Windows()
    pref, suff = getModel_Windows()
    #print('"%s" "%s" "%s"' % (pref, cpu, suff))
    return '%s%s%s' % (pref, cpu, suff)

def getHWInfo():
    global HwInfo

    if HwInfo is None:
        if sys.platform.find('linux') == 0:
            HwInfo = getHWInfo_Linux()
        elif sys.platform.find('win') == 0:
            HwInfo = getHWInfo_Windows()
        else:
            HwInfo = ''

    return HwInfo

if __name__ == '__main__':

    print(getHWInfo())

