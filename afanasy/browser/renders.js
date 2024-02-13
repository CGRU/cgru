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
	renders.js - methods and structs for monitoring and handling of render machines
*/

"use strict";

var renders_pools = null;

RenderNode.onMonitorCreate = function() {
	// Call pools on create first,
	// as there are actions and params creation,
	// that will be merged with render actions and params.
	PoolNode.onMonitorCreate();

	RenderNode.createActions();
	RenderNode.createParams();

	renders_pools = {};
};

function RenderNode()
{
}

RenderNode.prototype.init = function() {
	this.element.classList.add('render');

	cm_CreateStart(this);

	this.elTempBox = document.createElement('div');
	this.elTempBox.classList.add('temp_box');
	this.element.appendChild(this.elTempBox);
	this.elTempBar = document.createElement('div');
	this.elTempBar.classList.add('temp_bar');
	this.elTempBox.appendChild(this.elTempBar);

	this.elIdleBox = document.createElement('div');
	this.elIdleBox.classList.add('idle_box');
	this.element.appendChild(this.elIdleBox);
	this.elIdleBar = document.createElement('div');
	this.elIdleBar.classList.add('idle_bar');
	this.elIdleBox.appendChild(this.elIdleBar);

	this.elName = cm_ElCreateText(this.element, 'Client Host Name');
	this.elName.classList.add('name');
	this.elName.classList.add('prestar');

	this.elHWInfo = cm_ElCreateText(this.element, 'Hardware Info');

	if (cm_IsSith())
	{
		this.elPriority = document.createElement('div');
		this.element.appendChild(this.elPriority);
		this.elPriority.style.cssFloat = 'right';
		this.elPriority.title = 'Priority';
	}
	else
		this.elPriority = cm_ElCreateFloatText(this.element, 'right', 'Priority');

	this.elUserName = cm_ElCreateFloatText(this.element, 'right', 'User Name and "Nimby" Status');
	this.elEngine = cm_ElCreateFloatText(this.element, 'right', 'Client Version');

	this.elResources = document.createElement('div');
	this.element.appendChild(this.elResources);
	this.elResources.className = 'resources';
	this.elResources.style.display = 'none';

	this.elNewLine = document.createElement('br');
	this.element.appendChild(this.elNewLine);

	this.elRunTasks = cm_ElCreateText(this.element, 'Running Tasks');
	this.elRunTasks.classList.add('prestar');
	this.elCapacity = cm_ElCreateText(this.element, 'Capacity');
	this.elStateTime = cm_ElCreateFloatText(this.element, 'right', 'Busy/Free Status and Time');
	this.elProperties = cm_ElCreateFloatText(this.element, 'right', 'Host Properties');

	this.elServices = document.createElement('div');
	this.element.appendChild(this.elServices);
	this.elServices.classList.add('serivces');

	this.elTickets = document.createElement('div');
	this.element.appendChild(this.elTickets);
	this.elTickets.classList.add('tickets');

	this.elTasks = document.createElement('div');
	this.element.appendChild(this.elTasks);
	this.elTasks.classList.add('tasks');

	this.elAnnotation = document.createElement('div');
	this.element.appendChild(this.elAnnotation);
	this.elAnnotation.title = 'Annotation';
	this.elAnnotation.classList.add('annotation');

	this.plotters = [];
	this.plotterC = new Plotter(this.elResources, 'C', 'CPU');
	this.plotters.push(this.plotterC);
	this.plotterM = new Plotter(this.elResources, 'M', 'Memory');
	this.plotters.push(this.plotterM);
	this.plotterS = new Plotter(this.elResources, 'S', 'Swap');
	this.plotters.push(this.plotterS);
	this.plotterH = new Plotter(this.elResources, 'H', 'HDD Space');
	this.plotters.push(this.plotterH);
	this.plotterN = new Plotter(this.elResources, 'N', 'Network I/O');
	this.plotters.push(this.plotterN);
	this.plotterD = new Plotter(this.elResources, 'D', 'Disk I/O');
	this.plotters.push(this.plotterD);

	this.plotter_GPU_GPU = null;
	this.plotter_GPU_Mem = null;

	this.plotterC.addGraph();
	this.plotterC.setColor([200, 0, 0]);
	this.plotterC.addGraph();
	this.plotterC.setColor([50, 200, 20]);

	this.plotterM.addGraph();
	this.plotterM.setColor([50, 200, 20], [255, 0, 0]);
	this.plotterM.addGraph();
	this.plotterM.setColor([0, 50, 0]);

	this.plotterS.addGraph();
	this.plotterS.setColor([100, 200, 30], [255, 0, 0]);

	this.plotterH.addGraph(10);
	this.plotterH.setColor([50, 200, 20], [255, 0, 0]);

	this.plotterN.addGraph();
	this.plotterN.setColor([90, 200, 20], [255, 0, 10]);
	this.plotterN.addGraph();
	this.plotterN.setColor([20, 200, 90], [255, 0, 90]);
	this.plotterN.setScale(-1, 10000, 100000);
	this.plotterN.setAutoScale(1000, 100000);

	this.plotterD.addGraph();
	this.plotterD.setColor([90, 200, 20], [250, 50, 20]);
	this.plotterD.addGraph();
	this.plotterD.setColor([20, 200, 90], [250, 50, 90]);
	this.plotterD.setScale(-1, 10000, 100000);
	this.plotterD.setAutoScale(1000, 100000);

	this.plottersCs = [];

	this.elPower = document.createElement('div');
	this.elPower.classList.add('power');
	this.element.appendChild(this.elPower);

	this.state = {};
	this.tasks = [];
};

