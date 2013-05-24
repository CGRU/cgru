function RenderNode() {}

RenderNode.prototype.init = function()
{
	this.element.classList.add('render');

	cm_CreateStart( this);

	this.elIdleBox = document.createElement('div');
	this.elIdleBox.classList.add('idle_box');
	this.element.appendChild( this.elIdleBox);
	this.elIdleBar = document.createElement('div');
	this.elIdleBar.classList.add('idle_bar');
	this.elIdleBox.appendChild( this.elIdleBar);

	this.elName = cm_ElCreateText( this.element, 'Client Host Name');
	this.elName.classList.add('name');
	this.elName.classList.add('prestar');

	this.elVersion = cm_ElCreateText( this.element, 'Client Version');

	this.elPriority = document.createElement('span');
	this.element.appendChild( this.elPriority);
	this.elPriority.style.cssFloat = 'right';
	this.elPriority.title = 'Priority';

	this.elUserName = cm_ElCreateFloatText( this.element, 'right', 'User Name and "Nimby" Status');

	this.elResources = document.createElement('div');
	this.element.appendChild( this.elResources);
	this.elResources.className = 'resources';

	this.elNewLine = document.createElement('br');
	this.element.appendChild( this.elNewLine);

	this.elCapacity = cm_ElCreateText( this.element, 'Capacity: Used/Total');
	this.elCapacity.classList.add('prestar');
	this.elMaxTasks = cm_ElCreateText( this.element, 'Tasks: Running/Maximum');
	this.elStateTime = cm_ElCreateFloatText( this.element, 'right', 'Busy/Free Status and Time');

	this.elAnnotation = document.createElement('div');
	this.element.appendChild( this.elAnnotation);
	this.elAnnotation.title = 'Annotation';
	this.elAnnotation.style.textAlign = 'center';

	this.plotters = [];
	this.plotterC = new Plotter( this.plotters, this.elResources, 'C', 'CPU');
	this.plotterM = new Plotter( this.plotters, this.elResources, 'M', 'Memory');
	this.plotterS = new Plotter( this.plotters, this.elResources, 'S', 'Swap');
	this.plotterH = new Plotter( this.plotters, this.elResources, 'H', 'HDD Space');
	this.plotterN = new Plotter( this.plotters, this.elResources, 'N', 'Network I/O');
	this.plotterD = new Plotter( this.plotters, this.elResources, 'D', 'Disk I/O');

	this.plotterC.addGraph();
	this.plotterC.setColor([200,   0,  0]);
	this.plotterC.addGraph();
	this.plotterC.setColor([ 50, 200, 20]);

	this.plotterM.addGraph();
	this.plotterM.setColor([50, 200, 20], [255, 0, 0]);
	this.plotterM.addGraph();
	this.plotterM.setColor([ 0,  50,  0]);

	this.plotterS.addGraph();
	this.plotterS.setColor([100, 200, 30], [255, 0, 0]);

	this.plotterH.addGraph( 10);
	this.plotterH.setColor([50, 200, 20], [ 255, 0, 0]);

	this.plotterN.addGraph();
	this.plotterN.setColor([90, 200, 20], [ 255, 0, 10]);
	this.plotterN.addGraph();
	this.plotterN.setColor([20, 200, 90], [ 255, 0, 90]);
	this.plotterN.setScale(-1, 10000, 100000);
	this.plotterN.setAutoScale( 1000, 100000);

	this.plotterD.addGraph();
	this.plotterD.setColor([90, 200, 20], [ 250, 50, 20]);
	this.plotterD.addGraph();
	this.plotterD.setColor([20, 200, 90], [ 250, 50, 90]);
	this.plotterD.setScale(-1, 10000, 100000);
	this.plotterD.setAutoScale( 1000, 100000);

	this.elPower = document.createElement('div');
	this.elPower.classList.add('power');
	this.element.appendChild( this.elPower);

	this.state = {};
	this.tasks = [];
}

