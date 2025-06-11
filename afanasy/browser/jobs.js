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
	jobs.js - methods and structs for monitoring and handling of render jobs
	   JobBlock - a block if job nodes
	   JobNode  - the representation of a job (one committed work-payload)
*/

'use strict';

function JobNode() {}

var Block_ProgressBarLength = 128;
var Block_ProgressBarHeight = 10;

var BarDONrgb = '#363';
var BarSKPrgb = '#444';
var BarDWRrgb = '#2C2';
var BarSUSrgb = '#AA1';
var BarWDPrgb = '#A2A';
var BarRUNrgb = '#FF0';
var BarRWRrgb = '#FA0';
var BarRERrgb = '#F77';
var BarERRrgb = '#F00';
var BarWRCrgb = '#4AC';

JobNode.onMonitorCreate = function() {
	JobNode.createParams();
};

JobNode.prototype.init = function() {
	this.element.classList.add('job');

	cm_CreateStart(this);

	this.elName = document.createElement('span');
	this.elName.classList.add('name');
	this.element.appendChild(this.elName);
	this.elName.classList.add('prestar');

	this.elUserName = cm_ElCreateFloatText(this.element, 'right', 'User Name');
	this.elETA = cm_ElCreateFloatText(this.element, 'right', 'Estimated Time Of Arrival (Done)');

	this.element.appendChild(document.createElement('br'));

	this.elState = document.createElement('span');
	this.element.appendChild(this.elState);
	this.elState.title = 'Job State';
	this.elState.classList.add('prestar');

	this.elTime = cm_ElCreateFloatText(this.element, 'right', 'Running Time');
	this.elWork = cm_ElCreateFloatText(this.element, 'right');
	this.elLifeTime = cm_ElCreateFloatText(this.element, 'right', 'Life Time');
	this.elPPApproval = cm_ElCreateFloatText(this.element, 'right', 'Preview Pending Approval', '<b>PPA</b>');
	this.elMaintenance = cm_ElCreateFloatText(this.element, 'right', 'Maintenance', '<b>MNT</b>');
	this.elIgnoreNimby =
		cm_ElCreateFloatText(this.element, 'right', 'Ignore render "Nimby" state.', '<b>INB</b>');
	this.elIgnorePaused =
		cm_ElCreateFloatText(this.element, 'right', 'Ignore render "Paused" state.', '<b>IPS</b>');
	this.elDependMask = cm_ElCreateFloatText(this.element, 'right', 'Depend Mask');
	this.elDependMaskGlobal = cm_ElCreateFloatText(this.element, 'right', 'Global Depend Mask');
	this.elNeedProperties = cm_ElCreateFloatText(this.element, 'right', 'Properties');
	this.elNeedOS = cm_ElCreateFloatText(this.element, 'right', 'OS Needed');

	if (cm_IsSith())
	{
		this.elETA.classList.add('lowercase');
		this.elPPApproval.classList.add('lowercase');
		this.elMaintenance.classList.add('lowercase');
		this.elIgnoreNimby.classList.add('lowercase');
		this.elIgnorePaused.classList.add('lowercase');
	}

	this.blocks = [];
	for (var b = 0; b < this.params.blocks.length; b++)
		this.blocks.push(new JobBlock(this.element, this.params.blocks[b]));

	this.elThumbs = document.createElement('div');
	this.element.appendChild(this.elThumbs);
	this.elThumbs.classList.add('thumbnails');
	this.elThumbs.style.display = 'none';

	this.elReport = document.createElement('div');
	this.element.appendChild(this.elReport);
	this.elReport.title = 'Report';
	this.elReport.style.textAlign = 'center';

	this.elAnnotation = document.createElement('div');
	this.element.appendChild(this.elAnnotation);
	this.elAnnotation.title = 'Annotation';
	this.elAnnotation.classList.add('annotation');

	this.state = {};
	this.running_tasks = 0;
	this.percentage = 0;
};

JobNode.prototype.update = function(i_obj) {
	if (i_obj)
		this.params = i_obj;

	if (this.params.user_list_order != null)
		this.params.order = this.params.user_list_order;

	cm_GetState(this.params.state, this.state, this.element);
	if (this.params.state == null)
		this.params.state = '';
	this.elState.innerHTML = '<b>' + this.params.state + '</b>';

	var displayFull = false;
	if (this.state.ERR || this.state.RUN || this.state.SKP ||
		((this.state.DON == false) && (this.params.time_started > 0)))
		displayFull = true;

	this.elName.innerHTML = '<b>' + this.params.name + '</b>';
	this.elName.title = 'ID = ' + this.params.id;
	this.elUserName.innerHTML = '<b>' + this.params.user_name + '</b>';
	if (false == g_VISOR())
		this.elUserName.style.display = 'none';

	if (this.params.ppa)
		this.elPPApproval.style.display = 'block';
	else
		this.elPPApproval.style.display = 'none';

	if (this.params.maintenance)
		this.elMaintenance.style.display = 'block';
	else
		this.elMaintenance.style.display = 'none';

	if (this.params.ignorenimby)
		this.elIgnoreNimby.style.display = 'block';
	else
		this.elIgnoreNimby.style.display = 'none';

	if (this.params.ignorepaused)
		this.elIgnorePaused.style.display = 'block';
	else
		this.elIgnorePaused.style.display = 'none';

	if (this.params.thumb_path)
		this.showThumb(this.params.thumb_path);

	this.elWork.innerHTML = work_generateParamsString(this.params, 'job');

	if (cm_IsPadawan())
	{
		if (this.params.time_life)
			this.elLifeTime.innerHTML =
				'LifeTime(<b>' + cm_TimeStringFromSeconds(this.params.time_life) + '</b>)';
		else
			this.elLifeTime.textContent = '';

		if (this.params.depend_mask)
			this.elDependMask.innerHTML = 'DependMask(<b>' + this.params.depend_mask + '</b>)';
		else
			this.elDependMask.textContent = '';

		if (this.params.depend_mask_global)
			this.elDependMaskGlobal.innerHTML =
				'GlobalDepends(<b>' + this.params.depend_mask_global + '</b>)';
		else
			this.elDependMaskGlobal.textContent = '';

		if (this.params.need_properties)
			this.elNeedProperties.innerHTML = 'Properties(<b>' + this.params.need_properties + '</b>)';
		else
			this.elNeedProperties.textContent = '';

		if (this.params.need_os)
			this.elNeedOS.innerHTML = 'OS(<b>' + this.params.need_os + '</b>)';
		else
			this.elNeedOS.textContent = '';
	}
	else if (cm_IsJedi())
	{
		if (this.params.time_life)
			this.elLifeTime.innerHTML =
				'Life(<b>' + cm_TimeStringFromSeconds(this.params.time_life) + '</b>)';
		else
			this.elLifeTime.textContent = '';

		if (this.params.depend_mask)
			this.elDependMask.innerHTML = 'Dep(<b>' + this.params.depend_mask + '</b>)';
		else
			this.elDependMask.textContent = '';

		if (this.params.depend_mask_global)
			this.elDependMaskGlobal.innerHTML = 'GDep(<b>' + this.params.depend_mask_global + '</b>)';
		else
			this.elDependMaskGlobal.textContent = '';

		if (this.params.need_properties)
			this.elNeedProperties.innerHTML = 'Props(<b>' + this.params.need_properties + '</b>)';
		else
			this.elNeedProperties.textContent = '';

		if (this.params.need_os)
			this.elNeedOS.innerHTML = 'OS(<b>' + this.params.need_os + '</b>)';
		else
			this.elNeedOS.textContent = '';
	}
	else
	{
		if (this.params.time_life)
			this.elLifeTime.innerHTML = 'l(<b>' + cm_TimeStringFromSeconds(this.params.time_life) + '</b>)';
		else
			this.elLifeTime.textContent = '';

		if (this.params.depend_mask)
			this.elDependMask.innerHTML = 'd(<b>' + this.params.depend_mask + '</b>)';
		else
			this.elDependMask.textContent = '';

		if (this.params.depend_mask_global)
			this.elDependMaskGlobal.innerHTML = 'g(<b>' + this.params.depend_mask_global + '</b>)';
		else
			this.elDependMaskGlobal.textContent = '';

		if (this.params.need_properties)
			this.elNeedProperties.innerHTML = '<b>' + this.params.need_properties + '</b>';
		else
			this.elNeedProperties.textContent = '';

		if (this.params.need_os)
			this.elNeedOS.innerHTML = '<b>' + this.params.need_os + '</b>';
		else
			this.elNeedOS.textContent = '';
	}

	if (this.params.report)
		this.elReport.innerHTML = '<b>' + this.params.report + '</b>';
	else
		this.elReport.textContent = '';

	if (this.params.annotation)
		this.elAnnotation.innerHTML = this.params.annotation;
	else
		this.elAnnotation.textContent = '';

	this.running_tasks = 0;
	this.percentage = 0;
	for (var b = 0; b < this.params.blocks.length; b++)
	{
		this.blocks[b].params = this.params.blocks[b];
		this.blocks[b].update(displayFull);
		if (this.blocks[b].params.running_tasks_counter)
			this.running_tasks += this.blocks[b].params.running_tasks_counter;
		if (this.blocks[b].params.p_percentage)
			this.percentage += Math.round(this.blocks[b].params.p_percentage / this.params.blocks.length);
	}

	if (this.running_tasks)
	{
		this.elStar.style.display = 'block';
		this.elStarCount.textContent = this.running_tasks;
	}
	else
		this.elStar.style.display = 'none';

	this.params.service = this.blocks[0].params.service;

	this.refresh();
};

