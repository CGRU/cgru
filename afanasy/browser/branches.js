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
var branches_active_jobs = {};
var branches_active_jobs_selected = [];

BranchNode.onMonitorCreate = function() {
	branches = {};
	BranchNode.createParams();
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

	this.elWork = cm_ElCreateFloatText(this.element, 'right');

	this.element.appendChild(document.createElement('br'));

	this.elBranchedCounts = cm_ElCreateFloatText(this.element, 'left', 'Branches: All/Running');
	this.elJobsCounts = cm_ElCreateFloatText(this.element, 'left', 'Jobs: All/Running');

	this.elAnnotation = document.createElement('div');
	this.element.appendChild(this.elAnnotation);
	this.elAnnotation.classList.add('annotation');
	this.elAnnotation.title = 'Annotation';

	this.active_jobs = [];
	this.elActiveJobs = document.createElement('div');
	this.elActiveJobs.classList.add('active_jobs_div');
	this.element.appendChild(this.elActiveJobs);

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

	this.elWork.innerHTML = work_generateParamsString(this.params, 'branch');

	if (cm_IsPadawan())
	{
		var jobs = 'Jobs Total:';
		if (this.params.jobs_num)
			jobs += ' <b>' + this.params.jobs_num + '</b>';
		else
			jobs += ' <b>0</b>';
		if (this.params.running_jobs_num)
			jobs += ' / <b>' + this.params.running_jobs_num + '</b> Running';
		this.elJobsCounts.innerHTML = jobs;

		var counts = 'Branches Total:';
		if (this.params.branches_num)
			counts += ' <b>' + this.params.branches_num + '</b>';
		else
			counts += ' <b>0</b>';
		if (this.params.running_branches_num)
			counts += ' / <b>' + this.params.running_branches_num + '</b> Running';
		this.elBranchedCounts.innerHTML = counts;
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
		this.elJobsCounts.innerHTML = jobs;
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
		this.elJobsCounts.innerHTML = jobs;
	}

	// Set all current active jobs as not updated.
	// Later all not updated active jobs will be deleted.
	// This needed as we are not "listening" job_deleted event.
	for (var j = 0; j < this.active_jobs.length; j++)
		this.active_jobs[j].setNotUpdated();

	for (var j = 0; j < this.params.active_jobs.length; j++)
	{
		var jparams = this.params.active_jobs[j];
		var active_job = branches_active_jobs[jparams.id];

		if (active_job)
		{
			// Reparent if it is not in this branch:
			if (this.active_jobs.indexOf(active_job) == -1)
			{
				active_job.elParent.removeChild(active_job.el);
				active_job.branch.active_jobs.splice(active_job.branch.active_jobs.indexOf(active_job), 1);

				this.elActiveJobs.appendChild(active_job.el);
				active_job.elParent = this.elActiveJobs;
				active_job.branch = this;
				this.active_jobs.push(active_job);
			}

			active_job.update(jparams);
		}
		else
		{
			active_job = new BranchActiveJob(this, this.elActiveJobs, jparams);
			branches_active_jobs[jparams.id] = active_job;
			this.active_jobs.push(active_job);
		}
	}

	if (this.params.annotation)
		this.elAnnotation.innerHTML = this.params.annotation;
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
	// Work:
	work_CreatePanels(i_monitor, 'branches');
};

BranchNode.prototype.updatePanels = function() {
	// Info:
	var info = 'Created: ' + cm_DateTimeStrFromSec(this.params.time_creation);
	if (this.params.time_activity)
		info += '<br>Last Activity:<br> ' + cm_DateTimeStrFromSec(this.params.time_activity);
	this.monitor.setPanelInfo(info);

	BranchActiveJob.deselectAll(this.monitor);
};

BranchNode.prototype.onDoubleClick = function(e) {
	g_ShowObject({"object": this.params}, {"evt": e, "wnd": this.monitor.window});
};

BranchNode.params_branch = {};

BranchNode.createParams = function() {
	if (BranchNode.params_created)
		return;

	BranchNode.params = {};
	for (var p in work_params)
		BranchNode.params[p] = work_params[p];
	for (var p in BranchNode.params_branch)
		BranchNode.params[p] = BranchNode.params_branch[p];

	BranchNode.params_created = true;
};

BranchNode.sort = ['priority', 'name'];
BranchNode.filter = ['name'];


// ###################### Branch Active Job ###################################

function BranchActiveJob(i_branch, i_elParent, i_params)
{
	this.branch = i_branch;
	this.params = i_params;
	this.elParent = i_elParent;

	this.el = document.createElement('div');
	this.el.classList.add('branch_active_job');
	this.elParent.appendChild(this.el);
	this.el.active_job = this;

	this.el.oncontextmenu = function(e) {
		e.stopPropagation();
		e.currentTarget.active_job.onContextMenu();
		return false;
	};

	this.update(i_params);
};

BranchActiveJob.prototype.setNotUpdated = function() {
	this.el.style.display = 'none';
	this.updated = false;
};

BranchActiveJob.prototype.update = function(i_params) {
	this.params = i_params;

	var info = this.params.name + '[' + this.params.id + ']';
	this.el.textContent = info;

	this.el.style.display = 'block';
	this.updated = true;
	if (this.branch.monitor.m_cur_acrive_job == this)
		this.updatePanels();
};

