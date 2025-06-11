/* ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' *\
 *        .NN.        _____ _____ _____  _    _                 This file is part of CGRU
 *        hMMh       / ____/ ____|  __ \| |  | |       - The Free And Open Source CG Tools Pack.
 *       sMMMMs     | |   | |  __| |__) | |  | |  CGRU is licensed under the terms of LGPLv3, see files
 * <yMMMMMMMMMMMMMMy> |   | | |_ |  _  /| |  | |    COPYING and COPYING.lesser inside of this folder.
 *   `+mMMMMMMMMNo` | |___| |__| | | \ \| |__| |          Project-Homepage: http://cgru.info
 *     :MMMMMMMM:    \_____\_____|_|  \_\\____/        Sourcecode: https://github.com/CGRU/cgru
 *     dMMMdmMMMd     A   F   A   N   A   S   Y
 *    -Mmo.  -omM:                                           Copyright © by The CGRU team
 *    '          '
\* ....................................................................................................... */

/*
	monitors.js - methods and structs for monitoring and handling of running monitor instances
	These are the monitor sessions with username, ip, ...
*/

'use strict';

function MonitorNode() {}

MonitorNode.prototype.init = function() {
	this.element.classList.add('mnode');

	cm_CreateStart(this);

	this.elName = document.createElement('span');
	this.elName.classList.add('name');
	this.element.appendChild(this.elName);
	this.elName.title = 'Name';
	this.elName.classList.add('prestar');

	this.elHostName = cm_ElCreateFloatText(this.element, 'right', 'Host Name');
	this.elHostName.classList.add('name');

	this.elCenter = document.createElement('div');
	this.element.appendChild(this.elCenter);
	this.elCenter.style.position = 'absolute';
	this.elCenter.style.left = '0';
	this.elCenter.style.right = '0';
	this.elCenter.style.top = '1px';
	this.elCenter.style.textAlign = 'center';
	this.elCenter.classList.add('prestar');

	this.elUid = cm_ElCreateText(this.elCenter, 'User ID');
	this.elJobsIds = cm_ElCreateText(this.elCenter, 'Jobs IDs');

	//	this.element.appendChild( document.createElement('br'));

	this.elIP = cm_ElCreateFloatText(this.element, 'right');

	this.element.appendChild(document.createElement('br'));

	this.elEvents = document.createElement('div');
	this.element.appendChild(this.elEvents);

	this.elAnnotation = document.createElement('div');
	this.element.appendChild(this.elAnnotation);
	this.elAnnotation.title = 'Annotation';
	this.elAnnotation.style.textAlign = 'center';
	this.elAnnotation.classList.add('prestar');
};

MonitorNode.prototype.update = function(i_obj) {
	if (i_obj)
		this.params = i_obj;

	this.elUid.innerHTML = 'UID:<b>' + this.params.uid + '</b>';

	this.elName.innerHTML = '<b>' + this.params.name + '</b>';

	if (this.params.host_name)
		this.elHostName.innerHTML = '<b>' + this.params.host_name + '</b>';
	else
		this.elHostName.textContent = '';

	var jobs_ids = '';
	if (this.params.jobs_ids && this.params.jobs_ids.length)
	{
		for (var i = 0; i < this.params.jobs_ids.length; i++)
		{
			if (i)
				jobs_ids += ',';
			jobs_ids += ' <b>' + this.params.jobs_ids[i] + '</b>';
		}
		jobs_ids = 'JIDs:[' + jobs_ids + ']';
	}
	this.elJobsIds.innerHTML = jobs_ids;

	this.elIP.innerHTML = 'IP=<b>' + this.params.address.ip + '</b>';

	var events = '';
	if (this.params.events && this.params.events.length)
	{
		for (var i = 0; i < this.params.events.length; i++)
		{
			if (i)
				events += ',';
			events += ' <b>' + this.params.events[i] + '</b>';
		}
		events = ' EVTs: ' + events;
	}
	this.elEvents.innerHTML = events;

	if (this.params.annotation)
		this.elAnnotation.textContent = this.params.annotation;
	else
		this.elAnnotation.textContent = '';

	this.refresh();
};

MonitorNode.prototype.refresh = function() {};

MonitorNode.prototype.onDoubleClick = function(e) {
	g_ShowObject({'object': this.params}, {'evt': e, 'wnd': this.monitor.window});
};

MonitorNode.prototype.updatePanels = function() {
	// Info:
	var info = '';

	info += '<p>User name: ' + this.params.user_name + '</p>';

	if (this.params.address)
	{
		info += '<p>IP: ' + this.params.address.ip;
		if (this.params.address.port)
			info += ' Port: ' + this.params.address.port;
		info += '</p>';
	}

	info += '<p>Launched at: ' + cm_DateTimeStrFromSec(this.params.time_launch) + '</p>';
	info += '<p>Registered at: ' + cm_DateTimeStrFromSec(this.params.time_register) + '</p>';
	info += '<p>Last activity at: ' + cm_DateTimeStrFromSec(this.params.time_activity) + '</p>';

	info += '<p>ID = ' + this.params.id +
		'</p>'

		this.monitor.setPanelInfo(info);
};


MonitorNode.sort = ['name'];
MonitorNode.filter = ['name'];