RenderNode.prototype.update = function(i_obj) {
	// When render write resources, it writes resources only, event no name
	if ((i_obj != null) && (i_obj.name == null))
	{
		if (i_obj.idle_time)
			this.params.idle_time = i_obj.idle_time;
		if (i_obj.busy_time)
			this.params.busy_time = i_obj.busy_time;

		var r = i_obj.host_resources;

		if (r == null)
			return;

		if ((this.state.ONL != true) || (this.host_resources == null))
		{
			// If render just become online,
			// or resources received first time,
			// we need to set plotter scales:

			this.elResources.style.display = 'block';

			this.plotterC.setTitle('CPU: ' + r.cpu_mhz + ' MHz x' + r.cpu_num);

			this.plotterM.setLabel('M ' + Math.round(r.mem_total_mb / 1024.0));
			this.plotterM.setTitle('Memory:\nTotal: ' + r.mem_total_mb + ' Mb');
			this.plotterM.setScale(r.mem_total_mb, 85 * r.mem_total_mb / 100, r.mem_total_mb);

			if (r.swap_total_mb > 0)
			{
				this.plotterS.setLabel('S ' + Math.round(r.swap_total_mb / 1024.0));
				this.plotterS.setTitle('Swap: ' + r.swap_total_mb + ' Mb');
				this.plotterS.setScale(r.swap_total_mb, r.swap_total_mb / 10, r.swap_total_mb);
			}
			else
			{
				this.plotterS.setTitle('Swap I/O');
				this.plotterS.setScale(-1, 100, 10000);
				this.plotterS.setAutoScale(1000, 100000);
			}

			this.plotterH.setLabel('H ' + r.hdd_total_gb);
			this.plotterH.setTitle('HDD Space: ' + r.hdd_total_gb + ' Gb');
			this.plotterH.setScale(r.hdd_total_gb, 95 * r.hdd_total_gb / 100, r.hdd_total_gb);

			// Create GPU resources ptotters (if was not created before)
			if (r.gpu_mem_total_mb && (this.plotter_GPU_GPU == null))
			{
				this.plotter_GPU_GPU = new Plotter(this.elResources, 'GU', 'GPU');
				this.plotter_GPU_GPU.addGraph();
				this.plotter_GPU_GPU.setTitle(r.gpu_string + ':');
				this.plotter_GPU_GPU.setColor([50, 200, 20]);
				this.plotters.push(this.plotter_GPU_GPU);

				this.plotter_GPU_Mem = new Plotter(this.elResources, 'GM ' + (r.gpu_mem_total_mb / 1000.0).toFixed(1), 'GPU Mem');
				this.plotter_GPU_Mem.addGraph();
				this.plotter_GPU_Mem.setTitle(r.gpu_string + ':\nMemory Total: ' + r.gpu_mem_total_mb + 'Mb');
				this.plotter_GPU_Mem.setScale(r.gpu_mem_total_mb, 85 * r.gpu_mem_total_mb / 100, r.gpu_mem_total_mb);
				this.plotter_GPU_Mem.setColor([50, 200, 20], [255, 0, 0]);
				this.plotters.push(this.plotter_GPU_Mem);
			}
		}

		let pl_w = Math.round(0.6 * this.element.clientWidth / this.plotters.length);
		for (let i = 0; i < this.plotters.length; i++)
		{
			this.plotters[i].setWidth(pl_w);
			let left = this.plotters[i].width + 8;
			left = (i - 0.5 * this.plotters.length) * left - 20;
			this.plotters[i].element.style.left = Math.round(left) + 'px';
			this.plotters[i].element.style.top = '2px';
		}

		this.host_resources = r;

		if (r.hw_info && r.hw_info.length)
		{
			this.elHWInfo.textContent = r.hw_info;
			if (r.hw_info[0] == 'V')
				this.m_virtual = true;
		}

		if (r.cpu_temp && (r.cpu_temp > 0) && (true != this.m_virtual))
		{
			this.elTempBox.style.display = 'block';
			this.elTempBar.style.width = r.cpu_temp + '%';
			const clr_cold = [ 50, 50,100];
			const clr_warm = [100,120,100];
			const clr_hot  = [255,  5,  5];
			let hmin = cgru_Config.af_monitor_render_cpu_hot_min;
			let hmax = cgru_Config.af_monitor_render_cpu_hot_max;
			let clr = [0,0,0];
			let factor = r.cpu_temp / hmin;
			if (r.cpu_temp > hmin)
			{
				factor = (r.cpu_temp - hmin) / (hmax - hmin);
				if (factor > 1.0) factor = 1.0;
				for (let i = 0; i < 3; i++)
					clr[i] = (1.0 - factor) * clr_warm[i] + factor * clr_hot[i];
			}
			else
			{
				for (let i = 0; i < 3; i++)
					clr[i] = (1.0 - factor) * clr_cold[i] + factor * clr_warm[i];
			}
			this.elTempBar.style.backgroundColor = 'rgb('+clr[0]+','+clr[1]+','+clr[2]+')';

			this.plotterC.setLabel('C ' + r.cpu_num + '*' + (r.cpu_mhz / 1000.0).toFixed(1) + ' ' + r.cpu_temp + 'C');
		}
		else
			this.plotterC.setLabel('C ' + r.cpu_num + '*' + (r.cpu_mhz / 1000.0).toFixed(1));


		var usr = r.cpu_user + r.cpu_nice;
		var sys = r.cpu_system + r.cpu_iowait + r.cpu_irq + r.cpu_softirq;
		this.plotterC.appendTitle('\nUsage: ' + usr + '%\nSystem: ' + sys + '%');
		this.plotterC.addValues([sys, usr]);

		var mem = r.mem_total_mb - r.mem_free_mb;
		var buf = r.mem_cached_mb + r.mem_buffers_mb;
		this.plotterM.appendTitle(
			'\nUsed: ' + mem + 'Mb\nCached: ' + buf + ' Mb\nFree: ' + r.mem_free_mb + 'Mb');
		this.plotterM.addValues([mem, buf]);

		this.plotterS.addValues([r.swap_used_mb]);
		if (r.swap_total_mb > 0)
			this.plotterS.appendTitle('\nUsed: ' + r.swap_used_mb + ' Mb');
		else
			this.plotterS.appendTitle('\nUsage: ' + r.swap_used_mb + ' MBytes/s');

		this.plotterH.appendTitle('\nFree: ' + r.hdd_free_gb + ' Gb');
		this.plotterH.addValues([r.hdd_total_gb - r.hdd_free_gb]);

		this.plotterN.appendTitle(
			'\nReceive: ' + Math.round(r.net_recv_kbsec / 1024) + ' MBytes/s\nSend: ' +
			Math.round(r.net_send_kbsec / 1024) + ' MBytes/s');
		this.plotterN.addValues([r.net_recv_kbsec, r.net_send_kbsec]);

		this.plotterD.appendTitle(
			'\nRead: ' + Math.round(r.hdd_rd_kbsec / 1024) + 'MBytes/s\nWrite: ' +
			Math.round(r.hdd_wr_kbsec / 1024) + ' MBytes/s\nBusy: ' + r.hdd_busy + '%');
		this.plotterD.addValues([r.hdd_rd_kbsec, r.hdd_wr_kbsec], r.hdd_busy / 100);

		// GPU resources:
		if (r.gpu_mem_total_mb && this.plotter_GPU_GPU)
		{
			let plot_GPU_red_min = 65;
			let plot_GPU_red_max = 95;
			let plot_GPU_red = (r.gpu_gpu_temp - plot_GPU_red_min) * 100 / (plot_GPU_red_max - plot_GPU_red_min);
			if (plot_GPU_red < 0) plot_GPU_red = 0;
			if (plot_GPU_red > 100) plot_GPU_red = 100;
			let plot_GPU_temp_r = 50 + (2*plot_GPU_red);
			let plot_GPU_temp_g = 200 - (2*plot_GPU_red);
			let plot_GPU_temp_b = 20;
			this.plotter_GPU_GPU.setColor([plot_GPU_temp_r, plot_GPU_temp_g, plot_GPU_temp_b]);
			this.plotter_GPU_GPU.addValues([r.gpu_gpu_util]);
			this.plotter_GPU_GPU.setLabel('GU ' + r.gpu_gpu_temp + 'C');
			this.plotter_GPU_GPU.appendTitle('\nUtilization: ' + r.gpu_gpu_util + '%\nTemperature: ' + r.gpu_gpu_temp + 'C');

			this.plotter_GPU_Mem.addValues([r.gpu_mem_used_mb]);
			this.plotter_GPU_Mem.appendTitle('\nMemory used: ' + r.gpu_mem_used_mb + ' Mb');
		}

		this.params.tasks_percents = i_obj.tasks_percents;
		this.updateTasksPercents();

		if (r.custom)
		{
			if (this.plottersCs.length != r.custom.length)
			{
				this.plottersCsDelete();
				this.elPlottersCs = document.createElement('div');
				this.elPlottersCs.classList.add('plotters_custom_div');
				this.element.appendChild(this.elPlottersCs);
				for (var i = 0; i < r.custom.length; i++)
				{
					var plotter = new Plotter(this.elPlottersCs, 'Cs', 'custom');
					this.plottersCs.push(plotter);
					plotter.element.classList.add('custom');
					plotter.addGraph();
					plotter.setColor([50, 200, 20], [255, 0, 0]);
				}
			}

			for (var i = 0; i < r.custom.length; i++)
			{
				this.plottersCs[i].setWidth(Math.round(this.elPlottersCs.clientWidth / r.custom.length) - 6);
				this.plottersCs[i].setHeight(r.custom[i].height);
				this.plottersCs[i].setScale(r.custom[i].value_max);
				this.plottersCs[i].setColor(r.custom[i].graph_clr);
				this.plottersCs[i].addValues([r.custom[i].value]);
				this.plottersCs[i].setLabel(r.custom[i].label, r.custom[i].label_clr, r.custom[i].label_size);
				this.plottersCs[i].setBGColor(r.custom[i].back_clr);
				this.plottersCs[i].setTitle(r.custom[i].tooltip);
			}
		}
		else if ((r.custom == null) && (this.plottersCs.length))
		{
			this.plottersCsDelete();
		}

		return;
	}

	if (i_obj)
		this.params = i_obj;

	// We force sorting parameter,
	// as render should be sorted by pool first.
	this.sort_force = this.params.pool;

	// Collect render in renders_pools.
	// This needed to offset render to show hierarchy on pool update.
	// May be render will be created before its pooll,
	// so pool should know its childs (that were created first) to update.
	if (renders_pools[this.params.pool] == null)
		renders_pools[this.params.pool] = [];
	if (false == renders_pools[this.params.pool].includes(this))
		renders_pools[this.params.pool].push(this);

	// Offset hierarchy right now.
	// May be render was created after its pool.
	this.offsetHierarchy();


	cm_GetState(this.params.state, this.state, this.element);

	this.elName.innerHTML = '<b>' + this.params.name + '</b>';

	if (this.params.engine != null)
		this.elEngine.textContent = 'v' + this.params.engine;
	else
		this.elEngine.textContent = '';

	if (cm_IsPadawan())
	{
		this.elPriority.innerHTML = ' Priority:<b>' + this.params.priority + '</b>';
	}
	else if (cm_IsJedi())
	{
		this.elPriority.innerHTML = ' Pri:<b>' + this.params.priority + '</b>';
	}
	else
	{
		this.elPriority.innerHTML = '-<b>' + this.params.priority + '</b>';
	}

	var user = '<b>' + this.params.user_name + '</b>';
	if (this.state.PAU)
	{
		user = 'Paused(<b>' + user + '</b>)<b>P</b>';
		if (this.state.NBY)
			user += '+<b>N</b>';
		if (this.state.NbY)
			user += '+<b>n</b>';
	}
	else if (this.state.NbY)
		user = 'nimby(<b>' + user + '</b>)<b>n</b>';
	else if (this.state.NBY)
		user = 'NIMBY(<b>' + user + '</b>)<b>N</b>';
	this.elUserName.innerHTML = user;

	let properties = '';
	if (this.params.power_host)
		properties += ' ' + this.params.power_host;
	if (this.params.properties_host)
		properties += ' ' + this.params.properties_host;
	this.elProperties.textContent = properties;


	if (this.params.annotation)
		this.elAnnotation.innerHTML = this.params.annotation;
	else
		this.elAnnotation.textContent = '';

	if (this.state.WWK)
		this.offlineState = 'Waking Up';
	else if (this.state.WSL || this.state.WFL)
		this.offlineState = 'Sleeping';
	else
		this.offlineState = 'Offline';


	if (this.state.OFF == true)
	{
		this.elStar.style.display = 'none';
		this.clearTasks();
		this.plottersCsDelete();
		this.elResources.style.display = 'none';
		this.host_resources = null;
		this.elCapacity.textContent = '';
		this.elRunTasks.textContent = '';
		this.state.textContent = '';
		this.elStateTime.textContent = '';
		this.elNewLine.style.display = 'none';
		return;
	}
	this.elPower.textContent = '';
	this.elNewLine.style.display = 'block';

	// Run tasks and capacity:
	this.params.run_tasks = 0;
	if (this.params.tasks)
		this.params.run_tasks = this.params.tasks.length;
	let tasks_html = 'T:<b>' + this.params.run_tasks + '</b>';
	let cap_html = 'C:<b>' + this.params.capacity_used + '</b>';
	if (cm_IsPadawan())
	{
		tasks_html = 'Tasks: <b>' + this.params.run_tasks + '</b>';
		cap_html = 'Capacity: <b>' + this.params.capacity_used + '</b>';
	}
	else if (cm_IsJedi())
	{
		tasks_html = 'Tasks:<b>' + this.params.run_tasks + '</b>';
		cap_html = 'Cap:<b>' + this.params.capacity_used + '</b>';
	}
	if (this.params.max_tasks_host != null)
		tasks_html += '/<b>' + this.params.max_tasks_host + '</b>';
	if (this.params.capacity_host != null)
		cap_html += '/<b>' + this.params.capacity_host + '</b>';
	this.elRunTasks.innerHTML =	tasks_html;
	this.elCapacity.innerHTML = cap_html;

	if (this.state.RUN == true)
	{
		this.elStar.style.display = 'block';
		this.elStarCount.textContent = this.params.tasks.length;
	}
	else
		this.elStar.style.display = 'none';


	// Show servives:
	this.elServices.textContent = '';
	farm_showServices(this.elServices, this.params,'renders');

	// Show tickets:
	this.elTickets.textContent = '';
	farm_showTickets(this.elTickets, this.params.tickets_host, 'host', this);

	this.clearTasks();
	if (this.params.tasks != null)
		for (var t = 0; t < this.params.tasks.length; t++)
			this.tasks.push(new RenderTask(this.params.tasks[t], this.elTasks));

	this.updateTasksPercents();
	this.refresh();
};