JobNode.prototype.refresh = function() {
	var time_txt = '';
	var time_tip = '';
	var eta = '';

	if (this.params.time_wait && this.state.WTM)
	{
		time_txt = '<b>' + cm_TimeStringInterval(new Date().getTime() / 1000, this.params.time_wait) + '</b>';
		if (cm_IsPadawan())
			time_txt = 'Waiting:' + time_txt;
		else if (cm_IsJedi())
			time_txt = 'Wait:' + time_txt;
		time_tip = 'Waiting for: ' + cm_DateTimeStrFromSec(this.params.time_wait);
	}
	else if (this.params.time_started)
	{
		if (this.state.DON == true)
		{
			time_txt = cm_TimeStringInterval(this.params.time_started, this.params.time_done);
			time_tip = 'Done at: ' + cm_DateTimeStrFromSec(this.params.time_done);
			time_tip += '\nRunning time: ' + time_txt;
			time_txt = '<b>' + time_txt + '</b>';
			if (cm_IsPadawan())
				time_txt = 'RunningTime:' + time_txt;
			else if (cm_IsJedi())
				time_txt = 'RunTime:' + time_txt;
		}
		else
		{
			time_txt = '<b>' + cm_TimeStringInterval(this.params.time_started) + '</b>';
			if (cm_IsPadawan())
				time_txt = 'RunningTime:' + time_txt;
			else if (cm_IsJedi())
				time_txt = 'RunTime:' + time_txt;

			// ETA (but not for the system job which id == 1):
			if (this.params.id > 1)
			{
				var percentage = 0;
				for (var b = 0; b < this.blocks.length; b++)
				{
					if (this.blocks[b].params.p_percentage)
						percentage += this.blocks[b].params.p_percentage;
				}
				percentage /= this.blocks.length;
				if ((percentage > 0) && (percentage < 100))
				{
					var sec_now = (new Date()).valueOf() / 1000;
					var sec_run = sec_now - this.params.time_started;
					var sec_all = sec_run * 100.0 / percentage;
					eta = sec_all - sec_run;
					if (eta > 0)
					{
						eta = cm_TimeStringInterval(0, eta);
						eta = 'ETA≈<b>' + eta + '</b>';
					}
				}
			}
		}
		time_tip = 'Started at: ' + cm_DateTimeStrFromSec(this.params.time_started) + '\n' + time_tip;
	}

	time_tip = 'Created at: ' + cm_DateTimeStrFromSec(this.params.time_creation) + '\n' + time_tip;

	this.elTime.innerHTML = time_txt;
	this.elTime.title = time_tip;
	this.elETA.innerHTML = eta;
};

JobNode.prototype.onDoubleClick = function(i_evt) {
	g_OpenMonitor({
		'type': 'tasks',
		'evt': i_evt,
		'id': this.params.id,
		'name': this.params.name,
		'wnd': this.monitor.window
	});
};

JobNode.prototype.mh_Move = function(i_param) {
	if (g_uid < 1)
	{
		g_Error('Can`t move nodes for uid < 1');
		return;
	}
	var operation = {};
	operation.type = i_param.name;
	operation.jids = this.monitor.getSelectedIds();
	nw_Action('users', [g_uid], operation);
	this.monitor.info('Moving Jobs');
};

JobNode.prototype.showThumb = function(i_path) {
	i_path = i_path.replace(/\\/g, '/');
	// console.log('JobNode.prototype.showThumb = '+i_path);
	if (this.elThumbs.m_divs == null)
	{
		this.elThumbs.style.display = 'block';
		this.elThumbs.m_divs = [];
	}

	// Do nothing if the last image is the same
	if (this.elThumbs.m_divs.length)
		if (this.elThumbs.m_divs[this.elThumbs.m_divs.length - 1].m_path == i_path)
			return;

	var label = cm_PathBase(i_path).replace(/\.jpg$/, '');

	var thumb = document.createElement('div');
	this.elThumbs.appendChild(thumb);
	this.elThumbs.m_divs.push(thumb);
	thumb.m_path = i_path;
	thumb.title = label;

	var name = document.createElement('div');
	name.textContent = label;
	thumb.appendChild(name);

	if (i_path.lastIndexOf('.jpg') == (i_path.length - 4))
	{
		var img = document.createElement('img');
		thumb.appendChild(img);
		img.ondragstart = function() {
			return false;
		};
		img.src = '/@TMP@' + i_path;
		img.style.display = 'none';
		img.m_height = this.monitor.view_opts.jobs_thumbs_height;
		img.onload = function(e) {
			var img = e.currentTarget;
			if (img.height != img.m_height)
			{
				img.width = img.m_height * img.width / img.height;
				img.height = img.m_height;
			}
			img.style.display = 'block';
		}
	}
	else
		name.style.position = 'relative';

	if (this.elThumbs.m_divs.length > this.monitor.view_opts.jobs_thumbs_num)
	{
		this.elThumbs.removeChild(this.elThumbs.m_divs[0]);
		this.elThumbs.m_divs.splice(0, 1);
	}
};

// ###################### BLOCK ###################################

function JobBlock(i_elParent, i_block)
{
	this.params = i_block;
	this.job = i_elParent.item;

	this.tasks_num = this.params.tasks_num;

	this.elRoot = document.createElement('div');
	i_elParent.appendChild(this.elRoot);
	this.elRoot.style.clear = 'both';
	this.elRoot.block = this;
	this.elRoot.oncontextmenu = function(e) {
		e.currentTarget.block.onContextMenu(e);
		return false;
	};

	this.service = this.params.service;
	this.elIcon = document.createElement('img');
	this.elRoot.appendChild(this.elIcon);
	this.elIcon.src = '/icons/software/' + this.service + '.png';
	this.elIcon.style.position = 'absolute';
	//	this.elIcon.classList.add('icon');

	this.element = document.createElement('div');
	this.elRoot.appendChild(this.element);
	this.element.classList.add('jobblock');

	this.elTasks = cm_ElCreateText(this.element);
	this.elName = cm_ElCreateText(this.element, 'Block Name');
	this.elDepends = cm_ElCreateText(this.element);

	this.elTickets = cm_ElCreateFloatText(this.element, 'right', 'Task Tickets');
	this.elCapacity = cm_ElCreateFloatText(this.element, 'right', 'Tasks Capacity');
	this.elErrSolving = cm_ElCreateFloatText(this.element, 'right');
	this.elForgiveTime = cm_ElCreateFloatText(this.element, 'right', 'Errors Forgive Time');
	this.elMaxRunTime = cm_ElCreateFloatText(this.element, 'right', 'Task Maximum Run Time');
	this.elMinRunTime = cm_ElCreateFloatText(this.element, 'right', 'Task Minimum Run Time');
	this.elProgressTimeout = cm_ElCreateFloatText(this.element, 'right', 'Task Progress Change Timeout');
	this.elNeedMem = cm_ElCreateFloatText(this.element, 'right', 'Required Memory');
	this.elNeed_GPU_Mem = cm_ElCreateFloatText(this.element, 'right', 'Required GPU Memory');
	this.elNeed_CPU_Freq = cm_ElCreateFloatText(this.element, 'right', 'Required CPU Frequency');
	this.elNeed_CPU_Cores = cm_ElCreateFloatText(this.element, 'right', 'Required CPU Cores');
	this.elNeed_CPU_FreqCores = cm_ElCreateFloatText(this.element, 'right', 'Required CPU Frequency * Cores');
	this.elNeedHDD = cm_ElCreateFloatText(this.element, 'right', 'Required HDD Space');
	this.elNeedPower = cm_ElCreateFloatText(this.element, 'right', 'Needed Power');
	this.elNeedProperties = cm_ElCreateFloatText(this.element, 'right', 'Needed Properties');
	this.elRunTime = cm_ElCreateFloatText(this.element, 'right');
}

JobBlock.prototype.onContextMenu = function(i_e) {
	i_e.stopPropagation();
	g_cur_monitor = this.job.monitor;

	JobBlock.resetPanels(this.job.monitor);

	var selected_blocks = this.job.monitor.selected_blocks;

	// We should to deselect other jobs blocks,
	// as server can manipulate selected blocks of a one job only
	if (selected_blocks)
	{
		var blocks_to_deselect = [];

		for (var b = 0; b < selected_blocks.length; b++)
		{
			if (this.job.params.id == selected_blocks[b].job.params.id)
				continue;

			var blocks = selected_blocks[b].job.blocks;
			for (var j = 0; j < blocks.length; j++)
				if (blocks_to_deselect.indexOf(blocks[j]) == -1)
					blocks_to_deselect.push(blocks[j]);
		}

		for (var b = 0; b < blocks_to_deselect.length; b++)
			blocks_to_deselect[b].setSelected(false);
	}

	// If selected, we deselecting it and exit:
	if (this.selected)
	{
		this.setSelected(false);
		if (selected_blocks.length)
			selected_blocks[selected_blocks.length - 1].updatePanels();
		return;
	}

	// If jobs selected, select all selected jobs blocks,
	// as server can manipulate all blocks of a several blocks (not special blocks of a selected jobs)
	var jobs = this.job.monitor.getSelectedItems();
	if (jobs.length)
	{
		this.job.monitor.selectAll(false);
		for (var j = 0; j < jobs.length; j++)
			for (var b = 0; b < jobs[j].blocks.length; b++)
				jobs[j].blocks[b].setSelected(true);
	}

	// Select block:
	this.setSelected(true);

	// Update panels info:
	this.updatePanels();
};