RenderNode.prototype.update = function( i_obj)
{
	// When render write resources, it writes resources only, event no name
	if(( i_obj != null ) && ( i_obj.name == null ))
	{
		if( i_obj.idle_time )
			this.params.idle_time = i_obj.idle_time;
		if( i_obj.busy_time )
			this.params.busy_time = i_obj.busy_time;

		var r = i_obj.host_resources;

		if( r == null ) return;

		if(( this.state.ONL != true ) || ( this.host_resources == null ))
		{
			// If render just become online,
			// or resources reciedved fisrt time,
			// we need to set plotter scales:

			this.plotterC.setTitle('CPU: '+r.cpu_mhz+' MHz x'+r.cpu_num);

			this.plotterM.setTitle('Memory: '+r.mem_total_mb+' Mb');
			this.plotterM.setScale( r.mem_total_mb, 85 * r.mem_total_mb / 100, r.mem_total_mb);

			if( r.swap_total_mb > 0 )
			{
				this.plotterS.setTitle('Swap: '+r.swap_total_mb+' Mb');
				this.plotterS.setScale( r.swap_total_mb, r.swap_total_mb / 10, r.swap_total_mb);
			}
			else
			{
				this.plotterS.setTitle('Swap I/O');
				this.plotterS.setScale( -1, 100, 10000);
				this.plotterS.setAutoScale( 1000, 100000);
			}

			this.plotterH.setTitle('HDD Space: '+r.hdd_total_gb+' Gb');
			this.plotterH.setScale( r.hdd_total_gb, 95 * r.hdd_total_gb / 100, r.hdd_total_gb);
		}

		this.host_resources = r;

		var usr = r.cpu_user + r.cpu_nice;
		var sys = r.cpu_system + r.cpu_iowait + r.cpu_irq + r.cpu_softirq;
		this.plotterC.appendTitle('\nUsage: '+usr+'%\nSystem: '+sys+'%');
		this.plotterC.addValues([ sys, usr]);

		var mem = r.mem_total_mb - r.mem_free_mb;
		var buf = r.mem_cached_mb + r.mem_buffers_mb;
		this.plotterM.appendTitle('\n-buffered: '+buf+'\nUsed: '+mem+' Mb');
		this.plotterM.addValues([ mem, buf]);

		this.plotterS.addValues([ r.swap_used_mb]);
		if( r.swap_total_mb > 0 )
			this.plotterS.appendTitle('\nUsed: '+r.swap_used_mb+' Mb');
		else
			this.plotterS.appendTitle('\nUsage: '+r.swap_used_mb+' MBytes/s');

		this.plotterH.appendTitle('\nFree: '+r.hdd_free_gb+' Gb');
		this.plotterH.addValues([ r.hdd_total_gb - r.hdd_free_gb]);

		this.plotterN.appendTitle('\nReceive: '+Math.round(r.net_recv_kbsec/1024)+' MBytes/s\nSend: '+Math.round(r.net_send_kbsec/1024)+' MBytes/s')
		this.plotterN.addValues([ r.net_recv_kbsec, r.net_send_kbsec]);

		this.plotterD.appendTitle('\nRead: '+Math.round(r.hdd_rd_kbsec/1024)+ 'MBytes/s\nWrite: '+Math.round(r.hdd_wr_kbsec/1024)+' MBytes/s\nBusy: '+r.hdd_busy+'%');
		this.plotterD.addValues([ r.hdd_rd_kbsec, r.hdd_wr_kbsec], r.hdd_busy / 100);

		this.params.tasks_percents = i_obj.tasks_percents;
		this.updateTasksPercents();

		return;
	}

	if( i_obj ) this.params = i_obj;

	cm_GetState( this.params.state, this.state, this.element);

	this.elName.textContent = this.params.name;
	this.elName.title = this.params.host.os;

	if( this.params.version != null )
		this.elVersion.textContent = 'v' + this.params.version;
	else
		this.elVersion.textContent = '';

	this.elPriority.textContent = '-' + this.params.priority;

	var user = this.params.user_name;
	if( this.state.NbY ) user = 'nimby(' + user + ')n';
	else if( this.state.NBY ) user = 'NIMBY(' + user + ')N';
	this.elUserName.textContent = user;

	if( this.params.annotation )
		this.elAnnotation.textContent = this.params.annotation;
	else
		this.elAnnotation.textContent = '';

	for( var i = 0; i < this.plotters.length; i++)
		this.plotters[i].setHidden( this.state.OFF)

	if( this.state.WWK ) this.offlineState = 'Waking Up';
	else if( this.state.WSL || this.state.WFL) this.offlineState = 'Sleeping';
	else this.offlineState = 'Offline';


	if( this.state.OFF == true )
	{
		this.elStar.style.display = 'none';
		this.clearTasks();
		this.elCapacity.textContent = '';
		this.elMaxTasks.textContent = '';
		this.state.textContent = '';
		this.elStateTime.textContent = '';
		this.elNewLine.style.display = 'none';
		return;
	}
	this.elPower.textContent = '';
	this.elNewLine.style.display = 'block';

	if( this.params.capacity == null)
		this.params.capacity = this.params.host.capacity;
	var capacity = this.params.capacity_used + '/' + this.params.capacity;
	this.elCapacity.textContent = '['+capacity+']';

	if( this.params.max_tasks == null )
		this.params.max_tasks = this.params.host.max_tasks;

	if( this.state.RUN == true )
	{
		this.elStar.style.display = 'block';
		this.elStarCount.textContent = this.params.tasks.length;
		var max_tasks = '(' + this.params.tasks.length + '/' + this.params.max_tasks + ')';
	}
	else
	{
		this.elStar.style.display = 'none';
		var max_tasks = '(0/' + this.params.max_tasks + ')';
	}
	this.elMaxTasks.textContent = max_tasks;	

	this.clearTasks();
	if( this.params.tasks != null )
	for( var t = 0; t < this.params.tasks.length; t++)
		this.tasks.push( new RenderTask( this.params.tasks[t], this.element));

	this.updateTasksPercents();
	this.refresh();
}