RenderNode.prototype.offsetHierarchy = function() {
	var depth = 0;
	var parent_pool = pools[this.params.pool];
	if (parent_pool)
		depth = parent_pool.pool_depth + 1;
	this.element.style.marginLeft = (depth * 32 + 2) + 'px';
	this.m_parent_pool = parent_pool;
}

RenderNode.prototype.plottersCsDelete = function() {
	if (this.plottersCs.length == 0)
		return;

	for (var i = 0; i < this.plottersCs.length; i++)
	{
		this.elPlottersCs.removeChild(this.plottersCs[i].element);
		delete this.plottersCs[i];
	}

	this.element.removeChild(this.elPlottersCs);
	this.plottersCs = [];
};

RenderNode.prototype.clearTasks = function() {
	for (var t = 0; t < this.tasks.length; t++)
		this.tasks[t].destroy();
	this.tasks = [];
};

RenderNode.prototype.refresh = function() {
	var pool = pools[this.params.pool];

	if (this.state.OFF || this.state.WFL)
	{
		var power = this.offlineState;
		if (this.params.wol_operation_time)
			power += ' <b>' + cm_TimeStringInterval(this.params.wol_operation_time) + '</b>';
		this.elPower.innerHTML = power;
		if (this.state.WFL || this.state.WSL || this.state.WWK)
			this.elPower.style.color = '#FF0';
		else
			this.elPower.style.color = '#000';
		this.elIdleBox.style.display = 'none';
		if (this.state.OFF)
			return;
	}

	var stateTime = 'NEW';
	var stateTimeTitle = 'Idle time: ' + cm_TimeStringInterval(this.params.idle_time);

	// Draw idle bar:
	if (pool)
	{
		var curtime = new Date();
		var idle_sec = curtime.valueOf() / 1000.0 - this.params.idle_time;
		if (idle_sec < 0)
			idle_sec = 0;
		var busy_sec = curtime.valueOf() / 1000.0 - this.params.busy_time;
		if (busy_sec < 0)
			busy_sec = 0;
		var percent = 0;

		if ((pool.getParmParent('idle_free_time') > 0) && (this.state.RUN != true) &&
			(this.state.NbY || this.state.NBY))
		{
			stateTimeTitle +=
				'\nNimby idle free time: ' + cm_TimeStringFromSeconds(pool.getParmParent('idle_free_time'));
			percent = Math.round(100.0 * idle_sec / pool.getParmParent('idle_free_time'));

			idle_sec = Math.round(pool.getParmParent('idle_free_time') - idle_sec);
			if (idle_sec > 0)
				this.elIdleBox.title = 'Nimby idle free in ' + cm_TimeStringFromSeconds(idle_sec);
			else
				this.elIdleBox.title = 'Nimby free';
			this.elIdleBox.classList.remove('wol');
			this.elIdleBox.classList.add('free');
			this.elIdleBox.classList.remove('nimby');
		}
		else if (
			(pool.getParmParent('busy_nimby_time') > 0) && (busy_sec > 6) && (this.state.RUN != true) &&
			(this.state.NbY != true) && (this.state.NBY != true))
		{
			stateTimeTitle +=
				'\nBusy free Nimby time: ' + cm_TimeStringFromSeconds(pool.getParmParent('busy_nimby_time'));
			percent = Math.round(100.0 * busy_sec / pool.getParmParent('busy_nimby_time'));

			busy_sec = Math.round(pool.getParmParent('busy_nimby_time') - busy_sec);
			if (busy_sec > 0)
				this.elIdleBox.title = 'Nimby busy in ' + cm_TimeStringFromSeconds(busy_sec);
			else
				this.elIdleBox.title = 'Nimby busy';
			this.elIdleBox.classList.remove('wol');
			this.elIdleBox.classList.remove('free');
			this.elIdleBox.classList.add('nimby');
		}
		else if ((pool.getParmParent('idle_wolsleep_time') > 0) && (this.state.RUN != true))
		{
			stateTimeTitle +=
				'\nWOL idle sleep time: ' + cm_TimeStringFromSeconds(pool.getParmParent('idle_wolsleep_time'));
			percent = Math.round(100.0 * idle_sec / pool.getParmParent('idle_wolsleep_time'));

			idle_sec = Math.round(pool.getParmParent('idle_wolsleep_time') - idle_sec);
			if (idle_sec > 0)
				this.elIdleBox.title = 'WOL idle sleep in ' + cm_TimeStringFromSeconds(idle_sec);
			else
				this.elIdleBox.title = 'WOL sleep';
			this.elIdleBox.classList.add('wol');
			this.elIdleBox.classList.remove('free');
			this.elIdleBox.classList.remove('nimby');
		}
		else
		{
			stateTimeTitle +=
				'\nIdle bar time: ' + cm_TimeStringFromSeconds(cgru_Config.af_monitor_render_idle_bar_max);
			percent = Math.round(100.0 * idle_sec / cgru_Config.af_monitor_render_idle_bar_max);
			this.elIdleBox.classList.remove('wol');
			this.elIdleBox.classList.remove('free');
			this.elIdleBox.classList.remove('nimby');
		}

		if (percent > 100)
			percent = 100;
		if (percent < 0)
			percent = 0;
		this.elIdleBox.style.display = 'block';
		this.elIdleBar.style.width = percent + '%';
	}
	else
		this.elIdleBox.style.display = 'none';

	if ((this.params.task_start_finish_time != null) && (this.params.task_start_finish_time > 0))
	{
		stateTime = '<b>' + cm_TimeStringInterval(this.params.task_start_finish_time) + '</b>';
		if (this.state.RUN == true)
			stateTime += ' busy';
		else
			stateTime += ' free';
	}
	this.elStateTime.innerHTML = stateTime;
	this.elStateTime.title = stateTimeTitle;

	for (var t = 0; t < this.tasks.length; t++)
		this.tasks[t].refresh();
};