JobBlock.prototype.updatePanels = function() {
	let elBlocks = this.job.monitor.elPanelR.m_elBlocks;
	elBlocks.m_cur_block = this;
	elBlocks.classList.add('active');

	elBlocks.m_elName.style.display = 'block';
	elBlocks.m_elName.textContent = this.params.name;

	for (let p in JobBlock.params)
	{
		if (this.params[p] == null)
			continue;

		let el = elBlocks.m_elParams[p];
		el.style.display = 'block';
		el.m_elValue.textContent = cm_ValueToString(this.params[p], JobBlock.params[p].type);
	}

	this.job.monitor.setPanelInfo(this.params.name);
};

JobBlock.resetPanels = function(i_monitor) {
	var elBlocks = i_monitor.elPanelR.m_elBlocks;
	if (elBlocks.m_cur_block == null)
		return;

	elBlocks.classList.remove('active');
	elBlocks.m_elName.style.display = 'none';
	for (var p in JobBlock.params)
	{
		var el = elBlocks.m_elParams[p];
		el.style.display = 'none';
		el.m_elValue.textContent = '';
	}

	i_monitor.resetPanelInfo();

	elBlocks.m_cur_block = null;
};

JobBlock.prototype.setSelected = function(i_select) {
	if (this.job.monitor.selected_blocks == null)
		this.job.monitor.selected_blocks = [];

	if (i_select)
	{
		if (this.selected)
			return;

		this.selected = true;
		this.element.classList.add('selected');

		this.job.monitor.selected_blocks.push(this);
	}
	else
	{
		if (this.selected != true)
			return;

		this.selected = false;
		this.element.classList.remove('selected');

		this.job.monitor.selected_blocks.splice(this.job.monitor.selected_blocks.indexOf(this), 1);
	}
};

JobBlock.deselectAll = function(i_monitor) {
	if (i_monitor.selected_blocks)
		while (i_monitor.selected_blocks.length)
			i_monitor.selected_blocks[0].setSelected(false);

	JobBlock.resetPanels(i_monitor);
};

JobBlock.setDialog = function(i_args) {
	var block = i_args.monitor.elPanelR.m_elBlocks.m_cur_block;

	if (block == null)
	{
		g_Error('No block(s) selected.');
		return;
	}

	new cgru_Dialog({
		'wnd': i_args.monitor.window,
		'receiver': block,
		'handle': 'setParameter',
		'param': i_args.name,
		'type': i_args.type,
		'value': block.params[i_args.name],
		'name': 'jobblock_parameter'
	});
};

JobBlock.prototype.setParameter = function(i_value, i_parameter) {
	var params = {};
	params[i_parameter] = i_value;
	this.action(null, params);
};

JobBlock.prototype.action = function(i_operation, i_params) {
	var jids = [];
	var bids = [];

	for (var b = 0; b < this.job.monitor.selected_blocks.length; b++)
	{
		var block = this.job.monitor.selected_blocks[b];

		var jid = block.job.params.id;
		if (jids.indexOf(jid) == -1)
			jids.push(jid);

		bids.push(block.params.block_num);
	}

	// If several jobs selected, we can manipulate only all their blocks.
	if (jids.length > 1)
		bids = [-1];

	nw_Action('jobs', jids, i_operation, i_params, bids);
};

JobBlock.prototype.constructFull = function() {
	this.elIcon.style.width = '48px';
	this.elIcon.style.height = '48px';
	this.elIcon.style.marginTop = '4px';
	this.element.style.marginLeft = '54px';

	this.elFull = document.createElement('div');
	this.element.appendChild(this.elFull);
	this.elFull.style.clear = 'both';

	this.elProgress = document.createElement('div');
	this.elFull.appendChild(this.elProgress);
	this.elProgress.classList.add('progress');

	this.elBarDone = document.createElement('div');
	this.elProgress.appendChild(this.elBarDone);
	this.elBarDone.classList.add('bar');
	this.elBarDone.classList.add('DON');
	this.elBarDone.style.height = '4px';
	this.elBarDone.style.cssFloat = 'left';

	this.elBarErr = document.createElement('div');
	this.elProgress.appendChild(this.elBarErr);
	this.elBarErr.classList.add('bar');
	this.elBarErr.classList.add('ERR');
	this.elBarErr.style.height = '4px';
	this.elBarErr.style.cssFloat = 'left';

	this.elBarRun = document.createElement('div');
	this.elProgress.appendChild(this.elBarRun);
	this.elBarRun.classList.add('bar');
	this.elBarRun.classList.add('RUN');
	this.elBarRun.style.height = '4px';
	//	this.elBarRun.style.cssFloat = 'left';

	this.elBarPercentage = document.createElement('div');
	this.elProgress.appendChild(this.elBarPercentage);
	this.elBarPercentage.classList.add('bar');
	this.elBarPercentage.classList.add('DON');
	this.elBarPercentage.style.height = '4px';

	this.elCanvas = document.createElement('div');
	this.elProgress.appendChild(this.elCanvas);
	this.elCanvas.style.height = Block_ProgressBarHeight + 'px';
	this.elCanvas.style.width = '100%';

	this.canvas = document.createElement('canvas');
	this.elCanvas.appendChild(this.canvas);
	this.canvas.width = Block_ProgressBarLength;
	this.canvas.height = Block_ProgressBarHeight;
	this.canvas.style.height = Block_ProgressBarHeight + 'px';
	this.canvas.style.width = '100%';

	this.elPercentage = cm_ElCreateText(this.elFull, 'Block Done Percentage');
	this.elTasksRun = cm_ElCreateText(this.elFull, 'Running Tasks Counter');
	this.elTasksCap = cm_ElCreateText(this.elFull, 'Running Capacity Total');
	this.elTasksDon = cm_ElCreateText(this.elFull, 'Done Tasks Counter');
	this.elTasksRdy = cm_ElCreateText(this.elFull, 'Ready Tasks Counter');
	this.elTasksSkp = cm_ElCreateText(this.elFull, 'Skipped Tasks Counter');
	this.elTasksWDP = cm_ElCreateText(this.elFull, 'Waitind Depends Tasks Counter');
	this.elTasksSus = cm_ElCreateText(this.elFull, 'Suspended Tasks Counter');
	this.elTasksWrn = cm_ElCreateText(this.elFull, 'Warning Tasks Counter');
	this.elTasksWrc = cm_ElCreateText(this.elFull, 'Waiting Reconnect Tasks Counter');
	this.elTasksErr = cm_ElCreateText(this.elFull, 'Error Tasks Counter');

	this.elSrvInfo = cm_ElCreateText(this.elFull, 'Server Information String');

	//	this.elTasksDon.classList.add('font-done');
	//	this.elTasksRdy.classList.add('font-ready');
	//	this.elTasksRun.classList.add('font-run');
	//	this.elTasksErr.classList.add('font-error');
	this.elTasksErr.classList.add('ERR');
	this.elTasksErr.style.display = 'none';

	this.elTickets = cm_ElCreateFloatText(this.elFull, 'right', 'Tasks Tickets');
	this.elErrHosts = cm_ElCreateFloatText(this.elFull, 'right');
};

JobBlock.prototype.constructBrief = function() {
	this.elIcon.style.width = '20px';
	this.elIcon.style.height = '20px';
	this.element.style.marginLeft = '24px';

	if (this.elFull)
		this.element.removeChild(this.elFull);
	this.elFull = null;
};

