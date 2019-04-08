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
	users.js - methods and structs for monitoring and handling of users
*/

"use strict";

function UserNode()
{
}

UserNode.onMonitorCreate = function() {
	UserNode.createParams();
};

UserNode.prototype.init = function() {
	this.element.classList.add('user');

	cm_CreateStart(this);

	this.elName = document.createElement('span');
	this.elName.classList.add('name');
	this.element.appendChild(this.elName);
	this.elName.title = 'User Name';
	this.elName.classList.add('prestar');

	this.elWork = cm_ElCreateFloatText(this.element, 'right');
	this.elErrors = cm_ElCreateFloatText(this.element, 'right');
	this.elForgive = cm_ElCreateFloatText(this.element, 'right', 'Errors Forgive Time');
	this.elJobsLifeTime = cm_ElCreateFloatText(this.element, 'right', 'Jobs Life Time');

	this.element.appendChild(document.createElement('br'));

	this.elJobs = cm_ElCreateFloatText(this.element, 'left', 'Jobs: All/Running');
	this.elJobs.classList.add('prestar');

	this.elHostName = cm_ElCreateFloatText(this.element, 'right', 'Host Name');

	this.element.appendChild(document.createElement('br'));

	this.elAnnotation = document.createElement('div');
	this.element.appendChild(this.elAnnotation);
	this.elAnnotation.title = 'Annotation';
	this.elAnnotation.classList.add('annotation');
	this.elAnnotation.classList.add('prestar');

	this.elBarParent = document.createElement('div');
	this.element.appendChild(this.elBarParent);
	this.elBarParent.style.position = 'absolute';
	this.elBarParent.style.left = '220px';
	this.elBarParent.style.right = '220px';
	this.elBarParent.style.top = '18px';
	this.elBarParent.style.height = '12px';

	this.elBar = document.createElement('div');
	this.elBarParent.appendChild(this.elBar);
	this.elBar.classList.add('bar');
};

UserNode.prototype.update = function(i_obj) {
	if (i_obj)
		this.params = i_obj;

	this.elName.innerHTML = '<b>' + this.params.name + '</b>';

	if (this.params.running_tasks_num)
	{
		this.elStar.style.display = 'block';
		this.elStarCount.textContent = this.params.running_tasks_num;
	}
	else
		this.elStar.style.display = 'none';

	this.elWork.innerHTML = work_generateParamsString(this.params, 'user');

	if (cm_IsPadawan())
	{
		if (this.params.host_name)
			this.elHostName.innerHTML =
				'<small>Latest Activity Host:</small><b>' + this.params.host_name + '</b>';
		else
			this.elHostName.textContent = '';

		if (this.params.errors_forgive_time)
			this.elForgive.innerHTML =
				'ErrorsForgiveTime:<b>' + cm_TimeStringFromSeconds(this.params.errors_forgive_time) + '</b>';
		else
			this.elForgive.textContent = '';

		if (this.params.jobs_life_time)
			this.elJobsLifeTime.innerHTML =
				'JobsLifeTime:<b>' + cm_TimeStringFromSeconds(this.params.jobs_life_time) + '</b>';
		else
			this.elJobsLifeTime.textContent = '';

		var errstr = 'ErrorsSolving(';
		errstr += ' Avoid:<b>' + this.params.errors_avoid_host + '</b>';
		errstr += ' Task:<b>' + this.params.errors_task_same_host + '</b>';
		errstr += ' Retries:<b>' + this.params.errors_retries + '</b>';
		errstr += ')';
		this.elErrors.innerHTML = errstr;

		var jobs = 'Jobs Total:';
		if (this.params.jobs_num)
			jobs += ' <b>' + this.params.jobs_num + '</b>';
		else
			jobs += ' <b>0</b>';
		if (this.params.running_jobs_num)
			jobs += ' / <b>' + this.params.running_jobs_num + '</b> Running';
		this.elJobs.innerHTML = jobs;
	}
	else if (cm_IsJedi())
	{
		if (this.params.host_name)
			this.elHostName.innerHTML = '<b>' + this.params.host_name + '</b>';
		else
			this.elHostName.textContent = '';

		if (this.params.errors_forgive_time)
			this.elForgive.innerHTML =
				'ErrForgive:<b>' + cm_TimeStringFromSeconds(this.params.errors_forgive_time) + '</b>';
		else
			this.elForgive.textContent = '';

		if (this.params.jobs_life_time)
			this.elJobsLifeTime.innerHTML =
				'JobsLife:<b>' + cm_TimeStringFromSeconds(this.params.jobs_life_time) + '</b>';
		else
			this.elJobsLifeTime.textContent = '';

		var errstr = 'ErrSlv(';
		errstr += 'A:<b>' + this.params.errors_avoid_host + '</b>';
		errstr += ',T:<b>' + this.params.errors_task_same_host + '</b>';
		errstr += ',R:<b>' + this.params.errors_retries + '</b>';
		errstr += ')';
		this.elErrors.innerHTML = errstr;

		var jobs = 'Jobs:';
		if (this.params.jobs_num)
			jobs += '<b>' + this.params.jobs_num + '</b>';
		else
			jobs += '<b>0</b>';
		if (this.params.running_jobs_num)
			jobs += ' / <b>' + this.params.running_jobs_num + '</b>Run';
		this.elJobs.innerHTML = jobs;
	}
	else
	{
		if (this.params.host_name)
			this.elHostName.innerHTML = '<b>' + this.params.host_name + '</b>';
		else
			this.elHostName.textContent = '';

		if (this.params.errors_forgive_time)
			this.elForgive.innerHTML =
				'f<b>' + cm_TimeStringFromSeconds(this.params.errors_forgive_time) + '</b>';
		else
			this.elForgive.textContent = '';

		if (this.params.jobs_life_time)
			this.elJobsLifeTime.innerHTML =
				'l<b>' + cm_TimeStringFromSeconds(this.params.jobs_life_time) + '</b>';
		else
			this.elJobsLifeTime.textContent = '';

		var errstr = 'e:';
		errstr += '<b>' + this.params.errors_avoid_host + '</b>b';
		errstr += ',<b>' + this.params.errors_task_same_host + '</b>t';
		errstr += ',<b>' + this.params.errors_retries + '</b>r';
		this.elErrors.innerHTML = errstr;

		var jobs = 'j';
		if (this.params.jobs_num)
			jobs += '<b>' + this.params.jobs_num + '</b>';
		else
			jobs += '<b>0</b>';
		if (this.params.running_jobs_num)
			jobs += '/<b>' + this.params.running_jobs_num + '</b>r';
		this.elJobs.innerHTML = jobs;
	}

	var errtit = 'Errors solving:';
	errtit += '\nJob blocks to avoid host: ' + this.params.errors_avoid_host;
	errtit += '\nJob tasks to avoid host: ' + this.params.errors_task_same_host;
	errtit += '\nError task retries: ' + this.params.errors_retries;
	this.elErrors.title = errtit;

	if (this.params.annotation)
		this.elAnnotation.innerHTML = this.params.annotation;
	else
		this.elAnnotation.textContent = '';

	this.refresh();
};

