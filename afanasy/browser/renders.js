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

	if( cm_IsSith())
	{
		this.elPriority = document.createElement('div');
		this.element.appendChild( this.elPriority);
		this.elPriority.style.cssFloat = 'right';
		this.elPriority.title = 'Priority';
	}
	else
		this.elPriority = cm_ElCreateFloatText( this.element, 'right', 'Priority');

	this.elUserName = cm_ElCreateFloatText( this.element, 'right', 'User Name and "Nimby" Status');

	this.elResources = document.createElement('div');
	this.element.appendChild( this.elResources);
	this.elResources.className = 'resources';
	this.elResources.style.display = 'none';

	this.elNewLine = document.createElement('br');
	this.element.appendChild( this.elNewLine);

	this.elCapacity = cm_ElCreateText( this.element, 'Capacity: Used / Total');
	this.elCapacity.classList.add('prestar');
	this.elMaxTasks = cm_ElCreateText( this.element, 'Tasks: Running / Maximum');
	this.elStateTime = cm_ElCreateFloatText( this.element, 'right', 'Busy/Free Status and Time');

	this.elAnnotation = document.createElement('div');
	this.element.appendChild( this.elAnnotation);
	this.elAnnotation.title = 'Annotation';
	this.elAnnotation.style.textAlign = 'center';

	this.plotters = [];
	this.plotterC = new Plotter( this.elResources, 'C', 'CPU');         this.plotters.push( this.plotterC);
	this.plotterM = new Plotter( this.elResources, 'M', 'Memory');      this.plotters.push( this.plotterM);
	this.plotterS = new Plotter( this.elResources, 'S', 'Swap');        this.plotters.push( this.plotterS);
	this.plotterH = new Plotter( this.elResources, 'H', 'HDD Space');   this.plotters.push( this.plotterH);
	this.plotterN = new Plotter( this.elResources, 'N', 'Network I/O'); this.plotters.push( this.plotterN);
	this.plotterD = new Plotter( this.elResources, 'D', 'Disk I/O');    this.plotters.push( this.plotterD);

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

	this.plottersCs = [];

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

		if(( this.state.ONL != true ) || ( this.params.host_resources == null ))
		{
			// If render just become online,
			// or resources received first time,
			// we need to set plotter scales:

			this.elResources.style.display = 'block';

			this.plotterC.setTitle('CPU: '+r.cpu_mhz+' MHz x'+r.cpu_num);

			this.plotterM.setTitle('Memory:\nTotal: '+r.mem_total_mb+' Mb');
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

		var pl_w = Math.round( this.element.clientWidth / 11 - 4);
		for( var i = 0; i < this.plotters.length; i++)
		{
			this.plotters[i].setWidth( pl_w);
			var dx = this.plotters[i].width + 8;
			this.plotters[i].element.style.left = (-3*dx + i * dx) + 'px';
			this.plotters[i].element.style.top = '2px';
		}


		this.params.host_resources = r;

		var usr = r.cpu_user + r.cpu_nice;
		var sys = r.cpu_system + r.cpu_iowait + r.cpu_irq + r.cpu_softirq;
		this.plotterC.appendTitle('\nUsage: '+usr+'%\nSystem: '+sys+'%');
		this.plotterC.addValues([ sys, usr]);

		var mem = r.mem_total_mb - r.mem_free_mb;
		var buf = r.mem_cached_mb + r.mem_buffers_mb;
		this.plotterM.appendTitle('\nUsed: '+mem+'Mb\nCached: '+buf+' Mb\nFree: '+r.mem_free_mb+'Mb');
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

		if( r.custom )
		{
			if( this.plottersCs.length != r.custom.length )
			{
				this.plottersCsDelete();
				this.elPlottersCs = document.createElement('div');
				this.elPlottersCs.classList.add('plotters_custom_div');
				this.element.appendChild( this.elPlottersCs);
				for( var i = 0; i < r.custom.length; i++)
				{	
					var plotter = new Plotter( this.elPlottersCs, 'Cs', 'custom');
					this.plottersCs.push( plotter);
					plotter.element.classList.add('custom');
					plotter.addGraph();
					plotter.setColor([50, 200, 20], [ 255, 0, 0]);
				}
			}

			for( var i = 0; i < r.custom.length; i++)
			{
				this.plottersCs[i].setWidth( Math.round( this.elPlottersCs.clientWidth / r.custom.length ) - 6);
				this.plottersCs[i].setHeight( r.custom[i].height);
				this.plottersCs[i].setScale( r.custom[i].value_max);
				this.plottersCs[i].setColor( r.custom[i].graph_clr);
				this.plottersCs[i].addValues([r.custom[i].value]);
				this.plottersCs[i].setLabel( r.custom[i].label, r.custom[i].label_clr, r.custom[i].label_size);
				this.plottersCs[i].setBGColor( r.custom[i].back_clr);
				this.plottersCs[i].setTitle( r.custom[i].tooltip);
			}
		}
		else if(( r.custom == null ) && ( this.plottersCs.length ))
		{
			this.plottersCsDelete();
		}

		return;
	}

	if( i_obj ) this.params = i_obj;

	cm_GetState( this.params.state, this.state, this.element);

	this.elName.innerHTML = '<b>' + this.params.name + '</b>';
	this.elName.title = this.params.host.os;

	if( this.params.version != null )
		this.elVersion.textContent = 'v' + this.params.version;
	else
		this.elVersion.textContent = '';

	if( cm_IsPadawan())
	{
		this.elPriority.innerHTML = ' Priority:<b>' + this.params.priority + '</b>';
	}
	else if( cm_IsJedi())
	{
		this.elPriority.innerHTML = ' Pri:<b>' + this.params.priority + '</b>';
	}
	else
	{
		this.elPriority.innerHTML = '-<b>' + this.params.priority + '</b>';
	}

	var user = '<b>' + this.params.user_name + '</b>';
	if( this.state.PAU )
    {
        user = 'Paused(<b>' + user + '</b>)<b>P</b>';
        if ( this.state.NBY ) user += '+<b>N</b>';
        if ( this.state.Nby ) user += '+<b>n</b>';
    }
	else if( this.state.NbY ) user = 'nimby(<b>' + user + '</b>)<b>n</b>';
	else if( this.state.NBY ) user = 'NIMBY(<b>' + user + '</b>)<b>N</b>';
	this.elUserName.innerHTML = user;

	if( this.params.annotation )
		this.elAnnotation.innerHTML = '<b><i>' + this.params.annotation + '</b></i>';
	else
		this.elAnnotation.textContent = '';

	if( this.state.WWK ) this.offlineState = 'Waking Up';
	else if( this.state.WSL || this.state.WFL) this.offlineState = 'Sleeping';
	else this.offlineState = 'Offline';


	if( this.state.OFF == true )
	{
		this.elStar.style.display = 'none';
		this.clearTasks();
		this.plottersCsDelete();
		this.elResources.style.display = 'none';
		this.params.host_resources = null;
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
	if( this.params.max_tasks == null )
		this.params.max_tasks = this.params.host.max_tasks;
	this.params.run_tasks = 0;
	if( this.params.tasks )
		this.params.run_tasks = this.params.tasks.length;

	if( cm_IsPadawan())
	{
		this.elCapacity.innerHTML = 'Capacity[ Used:<b>' + this.params.capacity_used + '</b> / Total:<b>' + this.params.capacity + '</b> ]';
		this.elMaxTasks.innerHTML = '( Run:<b>' + this.params.run_tasks + '</b> / Max:<b>' + this.params.max_tasks + '</b> )Tasks';
	}
	else if( cm_IsJedi())
	{
		this.elCapacity.innerHTML = 'Cap[ <b>' + this.params.capacity_used + '</b> / <b>' + this.params.capacity + '</b> ]';
		this.elMaxTasks.innerHTML = '( <b>' + this.params.run_tasks + '</b> / <b>' + this.params.max_tasks + '</b> )Tasks';
	}
	else
	{
		this.elCapacity.innerHTML = '[<b>' + this.params.capacity_used + '</b>/<b>' + this.params.capacity + '</b>]';
		this.elMaxTasks.innerHTML = '(<b>' + this.params.run_tasks + '</b>/<b>' + this.params.max_tasks + '</b>)';
	}

	if( this.state.RUN == true )
	{
		this.elStar.style.display = 'block';
		this.elStarCount.textContent = this.params.tasks.length;
	}
	else
		this.elStar.style.display = 'none';

	this.clearTasks();
	if( this.params.tasks != null )
	for( var t = 0; t < this.params.tasks.length; t++)
		this.tasks.push( new RenderTask( this.params.tasks[t], this.element));

	this.updateTasksPercents();
	this.refresh();
}

