#!/usr/bin/env python
# -*- coding: utf-8 -*-

import json
import os
import socket
import sys

import cgruconfig
import cgruutils


def genHeader(i_data_size):
    """Missing DocString

    :param i_data_size:
    :return:
    """
    data = 'AFANASY %s JSON' % i_data_size
    return bytearray(data, 'utf-8')


def sendServer(i_data, i_verbose=False):
    """Missing DocString

    :param i_verbose:
    :return:
    """
    size = len(i_data)
    header = genHeader(size)
    i_data = header + bytearray(i_data, 'utf-8')
    datalen = len(i_data)
    # return True, None

    host = cgruconfig.VARS['af_servername']
    port = cgruconfig.VARS['af_serverport']

    s = None
    err_msg = ''
    reslist = []

    try:
        reslist = socket.getaddrinfo(host, port, socket.AF_UNSPEC,
                                     socket.SOCK_STREAM)
    except:  # TODO: Too broad exception clause
        print('Can`t solve "%s":' % host + str(sys.exc_info()[1]))

    for res in reslist:
        af, socktype, proto, canonname, sa = res
        if i_verbose:
            print('Trying to connect to "%s"' % str(sa[0]))
        try:
            s = socket.socket(af, socktype, proto)
        except:  # TODO: Too broad exception clause
            if err_msg != '':
                err_msg += '\n'
            err_msg += str(sa[0]) + ' : ' + str(sys.exc_info()[1])
            s = None
            continue
        try:
            s.connect(sa)
        except:  # TODO: Too broad exception clause
            if err_msg != '':
                err_msg += '\n'
            err_msg += str(sa[0]) + ' : ' + str(sys.exc_info()[1])
            s.close()
            s = None
            continue
        break

    if s is None:
        print('Could not open socket.')
        print(err_msg)
        return False, None

    if i_verbose:
        print('afnetwork.sendServer: send %d bytes' % datalen)

    # s.sendall( i_data) #<<< !!! May not work !!!!

    total_send = 0
    while total_send < len(i_data):
        sent = s.send(i_data[total_send:])
        if sent == 0:
            disconnectSocket(s)
            print('Error: Unable send data to socket')
            return False, None
        total_send += sent

    data = b''
    msg_len = None
    while True:
        buffer = s.recv(4096)

        if not buffer:
            break

        data += buffer

        if msg_len is None:
            dataStr = cgruutils.toStr(data)
            if dataStr.find('AFANASY') != -1 and dataStr.find('JSON') != -1:
                msg_len = dataStr[:dataStr.find('JSON') + 4]
                msg_len = len(msg_len) + int(msg_len.split(' ')[1])

        if i_verbose:
            print('Received %d of %d bytes.' % (len(data), msg_len))

        if msg_len is not None:
            if len(data) >= msg_len:
                break

    disconnectSocket(s)

    struct = None

    try:
        if not isinstance(data, str):
            data = cgruutils.toStr(data)
        data = data[data.find('JSON') + 4:]
        struct = json.loads(data, strict=False)
    except:  # TODO: Too broad exception clause
        print('afnetwork.py: Received data:')
        print(data)
        print('JSON loads error:')
        print(str(sys.exc_info()[1]))
        struct = None

    return True, struct


def disconnectSocket(i_sd):

    i_sd.close()