UserNode.prototype.refresh = function() {
	var percent = '';
	var label = '';
	if (this.params.running_tasks_num && (this.monitor.max_tasks > 0))
	{
		percent = 100 * this.params.running_tasks_num / this.monitor.max_tasks;
		var capacity = cm_ToKMG(this.params.running_capacity_total);
		if (cm_IsPadawan())
			label = 'Running Tasks: <b>' + this.params.running_tasks_num + '</b> / Total Capacity: <b>' +
				capacity + '</b>';
		else if (cm_IsJedi())
			label = 'Tasks:<b>' + this.params.running_tasks_num + '</b> / Capacity:<b>' + capacity + '</b>';
		else
			label = 't<b>' + this.params.running_tasks_num + '</b>/c<b>' + capacity + '</b>';
	}
	else
		percent = '0';
	this.elBar.style.width = percent + '%';
	this.elBar.innerHTML = label;
};

UserNode.createPanels = function(i_monitor) {
	// Work:
	work_CreatePanels(i_monitor, 'users');

	// Custom data:
	var acts = {};
	acts.custom_data =
		{'type': 'json', 'handle': 'mh_Dialog', 'label': 'CUSTOM', 'tooltip': 'Set user custom data.'};
	i_monitor.createCtrlBtns(acts);
};

UserNode.prototype.updatePanels = function() {
	// Info:
	var info = '<p>Registered: ' + cm_DateTimeStrFromSec(this.params.time_register) + '</p>';
	if (this.params.time_activity)
		info += '<p>Last Activity: ' + cm_DateTimeStrFromSec(this.params.time_activity) + '</p>';
	this.monitor.setPanelInfo(info);
};

UserNode.prototype.onDoubleClick = function(e) {
	g_ShowObject({"object": this.params}, {"evt": e, "wnd": this.monitor.window});
};

UserNode.params_user = {
	errors_retries /*********/: {'type': 'num', 'label': 'Errors Retries'},
	errors_avoid_host /******/: {'type': 'num', 'label': 'Errors Avoid Host'},
	errors_task_same_host /**/: {'type': 'num', 'label': 'Errors Task Same Host'},
	errors_forgive_time /****/: {'type': 'hrs', 'label': 'Errors Forgive Time'},
	jobs_life_time /*********/: {'type': 'hrs', 'label': 'Jobs Life Time'}
};
UserNode.createParams = function() {
	if (UserNode.params_created)
		return;

	UserNode.params = {};
	for (var p in work_params)
		UserNode.params[p] = work_params[p];
	for (var p in UserNode.params_user)
		UserNode.params[p] = UserNode.params_user[p];

	UserNode.params_created = true;
};

UserNode.sort = ['priority', 'name', 'host_name'];
UserNode.filter = ['name', 'host_name'];