RenderNode.prototype.updateTasksPercents = function() {
	if (this.params.tasks_percents)
		for (var t = 0; t < this.tasks.length; t++)
			this.tasks[t].setPercent(this.params.tasks_percents[t]);
};

RenderNode.prototype.onDoubleClick = function(e) {
	nw_request({
		"send": {"get": {"type": 'renders', "ids": [this.params.id], "mode": 'full'}},
		"func": g_ShowObject,
		"evt": e,
		"wnd": this.monitor.window
	});
};

RenderNode.setService = function(i_args) {
	new cgru_Dialog({
		"wnd": i_args.monitor.window,
		"receiver": i_args.monitor.cur_item,
		"handle": 'serviceApply',
		"param": i_args.name,
		"name": 'service',
		"title": i_args.tooltip,
		"info": 'Enter Service Name:'
	});
};

RenderNode.prototype.serviceApply = function(i_value, i_name) {
	g_Info('menuHandleService = ' + i_name + ',' + i_value);
	var operation = {};
	operation.type = 'farm';
	operation.mode = i_name;
	operation.name = i_value;
	operation.mask = i_value;
	nw_Action('renders', this.monitor.getSelectedIds(), operation, null);
};

RenderNode.clearServices = function(i_args) {
	g_Info('Clear services...');
	var operation = {};
	operation.type = 'farm';
	operation.mode = 'clear_services';
	nw_Action(i_args.monitor.cur_item.node_type, i_args.monitor.getSelectedIds(), operation, null);
};

