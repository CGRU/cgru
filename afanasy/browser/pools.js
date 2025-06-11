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
	pools.js - methods and structs for monitoring and handling of pools
*/

'use strict';

var pools = null;

PoolNode.onMonitorCreate = function() {
	PoolNode.createParams();

	pools = {};
};

function PoolNode() {}

PoolNode.prototype.init = function() {
	this.element.classList.add('pool');

	this.elName = document.createElement('span');
	this.elName.classList.add('name');
	this.element.appendChild(this.elName);
	this.elName.title = 'Pool name (path)';

	this.elParams = cm_ElCreateFloatText(this.element, 'right', 'Parameters');

	this.element.appendChild(document.createElement('br'));

	this.elPoolsCounts = cm_ElCreateFloatText(this.element, 'left', 'Pools: All/Running');
	this.elRendersCounts = cm_ElCreateFloatText(this.element, 'left', 'Renders: All/Running');

	this.elHostProperties = cm_ElCreateFloatText(this.element, 'right');

	this.element.appendChild(document.createElement('br'));

	this.elServices = document.createElement('div');
	this.element.appendChild(this.elServices);

	this.elTickets = document.createElement('div');
	this.element.appendChild(this.elTickets);

	this.elAnnotation = document.createElement('div');
	this.element.appendChild(this.elAnnotation);
	this.elAnnotation.classList.add('annotation');
	this.elAnnotation.title = 'Annotation';

	this.state = {};
};

PoolNode.prototype.getParmParent = function(i_parm) {
	if ((this.params[i_parm] == null) && pools[this.params.parent])
		return pools[this.params.parent].params[i_parm];
	else
		return this.params[i_parm];
};

PoolNode.prototype.getTicketHostCount = function(i_tk) {
	if (this.params.tickets_host && this.params.tickets_host[i_tk])
		return this.params.tickets_host[i_tk][0];

	if (this.m_parent_pool)
		return this.m_parent_pool.getTicketHostCount(i_tk);

	return -1;
};