JobBlock.prototype.update = function(i_displayFull) {
	if (this.displayFull != i_displayFull)
	{
		if (i_displayFull)
			this.constructFull();
		else
			this.constructBrief();
	}
	this.displayFull = i_displayFull;

	if (this.params.name)
	{
		this.elName.innerHTML = '<b>' + this.params.name + '</b>';

		// Tasks tooltip:
		var tasks_title = 'Block is not numeric.';
		if (cm_CheckBlockFlag(this.params.flags, 'numeric'))
		{
			tasks_title = 'Frame Range:';
			tasks_title += '\nFirst: ' + this.params.frame_first;
			tasks_title += '\nLast: ' + this.params.frame_last;
			if (this.params.frames_inc > 1)
				tasks_title += '\nIncrement: ' + this.params.frames_inc;
			if (this.params.frames_per_task > 1)
				tasks_title += '\nPer Task: ' + this.params.frames_per_task;
			if ((this.params.sequential != null) && (this.params.sequential != 1))
				tasks_title += '\nSequential: ' + this.params.sequential;
		}
		this.elTasks.title = tasks_title;

		// Tasks brief text:
		var tasks = '';
		if (cm_IsPadawan())
		{
			tasks = 'Tasks[<b>' + this.tasks_num + '</b>]';
			if (cm_CheckBlockFlag(this.params.flags, 'numeric'))
			{
				tasks = 'Frames[<b>' + this.tasks_num + '</b>]';
				tasks += '( <b>' + this.params.frame_first + '</b> - <b>' + this.params.frame_last + '</b>';
				if (this.params.frames_inc > 1)
					tasks += ' / Increment:<b>' + this.params.frames_inc + '</b>';
				if (this.params.frames_per_task > 1)
					tasks += ' : PerTask:<b>' + this.params.frames_per_task + '</b>';
				if ((this.params.sequential != null) && (this.params.sequential != 1))
					tasks += ' % Sequential:<b>' + this.params.sequential + '</b>';
				tasks += ' )';
			}
		}
		else if (cm_IsJedi())
		{
			tasks = 'Tasks[<b>' + this.tasks_num + '</b>]';
			if (cm_CheckBlockFlag(this.params.flags, 'numeric'))
			{
				tasks = 'Frames[<b>' + this.tasks_num + '</b>]';
				tasks += '( <b>' + this.params.frame_first + '</b> - <b>' + this.params.frame_last + '</b>';
				if (this.params.frames_inc > 1)
					tasks += ' / Inc:<b>' + this.params.frames_inc + '</b>';
				if (this.params.frames_per_task > 1)
					tasks += ' : FPT:<b>' + this.params.frames_per_task + '</b>';
				if ((this.params.sequential != null) && (this.params.sequential != 1))
					tasks += ' % Seq:<b>' + this.params.sequential + '</b>';
				tasks += ' )';
			}
		}
		else
		{
			tasks = 't<b>' + this.tasks_num + '</b>';
			if (cm_CheckBlockFlag(this.params.flags, 'numeric'))
			{
				tasks = 'f<b>' + this.tasks_num + '</b>';
				tasks += '(<b>' + this.params.frame_first + '</b>-<b>' + this.params.frame_last + '</b>';
				if (this.params.frames_inc > 1)
					tasks += '/<b>' + this.params.frames_inc + '</b>';
				if (this.params.frames_per_task > 1)
					tasks += ':<b>' + this.params.frames_per_task + '</b>';
				if ((this.params.sequential != null) && (this.params.sequential != 1))
					tasks += '%<b>' + this.params.sequential + '</b>';
				tasks += ')';
			}
		}

		this.elTasks.innerHTML = tasks + ':';

		if (this.service != this.params.service)
		{
			this.service = this.params.service;
			this.elIcon.src = 'icons/software/' + this.service + '.png';
		}
		this.elIcon.title = this.service;

		// Depends title:
		var deps_title = '';
		if (this.params.depend_mask)
		{
			if (deps_title.length)
				deps_title += '\n';
			deps_title += 'Depend mask = \"' + this.params.depend_mask + '\".'
		}
		if (this.params.tasks_depend_mask)
		{
			if (deps_title.length)
				deps_title += '\n';
			deps_title += 'Tasks depend mask = \"' + this.params.tasks_depend_mask + '\".'
		}
		if (this.params.depend_sub_task)
		{
			if (deps_title.length)
				deps_title += '\n';
			deps_title += 'Subtasks depend.'
		}
		this.elDepends.title = deps_title;

		// Depends brief info:
		var deps = '';
		if (cm_IsPadawan())
		{
			if (this.params.depend_mask)
				deps += ' Depends(<b>' + this.params.depend_mask + '</b>)';
			if (this.params.tasks_depend_mask)
				deps += ' TasksDepends[<b>' + this.params.tasks_depend_mask + '</b>]';
			if (this.params.depend_sub_task)
				deps += ' [<b>Sub-Task Dependence</b>]';
		}
		else if (cm_IsJedi())
		{
			if (this.params.depend_mask)
				deps += ' Dep(<b>' + this.params.depend_mask + '</b>)';
			if (this.params.tasks_depend_mask)
				deps += ' TDep[<b>' + this.params.tasks_depend_mask + '</b>]';
			if (this.params.depend_sub_task)
				deps += ' [<b>Sub-Task</b>]';
		}
		else
		{
			if (this.params.depend_mask)
				deps += ' d(<b>' + this.params.depend_mask + '</b>)';
			if (this.params.tasks_depend_mask)
				deps += ' t[<b>' + this.params.tasks_depend_mask + '</b>]';
			if (this.params.depend_sub_task)
				deps += ' [<b>sub</b>]';
		}
		this.elDepends.innerHTML = deps;

		//
		// Errors solving:
		var eah = -1, eth = -1, ert = -1;
		if (this.params.errors_avoid_host)
			eah = this.params.errors_avoid_host;
		if (this.params.errors_task_same_host)
			eth = this.params.errors_task_same_host;
		if (this.params.errors_retries)
			ert = this.params.errors_retries;
		// Tooltip:
		var errTit = '';
		if ((eah != -1) || (eth != -1) || (ert != -1))
		{
			errTit = 'Errors Solving:';

			errTit += '\nAvoid Job Block: ' + eah;
			if (eah == 0)
				errTit += ' (unlimited)';
			else if (eah == -1)
				errTit += ' (user settings used)';

			errTit += '\nAvoid Task Same Host: ' + eth;
			if (eth == 0)
				errTit += ' (unlimited)';
			else if (eth == -1)
				errTit += ' (user settings used)';

			errTit += '\nTask Errors Retries: ' + ert;
			if (ert == 0)
				errTit += ' (unlimited)';
			else if (ert == -1)
				errTit += ' (user settings used)';
		}
		this.elErrSolving.title = errTit;

		let runtime_sum = cm_TimeStringFromSeconds(this.params.p_tasks_run_time_sum);
		let runtime_avg =
			cm_TimeStringFromSeconds(Math.round(this.params.p_tasks_run_time_sum / this.params.p_tasks_done));

		if (cm_IsPadawan())
		{
			this.elCapacity.innerHTML = 'Capacity[<b>' + this.params.capacity + '</b>]';

			var errTxt = '';
			if (eah != -1)
				errTxt += ' Avoid:<b>' + eah + '</b>';
			if (eth != -1)
				errTxt += ' Task:<b>' + eth + '</b>';
			if (ert != -1)
				errTxt += ' Retries:<b>' + ert + '</b>';
			if (errTxt.length)
				errTxt = 'ErrorsSolving(' + errTxt + ')';
			this.elErrSolving.innerHTML = errTxt;

			if ((this.params.errors_forgive_time != null) && (this.params.errors_forgive_time >= 0))
				this.elForgiveTime.innerHTML = 'ErrorsForgiveTime:<b>' +
					cm_TimeStringFromSeconds(this.params.errors_forgive_time) + '</b>';
			else
				this.elForgiveTime.textContent = '';

			if (this.params.task_max_run_time != null)
				this.elMaxRunTime.innerHTML =
					'TaskMaxRunTime:<b>' + cm_TimeStringFromSeconds(this.params.task_max_run_time) + '</b>';
			else
				this.elMaxRunTime.textContent = '';

			if (this.params.task_min_run_time != null)
				this.elMinRunTime.innerHTML =
					'TaskMinRunTime:<b>' + cm_TimeStringFromSeconds(this.params.task_min_run_time) + '</b>';
			else
				this.elMinRunTime.textContent = '';

			if (this.params.task_progress_change_timeout != null)
				this.elProgressTimeout.innerHTML = 'TaskProgessTimeout:<b>' +
					cm_TimeStringFromSeconds(this.params.task_progress_change_timeout) + '</b>';
			else
				this.elProgressTimeout.textContent = '';

			if (this.params.need_memory)
				this.elNeedMem.innerHTML =
					'Memory><b>' + (this.params.need_memory / 1024.0).toFixed(1) + 'Gb</b>';
			else
				this.elNeedMem.textContent = '';

			if (this.params.need_gpu_mem_mb)
				this.elNeed_GPU_Mem.innerHTML =
					'GPU Memory><b>' + (this.params.need_gpu_mem_mb / 1024.0).toFixed(1) + 'Gb</b>';
			else
				this.elNeed_GPU_Mem.textContent = '';

			if (this.params.need_cpu_freq_mgz)
				this.elNeed_CPU_Freq.innerHTML =
					'CPU Frequency><b>' + (this.params.need_cpu_freq_mgz / 1000.0).toFixed(1) + 'GHz</b>';
			else
				this.elNeed_CPU_Freq.textContent = '';

			if (this.params.need_cpu_cores)
				this.elNeed_CPU_Cores.innerHTML = 'CPU Cores><b>' + this.params.need_cpu_cores + '</b>';
			else
				this.elNeed_CPU_Cores.textContent = '';

			if (this.params.need_cpu_freq_cores)
				this.elNeed_CPU_FreqCores.innerHTML = 'CPU Frequency*Cores><b>' +
					(this.params.need_cpu_freq_cores / 1000.0).toFixed(1) + 'GHz</b>';
			else
				this.elNeed_CPU_FreqCores.textContent = '';

			if (this.params.need_hdd)
				this.elNeedHDD.innerHTML = 'HDDSpace><b>' + this.params.need_hdd + '</b>';
			else
				this.elNeedHDD.textContent = '';

			if (this.params.need_power)
				this.elNeedPower.innerHTML = 'Power><b>' + this.params.need_power + '</b>';
			else
				this.elNeedPower.textContent = '';

			if (this.params.need_properties)
				this.elNeedProperties.innerHTML = 'Properties(<b>' + this.params.need_properties + '</b>)';
			else
				this.elNeedProperties.textContent = '';

			let timings = '';
			if (this.params.p_tasks_run_time_sum && this.params.p_tasks_done)
				timings += ' Sum:<b>' + runtime_sum + '</b> / Average:<b>' + runtime_avg + '</b>';
			if (this.params.p_tasks_run_time_min > 0)
				timings += ' Min:<b>' + cm_TimeStringFromSeconds(this.params.p_tasks_run_time_min) + '</b>';
			if (this.params.p_tasks_run_time_max > 0)
				timings += ' Max:<b>' + cm_TimeStringFromSeconds(this.params.p_tasks_run_time_max) + '</b>';
			if (timings.length)
				this.elRunTime.innerHTML = 'Render Timings:' + timings;
		}
		else if (cm_IsJedi())
		{
			this.elCapacity.innerHTML = 'Cap[<b>' + this.params.capacity + '</b>]';

			var errTxt = '';
			if (eah != -1)
				errTxt += ' Block:<b>' + eah + '</b>';
			if (eth != -1)
				errTxt += ' Task:<b>' + eth + '</b>';
			if (ert != -1)
				errTxt += ' Retries:<b>' + ert + '</b>';
			if (errTxt.length)
				errTxt = 'ErrSlv(' + errTxt + ')';
			this.elErrSolving.innerHTML = errTxt;

			if ((this.params.errors_forgive_time != null) && (this.params.errors_forgive_time >= 0))
				this.elForgiveTime.innerHTML =
					'ErrForgive:<b>' + cm_TimeStringFromSeconds(this.params.errors_forgive_time) + '</b>';
			else
				this.elForgiveTime.textContent = '';

			if (this.params.task_max_run_time != null)
				this.elMaxRunTime.innerHTML =
					'MaxRun:<b>' + cm_TimeStringFromSeconds(this.params.task_max_run_time) + '</b>';
			else
				this.elMaxRunTime.textContent = '';

			if (this.params.task_min_run_time != null)
				this.elMinRunTime.innerHTML =
					'MinRun:<b>' + cm_TimeStringFromSeconds(this.params.task_min_run_time) + '</b>';
			else
				this.elMinRunTime.textContent = '';

			if (this.params.task_progress_change_timeout != null)
				this.elProgressTimeout.innerHTML = 'ProgessTimeout:<b>' +
					cm_TimeStringFromSeconds(this.params.task_progress_change_timeout) + '</b>';
			else
				this.elProgressTimeout.textContent = '';

			if (this.params.need_memory)
				this.elNeedMem.innerHTML =
					'Mem><b>' + (this.params.need_memory / 1024.0).toFixed(1) + 'Gb</b>';
			else
				this.elNeedMem.textContent = '';

			if (this.params.need_gpu_mem_mb)
				this.elNeed_GPU_Mem.innerHTML =
					'GPU Mem><b>' + (this.params.need_gpu_mem_mb / 1024.0).toFixed(1) + 'Gb</b>';
			else
				this.elNeed_GPU_Mem.textContent = '';

			if (this.params.need_cpu_freq_mgz)
				this.elNeed_CPU_Freq.innerHTML =
					'CPU Freq><b>' + (this.params.need_cpu_freq_mgz / 1000.0).toFixed(1) + 'GHz</b>';
			else
				this.elNeed_CPU_Freq.textContent = '';

			if (this.params.need_cpu_cores)
				this.elNeed_CPU_Cores.innerHTML = 'CPU Cores><b>' + this.params.need_cpu_cores + '</b>';
			else
				this.elNeed_CPU_Cores.textContent = '';

			if (this.params.need_cpu_freq_cores)
				this.elNeed_CPU_FreqCores.innerHTML =
					'CPU Freq*Cores><b>' + (this.params.need_cpu_freq_cores / 1000.0).toFixed(1) + 'GHz</b>';
			else
				this.elNeed_CPU_FreqCores.textContent = '';

			if (this.params.need_hdd)
				this.elNeedHDD.innerHTML = 'HDD><b>' + this.params.need_hdd + '</b>';
			else
				this.elNeedHDD.textContent = '';

			if (this.params.need_power)
				this.elNeedPower.innerHTML = 'Pow><b>' + this.params.need_power + '</b>';
			else
				this.elNeedPower.textContent = '';

			if (this.params.need_properties)
				this.elNeedProperties.innerHTML = 'Props(<b>' + this.params.need_properties + '</b>)';
			else
				this.elNeedProperties.textContent = '';

			let timings = '';
			if (this.params.p_tasks_run_time_sum && this.params.p_tasks_done)
				timings += ' Sum:<b>' + runtime_sum + '</b> / Average:<b>' + runtime_avg + '</b>';
			if (this.params.p_tasks_run_time_min > 0)
				timings += ' Min:<b>' + cm_TimeStringFromSeconds(this.params.p_tasks_run_time_min) + '</b>';
			if (this.params.p_tasks_run_time_max > 0)
				timings += ' Max:<b>' + cm_TimeStringFromSeconds(this.params.p_tasks_run_time_max) + '</b>';
			if (timings.length)
				this.elRunTime.innerHTML = 'Timings:' + timings;
		}
		else
		{
			this.elCapacity.innerHTML = '[<b>' + this.params.capacity + '</b>]';

			var errTxt = '';
			if ((eah != -1) || (eth != -1) || (ert != -1))
			{
				errTxt = 'e:';
				errTxt += '<b>' + eah + '</b>b';
				errTxt += ',<b>' + eth + '</b>t';
				errTxt += ',<b>' + ert + '</b>r';
			}
			this.elErrSolving.innerHTML = errTxt;

			if ((this.params.errors_forgive_time != null) && (this.params.errors_forgive_time >= 0))
				this.elForgiveTime.innerHTML =
					'f<b>' + cm_TimeStringFromSeconds(this.params.errors_forgive_time) + '</b>';
			else
				this.elForgiveTime.textContent = '';

			if (this.params.task_max_run_time != null)
				this.elMaxRunTime.innerHTML =
					'Mrt<b>' + cm_TimeStringFromSeconds(this.params.task_max_run_time) + '</b>';
			else
				this.elMaxRunTime.textContent = '';

			if (this.params.task_min_run_time != null)
				this.elMinRunTime.innerHTML =
					'mrt<b>' + cm_TimeStringFromSeconds(this.params.task_min_run_time) + '</b>';
			else
				this.elMinRunTime.textContent = '';

			if (this.params.task_progress_change_timeout != null)
				this.elProgressTimeout.innerHTML =
					'tpt:<b>' + cm_TimeStringFromSeconds(this.params.task_progress_change_timeout) + '</b>';
			else
				this.elProgressTimeout.textContent = '';

			if (this.params.need_memory)
				this.elNeedMem.innerHTML = 'm><b>' + (this.params.need_memory / 1024.0).toFixed(1) + '</b>';
			else
				this.elNeedMem.textContent = '';

			if (this.params.need_gpu_mem_mb)
				this.elNeed_GPU_Mem.innerHTML =
					'gm><b>' + (this.params.need_gpu_mem_mb / 1024.0).toFixed(1) + 'Gb</b>';
			else
				this.elNeed_GPU_Mem.textContent = '';

			if (this.params.need_cpu_freq_mgz)
				this.elNeed_CPU_Freq.innerHTML =
					'cf><b>' + (this.params.need_cpu_freq_mgz / 1000.0).toFixed(1) + 'GHz</b>';
			else
				this.elNeed_CPU_Freq.textContent = '';

			if (this.params.need_cpu_cores)
				this.elNeed_CPU_Cores.innerHTML = 'cr><b>' + this.params.need_cpu_cores + '</b>';
			else
				this.elNeed_CPU_Cores.textContent = '';

			if (this.params.need_cpu_freq_cores)
				this.elNeed_CPU_FreqCores.innerHTML =
					'cfc><b>' + (this.params.need_cpu_freq_cores / 1000.0).toFixed(1) + 'GHz</b>';
			else
				this.elNeed_CPU_FreqCores.textContent = '';

			if (this.params.need_hdd)
				this.elNeedHDD.innerHTML = 'h><b>' + this.params.need_hdd + '</b>';
			else
				this.elNeedHDD.textContent = '';

			if (this.params.need_power)
				this.elNeedPower.innerHTML = 'p><b>' + this.params.need_power + '</b>';
			else
				this.elNeedPower.textContent = '';

			if (this.params.need_properties)
				this.elNeedProperties.innerHTML = '<b>' + this.params.need_properties + '</b>';
			else
				this.elNeedProperties.textContent = '';

			if (this.params.p_tasks_run_time_sum && this.params.p_tasks_done)
				this.elRunTime.innerHTML = 'rt:s<b>' + runtime_sum + '</b>/a<b>' + runtime_avg + '</b>';

			let timings = '';
			if (this.params.p_tasks_run_time_sum && this.params.p_tasks_done)
				timings += 's<b>' + runtime_sum + '</b>/a<b>' + runtime_avg + '</b>';
			if (this.params.p_tasks_run_time_min > 0)
				timings += ':(<b>' + cm_TimeStringFromSeconds(this.params.p_tasks_run_time_min) + '</b>';
			if (this.params.p_tasks_run_time_max > 0)
				timings += '-<b>' + cm_TimeStringFromSeconds(this.params.p_tasks_run_time_max) + '</b>)';
			if (timings.length)
				this.elRunTime.innerHTML = 'rt:' + timings;
		}

		if ((this.params.p_tasks_run_time_sum && this.params.p_tasks_done) ||
			(this.params.p_tasks_run_time_min > 0) || (this.params.p_tasks_run_time_max > 0))
		{
			let timings = 'Tasks Running Time:';
			if (this.params.p_tasks_run_time_sum && this.params.p_tasks_done)
				timings += '\nTotal Sum: ' + runtime_sum + '\nAverage per task: ' + runtime_avg;
			if (this.params.p_tasks_run_time_min > 0)
				timings += '\nMinimum: ' + cm_TimeStringFromSeconds(this.params.p_tasks_run_time_min);
			if (this.params.p_tasks_run_time_max > 0)
				timings += '\nMaximum: ' + cm_TimeStringFromSeconds(this.params.p_tasks_run_time_max);
			this.elRunTime.title = timings;
			this.elRunTime.style.display = 'inline';
		}
		else
		{
			this.elRunTime.style.display = 'none';
			this.elRunTime.textContent = '';
			this.elRunTime.title = '';
		}

		this.updateTickets();
	}

	if (this.displayFull)
	{
		var percentage = 0;
		if (this.params.p_percentage)
			percentage = this.params.p_percentage;
		this.elPercentage.innerHTML = '<b>' + percentage + '%</b>';

		// This 4 parameters we need even if they are null in params
		// ( default values are not wrote to json )
		var tasks_done = 0;
		var tasks_rdy = 0;
		var tasks_run = 0;
		var tasks_err = 0;
		if (this.params.p_tasks_done)
			tasks_done = this.params.p_tasks_done;
		if (this.params.p_tasks_ready)
			tasks_rdy = this.params.p_tasks_ready;
		if (this.params.running_tasks_counter)
			tasks_run = this.params.running_tasks_counter;
		if (this.params.p_tasks_error)
			tasks_err = this.params.p_tasks_error;

		if (cm_IsPadawan())
		{
			this.elTasksDon.innerHTML = 'Done:<b>' + tasks_done + '</b>';
			this.elTasksRdy.innerHTML = 'Ready:<b>' + tasks_rdy + '</b>';

			if (tasks_run)
				this.elTasksRun.innerHTML = 'Running:<b>' + tasks_run + '</b>';
			else
				this.elTasksRun.textContent = '';

			if (this.params.running_capacity_total)
				this.elTasksCap.innerHTML =
					'Capacity:<b>' + cm_ToKMG(this.params.running_capacity_total) + '</b>';
			else
				this.elTasksCap.textContent = '';

			if (tasks_err)
				this.elTasksErr.innerHTML = 'Errors:<b>' + tasks_err + '</b>';

			if (this.params.p_tasks_skipped)
				this.elTasksSkp.innerHTML = 'Skipped:<b>' + this.params.p_tasks_skipped + '</b>';
			else
				this.elTasksSkp.textContent = '';

			if (this.params.p_tasks_waitdep)
				this.elTasksWDP.innerHTML = 'Wait Depends:<b>' + this.params.p_tasks_waitdep + '</b>';
			else
				this.elTasksWDP.textContent = '';

			if (this.params.p_tasks_suspended)
				this.elTasksSus.innerHTML = 'Suspended:<b>' + this.params.p_tasks_suspended + '</b>';
			else
				this.elTasksSus.textContent = '';

			if (this.params.p_tasks_warning)
				this.elTasksWrn.innerHTML = 'Warnings:<b>' + this.params.p_tasks_warning + '</b>';
			else
				this.elTasksWrn.textContent = '';

			if (this.params.p_tasks_waitrec)
				this.elTasksWrc.innerHTML = 'WaitingReconnect:<b>' + this.params.p_tasks_waitrec + '</b>';
			else
				this.elTasksWrc.textContent = '';

			var he_txt = '';
			if (this.params.p_error_hosts)
			{
				he_txt = 'ErrorHosts: <b>' + this.params.p_error_hosts + '</b>';
				if (this.params.p_avoid_hosts)
					he_txt += ' / <b>' + this.params.p_avoid_hosts + '</b> Avoiding';
			}
			this.elErrHosts.innerHTML = he_txt;
		}
		else if (cm_IsJedi())
		{
			this.elTasksDon.innerHTML = 'Don:<b>' + tasks_done + '</b>';
			this.elTasksRdy.innerHTML = 'Rdy:<b>' + tasks_rdy + '</b>';

			if (tasks_run)
				this.elTasksRun.innerHTML = 'Run:<b>' + tasks_run + '</b>';
			else
				this.elTasksRun.textContent = '';

			if (this.params.running_capacity_total)
				this.elTasksCap.innerHTML = 'Cap:<b>' + cm_ToKMG(this.params.running_capacity_total) + '</b>';
			else
				this.elTasksCap.textContent = '';

			if (tasks_err)
				this.elTasksErr.innerHTML = 'Err:<b>' + tasks_err + '</b>';

			if (this.params.p_tasks_skipped)
				this.elTasksSkp.innerHTML = 'Skp:<b>' + this.params.p_tasks_skipped + '</b>';
			else
				this.elTasksSkp.textContent = '';

			if (this.params.p_tasks_waitdep)
				this.elTasksWDP.innerHTML = 'WaitDep:<b>' + this.params.p_tasks_waitdep + '</b>';
			else
				this.elTasksWDP.textContent = '';

			if (this.params.p_tasks_suspended)
				this.elTasksSus.innerHTML = 'Suspended:<b>' + this.params.p_tasks_suspended + '</b>';
			else
				this.elTasksSus.textContent = '';

			if (this.params.p_tasks_warning)
				this.elTasksWrn.innerHTML = 'Wrn:<b>' + this.params.p_tasks_warning + '</b>';
			else
				this.elTasksWrn.textContent = '';

			if (this.params.p_tasks_waitrec)
				this.elTasksWrc.innerHTML = 'WRC:<b>' + this.params.p_tasks_waitrec + '</b>';
			else
				this.elTasksWrc.textContent = '';

			var he_txt = '';
			if (this.params.p_error_hosts)
			{
				he_txt = 'ErrHosts: <b>' + this.params.p_error_hosts + '</b>';
				if (this.params.p_avoid_hosts)
					he_txt += ' / <b>' + this.params.p_avoid_hosts + '</b> Avoid';
			}
			this.elErrHosts.innerHTML = he_txt;
		}
		else
		{
			this.elTasksDon.innerHTML = 'd<b>' + tasks_done + '</b>';
			this.elTasksRdy.innerHTML = 'r<b>' + tasks_rdy + '</b>';

			if (tasks_run)
				this.elTasksRun.innerHTML = 'run<b>' + tasks_run + '</b>';
			else
				this.elTasksRun.textContent = '';

			if (this.params.running_capacity_total)
				this.elTasksCap.innerHTML = 'c<b>' + cm_ToKMG(this.params.running_capacity_total) + '</b>';
			else
				this.elTasksCap.textContent = '';

			if (tasks_err)
				this.elTasksErr.innerHTML = 'e<b>' + tasks_err + '</b>';

			if (this.params.p_tasks_skipped)
				this.elTasksSkp.innerHTML = 's<b>' + this.params.p_tasks_skipped + '</b>';
			else
				this.elTasksSkp.textContent = '';

			if (this.params.p_tasks_waitdep)
				this.elTasksWDP.innerHTML = 'wdp:<b>' + this.params.p_tasks_waitdep + '</b>';
			else
				this.elTasksWDP.textContent = '';

			if (this.params.p_tasks_suspended)
				this.elTasksSus.innerHTML = 'sus:<b>' + this.params.p_tasks_suspended + '</b>';
			else
				this.elTasksSus.textContent = '';

			if (this.params.p_tasks_warning)
				this.elTasksWrn.innerHTML = 'w<b>' + this.params.p_tasks_warning + '</b>';
			else
				this.elTasksWrn.textContent = '';

			if (this.params.p_tasks_waitrec)
				this.elTasksWrc.innerHTML = 'wrc<b>' + this.params.p_tasks_waitrec + '</b>';
			else
				this.elTasksWrc.textContent = '';

			var he_txt = '';
			if (this.params.p_error_hosts)
			{
				he_txt = 'rh:<b>' + this.params.p_error_hosts + '</b>';
				if (this.params.p_avoid_hosts)
					he_txt += '/<b>' + this.params.p_avoid_hosts + '</b>a';
			}
			this.elErrHosts.innerHTML = he_txt;
		}

		if (this.params.srv_info)
			this.elSrvInfo.innerHTML = '<b>' + this.params.srv_info + '</b>';
		else
			this.elSrvInfo.textContent = '';

		// Show/Hire error hosts counter as it has a special style.
		// And set an empty string (like on other counters) is not enough.
		if (tasks_err)
			this.elTasksErr.style.display = 'inline';
		else
			this.elTasksErr.style.display = 'none';

		var he_tit = '';
		this.elErrHosts.classList.remove('ERR');
		if (this.params.p_error_hosts)
		{
			he_tit = 'Error Hosts: ' + this.params.p_error_hosts;
			if (this.params.p_avoid_hosts)
			{
				he_tit += '\nAvoiding Hosts: ' + this.params.p_avoid_hosts;
				this.elErrHosts.classList.add('ERR');
			}
		}
		this.elErrHosts.title = he_tit;

		this.elBarPercentage.style.width = percentage + '%';
		this.elBarDone.style.width = Math.floor(100 * tasks_done / this.tasks_num) + '%';
		this.elBarErr.style.width = Math.ceil(100 * tasks_err / this.tasks_num) + '%';
		this.elBarRun.style.width = Math.ceil(100 * tasks_run / this.tasks_num) + '%';

		if (this.params.p_progressbar)
		{
			var ctx = this.canvas.getContext('2d');
			ctx.clearRect(0, 0, Block_ProgressBarLength, Block_ProgressBarHeight);
			ctx.lineWidth = 1;
			ctx.lineCap = 'square';
			for (var i = 0; i < Block_ProgressBarLength; i++)
			{
				var rgb = '#000';
				switch (this.params.p_progressbar.charAt(i))
				{
					case 'r': continue;	 // RDY
					case 'D': rgb = BarDONrgb; break;
					case 'S': rgb = BarSKPrgb; break;  // SKP
					case 'G': rgb = BarDWRrgb; break;  // DON | WRN
					case 'U': rgb = BarSUSrgb; break;  // SUS
					case 'W': rgb = BarWDPrgb; break;  // WDP
					case 'R': rgb = BarRUNrgb; break;  // RUN
					case 'N': rgb = BarRWRrgb; break;  // RUN | WRN
					case 'Y': rgb = BarRERrgb; break;  // RER
					case 'E': rgb = BarERRrgb; break;  // ERR
					case 'C': rgb = BarWRCrgb; break;  // WRC
				}
				ctx.strokeStyle = rgb;
				ctx.beginPath();
				ctx.moveTo(i + .5, 0);
				ctx.lineTo(i + .5, Block_ProgressBarHeight);
				ctx.stroke();
			}
		}
	}
};