RenderNode.editTicket = function(i_args) {
	new cgru_Dialog({
		"wnd": i_args.monitor.window,
		"receiver": i_args.monitor.cur_item,
		"handle": 'editTicket',
		"param": i_args.name,
		"name": i_args.name,
		"title": i_args.tooltip,
		"info": 'Enter Ticket Name:'
	});
};

RenderNode.prototype.editTicket = function(i_value, i_param) {
	var type;
	var count = 1;
	var max_hosts = null;

	if (this.params.tickets_host && this.params.tickets_host[i_value])
		count = this.params.tickets_host[i_value][0];

	farm_ticketEditDialog(i_value, count, max_hosts, 'host', this);
};

function RenderTask(i_task, i_elParent)
{
	this.elParent = i_elParent;

	this.elRoot = document.createElement('div');
	i_elParent.appendChild(this.elRoot);

	this.elIcon = document.createElement('img');
	this.elRoot.appendChild(this.elIcon);
	this.elIcon.src = 'icons/software/' + i_task.service + '.png';
	this.elIcon.style.position = 'absolute';
	this.elIcon.style.width = '18px';
	this.elIcon.style.height = '18px';
	this.elIcon.title = i_task.service;
	//	this.elIcon.classList.add('icon');

	this.element = document.createElement('div');
	this.elRoot.appendChild(this.element);
	this.element.style.marginLeft = '20px';
	this.element.classList.add('rendertask');

	this.elBar = document.createElement('div');
	this.element.appendChild(this.elBar);
	this.elBar.classList.add('bar');
	this.elBar.style.display = 'none';

	this.elBody = document.createElement('div');
	this.element.appendChild(this.elBody);
	this.elBody.classList.add('body');

	if (i_task.tickets)
	{
		this.elTickets = cm_ElCreateText(this.elBody, 'Tickets');
		for (let tk in i_task.tickets)
		{
			let elTk = document.createElement('div');
			this.elTickets.appendChild(elTk);
			elTk.classList.add('ticket');

			let label = '';

			if (cm_TicketsIcons.includes(tk + '.png'))
			{
				let elIcon = document.createElement('img');
				elTk.appendChild(elIcon);
				elIcon.src = ('icons/tickets/' + tk + '.png');
			}
			else
				label = tk;

			let elLabel = document.createElement('div');
			elTk.appendChild(elLabel);
			elLabel.classList.add('label');
			elLabel.textContent = label + 'x' + i_task.tickets[tk];
		}
	}

	this.elCapacity = cm_ElCreateText(this.elBody, 'Capacity');
	this.elCapacity.textContent = '[' + i_task.capacity + ']';

	this.elJob = cm_ElCreateText(this.elBody, 'Job Name');
	this.elJob.textContent = i_task.job_name;

	this.elBlock = cm_ElCreateText(this.elBody, 'Block Name');
	this.elBlock.textContent = '[' + i_task.block_name + ']';

	this.elName = cm_ElCreateText(this.elBody, 'Task Name');
	this.elName.textContent = '[' + i_task.name + ']';

	this.elTime = cm_ElCreateFloatText(this.elBody, 'right', 'Running Time');
	this.time = i_task.time_start;

	this.elUser = cm_ElCreateFloatText(this.elBody, 'right', 'User Name');
	this.elUser.textContent = i_task.user_name;

	this.refresh();
}