RenderNode.prototype.plottersCsDelete = function()
{
	if( this.plottersCs.length == 0 )
		return;

	for( var i = 0; i < this.plottersCs.length; i++ )
	{
		this.elPlottersCs.removeChild( this.plottersCs[i].element );
		delete this.plottersCs[i];
	}

	this.element.removeChild( this.elPlottersCs);
	this.plottersCs = [];
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
			power += ' <b>' + cm_TimeStringInterval( this.params.wol_operation_time) + '</b>';
		this.elPower.innerHTML = power;
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
		stateTime = '<b>' + cm_TimeStringInterval( this.params.task_start_finish_time) + '</b>';
		if( this.state.RUN == true )
			stateTime += ' busy';
		else
			stateTime += ' free';
	}
	this.elStateTime.innerHTML = stateTime;
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

RenderNode.prototype.onDoubleClick = function( e)
{
	nw_request({"send":{"get":{"type":'renders',"ids":[this.params.id],"mode":'full'}},"func":g_ShowObject,"evt":e,"wnd":this.monitor.window});
}

RenderNode.setService = function( i_args)
{
	new cgru_Dialog({"wnd":i_args.monitor.window,"receiver":i_args.monitor.cur_item,"handle":'serviceApply',"param":i_args.name,
		"name":'serivce',"title":(i_args.name == 'enable' ? 'Enable':'Disable') + ' Service',"info":'Enter Service Name:'});
}
RenderNode.prototype.serviceApply = function( i_value, i_name)
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