RenderNode.prototype.clearTasks = function()
{
	for( var t = 0; t < this.tasks.length; t++)
		this.tasks[t].destroy();
	this.tasks = [];
}

RenderNode.prototype.refresh = function()
{
	if( this.state.OFF || this.state.WFL )
	{
		var power = this.offlineState;
		if( this.params.wol_operation_time )
			power += ' ' + cm_TimeStringInterval( this.params.wol_operation_time);
		this.elPower.textContent = power;
		if( this.state.WFL || this.state.WSL || this.state.WWK )
			this.elPower.style.color = '#FF0';
		else
			this.elPower.style.color = '#000';
		this.elIdleBox.style.display = 'none';
		if( this.state.OFF )
			return;
	}

	var stateTime = 'NEW';
	var stateTimeTitle = 'Idle time: ' + cm_TimeStringInterval( this.params.idle_time);
	stateTimeTitle += '\nIdle CPU < ' + this.params.host.idle_cpu + '%';

	// Draw idle bar (almost in all cases)
	if(( this.params.host.wol_idlesleep_time > 0 ) ||
		( this.params.host.nimby_idlefree_time > 0 ) ||
		( this.params.host.nimby_busyfree_time > 0 ) ||
		( cgru_Config.af_monitor_render_idle_bar_max > 0 ))
	{
		var curtime = new Date();
		var idle_sec = curtime.valueOf() / 1000.0 - this.params.idle_time;
		if( idle_sec < 0 ) idle_sec = 0;
		var busy_sec = curtime.valueOf() / 1000.0 - this.params.busy_time;
		if( busy_sec < 0 ) busy_sec = 0;
		var percent = null;

		if(( this.params.host.nimby_idlefree_time > 0 ) &&
			( this.state.RUN != true ) &&
			( this.state.NbY || this.state.NBY ))
		{
			stateTimeTitle += '\nNimby idle free time: ' + cm_TimeStringFromSeconds( this.params.host.nimby_idlefree_time);
			percent = Math.round( 100.0 * idle_sec / this.params.host.nimby_idlefree_time );

			idle_sec = Math.round( this.params.host.nimby_idlefree_time - idle_sec);
			if( idle_sec > 0 )
				this.elIdleBox.title = 'Nimby idle free in '+cm_TimeStringFromSeconds( idle_sec);
			else
				this.elIdleBox.title = 'Nimby free';
			this.elIdleBox.classList.remove('wol');
			this.elIdleBox.classList.add('free');
			this.elIdleBox.classList.remove('nimby');
		}
		else if(( this.params.host.nimby_busyfree_time > 0 ) &&
			( busy_sec > 6 ) &&
			( this.state.RUN != true ) &&
			( this.state.NbY != true ) && ( this.state.NBY != true ))
		{
			stateTimeTitle += '\nBusy free Nimby time: ' + cm_TimeStringFromSeconds( this.params.host.nimby_busyfree_time);
			percent = Math.round( 100.0 * busy_sec / this.params.host.nimby_busyfree_time );

			busy_sec = Math.round( this.params.host.nimby_busyfree_time - busy_sec);
			if( busy_sec > 0 )
				this.elIdleBox.title = 'Nimby busy in '+cm_TimeStringFromSeconds( busy_sec);
			else
				this.elIdleBox.title = 'Nimby busy';
			this.elIdleBox.classList.remove('wol');
			this.elIdleBox.classList.remove('free');
			this.elIdleBox.classList.add('nimby');
		}
		else if(( this.params.host.wol_idlesleep_time > 0 ) &&
			( this.state.RUN != true ))
		{
			stateTimeTitle += '\nWOL idle sleep time: ' + cm_TimeStringFromSeconds( this.params.host.wol_idlesleep_time);
			percent = Math.round( 100.0 * idle_sec / this.params.host.wol_idlesleep_time );

			idle_sec = Math.round( this.params.host.wol_idlesleep_time - idle_sec);
			if( idle_sec > 0 )
				this.elIdleBox.title = 'WOL idle sleep in '+cm_TimeStringFromSeconds( idle_sec);
			else
				this.elIdleBox.title = 'WOL sleep';
			this.elIdleBox.classList.add('wol');
			this.elIdleBox.classList.remove('free');
			this.elIdleBox.classList.remove('nimby');
		}
		else
		{
			stateTimeTitle += '\nIdle bar time: ' + cm_TimeStringFromSeconds( cgru_Config.af_monitor_render_idle_bar_max);
			percent = Math.round( 100.0 * idle_sec / cgru_Config.af_monitor_render_idle_bar_max );
			this.elIdleBox.classList.remove('wol');
			this.elIdleBox.classList.remove('free');
			this.elIdleBox.classList.remove('nimby');
		}

		if( percent > 100 ) percent = 100;
		if( percent <   0 ) percent = 0;
		this.elIdleBox.style.display = 'block';
		this.elIdleBar.style.width = percent+'%';
	}
	else
		this.elIdleBox.style.display = 'none';
	if(( this.params.task_start_finish_time != null ) && ( this.params.task_start_finish_time > 0 ))
	{
		stateTime = cm_TimeStringInterval( this.params.task_start_finish_time);
		if( this.state.RUN == true )
			stateTime += ' busy';
		else
			stateTime += ' free';
	}
	this.elStateTime.textContent = stateTime;
	this.elStateTime.title = stateTimeTitle;

	for( var t = 0; t < this.tasks.length; t++)
		this.tasks[t].refresh();
}