RenderTask.prototype.refresh = function() {
	this.elTime.textContent = cm_TimeStringInterval(this.time);
};

RenderTask.prototype.setPercent = function(i_percent) {
	if (i_percent == null)
		return;
	if (i_percent < 1)
		return;
	this.elBar.style.display = 'block';
	this.elBar.style.width = i_percent + '%';
};

RenderTask.prototype.destroy = function() {
	this.elParent.removeChild(this.elRoot);
};

RenderNode.launchCmdExit = function(i_args) {
	// console.log( i_args);
	new cgru_Dialog({
		"wnd": i_args.monitor.window,
		"receiver": i_args.monitor.cur_item,
		"handle": 'launchCmdExitDo',
		"param": i_args.name,
		"name": 'farm',
		"title": 'Launch Command' + (i_args.name == 'lcex' ? ' And Exit' : ''),
		"info": 'Enter command:'
	});
};

RenderNode.prototype.launchCmdExitDo = function(i_value, i_name) {
	g_Info('launchCmdExit = ' + i_name + ',' + i_value);
	var operation = {};
	operation.type = 'launch_cmd';
	operation.cmd = i_value;
	if (i_name == 'lcex')
		operation.exit = true;
	nw_Action('renders', this.monitor.getSelectedIds(), operation, null);
};

RenderNode.setPoolDialog = function(i_args) {
	new cgru_Dialog({
		"wnd": i_args.monitor.window,
		"receiver": i_args.monitor.cur_item,
		"handle": 'setPoolDo',
		"param": i_args.name,
		"name": 'set_pool',
		"title": 'Assign render to poll',
		"info": 'Enter a new pool name:'
	});
};

