#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import re
import os
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
'''

RegExps = [
    [re.compile(r'AMD.* (\w*\d+\w*) .*', re.I),r'A-\1'],
    [re.compile(r'.*Intel.*Core.* (i\d+\S\d+\S+) .*', re.I),r'\1']
]

HwInfo = None

def outError(i_msg):
    print('Error getting hardware info: ' + i_msg)

def subInfo(i_info):
    if len(i_info) == 0:
        return i_info
    for regexp in RegExps:
        i_info = re.sub(regexp[0], regexp[1], i_info)
    return i_info


def getHWInfoLinux():
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
    return subInfo(info)

def getHWInfoWindows():
    return ''

def getHWInfo():
    global HwInfo

    if HwInfo is None:
        if sys.platform.find('linux') == 0:
            HwInfo = getHWInfoLinux()
        elif sys.platform.find('win') == 0:
            HwInfo = getHWInfoWindows()
        else:
            HwInfo = ''

    return HwInfo

if __name__ == '__main__':

    print(getHWInfo())