RenderNode.prototype.updateTasksPercents = function()
{
	if( this.params.tasks_percents )
		for( var t = 0; t < this.tasks.length; t++)
			this.tasks[t].setPercent( this.params.tasks_percents[t]);
}

RenderNode.prototype.onDoubleClick = function()
{
	nw_GetNodes('renders', [this.params.id], 'full');
}

RenderNode.prototype.menuHandleServiceDialog = function( i_name)
{
	new cgru_Dialog( this.monitor.window, this, 'menuHandleServiceApply', i_name, 'str', null, this.type+'_parameter', (i_name == 'enable' ? 'Enable':'Disable') + ' Service', 'Enter Service Name:');
}
RenderNode.prototype.menuHandleServiceApply = function( i_name, i_value)
{
g_Info('menuHandleService = ' + i_name + ',' + i_value);
	var operation = {};
	operation.type = 'service';
	operation.name = i_value;
	if( i_name == 'enable' )
		operation.enable = true;
	else if( i_name == 'disable' )
		operation.enable = false;
	else return;
	nw_Action( 'renders', this.monitor.getSelectedIds(), operation, null);
}

function RenderTask( i_task, i_elParent)
{
	this.elParent = i_elParent;

	this.elRoot = document.createElement('div');
	i_elParent.appendChild( this.elRoot);

	this.elIcon = document.createElement('img');
	this.elRoot.appendChild( this.elIcon);
	this.elIcon.src = 'icons/software/'+i_task.service+'.png';
	this.elIcon.style.position = 'absolute';
	this.elIcon.style.width = '18px';
	this.elIcon.style.height = '18px';
	this.elIcon.title = i_task.service;
//	this.elIcon.classList.add('icon');

	this.element = document.createElement('div');
	this.elRoot.appendChild( this.element);
	this.element.style.marginLeft = '20px';
	this.element.classList.add('rendertask');

	this.elBar = document.createElement('div');
	this.element.appendChild( this.elBar);
	this.elBar.classList.add('bar');
	this.elBar.style.display = 'none';

	this.elBody = document.createElement('div');
	this.element.appendChild( this.elBody);
	this.elBody.classList.add('body');

	this.elCapacity = cm_ElCreateText( this.elBody, 'Task Capacity');
	this.elCapacity.textContent = '['+i_task.capacity+']';

	this.elJob = cm_ElCreateText( this.elBody, 'Job Name');
	this.elJob.textContent = i_task.job_name;

	this.elBlock = cm_ElCreateText( this.elBody, 'Block Name');
	this.elBlock.textContent = '['+i_task.block_name+']';

	this.elName = cm_ElCreateText( this.elBody, 'Task Name');
	this.elName.textContent = '['+i_task.name+']';

	this.elTime = cm_ElCreateFloatText( this.elBody, 'right', 'Running Time');
	this.time = i_task.time_start;

	this.elUser = cm_ElCreateFloatText( this.elBody, 'right', 'User Name');
	this.elUser.textContent = i_task.user_name;

	this.refresh();
}