JobBlock.prototype.updateTickets =
	function() {
	this.elTickets.textContent = '';

	var tickets = this.params.tickets;
	if (tickets == null)
		return;

	for (let tk in tickets)
	{
		let elTk = document.createElement('div');
		elTk.classList.add('ticket');
		this.elTickets.appendChild(elTk);

		let label = '';

		if (cm_TicketsIcons.includes(tk + '.png'))
		{
			let elIcon = document.createElement('img');
			elTk.appendChild(elIcon);
			elIcon.src = ('/icons/tickets/' + tk + '.png');
		}
		else
			label += tk;

		let elLabel = document.createElement('div');
		elTk.appendChild(elLabel);
		elLabel.classList.add('label');
		elLabel.textContent = label + 'x' + tickets[tk];
	}
}

	JobNode.prototype.updatePanels = function(i_selected) {
	var elPanelR = this.monitor.elPanelR;

	// Blocks:
	JobBlock.deselectAll(this.monitor);
	elPanelR.m_elBlocks.classList.remove('active');

	// Errors control buttons:
	var errors = 0;
	var avoids = 0;
	for (var b = 0; b < this.params.blocks.length; b++)
	{
		var block = this.params.blocks[b];
		if (block.p_error_hosts)
			errors += block.p_error_hosts;
		if (block.p_avoid_hosts)
			avoids += block.p_avoid_hosts;
	}
	if (errors || avoids || this.state.ERR)
	{
		this.monitor.ctrl_btns.errors.classList.remove('hide_childs');
		if (avoids || this.state.ERR)
			this.monitor.ctrl_btns.errors.classList.add('errors');
	}
	else
	{
		this.monitor.ctrl_btns.errors.classList.remove('active');
	}


	// Info:
	var info = '<p>ID:' + this.params.id + ' S/N:' + this.params.serial + '</p>';
	info += '<p>Branch: ' + this.params.branch + '</p>';
	info += '<p>Created: ' + cm_DateTimeStrFromSec(this.params.time_creation) + '</p>';
	if (this.params.time_started)
		info += '<p>Started: ' + cm_DateTimeStrFromSec(this.params.time_started) + '</p>';
	if (this.params.time_done)
		info += '<p>Finished: ' + cm_DateTimeStrFromSec(this.params.time_done) + '</p>';
	if (i_selected && i_selected.length && (i_selected.length > 1))
		info += '<p>' + JobNode.getMultiSelectionInfo(i_selected) + '</p>';
	this.monitor.setPanelInfo(info);


	// Folders:
	var elFolders = elPanelR.m_elFolders;
	elFolders.m_elFolders = [];
	var folders = this.params.folders;

	// console.log(JSON.stringify( folders));
	if ((folders == null) || (folders.length == 0))
	{
		return;
	}

	var rules_link = folders.output;

	for (var name in folders)
	{
		if (rules_link == null)
			rules_link = folders[name];

		var path = cgru_PM(folders[name]);

		var elDiv = document.createElement('div');
		elFolders.appendChild(elDiv);
		elFolders.m_elFolders.push(elDiv);
		elDiv.classList.add('param');
		elDiv.classList.add('folder');

		var elCmdExec = cgru_CmdExecCreateOpen({'parent': elDiv, 'path': path});

		var elLabel = document.createElement('div');
		elDiv.appendChild(elLabel);
		elLabel.classList.add('label');
		elLabel.textContent = name;

		var elValue = document.createElement('div');
		elDiv.appendChild(elValue);
		elValue.classList.add('value');
		elValue.textContent = path;
	}

	var rules_href = cgru_RulesLink(rules_link);
	if (rules_href)
	{
		elFolders.m_elRules.style.display = 'block';
		elFolders.m_elRules.href = cgru_RulesLink(rules_link);
	}
};