RenderNode.launchCmdExit = function( i_args)
{
//console.log( i_args);
	new cgru_Dialog({"wnd":i_args.monitor.window,"receiver":i_args.monitor.cur_item,"handle":'launchCmdExitDo',"param":i_args.name,
		"name":'serivce',"title":'Launch Command' + (i_args.name == 'lcex' ? ' And Exit':''),"info":'Enter command:'});
}
RenderNode.prototype.launchCmdExitDo = function( i_value, i_name)
{
g_Info('launchCmdExit = ' + i_name + ',' + i_value);
	var operation = {};
	operation.type = 'launch_cmd';
	operation.cmd = i_value;
	if( i_name == 'lcex' )
		operation.exit = true;
	nw_Action( 'renders', this.monitor.getSelectedIds(), operation, null);
}

RenderNode.createPanels = function( i_monitor)
{
	// Info:
	var acts = {};
	acts.tasks_log = {'label':'TSK' ,'tooltip':'Get tasks Log.'};
	acts.full      = {'label':'FULL','tooltip':'Request full render node info.'};
	i_monitor.createCtrlBtn({'name':'info','label':'INFO','tooltip':'Get render info.','handle':'mh_Get','sub_menu':acts});


	// Nimby:
	var acts = {};
	acts.free =  {'name':'nimby', 'value':false,'handle':'mh_Param','label':'FRE','tooltip':'Set render free.'};
	acts.nimby = {'name':'nimby', 'value':true, 'handle':'mh_Param','label':'Nim','tooltip':'Set render nimby.\nRun only owner tasks.'};
	acts.NIMBY = {'name':'NIMBY', 'value':true, 'handle':'mh_Param','label':'NBY','tooltip':'Set render NIMBY.\nDo not run any tasks.'};
	i_monitor.createCtrlBtns( acts);


	// Eject tasks:
	var acts = {};
	acts.eject_tasks         = {'label':'ALL','tooltip':'Eject all running tasks.'};
	acts.eject_tasks_keep_my = {'label':'NOM','tooltip':'Eject not my tasks.'};
	i_monitor.createCtrlBtn({'name':'eject','label':'EJT','tooltip':'Eject tasks from render.','sub_menu':acts});


	// Custom commands:
	var el = document.createElement('div');
	i_monitor.elPanelL.appendChild( el);
	el.classList.add('ctrl_button');
	el.textContent = 'CMD';
	el.monitor = i_monitor;
	el.onclick = function(e){ e.currentTarget.monitor.showMenu(e,'cgru_cmdexec'); return false;}
	el.oncontextmenu = el.onclick;


	// Admin related functions:
	if( ! g_GOD()) return;


	// Paused:
	var acts = {};
	acts.pause   = {'name':'paused', 'value':true,  'handle':'mh_Param','label':'PAU','tooltip':'Set render paused.'};
	acts.unpause = {'name':'paused', 'value':false, 'handle':'mh_Param','label':'UNP','tooltip':'Unset render pause.'};
	i_monitor.createCtrlBtns( acts);


	// Services:
	var acts = {};
	acts.enable           = {'handle':'setService','label':'ENS','tooltip':'Enable service.'};
	acts.disable          = {'handle':'setService','label':'DIS','tooltip':'Disable service.'};
	acts.restore_defaults = {'handle':'mh_Oper',   'label':'DEF','tooltip':'Restore default farm settings.'};
	i_monitor.createCtrlBtn({'name':'services','label':'SRV','tooltip':'Enable/Disable service\nRestore defaults.','sub_menu':acts});


	// Power/WOL:
	var acts = {};
	acts.wol_sleep = {'label':'WSL','tooltip':'Wake-On-Lan sleep.'};
	acts.wol_wake  = {'label':'WWK','tooltip':'Wake-On-Lan wake.'};
	acts.exit      = {'label':'EXT','tooltip':'Exit client.'};
	acts.reboot    = {'label':'REB','tooltip':'Reboot machine.'};
	acts.shutdown  = {'label':'SHD','tooltip':'Shutdown machine.'};
	acts.delete    = {'label':'DEL','tooltip':'Delete render from Afanasy database.'};
	i_monitor.createCtrlBtn({'name':'power','label':'POW','tooltip':'Power / Exit / Delete.','sub_menu':acts});


	// Launch and Exit:
	var acts = {};
	acts.lcmd = {'name':'lcmd','label':'LCMD','handle':'launchCmdExit','tooltip':'Launch command.'};
	acts.lcex = {'name':'lcex','label':'LCEX','handle':'launchCmdExit','tooltip':'Launch command and exit.'};
	i_monitor.createCtrlBtns( acts);
}