RenderTask.prototype.refresh = function()
{
	this.elTime.textContent = cm_TimeStringInterval( this.time);
}

RenderTask.prototype.setPercent = function( i_percent)
{
	if( i_percent == null ) return;
	if( i_percent < 1 ) return;
	this.elBar.style.display = 'block';
	this.elBar.style.width = i_percent+'%';
}

RenderTask.prototype.destroy = function()
{
	this.elParent.removeChild( this.elRoot);
}


RenderNode.actions = [];

RenderNode.actions.push(['context', 'log',       null, 'menuHandleGet',   'Show Log']);
RenderNode.actions.push(['context', 'tasks_log', null, 'menuHandleGet',   'Tasks Log']);
RenderNode.actions.push(['context', 'full',      null, 'menuHandleGet',   'Full Info']);
RenderNode.actions.push(['context',  null]);
RenderNode.actions.push(['context', 'nimby',     true, 'menuHandleParam', 'Set nimby']);
RenderNode.actions.push(['context', 'NIMBY',     true, 'menuHandleParam', 'Set NIMBY']);
RenderNode.actions.push(['context', 'nimbyOff', false, 'menuHandleParam', 'Set Free', null, 'nimby']);
RenderNode.actions.push(['context',  null]);
RenderNode.actions.push(['context', 'eject_tasks',         null, 'menuHandleOperation', 'Eject Tasks']);
RenderNode.actions.push(['context', 'eject_tasks_keep_my', null, 'menuHandleOperation', 'Eject Not My']);

RenderNode.actions.push(['set', 'priority',     'num', 'menuHandleDialog', 'Priority']);
RenderNode.actions.push(['set', 'capacity',     'num', 'menuHandleDialog', 'Capacity']);
RenderNode.actions.push(['set', 'max_tasks',    'num', 'menuHandleDialog', 'Maximum Tasks']);
RenderNode.actions.push(['set', 'restore_defaults','num', 'menuHandleOperation', 'Restore Defaults']);
RenderNode.actions.push(['set',  null]);
RenderNode.actions.push(['set', 'enable',  null, 'menuHandleServiceDialog', 'Enable Service']);
RenderNode.actions.push(['set', 'disable', null, 'menuHandleServiceDialog', 'Disable Service']);
RenderNode.actions.push(['set',  null]);
RenderNode.actions.push(['set', 'hidden',       'bl1', 'menuHandleDialog', 'Hide/Unhide']);
RenderNode.actions.push(['set',  null]);
RenderNode.actions.push(['set', 'user_name',    'str', 'menuHandleDialog', 'User Name']);
RenderNode.actions.push(['set',  null]);
RenderNode.actions.push(['set', 'annotation',   'str', 'menuHandleDialog', 'Annotation']);

RenderNode.actions.push(['pow', 'wol_sleep', null, 'menuHandleOperation', 'WOL Sleep']);
RenderNode.actions.push(['pow', 'wol_wake' , null, 'menuHandleOperation', 'WOL Wake']);
RenderNode.actions.push(['pow', 'exit',      null, 'menuHandleOperation', 'Exit Client']);
RenderNode.actions.push(['pow', 'reboot',    null, 'menuHandleOperation', 'Reboot Machine']);
RenderNode.actions.push(['pow', 'shutdown',  null, 'menuHandleOperation', 'Shutdown Machine']);
RenderNode.actions.push(['pow', 'delete',    null, 'menuHandleOperation', 'Delete From DB']);

RenderNode.sort = ['priority','user_name','name'];
RenderNode.filter = ['user_name','name','host_name'];

RenderNode.actionsCreated = false;
RenderNode.createActions = function()
{
	if( RenderNode.actionsCreated ) return;

	if( cgru_Config.af_rendercmds ) for( var i = 0; i < cgru_Config.af_rendercmds.length; i++)
		RenderNode.actions.push(['cmd', 'cmd', null, cgru_Config.af_rendercmds[i], cgru_Config.af_rendercmds[i]]);

	if( cgru_Config.af_rendercmds_admin ) for( var i = 0; i < cgru_Config.af_rendercmds_admin.length; i++)
		RenderNode.actions.push(['cmd', 'cmd', null, cgru_Config.af_rendercmds_admin[i], cgru_Config.af_rendercmds_admin[i], 'god']);

	RenderNode.actionsCreated = true;
}

