# !/usr/bin/env python
# -*- coding: utf-8 -*-

import json
import os
import socket
import struct
import sys
import threading

import cgruconfig

import afnetwork

import simtracker

from optparse import OptionParser

Parser = OptionParser(usage="usage: %prog [Options]", version="%prog 1.0")
Parser.add_option('-P', '--port',      dest='port',      type='int',  default=0,    help='Tracker port.')
Parser.add_option('-W', '--webport',   dest='webport',   type='int',  default=0,    help='Web port.')
Parser.add_option('-J', '--jobid',     dest='jobid',     type='int',  default=None, help='Afanasy job ID.')
Parser.add_option(      '--envblocks', dest='envblocks', type='string', default=None, help='Afanasy job blocks names mask.')
Parser.add_option(      '--depblocks', dest='depblocks', type='string', default=None, help='Afanasy job blocks names mask.')
Parser.add_option(      '--start',     dest='start',     action='store_true', default=False, help='Start tracker.')
Parser.add_option(      '--stop',      dest='stop',      action='store_true', default=False, help='Stop tracker.')
Parser.add_option('-A', '--address',   dest='address',   type='string', default='localhost', help='Tracker address to stop.')

Options, Args = Parser.parse_args()

if Options.start == Options.stop:
    Parser.error('You should choose to start or to stop a tracker.')


def sendJob( i_port):

    # Prepare action object.
    act = dict()
    act['user_name'] = cgruconfig.VARS['USERNAME']
    act['host_name'] = cgruconfig.VARS['HOSTNAME']
    act['type'] = 'jobs'
    act['ids'] = [Options.jobid]

    # Set environment for env-blocks.
    act['block_mask'] = Options.envblocks
    par = dict()
    par['environment'] = dict()
    par['environment']['TRACKER_ADDRESS'] = cgruconfig.VARS['HOSTNAME']
    par['environment']['TRACKER_PORT'] = str(i_port)
    act['params'] = par
    data = json.dumps({'action':act}, sort_keys=True, indent=4)
    print( data)
    res = afnetwork.sendServer( data)
    print( res)

    # Reset depend mask for dep-blocks.
    act['block_mask'] = Options.depblocks
    par = dict()
    par['depend_mask'] = ''
    act['params'] = par
    data = json.dumps({'action':act}, sort_keys=True, indent=4)
    print( data)
    res = afnetwork.sendServer( data)
    print( res)

    # Flush printed text to stdout out.
    sys.stdout.flush()


def startTracker():
    """Starts the tracker process.
    """
    # First spawn the tracker, turning off verbosity.
    simtracker.setVerbosity(False)
    tracker_thread = threading.Thread(target=lambda: simtracker.serve( Options.port, Options.webport))
    tracker_thread.start()

    # Wait for the tracker to be ready and get the port.
    simtracker.waitForListener()
    tracker_port = simtracker.getListenPort()
    web_port = simtracker.getWebPort()

    # Output some useful information.
    print("Tracker listening on port %d" % tracker_port)
    print("To view simulation statistics, go to http://%s:%d" % (cgruconfig.VARS['HOSTNAME'], web_port))
    # And output it for parser to set job info:
    info = 'Tracker %s:%d WEB:%d' % ( cgruconfig.VARS['HOSTNAME'], tracker_port, web_port)
    print('REPORT: ' + info)
    print('ACTIVITY: ' + info)

    # Flush printed text to stdout out.
    sys.stdout.flush()

    if Options.jobid:
        sendJob( tracker_port)

    # Wait for the tracker to finish.
    simtracker.waitForCompletion()
    print('Tracker finish.')
    sys.stdout.flush()


def stopTracker():
    """Stops the tracker process.
    """
    # Get tracker address and port parameters:
    address = os.getenv('TRACKER_ADDRESS', Options.address)
    port = int( os.getenv('TRACKER_PORT', Options.port))

    # Connect to the tracker.
    print('Trying to stop tracker at %s:%d' % ( address, port))
    s = socket.socket( socket.AF_INET, socket.SOCK_STREAM)
    s.connect(( address, port))

    # Send the quit message.
    msg = "quit"
    msg_len = struct.pack("!L", len(msg))
    msg = msg_len + msg
    s.send(msg)

    # Read the ack from tracker and send back an empty message to indicate
    # success.
    s.recv(1)
    s.send('')

    s.close()

    # And output it for parser to set job info:
    info = 'Tracker stopped: %s:%d' % ( address, port)
    print('REPORT: ' + info)
    print('ACTIVITY: ' + info)


if Options.start:
    startTracker()

if Options.stop:
    stopTracker()