RenderNode.prototype.setPoolDo = function(i_value, i_name) {
	g_Info('Setting a pool "' + i_value + '" for "' + this.params.name + '"');
	var operation = {};
	operation.type = 'set_pool';
	operation.name = i_value;
	nw_Action('renders', this.monitor.getSelectedIds(), operation, null);
};

RenderNode.createPanels = function(i_monitor) {
	// Create pool buttons first
	PoolNode.createPanels(i_monitor);
	var acts;

	// Info:
	acts = {};
	acts.tasks_log = {'label': 'TasksLog', 'tooltip': 'Get tasks Log.'};
	acts.full = {'label': 'FullInfo', 'tooltip': 'Request full render node info.'};
	i_monitor.createCtrlBtn({
		'name': 'info',
		'label': 'Info',
		'tooltip': 'Get render info.',
		'handle': 'mh_Get',
		'node_type': 'renders',
		'sub_menu': acts
	});

	// Pools:
	acts = {};
	acts.set_pool =      {'label':'Set',      'handle':'setPoolDialog', 'tooltip':'Set pool'};
	acts.reassign_pool = {'label':'ReAssign', 'handle':'mh_Oper',       'tooltip':'Reassign pool'};
	i_monitor.createCtrlBtn({
		'name': 'pool',
		'label': 'Pool',
		'tooltip': 'Manipulate render pool.',
		'node_type': 'renders',
		'sub_menu': acts
	});

	// Nimby:
	acts = {
		free: {
			'name': 'nimby',
			'value': false,
			'handle': 'mh_Param',
			'label': 'FREE',
			'node_type': 'renders',
			'tooltip': 'Set render free.'
		},
		nimby: {
			'name': 'nimby',
			'value': true,
			'handle': 'mh_Param',
			'label': 'Nimby',
			'node_type': 'renders',
			'tooltip': 'Set render nimby.\nRun only owner tasks.'
		},
		NIMBY: {
			'name': 'NIMBY',
			'value': true,
			'handle': 'mh_Param',
			'label': 'NIMBY',
			'node_type': 'renders',
			'tooltip': 'Set render NIMBY.\nDo not run any tasks.'
		}
	};
	i_monitor.createCtrlBtns(acts);

	// Heal:
	acts = {};
	acts.heal_sick = {'label': 'HEAL', 'tooltip': 'Heal sick renders.'};
	i_monitor.createCtrlBtns(acts);

	// Eject tasks:
	acts = {};
	acts.eject_tasks = {'label': 'All Tasks', 'tooltip': 'Eject all running tasks.'};
	acts.eject_tasks_keep_my = {'label': 'Not My', 'tooltip': 'Eject not my tasks.'};
	i_monitor.createCtrlBtn(
		{'name': 'eject', 'label': 'Eject', 'tooltip': 'Eject tasks from render.', 'sub_menu': acts});


	// Custom commands:
	let el = document.createElement('div');
	i_monitor.elPanelL.appendChild(el);
	el.classList.add('ctrl_button');
	el.textContent = 'CUSTOM';
	el.monitor = i_monitor;
	el.onclick = function(e) {
		e.currentTarget.monitor.showMenu(e, 'cgru_cmdexec');
		return false;
	};
	el.oncontextmenu = el.onclick;
	// We can execute custom commands on renders only
	el.m_act = {'node_type':'renders'};

	// Admin related functions:
	if (!g_GOD())
		return;

	// Paused:
	acts = {};
	acts.pause = {
		'name': 'paused',
		'value': true,
		'handle': 'mh_Param',
		'label': 'PAUSE',
		'tooltip': 'Set render paused.'
	};
	acts.unpause = {
		'name': 'paused',
		'value': false,
		'handle': 'mh_Param',
		'label': 'UNPAUSE',
		'tooltip': 'Unset render pause.'
	};
	i_monitor.createCtrlBtns(acts);


	// Services:
	acts = {};
	acts.service_add     = {'handle':'setService', 'label':'Add',     'tooltip':'Add service.'};
	acts.service_remove  = {'handle':'setService', 'label':'Remove',  'tooltip':'Remove service.'};
	acts.service_enable  = {'handle':'setService', 'label':'Enable',  'tooltip':'Enable service.'};
	acts.service_disable = {'handle':'setService', 'label':'Disable', 'tooltip':'Disable service.'};
	acts.clear_services =
		{'handle':'clearServices','label':'Clear','tooltip':'Double click to clear services.','ondblclick':true};
	i_monitor.createCtrlBtn({
		'name': 'services',
		'label': 'Services',
		'tooltip': 'Enable/Disable services.',
		'sub_menu': acts
	});


	// Tickets:
	acts = {};
	acts.ticket_edit_pool = {'handle':'editTicket', 'label':'Ticket Pool', 'tooltip':'Add or edit pool ticket.','node_type':'pools'};
	acts.ticket_edit_host = {'handle':'editTicket', 'label':'Ticket Host', 'tooltip':'Add or edit host ticket.'};
	i_monitor.createCtrlBtns(acts);
//	i_monitor.createCtrlBtn({
//		'name': 'tickets',
//		'label': 'Tickets',
//		'tooltip': 'Edit tickets.',
//		'sub_menu': acts
//	});


	// Power/WOL:
	acts = {
		wol_sleep /**/: {'label': 'WOLSleep', 'tooltip': 'Wake-On-Lan sleep.'},
		wol_wake /***/: {'label': 'WOLWake',  'tooltip': 'Wake-On-Lan wake.'},
		exit /*******/: {'label': 'Exit',     'tooltip': 'Exit client.'},
		reboot /*****/: {'label': 'Reboot',   'tooltip': 'Reboot machine.'},
		shutdown /***/: {'label': 'Shutdown', 'tooltip': 'Shutdown machine.'}
	};
	i_monitor.createCtrlBtn(
		{'name': 'power', 'label': 'Power', 'tooltip': 'Power / Exit / Delete.', 'sub_menu': acts});

	// Launch and Exit:
	acts = {};
	acts.lcmd = {'name': 'lcmd', 'label': 'Command',  'handle': 'launchCmdExit', 'tooltip': 'Launch command.'};
	acts.lcex = {'name': 'lcex', 'label': 'Cmd&Exit', 'handle': 'launchCmdExit', 'tooltip': 'Launch command and exit.'};
	i_monitor.createCtrlBtn({
		'name': 'launch',
		'label': 'Launch',
		'tooltip': 'Launch command by afrender.',
		'sub_menu': acts
	});

	acts = {};
	acts.delete = {"label": "DELETE", "tooltip": 'Double click to delete.', "ondblclick": true};
	i_monitor.createCtrlBtns(acts);
};

