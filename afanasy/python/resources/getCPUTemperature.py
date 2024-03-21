#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json
import subprocess
import sys
import traceback

CallCounter = 0

def outError(i_msg):
    if CallCounter == 1:
        print('Error getting CPU temperature: ' + i_msg)

def getCPUTemperatureLinux():
    try:
        proc = subprocess.run(['sensors','-j'], timeout=1, check=True, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    except:
        outError(traceback.format_exc())
        return 0
    text = proc.stdout
    try:
        obj = json.loads(text)
    except:
        outError(traceback.format_exc())
        return 0
    if type(obj) is not dict:
        outError('Root is not an object.')
        return 0
    temperatures = []
    for dev in obj:
        dev = obj[dev]
        if type(dev) is not dict:
            continue
        for part in dev:
            part = dev[part]
            if type(part) is not dict:
                continue
            for parm in part:
                if parm.find('temp') == -1:
                    continue
                if parm.find('input') == -1:
                    continue
                t = part[parm]
                if type(t) is not float:
                    continue
                temperatures.append(int(t))
    if len(temperatures) == 0:
        outError('No valid sensors found.')
        return 0
    temperatures.sort()
    return temperatures[-1]

def getCPUTemperatureWindows():
    return 40

def getCPUTemperature():
    global CallCounter
    CallCounter += 1
    if sys.platform.find('linux') == 0:
        return getCPUTemperatureLinux()
    if sys.platform.find('win') == 0:
        return getCPUTemperatureWindows()
    return 0

if __name__ == '__main__':

    print(getCPUTemperature())