JobNode.getMultiSelectionInfo = function(i_selected) {
	let jobs_count = 0;
	let time_started_min = 0;
	let time_finished_max = 0;
	let time_run_sum = 0;
	let time_run_count = 0;

	for (let job of i_selected)
	{
		if (job.node_type != 'jobs')
			continue;

		jobs_count++;

		if (job.params.time_started &&
			((job.params.time_started < time_started_min) || (time_started_min == 0)))
			time_started_min = job.params.time_started;

		if ((job.state.DON == true) && (job.params.time_done > time_finished_max))
			time_finished_max = job.params.time_done;

		if (job.state.DON == true)
		{
			time_run_sum += job.params.time_done - job.params.time_started;
			time_run_count++;
		}
	}

	let info = '<b><i>' + jobs_count + ' Jobs Selected</i></b>';

	if (time_started_min)
		info += '<br>Started First: <b>' + cm_DateTimeStrFromSec(time_started_min) + '</b>';
	if (time_finished_max)
		info += '<br>Finished Last: <b>' + cm_DateTimeStrFromSec(time_finished_max) + '</b>';
	if (time_run_count > 1)
	{
		info += '<br>Time Run Avg: <b>' + cm_TimeStringFromSeconds(time_run_sum / time_run_count) + '</b>';
		info += '<br>Time Run Sum: <b>' + cm_TimeStringFromSeconds(time_run_sum) + '</b>';
	}

	return info;
};