PoolNode.prototype.update = function(i_obj) {
	if (i_obj)
		this.params = i_obj;


	// Sort needed to force sort by this first:
	this.sort_force = this.params.name;


	// Offset hierarchy:
	this.pool_depth = 0;
	pools[this.params.name] = this;
	var parent_pool = pools[this.params.parent];
	if (parent_pool)
		this.pool_depth = parent_pool.pool_depth + 1;
	this.element.style.marginLeft = (this.pool_depth * 32 + 2) + 'px';
	this.m_parent_pool = parent_pool;


	// Offset child renders hierarchy:
	if (renders_pools[this.params.name])
		for (let render of renders_pools[this.params.name])
			render.offsetHierarchy();
	//^ this function requires filled in pools object

	cm_GetState(this.params.state, this.state, this.element);

	var params = '';
	if (cm_IsPadawan())
	{
		if (this.params.new_nimby)
			params += ' NewRender:<b>Nimby</b>';
		if (this.params.new_paused)
			params += ' NewRender:<b>Paused</b>';
		if (this.params.max_tasks_host != null)
			params += ' HostMaxTasks:<b>' + this.params.max_tasks_host + '</b>';
		if (this.params.capacity_host != null)
			params += ' HostCapacity:<b>' + this.params.capacity_host + '</b>';
		if (this.params.sick_errors_count != null)
			params += ' SickErrors:<b>' + this.params.sick_errors_count + '</b>';
		params += ' Priority:<b>' + this.params.priority + '</b>';
	}
	else if (cm_IsJedi())
	{
		if (this.params.new_nimby)
			params += ' New:<b>Nimby</b>';
		if (this.params.new_paused)
			params += ' New:<b>Paused</b>';
		if (this.params.max_tasks_host)
			params += ' HostTasks:<b>' + this.params.max_tasks_host + '</b>';
		if (this.params.capacity_host)
			params += ' HostCapacity:<b>' + this.params.capacity_host + '</b>';
		if (this.params.sick_errors_count != null)
			params += ' SickErr:<b>' + this.params.sick_errors_count + '</b>';
		params += ' Pri:<b>' + this.params.priority + '</b>';
	}
	else
	{
		if (this.params.new_nimby)
			params += ' n:<b>n</b>';
		if (this.params.new_paused)
			params += ' n:<b>p</b>';
		if (this.params.max_tasks_host)
			params += ' ht:<b>' + this.params.max_tasks_host + '</b>';
		if (this.params.capacity_host)
			params += ' hc:<b>' + this.params.capacity_host + '</b>';
		if (this.params.sick_errors_count != null)
			params += ' se:<b>' + this.params.sick_errors_count + '</b>';
		params += ' p<b>' + this.params.priority + '</b>';
	}

	if (this.state.PAU)
		params += ' <b>PAUSED</b>';

	this.elParams.innerHTML = params;


	// Add/Remove CSS classes to highlight/colorize/mute:
	if (this.params.renders_total == null)
	{
		this.element.classList.add('empty');
	}
	else
	{
		this.element.classList.remove('empty');
	}
	if (this.params.running_tasks_num)
		this.element.classList.add('running');
	else
		this.element.classList.remove('running');


	var name = '<b>' + cm_PathBase(this.params.name) + '/</b>';
	if (this.params.pattern)
		name += ' [' + this.params.pattern + ']';
	this.elName.innerHTML = name;


	if (cm_IsPadawan())
	{
		let counts = '';
		if (this.params.pools_total)
			counts = 'Pools Total:<b>' + this.params.pools_total + '</b>';
		this.elPoolsCounts.innerHTML = counts;

		let renders = '';
		if (this.params.renders_total)
			renders += 'Renders Total:<b>' + this.params.renders_total + '</b>';
		if (this.params.renders_busy)
			renders += ' Busy:<b>' + this.params.renders_busy + '</b>';
		if (this.params.renders_ready)
			renders += ' Ready:<b>' + this.params.renders_ready + '</b>';
		if (this.params.renders_online)
			renders += ' Online:<b>' + this.params.renders_online + '</b>';
		if (this.params.renders_offline)
			renders += ' Offline:<b>' + this.params.renders_offline + '</b>';
		if (this.params.renders_nimby)
			renders += ' Nimby:<b>' + this.params.renders_nimby + '</b>';
		if (this.params.renders_paused)
			renders += ' Paused:<b>' + this.params.renders_paused + '</b>';
		if (this.params.renders_sick)
			renders += ' Sick:<b>' + this.params.renders_sick + '</b>';

		if (this.params.run_tasks)
			renders += ' Running Tasks:<b>' + this.params.run_tasks + '</b>';
		if (this.params.run_capacity)
			renders += ' Capacity:<b>' + this.params.run_capacity + '</b>';

		this.elRendersCounts.innerHTML = renders;
	}
	else if (cm_IsJedi())
	{
		let counts = '';
		if (this.params.pools_total)
			counts = 'Pools:<b>' + this.params.pools_total + '</b>';
		this.elPoolsCounts.innerHTML = counts;

		let renders = '';
		if (this.params.renders_total)
			renders += 'Renders:<b>' + this.params.renders_total + '</b>';
		if (this.params.renders_busy)
			renders += ' Busy:<b>' + this.params.renders_busy + '</b>';
		if (this.params.renders_ready)
			renders += ' RDY:<b>' + this.params.renders_ready + '</b>';
		if (this.params.renders_online)
			renders += ' ON:<b>' + this.params.renders_online + '</b>';
		if (this.params.renders_offline)
			renders += ' OFF:<b>' + this.params.renders_offline + '</b>';
		if (this.params.renders_nimby)
			renders += ' Nby:<b>' + this.params.renders_nimby + '</b>';
		if (this.params.renders_paused)
			renders += ' Pau:<b>' + this.params.renders_paused + '</b>';
		if (this.params.renders_sick)
			renders += ' Sick:<b>' + this.params.renders_sick + '</b>';

		if (this.params.run_tasks)
			renders += ' Tasks:<b>' + this.params.run_tasks + '</b>';
		if (this.params.run_capacity)
			renders += ' Capacity:<b>' + this.params.run_capacity + '</b>';

		this.elRendersCounts.innerHTML = renders;
	}
	else
	{
		let counts = '';
		if (this.params.pools_total)
			counts = 'Pools:<b>' + this.params.pools_total + '</b>';
		this.elPoolsCounts.innerHTML = counts;

		let renders = '';
		if (this.params.renders_total)
			renders += 'Renders:<b>' + this.params.renders_total + '</b>';
		if (this.params.renders_busy)
			renders += ' B:<b>' + this.params.renders_busy + '</b>';
		if (this.params.renders_ready)
			renders += ' RDY:<b>' + this.params.renders_ready + '</b>';
		if (this.params.renders_online)
			renders += ' ON:<b>' + this.params.renders_online + '</b>';
		if (this.params.renders_offline)
			renders += ' OFF:<b>' + this.params.renders_offline + '</b>';
		if (this.params.renders_nimby)
			renders += ' N:<b>' + this.params.renders_nimby + '</b>';
		if (this.params.renders_paused)
			renders += ' P:<b>' + this.params.renders_paused + '</b>';
		if (this.params.renders_sick)
			renders += ' S:<b>' + this.params.renders_sick + '</b>';

		if (this.params.run_tasks)
			renders += ' T:<b>' + this.params.run_tasks + '</b>';
		if (this.params.run_capacity)
			renders += ' C:<b>' + this.params.run_capacity + '</b>';

		this.elRendersCounts.innerHTML = renders;
	}

	// Host properites
	let hostProps = '';
	if (this.params.power_host != null)
		hostProps += this.params.power_host;
	if (this.params.properties_host)
		hostProps += this.params.properties_host;
	this.elHostProperties.innerHTML = '<b>' + hostProps + '</b>';

	// Show servives:
	this.elServices.textContent = '';
	farm_showServices(this.elServices, this.params, 'pool');

	// Show tickets:
	this.elTickets.textContent = '';
	farm_showTickets(this.elTickets, this.params.tickets_pool, 'pool', this);
	farm_showTickets(this.elTickets, this.params.tickets_host, 'host', this);

	// Annotation
	if (this.params.annotation)
		this.elAnnotation.innerHTML = this.params.annotation;
	else
		this.elAnnotation.textContent = '';
};

