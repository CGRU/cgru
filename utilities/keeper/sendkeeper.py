# -*- coding: utf-8 -*-

import json
import socket
import sys

import cgruconfig

def sendkeeper(cmdexec, port=None, host=None):
    
    if port is None: port = cgruconfig.VARS['keeper_port']
    if host is None: host = 'localhost'

    obj = dict()

    obj['headers'] = dict()
    obj['headers']['username'] = cgruconfig.VARS['USERNAME']
    obj['headers']['hostname'] = cgruconfig.VARS['HOSTNAME']

    obj['cmdexec'] = cmdexec
    if len(obj['cmdexec']) == 0:
        print('ERROR: No commands to send, cmdexec is empty.')
        return False

    data = json.dumps(obj, sort_keys=True, indent=4)

    s = None
    for res in socket.getaddrinfo(host, port, socket.AF_UNSPEC, socket.SOCK_STREAM):
        family, socktype, proto, canonname, sockaddr = res
        print('Trying to connect to "%s"' % str(sockaddr[0]))

        try:
            s = socket.socket(family, socktype, proto)
        except Exception as e:
            print(str(e))
            s = None
            continue

        try:
            s.connect(sockaddr)
        except Exception as e:
            print(str(e))
            s.close()
            s = None
            continue
        break

    if s is None:
        print('Could not open socket.')
        return False
    
    print('Sending:')
    print(data)
    s.sendall(bytearray(data, 'utf-8'))

    return True