BranchActiveJob.prototype.onContextMenu = function() {
	g_cur_monitor = this.branch.monitor;

	// Clear not updated (deleted) jobs:
	for (var jid in branches_active_jobs)
	{
		var active_job = branches_active_jobs[jid];
		if (active_job == null)
			continue;
		if (active_job.updated)
			continue;

		branches_active_jobs[jid] = null;
		var i = branches_active_jobs_selected.indexOf(active_job);
		if (i != -1)
			branches_active_jobs_selected.splice(i, 1);
	}

	if (this.selected)
	{
		this.setSelected(false);

		// If several active jobs were selected, we should show panels info of a previously selected jobs:
		if (branches_active_jobs_selected.length)
			branches_active_jobs_selected[branches_active_jobs_selected.length - 1].updatePanels();
		else
			BranchActiveJob.resetPanels(this.branch.monitor);

		return;
	}

	// If branches selected, select all selected branches active jobs.
	var selected_branches = this.branch.monitor.getSelectedItems();
	if (selected_branches.length)
	{
		this.branch.monitor.selectAll(false);
		for (var b = 0; b < selected_branches.length; b++)
			for (var j = 0; j < selected_branches[b].active_jobs.length; j++)
				selected_branches[b].active_jobs[j].setSelected(true);
	}

	this.setSelected(true);

	this.updatePanels();
};

BranchActiveJob.prototype.setSelected = function(i_select) {
	if (i_select)
	{
		if (this.selected)
			return;

		this.selected = true;
		this.el.classList.add('selected');

		branches_active_jobs_selected.push(this);
	}
	else
	{
		if (this.selected != true)
			return;

		this.selected = false;
		this.el.classList.remove('selected');

		branches_active_jobs_selected.splice(branches_active_jobs_selected.indexOf(this), 1);
	}
};

BranchActiveJob.deselectAll = function(i_monitor) {
	while (branches_active_jobs_selected.length)
		branches_active_jobs_selected[0].setSelected(false);

	BranchActiveJob.resetPanels(i_monitor);
};

BranchActiveJob.prototype.updatePanels = function() {
	BranchActiveJob.resetPanels(this.branch.monitor);
	this.branch.monitor.m_cur_acrive_job = this;

	var elPanelR = this.branch.monitor.elPanelR;

	var elActiveJob = document.createElement('div');
	elPanelR.appendChild(elActiveJob);
	elPanelR.m_elActiveJob = elActiveJob;
	elActiveJob.m_active_job = this;
	elActiveJob.classList.add('section');
	elActiveJob.classList.add('active');

	var elCaption = document.createElement('div');
	elActiveJob.appendChild(elCaption);
	elCaption.textContent = 'Active Job:';
	elCaption.classList.add('caption');

	var elName = document.createElement('div');
	elActiveJob.appendChild(elName);
	elName.classList.add('name');
	elName.textContent = this.params.name;

	var elBranchCtrl = document.createElement('div');
	elActiveJob.appendChild(elBranchCtrl);

	var elLabel = document.createElement('div');
	elBranchCtrl.appendChild(elLabel);
	elLabel.textContent = 'Job Branch:';

	var elName = document.createElement('div');
	elBranchCtrl.appendChild(elName);
	elName.textContent = this.branch.params.name;

	var elChange = document.createElement('div');
	elBranchCtrl.appendChild(elChange);
	elChange.classList.add('button');
	elChange.textContent = 'Change Branch';
	elChange.m_active_job = this;
	elChange.onclick = function(e) { e.currentTarget.m_active_job.changeBranch(); };
};

BranchActiveJob.resetPanels = function(i_monitor) {
	if (i_monitor.m_cur_acrive_job == null)
		return;

	i_monitor.elPanelR.removeChild(i_monitor.elPanelR.m_elActiveJob);
	i_monitor.elPanelR.m_elActiveJob = null;
	i_monitor.m_cur_acrive_job = null;
};

BranchActiveJob.prototype.changeBranch = function() {
	new cgru_Dialog({
		"wnd": this.branch.monitor.window,
		"receiver": this,
		"handle": 'setParameter',
		"param": 'branch',
		"type": 'str',
		"value": this.branch.params.name,
		"name": 'Change Job Branch'
	});

};

BranchActiveJob.prototype.setParameter = function(i_value, i_param) {
	var params = {};
	params[i_param] = i_value;
	this.action(null, params);
};

BranchActiveJob.prototype.action = function(i_operation, i_params) {
	var jids = [];
	for (var j = 0; j < branches_active_jobs_selected.length; j++)
	{
		if (branches_active_jobs_selected[j].updated)
		{
			var id = branches_active_jobs_selected[j].params.id;
			if (jids.indexOf(id) == -1)
				jids.push(id);
		}
	}

	nw_Action('jobs', jids, i_operation, i_params);
};

BranchActiveJob.params = {
	priority /***************/: {'type': 'num', 'label': 'Priority'},
	max_running_tasks /******/: {'type': 'num', 'label': 'Max Running Tasks'},
	max_running_tasks_per_host: {'type': 'num', 'label': 'Max Running Tasks Per Host'}
};