PoolNode.prototype.refresh = function() {};

PoolNode.createPanels = function(i_monitor) {
	// Admin related functions:
	if (!g_GOD())
		return;

	var acts = {};
	acts.add_pool =
		{'label': 'ADD POOL', 'node_type': 'pools', 'handle': 'addPoolDialog', 'tooltip': 'Add pool'};
	/*	acts.revert_renders = {
			'label': 'REVERT',
			'node_type': 'pools',
			'handle': 'mh_Oper',
			'tooltip': 'Revert renders'};*/
	i_monitor.createCtrlBtns(acts);


	var acts = {};
	acts.new_nimby = {
		'name': 'new_nimby',
		'value': true,
		'handle': 'mh_Param',
		'label': 'Nimby',
		'tooltip': 'New render will be registered as nimby.'
	};
	acts.new_free = {
		'name': 'new_nimby',
		'value': false,
		'handle': 'mh_Param',
		'label': 'Free',
		'tooltip': 'New render will NOT be registered as nimby.'
	};
	acts.new_paused = {
		'name': 'new_paused',
		'value': true,
		'handle': 'mh_Param',
		'label': 'Paused',
		'tooltip': 'New render will be registered as paused.'
	};
	acts.new_notpaused = {
		'name': 'new_paused',
		'value': false,
		'handle': 'mh_Param',
		'label': 'Ready',
		'tooltip': 'New render will NOT be registered as paused.'
	};
	i_monitor.createCtrlBtn({
		'name': 'new_render',
		'label': 'New Render',
		'node_type': 'pools',
		'tooltip': 'New render status.',
		'sub_menu': acts
	});
};

PoolNode.prototype.updatePanels = function() {
	// Info:
	var info = '';
	info += '<p>ID = ' + this.params.id + '</p>';
	info += '<p>Created at: ' + cm_DateTimeStrFromSec(this.params.time_creation) + '</p>';
	if (this.params.time_empty)
	{
		info += '<p>Empty for: ' + cm_TimeStringInterval(this.params.time_empty);
		info += ', since: ' + cm_DateTimeStrFromSec(this.params.time_empty) + '</p>';
	}

	this.monitor.setPanelInfo(info);

	farm_showServicesInfo(this);

	farm_showTicketsInfo(this, 'pool');
	farm_showTicketsInfo(this, 'host');
};

