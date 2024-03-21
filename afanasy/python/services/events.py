# -*- coding: utf-8 -*-

import json
import sys

import cgruutils
import cgruconfig

from services import service


# TODO: Class names should follow CamelCase naming convention
class events(service.service):
    """Events Trigger
    """

    def __init__(self, task_info, i_verbose):
        service.service.__init__(self, task_info, i_verbose)
        data = self.taskInfo['command']
        self.taskInfo['command'] = ''
        self.skip_task = True

        # print('Event data:\n%s' % data)

        try:
            if not isinstance(data, str):
                data = str(data, 'utf-8')
            objects = json.loads(data)
        except:  # TODO: Too broad exception clause
            error = str(sys.exc_info()[1])
            print(error)
            print('Event data:\n%s' % data)
            objects = None

        if objects is None:
            return

        # Check received events:
        if 'events' not in objects:
            print('ERROR: Received data does not contain events.')
            print('Event data:\n%s' % data)
            return
        if not isinstance(objects['events'], list):
            print('ERROR: Received events is not a list.')
            print('Event data:\n%s' % data)
            return
        if len(objects['events']) == 0:
            print('ERROR: Received events list is empty.')
            print('Event data:\n%s' % data)
            return

        # Get and combine custom data objects:
        custom_obj = dict()
        for key in objects:
            self.combineCustomObj(custom_obj, objects[key])

        if len(custom_obj) == 0:
            # print('No configured data found.')
            return

        if 'events' not in custom_obj:
            # print('No configured events found.')
            return

        email_events = []

        # Iterate all interested events:
        for event in custom_obj['events']:

            if event not in objects['events']:
                # print('Skipping not received event "%s"' % event)
                continue

            event_obj = custom_obj['events'][event]

            # Event should be a dictionary:
            if not isinstance(event_obj, dict):
                print('ERROR: Configured event["%s"] is not an object.' % event)
                print('Event data:\n%s' % data)
                return

            if 'methods' not in event_obj:
                print('ERROR: Configured event["%s"] does not have methods.' % event)
                print('Event data:\n%s' % data)
                continue

            methods = event_obj['methods']
            if not isinstance(methods, list):
                print('ERROR: Configured event["%s"] methods is not an array.' % event)
                print('Event data:\n%s' % data)
                continue

            if 'email' in methods and 'emails' in custom_obj and len(custom_obj['emails']):
                email_events.append(event)

            # Essentially for debugging
            if 'notify-send' in methods:
                self.taskInfo['command'] = "notify-send Afanasy 'Job " + task_info['job_name'].replace("'", "'\\''") + ": " + event + "'"

            # execute any command and give job ID of the source job
            for m in methods:
                if m not in ["email", "notify-send"]:
                    self.taskInfo["command"] = "%s -jobid %s" % (m, objects['job']['id'])


        if len(email_events):
            cmd = cgruconfig.VARS['email_send_cmd']
            cmd += ' -V'  # Verbose mode
            cmd += ' -f "noreply@%s"' % cgruconfig.VARS['email_sender_address_host']
            for addr in custom_obj['emails']:
                cmd += ' -t "%s"' % addr
            _fmt_events = ', '.join(email_events).replace("_", " ").title()
            cmd += ' -s "%s - %s"' % (_fmt_events, cgruutils.toStr(task_info['job_name'])) # e.g.: Job Error - MY_JOB_v01
            cmd += ' "<p>Events: <b>%s</b></p>"' % (_fmt_events)
            if 'render' in objects:
                cmd += ' "<p>Render Name: <b>%s</b>' % objects['render']['name']
                if 'host_resources' in objects:
                    hres = objects['host_resources']
                    cmd += '<ul>'
                    cmd += '<li>CPU: %(cpu_mhz)dMHz x%(cpu_num)d / idle = %(cpu_idle)d%%</li>' % hres
                    cmd += '<li>MEM: %dGB / free = %dGB</li>' % (hres['mem_total_mb']/1024, hres['mem_free_mb']/1024)
                    cmd += '<li>SWP: %dGB / free = %dGB</li>' % (hres['swap_total_mb']/1024, (hres['swap_total_mb']-hres['swap_used_mb'])/1024)
                    cmd += '<li>HDD: %(hdd_total_gb)dGB / free = %(hdd_free_gb)dGB / busy = %(hdd_busy)d%%</li>' % hres
                    cmd += '</ul>'
                cmd += '</p>"'
            cmd += ' "<p>Job Name: <b>%s</b></p>"' % cgruutils.toStr(task_info['job_name'])
            cmd += ' "<p>User Name: <b>%s</b></p>"' % cgruutils.toStr(task_info['user_name'])
            self.taskInfo['command'] = cmd

        if len(self.taskInfo['command']):
            self.skip_task = False


    def combineCustomObj(self, o_output_obj, i_input_obj):

        if isinstance(i_input_obj, list):
            for obj in i_input_obj:
                self.combineCustomObj(o_output_obj, obj)
            return

        if not isinstance(i_input_obj, dict):
            return

        if not 'custom_data' in i_input_obj:
            # Object does not contain any custom_data, nothing to combine
            return

        custom_obj = None

        try:
            custom_obj = json.loads(i_input_obj['custom_data'])
        except:  # TODO: too broad exception clause
            print('JSON error in custom data:')
            print(i_input_obj['custom_data'])
            print(sys.exc_info()[1])
            return

        if not isinstance(custom_obj, dict):
            print('ERROR: "%s" custom data is not an object:')
            print(custom_obj)
            return

        self.updateObj(o_output_obj, custom_obj)


    def updateObj(self, o_obj, i_obj):

        for key in i_obj:
            if len(key) == 0:
                continue

            if key[0] == '-':
                continue

            if isinstance(i_obj[key], dict):
                if key in o_obj:
                    if isinstance(o_obj[key], dict):
                        self.updateObj(o_obj[key], i_obj[key])
                        continue

            if isinstance(i_obj[key], list):
                if key in o_obj:
                    if isinstance(o_obj[key], list):
                        o_obj[key].extend(i_obj[key])
                        continue

            o_obj[key] = i_obj[key]
