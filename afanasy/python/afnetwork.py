# -*- coding: utf-8 -*-

import json
import socket
import sys
import time

try:
    from concurrent.futures import ThreadPoolExecutor, TimeoutError as FuturesTimeout
except ImportError:
    ThreadPoolExecutor = None
    FuturesTimeout = Exception

import cgruconfig
import cgruutils

# Failed DNS / connect cache: host -> monotonic deadline
_fail_until = {}
_FAIL_TTL_SEC = 60
_DNS_TIMEOUT_SEC = 1.0
_CONNECT_TIMEOUT_SEC = 2.0

# Persistent pool: do NOT use "with ThreadPoolExecutor()" — exiting the
# context waits for getaddrinfo and re-blocks the caller for the full DNS delay.
_dns_pool = ThreadPoolExecutor(max_workers=2) if ThreadPoolExecutor else None


def genHeader(i_data_size):
    """Missing DocString

    :param i_data_size:
    :return:
    """
    data = 'AFANASY %s JSON' % i_data_size
    return bytearray(data, 'utf-8')


def _host_is_cached_fail(host):
    until = _fail_until.get(host)
    if until is None:
        return False
    if time.monotonic() >= until:
        _fail_until.pop(host, None)
        return False
    return True


def _mark_host_fail(host):
    _fail_until[host] = time.monotonic() + _FAIL_TTL_SEC


def _resolve(host, port):
    """getaddrinfo with a short timeout so UI threads do not freeze on bad DNS."""
    if _dns_pool is None:
        return socket.getaddrinfo(host, port, socket.AF_UNSPEC, socket.SOCK_STREAM)

    fut = _dns_pool.submit(
        socket.getaddrinfo, host, port, socket.AF_UNSPEC, socket.SOCK_STREAM
    )
    return fut.result(timeout=_DNS_TIMEOUT_SEC)


def sendServer(i_data, i_verbose=False, i_without_answer=False):
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

    if _host_is_cached_fail(host):
        if i_verbose:
            print('Skipping "%s" (recent connect/DNS failure cached)' % host)
        return False, None

    s = None
    err_msg = ''
    reslist = []

    try:
        reslist = _resolve(host, port)
    except FuturesTimeout:
        print('Can`t solve "%s": DNS timeout (%.1fs)' % (host, _DNS_TIMEOUT_SEC))
        _mark_host_fail(host)
        return False, None
    except:  # TODO: Too broad exception clause
        print('Can`t solve "%s":' % host + str(sys.exc_info()[1]))
        _mark_host_fail(host)
        return False, None

    for res in reslist:
        af, socktype, proto, canonname, sa = res
        if i_verbose:
            print('Trying to connect to "%s:%s"' % (str(sa[0]), str(port)))
        try:
            s = socket.socket(af, socktype, proto)
            s.settimeout(_CONNECT_TIMEOUT_SEC)
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
        _mark_host_fail(host)
        return False, None

    if i_verbose:
        print('afnetwork.sendServer: send %d bytes' % datalen)

    # s.sendall( i_data) #<<< !!! May not work !!!!

    try:
        total_send = 0
        while total_send < len(i_data):
            sent = s.send(i_data[total_send:])
            if sent == 0:
                disconnectSocket(s)
                print('Error: Unable send data to socket')
                _mark_host_fail(host)
                return False, None
            total_send += sent

        data = b''
        msg_len = None
        while True:
            data_buffer = s.recv(4096)

            if not data_buffer:
                break

            data += data_buffer

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
    except socket.timeout:
        disconnectSocket(s)
        print('Error: Socket timeout talking to "%s"' % host)
        _mark_host_fail(host)
        return False, None

    disconnectSocket(s)

    struct = None

    if i_without_answer is True:
        return True, struct

    try:
        if not isinstance(data, str):
            data = cgruutils.toStr(data)
        if msg_len is not None:
            data = data[data.find('JSON') + 4:msg_len]
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