RenderNode.prototype.updatePanels = function()
{
	// Info:
	var info = '';

	var r = this.params.host_resources;
	if( r )
	{
		info += r.cpu_mhz+'x'+r.cpu_num+'MHz';
		info += ' '+Math.round(r.mem_total_mb/1024)+'Gb';
		info += ' HDD'+r.hdd_total_gb+'Gb';
		info += '<br>';
	}

	if( this.params.host.nimby_idlefree_time || this.params.host.nimby_busyfree_time )
	{
		info += '<br>Auto Nimby:';
		if( this.params.host.nimby_busyfree_time )
			info += '<br>Busy time: '
				+ cm_TimeStringFromSeconds( this.params.host.nimby_busyfree_time)
				+ ' CPU > ' + this.params.host.nimby_busy_cpu + '%';
		if( this.params.host.nimby_idlefree_time )
			info += '<br>Free time: '
				+ cm_TimeStringFromSeconds( this.params.host.nimby_idlefree_time)
				+ ' CPU: < ' + this.params.host.nimby_idle_cpu + '%';
	}
	info += '<br>';

	info += '<br>Registered:<br> ' + cm_DateTimeStrFromSec( this.params.time_register);
	if( this.params.time_launch )
		info += '<br>Launched:<br> ' + cm_DateTimeStrFromSec( this.params.time_launch);
	if( this.params.idle_time )
		info += '<br>Idle since:<br> ' + cm_DateTimeStrFromSec( this.params.idle_time);
	if( this.params.busy_time )
		info += '<br>Busy since:<br> ' + cm_DateTimeStrFromSec( this.params.busy_time);
	if( this.task_start_finish_time )
		info += '<br>Task finished at:<br> ' + cm_DateTimeStrFromSec( this.params.task_start_finish_time);

	this.monitor.setPanelInfo( info);
}


RenderNode.params = {};
RenderNode.params.priority   = {"type":'num', "permissions":'god', "label":'Priority'};
RenderNode.params.capacity   = {"type":'num', "permissions":'god', "label":'Capacity'};
RenderNode.params.max_tasks  = {"type":'num', "permissions":'god', "label":'Maximum Tasks'};
RenderNode.params.user_name  = {"type":'str', "permissions":'god', "label":'User Name'};
RenderNode.params.annotation = {"type":'str', "permissions":'god', "label":'Annotation'};
RenderNode.params.hidden     = {"type":'bl1', "permissions":'god', "label":'Hide/Unhide'};

RenderNode.sort = ['priority','user_name','name'];
RenderNode.filter = ['user_name','name','host_name'];

RenderNode.actions = [];
RenderNode.actionsCreated = false;
RenderNode.createActions = function()
{
	if( RenderNode.actionsCreated ) return;

	if( cgru_Config.af_rendercmds ) for( var i = 0; i < cgru_Config.af_rendercmds.length; i++)
		RenderNode.actions.push({"mode":'cgru_cmdexec',"name":'cmd',"handle":cgru_Config.af_rendercmds[i],"label":cgru_Config.af_rendercmds[i]});

	if( cgru_Config.af_rendercmds_admin ) for( var i = 0; i < cgru_Config.af_rendercmds_admin.length; i++)
		RenderNode.actions.push({"mode":'cgru_cmdexec',"name":'cmd',"handle":cgru_Config.af_rendercmds_admin[i],"label":cgru_Config.af_rendercmds_admin[i],"permissions":'god'});

	RenderNode.actionsCreated = true;
}

