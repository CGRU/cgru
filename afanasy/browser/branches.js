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

BranchNode.generateParamsString = function(i_p, i_type) {
	var str = '';

	if (i_type == null)
		i_type = 'branch';

	if (cm_IsPadawan())
	{
		if ((i_p.max_running_tasks != null) && (i_p.max_running_tasks != -1))
			str += " MaxRunTasks:<b>" + i_p.max_running_tasks + "</b>"
		if ((i_p.max_running_tasks_per_host != null) && (i_p.max_running_tasks_per_host != -1))
			str += " MaxPerHost:<b>" + i_p.max_running_tasks_per_host + "</b>"
		if (i_p.hosts_mask)
			str += " HostsMask:<b>" + i_p.hosts_mask + "</b>"
		if (i_p.hosts_mask_exclude)
			str += " ExcludeHosts:<b>" + i_p.hosts_mask + "</b>"
		if (i_type != 'job')
			str += " Solving:<b>" + i_p.solve_method + "</b>"
		str += " Priority:<b>" + i_p.priority + "</b>"
	}
	else if (cm_IsJedi())
	{
		if ((i_p.max_running_tasks != null) && (i_p.max_running_tasks != -1))
			str += " Max:<b>" + i_p.max_running_tasks + "</b>"
		if ((i_p.max_running_tasks_per_host != null) && (i_p.max_running_tasks_per_host != -1))
			str += " MPH:<b>" + i_p.max_running_tasks_per_host + "</b>"
		if (i_p.hosts_mask)
			str += " Hosts:<b>" + i_p.hosts_mask + "</b>"
		if (i_p.hosts_mask_exclude)
			str += " Exclude:<b>" + i_p.hosts_mask + "</b>"
		if (i_type != 'job')
			str += " Slv:<b>" + i_p.solve_method + "</b>"
		str += " Pri:<b>" + i_p.priority + "</b>"
	}
	else
	{
		if ((i_p.max_running_tasks != null) && (i_p.max_running_tasks != -1))
			str += " m:<b>" + i_p.max_running_tasks + "</b>"
		if ((i_p.max_running_tasks_per_host != null) && (i_p.max_running_tasks_per_host != -1))
			str += " mph:<b>" + i_p.max_running_tasks_per_host + "</b>"
		if (i_p.hosts_mask)
			str += " h:<b>" + i_p.hosts_mask + "</b>"
		if (i_p.hosts_mask_exclude)
			str += " e:<b>" + i_p.hosts_mask + "</b>"
		if (i_type != 'job')
			str += " <b>" + i_p.solve_method + "</b>"
		str += " <b>" + i_p.priority + "</b>"
	}

	return str;
}

BranchNode.prototype.init = function() {
	this.element.classList.add('branch');

	this.elName = document.createElement('span');
	this.elName.classList.add('name');
	this.element.appendChild(this.elName);
	this.elName.title = 'User Name';

	this.elWork = cm_ElCreateFloatText(this.element,'right');

	this.element.appendChild(document.createElement('br'));

	this.elBranches = cm_ElCreateFloatText(this.element, 'left', 'Branches: All/Running');
	this.elJobs = cm_ElCreateFloatText(this.element, 'left', 'Jobs: All/Running');

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

	this.elWork.innerHTML = BranchNode.generateParamsString(this.params,'branch');

	if (cm_IsPadawan())
	{
		var jobs = 'Jobs Total:';
		if (this.params.jobs_num)
			jobs += ' <b>' + this.params.jobs_num + '</b>';
		else
			jobs += ' <b>0</b>';
		if (this.params.running_jobs_num)
			jobs += ' / <b>' + this.params.running_jobs_num + '</b> Running';
		this.elJobs.innerHTML = jobs;

		var counts = 'Branches Total:';
		if (this.params.branches_num)
			 counts += ' <b>' + this.params.branches_num + '</b>';
		else
			counts += ' <b>0</b>';
		if (this.params.running_branches_num)
			counts += ' / <b>' + this.params.running_branches_num + '</b> Running';
		this.elBranches.innerHTML = counts;
	}
	else if (cm_IsJedi())
	{
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
		var jobs = 'j';
		if (this.params.jobs_num)
			jobs += '<b>' + this.params.jobs_num + '</b>';
		else
			jobs += '<b>0</b>';
		if (this.params.running_jobs_num)
			jobs += '/<b>' + this.params.running_jobs_num + '</b>r';
		this.elJobs.innerHTML = jobs;

	}

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
	max_running_tasks_per_host: {'type': 'num', 'label': 'Max Running Tasks Per Host'},
	hosts_mask /*************/: {'type': 'reg', 'label': 'Hosts Mask'},
	hosts_mask_exclude /*****/: {'type': 'reg', 'label': 'Exclude Hosts Mask'},
	annotation /*************/: {'type': 'str', 'label': 'Annotation'}
};

BranchNode.sort = ['priority', 'name'];
BranchNode.filter = ['name'];