JobNode.resetPanels = function(i_monitor) {
	i_monitor.ctrl_btns.errors.classList.remove('errors');
	i_monitor.ctrl_btns.errors.classList.add('hide_childs');

	// Remove folders items:
	var elFolders = i_monitor.elPanelR.m_elFolders;
	if (elFolders.m_elFolders)
		for (var i = 0; i < elFolders.m_elFolders.length; i++)
			elFolders.removeChild(elFolders.m_elFolders[i]);
	elFolders.m_elFolders = [];
	elFolders.m_elRules.style.display = 'none';
};

JobNode.createPanels = function(i_monitor) {
	// Left Panel:


	// Errors:
	var acts = {};
	acts.error_hosts = {'label': 'Get', 'handle': 'mh_Get', 'tooltip': 'Show error hosts.'};
	acts.reset_error_hosts = {'label': 'Reset', 'handle': 'mh_Oper', 'tooltip': 'Reset error hosts.'};
	acts.restart_errors = {'label': 'Restart', 'handle': 'mh_Oper', 'tooltip': 'Restart error tasks.'};
	i_monitor.createCtrlBtn(
		{'name': 'errors', 'label': 'Errors', 'tooltip': 'Error tasks and hosts.', 'sub_menu': acts});


	// Restart:
	var acts = {};
	acts.restart = {'label': 'All', 'tooltip': 'Restart all tasks.'};
	acts.restart_pause = {'label': 'All&Pause', 'tooltip': 'Restart all and pause.'};
	acts.restart_errors = {'label': 'Errors', 'tooltip': 'Restart error tasks.'};
	acts.restart_running = {'label': 'Running', 'tooltip': 'Restart running tasks.'};
	acts.restart_skipped = {'label': 'Skipped', 'tooltip': 'Restart skipped tasks.'};
	acts.restart_done = {'label': 'Done', 'tooltip': 'Restart done task.'};
	i_monitor.createCtrlBtn(
		{'name': 'restart_tasks', 'label': 'Restart', 'tooltip': 'Restart job tasks.', 'sub_menu': acts});


	// Move:
	if (false == g_VISOR())
	{
		var acts = {};
		acts.move_jobs_top = {'label': 'Top', 'tooltip': 'Move jobs top.'};
		acts.move_jobs_up = {'label': 'Up', 'tooltip': 'Move jobs up.'};
		acts.move_jobs_down = {'label': 'Down', 'tooltip': 'Move jobs down.'};
		acts.move_jobs_bottom = {'label': 'Bottom', 'tooltip': 'Move jobs bottom.'};
		i_monitor.createCtrlBtn({
			'name': 'move_jobs',
			'label': 'Move',
			'tooltip': 'Move jobs.',
			'sub_menu': acts,
			'handle': 'moveJobs'
		});
	}

	// Actions:
	var acts = {};
	acts.start = {'label': 'START', 'tooltip': 'Start job.'};
	acts.pause = {'label': 'PAUSE', 'tooltip': 'Pause job.'};
	acts.stop = {'label': 'STOP', 'tooltip': 'Double click to stop job running tasks.', 'ondblclick': true};
	acts.listen =
		{'label': 'LISTEN', 'tooltip': 'Double click to listen job.', 'ondblclick': true, 'handle': 'listen'};
	acts.delete = {'label': 'DELETE', 'tooltip': 'Double click to delete job(s).', 'ondblclick': true};
	if (false == g_VISOR())
	{
		acts.deldone = {
			'label': 'DELDONE',
			'tooltip': 'Double click to delete all done jobs.',
			'ondblclick': true,
			'always_active': true,
			'handle': 'delDoneJobs'
		};
	}
	i_monitor.createCtrlBtns(acts);


	// Right Panel:
	var elPanelR = i_monitor.elPanelR;


	// Folders:
	var el = document.createElement('div');
	elPanelR.appendChild(el);
	el.classList.add('section');
	el.classList.add('folders');
	elPanelR.m_elFolders = el;
	var el = document.createElement('a');
	elPanelR.m_elFolders.appendChild(el);
	elPanelR.m_elFolders.m_elRules = el;
	el.classList.add('rules_link');
	el.classList.add('caption');
	el.title = 'Open RULES shot in a new window(tab).';
	el.textContent = 'RULES';
	el.setAttribute('target', '_blank');
	var el = document.createElement('div');
	elPanelR.m_elFolders.appendChild(el);
	el.textContent = 'Folders';
	el.classList.add('caption');


	// Work:
	work_CreatePanels(i_monitor, 'jobs');


	// Blocks:
	var el = document.createElement('div');
	elPanelR.appendChild(el);
	el.classList.add('section');
	el.classList.add('blocks');
	elPanelR.m_elBlocks = el;
	var elCaption = document.createElement('div');
	elPanelR.m_elBlocks.appendChild(elCaption);
	elCaption.textContent = 'Blocks';
	elCaption.classList.add('caption');

	var elName = document.createElement('div');
	elPanelR.m_elBlocks.appendChild(elName);
	elPanelR.m_elBlocks.m_elName = elName;
	elName.classList.add('name');
	elName.style.display = 'none';

	elPanelR.m_elBlocks.m_elParams = {};
	for (var p in JobBlock.params)
	{
		var elDiv = document.createElement('div');
		elPanelR.m_elBlocks.appendChild(elDiv);
		elPanelR.m_elBlocks.m_elParams[p] = elDiv;
		elDiv.classList.add('param');
		elDiv.style.display = 'none';

		var elLabel = document.createElement('div');
		elDiv.appendChild(elLabel);
		elLabel.classList.add('label');
		elLabel.textContent = JobBlock.params[p].label;

		var elValue = document.createElement('div');
		elDiv.appendChild(elValue);
		elDiv.m_elValue = elValue;
		elValue.classList.add('value');

		var el = elDiv;
		el.title = 'Double click to edit.';
		el.monitor = i_monitor;
		el.name = p;
		el.param = JobBlock.params[p];
		el.monitor = i_monitor;
		el.ondblclick = function(e) {
			var el = e.currentTarget;
			JobBlock.setDialog({'name': el.name, 'type': el.param.type, 'monitor': el.monitor});
		}
	}

	var el = elCaption;
	el.title = 'Click to edit all parameters.';
	el.m_elBlocks = elPanelR.m_elBlocks;
	el.onclick = function(e) {
		var el = e.currentTarget;
		if (el.m_elBlocks.classList.contains('active') != true)
			return false;
		var elParams = el.m_elBlocks.m_elParams;
		for (var p in elParams)
			elParams[p].style.display = 'block';
		return false;
	}
};