RenderNode.prototype.updatePanels = function() {
	// Info:
	var info = '';

	info += '<p>OS: <b>' + this.params.os + '</b> - ' + this.params.engine + '</p>';

	info += '<p>Pool: <b>' + this.params.pool + '</b></p>';

	info += '<p>IP: <b>' + this.params.address.ip + '</b></p>';

	var r = this.host_resources;
	if (r)
	{
		info += '<p>';
		info += ' ' + r.hw_info;
		info += '<br> CPU: <b>' + r.cpu_num + '</b>x<b>' + (r.cpu_mhz / 1000.0).toFixed(1) + '</b> GHz';
		info += ' T=<b>' + r.cpu_temp + '</b>C';
		info += '<br> MEM: <b>' + (r.mem_total_mb / 1024.0).toFixed(1) + '</b> Gb';
		info += ' SWP: <b>' + (r.swap_total_mb / 1024.0).toFixed(1) + '</b> Gb';
		info += ' HDD: <b>' + r.hdd_total_gb + '</b> Gb';
		if (r.gpu_string)
		{
			info += '<br>GPU: <b>' + r.gpu_string + '</b> Mem: <b>' + (r.gpu_mem_total_mb / 1000.0).toFixed(1) + '</b>Gb'
		}
		info += '</p>';

		if (r.logged_in_users && r.logged_in_users.length)
		{
			info += '<p>Logged in:<b>';
			for (let usr of r.logged_in_users)
				info += ' ' + usr;
			info += '</b></p>';
		}
	}

	info += '<div>Registered: ' + cm_DateTimeStrFromSec(this.params.time_register) + '</div>';
	if (this.params.time_launch)
		info += '<div>Launched: ' + cm_DateTimeStrFromSec(this.params.time_launch) + '</div>';
	if (this.params.idle_time)
		info += '<div>Idle since: ' + cm_DateTimeStrFromSec(this.params.idle_time) + '</div>';
	if (this.params.busy_time)
		info += '<div>Busy since: ' + cm_DateTimeStrFromSec(this.params.busy_time) + '</div>';
	if (this.task_start_finish_time)
		info += '<div>Task finished at: ' + cm_DateTimeStrFromSec(this.params.task_start_finish_time) + '</div>';

	this.monitor.setPanelInfo(info);

	farm_showServicesInfo(this);

	farm_showTicketsInfo(this, 'host');
};

RenderNode.actions = [];
RenderNode.actionsCreated = false;
RenderNode.createActions = function() {
	if (RenderNode.actionsCreated)
		return;

	if (cgru_Config.af_rendercmds)
		for (var i = 0; i < cgru_Config.af_rendercmds.length; i++)
			RenderNode.actions.push({
				"mode": 'cgru_cmdexec',
				"name": 'cmd',
				"handle": cgru_Config.af_rendercmds[i],
				"label": cgru_Config.af_rendercmds[i]
			});

	if (cgru_Config.af_rendercmds_admin)
		for (var i = 0; i < cgru_Config.af_rendercmds_admin.length; i++)
			RenderNode.actions.push({
				"mode": 'cgru_cmdexec',
				"name": 'cmd',
				"handle": cgru_Config.af_rendercmds_admin[i],
				"label": cgru_Config.af_rendercmds_admin[i],
				"permissions": 'god'
			});

	for (let p in PoolNode.params)
		PoolNode.params[p] = PoolNode.params[p];

	RenderNode.actionsCreated = true;
};

RenderNode.params_render = {
	priority        : {'type':'num', 'label':'Priority'},
	capacity_host   : {'type':'num', 'label':'Capacity'},
	max_tasks_host  : {'type':'num', 'label':'Maximum Tasks'},
	user_name       : {'type':'str', 'label':'User Name'},
	annotation      : {'type':'str', 'label':'Annotation'}
};

RenderNode.sort = ['name', 'priority', 'user_name'];
RenderNode.filter = ['user_name', 'name', 'host_name'];

RenderNode.createParams = function() {
	if (RenderNode.params_created)
		return;

	RenderNode.params = {};

	// Add pool node params:
	for (let p in PoolNode.params)
		RenderNode.params[p] = PoolNode.params[p];

	// Add render node params:
	for (let p in RenderNode.params_render)
	{
		RenderNode.params[p] = RenderNode.params_render[p];
		RenderNode.params[p].permissions = 'god';
		RenderNode.params[p].node_type = 'renders';
	}

	RenderNode.params_created = true;
};
