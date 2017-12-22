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
	branches.js - methods and structs for monitoring and handling of branches
*/

"use strict";

var branches = null;

BranchNode.onMonitorCreate = function() {
	branches = {};
};

function BranchNode()
{
}

BranchNode.prototype.init = function() {
	this.element.classList.add('branch');

	this.elName = document.createElement('span');
	this.elName.classList.add('name');
	this.element.appendChild(this.elName);
	this.elName.title = 'User Name';

	if (false == cm_IsPadawan())
	{
		this.elPriority = document.createElement('span');
		this.element.appendChild(this.elPriority);
		this.elPriority.title = 'Priority';
	}

	this.elCenter = document.createElement('div');
	this.element.appendChild(this.elCenter);
	this.elCenter.style.position = 'absolute';
	this.elCenter.style.left = '0';
	this.elCenter.style.right = '0';
	this.elCenter.style.top = '1px';
	this.elCenter.style.textAlign = 'center';

	if (cm_IsPadawan())
		this.elPriority = cm_ElCreateText(this.elCenter, 'Priority');
	this.elMaxRunningTasks = cm_ElCreateText(this.elCenter, 'Maximum Running Tasks');
	this.elHostsMask = cm_ElCreateText(this.elCenter, 'Hosts Mask');
	this.elHostsMaskExclude = cm_ElCreateText(this.elCenter, 'Exclude Hosts Mask');

	this.element.appendChild(document.createElement('br'));

	this.elJobs = cm_ElCreateFloatText(this.element, 'left', 'Jobs: All/Running');

	this.elSolving = cm_ElCreateFloatText(this.element, 'right');

	this.element.appendChild(document.createElement('br'));

	this.elAnnotation = document.createElement('div');
	this.element.appendChild(this.elAnnotation);
	this.elAnnotation.title = 'Annotation';
	this.elAnnotation.style.textAlign = 'center';

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

BranchNode.prototype.update = function(i_obj) {
	if (i_obj)
		this.params = i_obj;

	this.branch_depth = 0;
	branches[this.params.name] = this;
	var parent_branch = branches[this.params.parent];
	if (parent_branch)
		this.branch_depth = parent_branch.branch_depth + 1;
	this.element.style.marginLeft = (this.branch_depth * 32 + 2) + 'px';

	var name = this.params.name;
	if (name == '/')
		name = 'ROOT';
	else
		name = cm_PathBase(name);
	this.elName.innerHTML = '<b>' + name + '/</b>';

	if (cm_IsPadawan())
	{
		this.elPriority.innerHTML = 'Priority:<b>' + this.params.priority + '</b>';

		if (this.params.max_running_tasks)
			this.elMaxRunningTasks.innerHTML = 'MaxRunTasks:<b>' + this.params.max_running_tasks + '</b>';
		else
			this.elMaxRunningTasks.textContent = '';

		if (this.params.hosts_mask)
			this.elHostsMask.innerHTML = 'HostsMask(<b>' + this.params.hosts_mask + '</b>)';
		else
			this.elHostsMask.textContent = '';

		if (this.params.hosts_mask_exclude)
			this.elHostsMaskExclude.innerHTML = 'ExcludeHosts(<b>' + this.params.hosts_mask_exclude + '</b>)';
		else
			this.elHostsMaskExclude.textContent = '';

		var jobs = 'Jobs Total:';
		if (this.params.jobs_num)
			jobs += ' <b>' + this.params.jobs_num + '</b>';
		else
			jobs += ' <b>0</b>';
		if (this.params.running_jobs_num)
			jobs += ' / <b>' + this.params.running_jobs_num + '</b> Running';
		this.elJobs.innerHTML = jobs;

		if (this.params.solve_parallel)
			this.elSolving.innerHTML = 'Solving:<b>Parallel</b>';
		else
			this.elSolving.innerHTML = 'Solving:<b>Ordered</b>';
	}
	else if (cm_IsJedi())
	{
		this.elPriority.innerHTML = '-<b>' + this.params.priority + '</b>';

		if (this.params.max_running_tasks)
			this.elMaxRunningTasks.innerHTML = 'Max:<b>' + this.params.max_running_tasks + ')</b>';
		else
			this.elMaxRunningTasks.textContent = '';

		if (this.params.hosts_mask)
			this.elHostsMask.innerHTML = 'Hosts(<b>' + this.params.hosts_mask + ')</b>';
		else
			this.elHostsMask.textContent = '';

		if (this.params.hosts_mask_exclude)
			this.elHostsMaskExclude.innerHTML = 'Exclude(<b>' + this.params.hosts_mask_exclude + ')</b>';
		else
			this.elHostsMaskExclude.textContent = '';

		var jobs = 'Jobs:';
		if (this.params.jobs_num)
			jobs += '<b>' + this.params.jobs_num + '</b>';
		else
			jobs += '<b>0</b>';
		if (this.params.running_jobs_num)
			jobs += ' / <b>' + this.params.running_jobs_num + '</b>Run';
		this.elJobs.innerHTML = jobs;

		if (this.params.solve_parallel)
			this.elSolving.innerHTML = '<b>Parallel</b>';
		else
			this.elSolving.innerHTML = '<b>Ordered</b>';
	}
	else
	{
		this.elPriority.innerHTML = '-<b>' + this.params.priority + '</b>';

		if (this.params.max_running_tasks)
			this.elMaxRunningTasks.innerHTML = 'm<b>' + this.params.max_running_tasks + '</b>';
		else
			this.elMaxRunningTasks.textContent = '';

		if (this.params.hosts_mask)
			this.elHostsMask.innerHTML = 'h(<b>' + this.params.hosts_mask + ')</b>';
		else
			this.elHostsMask.textContent = '';

		if (this.params.hosts_mask_exclude)
			this.elHostsMaskExclude.innerHTML = 'e(<b>' + this.params.hosts_mask_exclude + ')</b>';
		else
			this.elHostsMaskExclude.textContent = '';

		var jobs = 'j';
		if (this.params.jobs_num)
			jobs += '<b>' + this.params.jobs_num + '</b>';
		else
			jobs += '<b>0</b>';
		if (this.params.running_jobs_num)
			jobs += '/<b>' + this.params.running_jobs_num + '</b>r';
		this.elJobs.innerHTML = jobs;

		if (this.params.solve_parallel)
			this.elSolving.innerHTML = '<b>par</b>';
		else
			this.elSolving.innerHTML = '<b>ord</b>';
	}

	var solving = 'User jobs solving method:';
	if (this.params.solve_parallel)
		solving += '\nParallel: All together according to jobs priority.\n';
	else
		solving += '\nOrdered: Queued by jobs priority and order.\n';
	this.elSolving.title = solving;

	if (this.params.annotation)
		this.elAnnotation.innerHTML = '<b><i>' + this.params.annotation + '</i></b>';
	else
		this.elAnnotation.textContent = '';

	var title = '';
	title += 'Time Created: ' + cm_DateTimeStrFromSec(this.params.time_creation) + '\n';
	title += 'ID = ' + this.params.id + '\n';
	this.element.title = title;

	this.refresh();
};

BranchNode.prototype.refresh = function() {
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

BranchNode.createPanels = function(i_monitor) {
	// Jobs solving:
	var acts = {};
	acts.solve_ord = {
		'name': 'solve_parallel',
		'value': false,
		'label': 'ORD',
		'tooltip': 'Solve jobs by order.',
		'handle': 'mh_Param'
	};
	acts.solve_par = {
		'name': 'solve_parallel',
		'value': true,
		'label': 'PAR',
		'tooltip': 'Solve jobs parallel.',
		'handle': 'mh_Param'
	};
	i_monitor.createCtrlBtns(acts);
};

BranchNode.prototype.updatePanels = function() {
	// Info:
	var info = 'Created: ' + cm_DateTimeStrFromSec(this.params.time_creation);
	if (this.params.time_activity)
		info += '<br>Last Activity:<br> ' + cm_DateTimeStrFromSec(this.params.time_activity);
	this.monitor.setPanelInfo(info);
};

BranchNode.prototype.onDoubleClick = function(e) {
	g_ShowObject({"object": this.params}, {"evt": e, "wnd": this.monitor.window});
};

BranchNode.params = {
	priority /***************/: {'type': 'num', 'label': 'Priority'},
	max_running_tasks /******/: {'type': 'num', 'label': 'Max Running Tasks'},
	hosts_mask /*************/: {'type': 'reg', 'label': 'Hosts Mask'},
	hosts_mask_exclude /*****/: {'type': 'reg', 'label': 'Exclude Hosts Mask'},
	annotation /*************/: {'type': 'str', 'label': 'Annotation'}
};

BranchNode.sort = ['priority', 'name'];
BranchNode.filter = ['name'];