JobNode.moveJobs = function(i_args) {
	nw_Action('users', [g_uid], {'type': i_args.name, 'jids': i_args.monitor.getSelectedIds()});
	i_args.monitor.info('Moving Jobs');
};

JobNode.delDoneJobs = function(i_args) {
	var ids = [];
	for (var i = 0; i < i_args.monitor.items.length; i++)
	{
		var job = i_args.monitor.items[i];
		if (job.state.DON == true)
			ids.push(job.params.id);
	}

	if (ids.length == 0)
	{
		g_Error('No done jobs.');
		return;
	}

	nw_Action('jobs', ids, {'type': 'delete'});
	i_args.monitor.info('Deleting all done jobs.');
};

JobNode.listen = function(i_args) {
	var job = i_args.monitor.cur_item;
	if (job == null)
	{
		g_Error('No task selected to listen.');
		return;
	}

	var args = {};
	args.job = job.params.id;
	args.parent_window = i_args.monitor.window;

	listen_Start(args);
};

JobNode.params_job = {
	depend_mask /************/: {'type': 'reg', 'label': 'Depend Mask'},
	depend_mask_global /*****/: {'type': 'reg', 'label': 'Global Depend Mask'},
	time_wait /**************/: {'type': 'tim', 'label': 'Time Wait'},
	need_os /****************/: {'type': 'reg', 'label': 'OS Needed'},
	need_properties /********/: {'type': 'reg', 'label': 'Need Properties'},
	time_life /**************/: {'type': 'hrs', 'label': 'Life Time'},
	hidden /*****************/: {'type': 'bl1', 'label': 'Hidden'},
	ppa /********************/: {'type': 'bl1', 'label': 'Preview Pending Approval'},
	user_name /**************/: {'type': 'str', 'label': 'Owner', 'permissions': 'visor'}
};

JobNode.createParams = function() {
	if (JobNode.params_created)
		return;

	JobNode.params = {};
	for (var p in work_params)
		JobNode.params[p] = work_params[p];
	for (var p in JobNode.params_job)
		JobNode.params[p] = JobNode.params_job[p];

	JobNode.params_created = true;
};

JobNode.view_opts = {
	jobs_thumbs_num:
		{'type': 'num', 'label': 'THUMBNAIS COUNT', 'tooltip': 'Thumbnails quantity.', 'default': 12},
	jobs_thumbs_height:
		{'type': 'num', 'label': 'THUMBNAIS SIZE', 'tooltip': 'Thumbnails height.', 'default': 100}
};


JobBlock.params = {
	capacity /***************/: {'type': 'num', 'label': 'Capacity'},
	sequential /*************/: {'type': 'num', 'label': 'Sequential'},
	max_running_tasks /******/: {'type': 'num', 'label': 'Max Running Tasks'},
	max_running_tasks_per_host: {'type': 'num', 'label': 'Max Run Tasks Per Host'},
	errors_retries /*********/: {'type': 'num', 'label': 'Errors Retries'},
	errors_avoid_host /******/: {'type': 'num', 'label': 'Errors Avoid Host'},
	errors_task_same_host /**/: {'type': 'num', 'label': 'Errors Task Same Host'},
	errors_forgive_time /****/: {'type': 'hrs', 'label': 'Errors Forgive Time'},
	task_max_run_time /******/: {'type': 'hrs', 'label': 'Task Max Run Time'},
	task_min_run_time /******/: {'type': 'num', 'label': 'Task Min Run Time'},
	task_progress_change_timeout: {'type': 'hrs', 'label': 'Task Progress Change Timeout'},
	hosts_mask /*************/: {'type': 'reg', 'label': 'Hosts Mask'},
	hosts_mask_exclude /*****/: {'type': 'reg', 'label': 'Exclude Hosts Mask'},
	depend_mask /************/: {'type': 'reg', 'label': 'Depend Mask'},
	tasks_depend_mask /******/: {'type': 'reg', 'label': 'Tasks Depend Mask'},
	need_memory: {'type': 'mib', 'label': 'Needed Free Memory GB'},
	need_gpu_mem_mb: {'type': 'mib', 'label': 'Needed Free GPU Memory GB'},
	need_cpu_freq_mgz: {'type': 'meg', 'label': 'Needed GPU Frequency GHz'},
	need_cpu_cores: {'type': 'num', 'label': 'Needed GPU Cores'},
	need_cpu_freq_cores: {'type': 'meg', 'label': 'Needed GPU Freq*Cores GHz'},
	need_hdd: {'type': 'gib', 'label': 'Needed Free HDD Space GB'},
	need_properties /********/: {'type': 'reg', 'label': 'Properties Needed'}
};

// First array item will be used by default (on load)
JobNode.sort = ['order', 'time_creation', 'priority', 'user_name', 'name', 'host_name', 'service'];
JobNode.sortVisor = 'time_creation';
// If user is visor, special parameter will be used as the default
JobNode.filter = ['name', 'host_name', 'user_name', 'service'];
JobNode.filterVisor = 'user_name';