// RenderNode here! This not an error!
// We can't use PoolNode as this is not prototype function.
RenderNode.addPoolDialog = function(i_args) {
	new cgru_Dialog({
		'wnd': i_args.monitor.window,
		'receiver': i_args.monitor.cur_item,
		'handle': 'addPoolDo',
		'param': i_args.name,
		'name': 'add_pool',
		'title': 'Add a child pool to this poll',
		'info': 'Enter a new child pool name:'
	});
};

PoolNode.prototype.addPoolDo = function(i_value, i_name) {
	g_Info('Adding a pool "' + i_value + '" to "' + this.params.name + '"');
	var operation = {};
	operation.type = 'add_pool';
	operation.name = i_value;
	nw_Action('pools', [this.monitor.getSelectedIds()[0]], operation);
};

PoolNode.prototype.serviceApply = function(i_value, i_name) {
	g_Info('menuHandleService = ' + i_name + ',' + i_value);
	var operation = {};
	operation.type = 'farm';
	operation.mode = i_name;
	operation.name = i_value;
	operation.mask = i_value;
	nw_Action('pools', this.monitor.getSelectedIds(), operation);
};

PoolNode.prototype.editTicket = function(i_value, i_param) {
	var type;
	var count = 1;
	var max_hosts = null;

	if (i_param == 'ticket_edit_host')
	{
		type = 'host';
		if (this.params.tickets_host && this.params.tickets_host[i_value])
			count = this.params.tickets_host[i_value][0];
	}
	else
	{
		type = 'pool';
		if (this.params.tickets_pool && this.params.tickets_pool[i_value])
		{
			count = this.params.tickets_pool[i_value][0];
			max_hosts = this.params.tickets_pool[i_value][3];
		}
	}

	farm_ticketEditDialog(i_value, count, max_hosts, type, this);
};


PoolNode.prototype.onDoubleClick = function(e) {
	g_ShowObject({'object': this.params}, {'evt': e, 'wnd': this.monitor.window});
};

PoolNode.params_pool = {
	pattern: {'type': 'str', 'label': 'Pattern'},
	priority: {'type': 'num', 'label': 'Priority'},
	max_tasks_host: {'type': 'num', 'label': 'Host Max Tasks'},
	capacity_host: {'type': 'num', 'label': 'Host Max Capacity'},
	sick_errors_count: {'type': 'num', 'label': 'Sick Errors Count'},
	annotation: {'type': 'str', 'label': 'Annotation'},

	idle_wolsleep_time: {'type': 'num', 'label': 'Idle WOL sleep time'},
	idle_free_time: {'type': 'num', 'label': 'Idle FREE time'},
	busy_nimby_time: {'type': 'num', 'label': 'Busy NIMBY time'},
	idle_cpu: {'type': 'num', 'label': 'Idle CPU percent'},
	busy_cpu: {'type': 'num', 'label': 'Busy CPU percent'},
	idle_mem: {'type': 'num', 'label': 'Idle memory percent'},
	busy_mem: {'type': 'num', 'label': 'Busy memory percent'},
	idle_swp: {'type': 'num', 'label': 'Idle swap percent'},
	busy_swp: {'type': 'num', 'label': 'Busy swap percent'},
	idle_hddgb: {'type': 'num', 'label': 'Idle HDD Gb'},
	busy_hddgb: {'type': 'num', 'label': 'Busy HDD Gb'},
	idle_hddio: {'type': 'num', 'label': 'Idle HDD I/O'},
	busy_hddio: {'type': 'num', 'label': 'Busy HDD I/O'},
	idle_netmbs: {'type': 'num', 'label': 'Idle Network Mb/sec'},
	busy_netmbs: {'type': 'num', 'label': 'Busy Network Mb/sec'}
};

PoolNode.createParams = function() {
	if (PoolNode.params_created)
		return;

	PoolNode.params = {};
	for (let p in PoolNode.params_pool)
	{
		PoolNode.params[p] = PoolNode.params_pool[p];
		PoolNode.params[p].permissions = 'god';
		PoolNode.params[p].node_type = 'pools';
	}

	PoolNode.params_created = true;
};
