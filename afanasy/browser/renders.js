function RenderNode() {}

RenderNode.prototype.init = function()
{
	this.elName = document.createElement('span');
	this.elName.classList.add('name');
	this.element.appendChild( this.elName);
//	this.elName.style.cssFloat = 'left';
	this.elName.title = 'Client host name';

	this.elVersion = document.createElement('span');
	this.element.appendChild( this.elVersion);
//	this.elVersion.style.cssFloat = 'left';
	this.elVersion.title = 'Client version';

	this.elPriority = document.createElement('span');
	this.element.appendChild( this.elPriority);
	this.elPriority.style.cssFloat = 'right';
	this.elPriority.title = 'Priority';

	this.elUserName = document.createElement('span');
	this.element.appendChild( this.elUserName);
	this.elUserName.style.cssFloat = 'right';
	this.elUserName.title = 'User name and "Nimby" status'

	this.elResources = document.createElement('div');
	this.element.appendChild( this.elResources);
	this.elResources.className = 'resources';
//	this.elResources.style.textAlign = 'center';

	this.elPower = document.createElement('div');
	this.element.appendChild( this.elPower);
//	this.elResources.appendChild( this.elPower);
	this.elPower.style.position = 'absolute';
	this.elPower.style.textAlign = 'center';
	this.elPower.style.top = '1px';
	this.elPower.style.left = '0';
	this.elPower.style.right = '0';

	this.element.appendChild( document.createElement('br'));

	this.elCapacity = document.createElement('span');
	this.element.appendChild( this.elCapacity);
	this.elCapacity.title = 'Capacity used / total'

	this.elMaxTasks = document.createElement('span');
	this.element.appendChild( this.elMaxTasks);
	this.elMaxTasks.title = 'Running tasks / maximum'

	this.elStateTime = document.createElement('span');
	this.element.appendChild( this.elStateTime);
	this.elStateTime.style.cssFloat = 'right';
	this.elStateTime.title = 'Busy / free status and time';

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

	this.state = {};
}

RenderNode.prototype.update = function()
{
	var was_online = ( this.state.ONL == true )
	cm_GetState( this.params.state, this.element, this.state);
	var became_online = false;
	if( this.state.ONL && ( was_online == false ))
		became_online = true;

	this.elName.innerHTML = this.params.name;
	this.elName.title = this.params.host.os;

	if( this.params.version != null )
		this.elVersion.innerHTML = ' ' + this.params.version;
	else
		this.elVersion.innerHTML = '';

	this.elPriority.innerHTML = '-' + this.params.priority;

	var user = this.params.user_name;
	if( this.state.NBY )
		user = 'NIMBY(' + user + ')N';
	this.elUserName.innerHTML = user;

	if( this.params.annotation )
		this.elAnnotation.innerHTML = this.params.annotation;
	else
		this.elAnnotation.innerHTML = '';

	for( var i = 0; i < this.plotters.length; i++)
		this.plotters[i].setHidden( this.state.OFF)

	if( this.state.WWK ) this.offlineState = 'Waking Up';
	else if( this.state.WSL || this.state.WFL) this.offlineState = 'Sleeping';
	else this.offlineState = 'Offline';


	if( this.state.OFF == true )
	{
		this.elPower.innerHTML = 'Offline';
		this.elCapacity.innerHTML = '';
		this.elMaxTasks.innerHTML = '';
		this.state.innerHTML = '';
		this.elStateTime.innerHTML = '';
		return;
	}
	this.elPower.innerHTML = '';

	var capacity = this.params.capacity;
	if( capacity == null )
		capacity = this.params.host.capacity;
	capacity = this.params.capacity_used + '/' + capacity;
	this.elCapacity.innerHTML = capacity;

	var max_tasks = this.params.max_tasks;
	if( max_tasks == null )
		max_tasks = this.params.host.max_tasks;
	if( this.params.busy === true )
		max_tasks = '(' + this.params.tasks.length + '/' + max_tasks + ')';
	else
		max_tasks = '(0/' + max_tasks + ')';
	max_tasks = ' ' + max_tasks;
	this.elMaxTasks.innerHTML = max_tasks;
	
	var r = this.params.host_resources;

	if( became_online )
	{
		if( r != null )
		{
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
	}

	if( r != null )
	{
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
	}

	this.refresh();
}

RenderNode.prototype.refresh = function()
{
	if( this.state.OFF == true )
	{
		var power = this.offlineState;
		if( this.params.wol_operation_time )
			power += ' ' + cm_TimeStringInterval( this.params.wol_operation_time);
		this.elPower.innerHTML = power;
		return;
	}

	var stateTime = 'NEW';
	if(( this.params.task_start_finish_time != null ) && ( this.params.task_start_finish_time > 0 ))
	{
		stateTime = cm_TimeStringInterval( this.params.task_start_finish_time);
		if( this.state.RUN == true )
			stateTime += ' busy';
		else
			stateTime += ' free';
	}
	this.elStateTime.innerHTML = stateTime;
}

RenderNode.prototype.onDoubleClick = function()
{
}
