function UserNode() {}

UserNode.prototype.init = function() 
{
	this.element.classList.add('user');

	cm_CreateStart( this);

	this.elName = document.createElement('span');
	this.elName.classList.add('name');
	this.element.appendChild( this.elName);
	this.elName.title = 'User Name';
	this.elName.classList.add('prestar');

	if( false == cm_IsPadawan())
	{
		this.elPriority = document.createElement('span');
		this.element.appendChild( this.elPriority);
		this.elPriority.title = 'Priority';
	}

	this.elHostName = cm_ElCreateFloatText( this.element, 'right', 'Host Name');
	this.elHostName.classList.add('name');

	this.elCenter = document.createElement('div');
	this.element.appendChild( this.elCenter);
	this.elCenter.style.position = 'absolute';
	this.elCenter.style.left = '0';
	this.elCenter.style.right = '0';
	this.elCenter.style.top = '1px';
	this.elCenter.style.textAlign = 'center';
	this.elCenter.classList.add('prestar');

	if( cm_IsPadawan())
		this.elPriority = cm_ElCreateText( this.elCenter, 'Priority');
	this.elMaxRunningTasks = cm_ElCreateText( this.elCenter, 'Maximum Running Tasks');
	this.elHostsMask = cm_ElCreateText( this.elCenter, 'Hosts Mask');
	this.elHostsMaskExclude = cm_ElCreateText( this.elCenter, 'Exclude Hosts Mask');
	this.elErrors = cm_ElCreateText( this.elCenter);
	this.elForgive = cm_ElCreateText( this.elCenter, 'Errors Forgive Time');
	this.elJobsLifeTime = cm_ElCreateText( this.elCenter, 'Jobs Life Time');

	this.element.appendChild( document.createElement('br'));

	this.elJobs = cm_ElCreateFloatText( this.element, 'left', 'Jobs: All/Running');
	this.elJobs.classList.add('prestar');

	this.elSolving = cm_ElCreateFloatText( this.element, 'right');

	this.element.appendChild( document.createElement('br'));

	this.elAnnotation = document.createElement('div');
	this.element.appendChild( this.elAnnotation);
	this.elAnnotation.title = 'Annotation';
	this.elAnnotation.style.textAlign = 'center';
	this.elAnnotation.classList.add('prestar');

	this.elBarParent = document.createElement('div');
	this.element.appendChild( this.elBarParent);
	this.elBarParent.style.position = 'absolute';
	this.elBarParent.style.left = '220px';
	this.elBarParent.style.right = '220px';
	this.elBarParent.style.top = '18px';
	this.elBarParent.style.height = '12px';

	this.elBar = document.createElement('div');
	this.elBarParent.appendChild( this.elBar);
	this.elBar.classList.add('bar');
}

UserNode.prototype.update = function( i_obj)
{
	if( i_obj ) this.params = i_obj;

	this.elName.innerHTML = '<b>'+this.params.name+'</b>';

	if( this.params.running_tasks_num )
	{
		this.elStar.style.display = 'block';
		this.elStarCount.textContent = this.params.running_tasks_num;
	}
	else
		this.elStar.style.display = 'none';

	if( cm_IsPadawan())
	{
		this.elPriority.innerHTML = 'Priority:<b>' + this.params.priority + '</b>';

		if( this.params.host_name )
			this.elHostName.innerHTML = '<small>Latest Activity Host:</small><b>' + this.params.host_name + '</b>';
		else this.elHostName.textContent = '';

		if( this.params.max_running_tasks )
			this.elMaxRunningTasks.innerHTML = 'MaxRunTasks:<b>'+this.params.max_running_tasks+'</b>';
		else this.elMaxRunningTasks.textContent = '';

		if( this.params.hosts_mask )
			this.elHostsMask.innerHTML = 'HostsMask(<b>'+this.params.hosts_mask+'</b>)';
		else this.elHostsMask.textContent = '';

		if( this.params.hosts_mask_exclude )
			this.elHostsMaskExclude.innerHTML = 'ExcludeHosts(<b>'+this.params.hosts_mask_exclude+'</b>)';
		else this.elHostsMaskExclude.textContent = '';

		if( this.params.errors_forgive_time )
			this.elForgive.innerHTML = 'ErrorsForgiveTime:<b>'+ cm_TimeStringFromSeconds( this.params.errors_forgive_time) + '</b>';
		else this.elForgive.textContent = '';

		if( this.params.jobs_life_time )
			this.elJobsLifeTime.innerHTML = 'JobsLifeTime:<b>'+ cm_TimeStringFromSeconds( this.params.jobs_life_time) + '</b>';
		else this.elJobsLifeTime.textContent = '';

		var errstr = 'ErrorsSolving(';
		errstr += ' Avoid:<b>' + this.params.errors_avoid_host + '</b>';
		errstr += ' Task:<b>' + this.params.errors_task_same_host + '</b>';
		errstr += ' Retries:<b>' + this.params.errors_retries + '</b>';
		errstr += ')';
		this.elErrors.innerHTML = errstr;

		var jobs = 'Jobs Total:';
		if( this.params.jobs_num)
			jobs += ' <b>' + this.params.jobs_num + '</b>';
		else
			jobs += ' <b>0</b>';
		if( this.params.running_jobs_num )
			jobs += ' / <b>' + this.params.running_jobs_num + '</b> Running';
		this.elJobs.innerHTML = jobs;

		if( this.params.solve_parallel )
			this.elSolving.innerHTML = 'Solving:<b>Parrallel</b>';
		else
			this.elSolving.innerHTML = 'Solving:<b>Ordered</b>';
	}
	else if( cm_IsJedi())
	{
		this.elPriority.innerHTML = '-<b>' + this.params.priority + '</b>';

		if( this.params.host_name )
			this.elHostName.innerHTML = '<b>' + this.params.host_name + '</b>';
		else this.elHostName.textContent = '';

		if( this.params.max_running_tasks )
			this.elMaxRunningTasks.innerHTML = 'Max:<b>'+this.params.max_running_tasks+')</b>';
		else this.elMaxRunningTasks.textContent = '';

		if( this.params.hosts_mask )
			this.elHostsMask.innerHTML = 'Hosts(<b>'+this.params.hosts_mask+')</b>';
		else this.elHostsMask.textContent = '';

		if( this.params.hosts_mask_exclude )
			this.elHostsMaskExclude.innerHTML = 'Exclude(<b>'+this.params.hosts_mask_exclude+')</b>';
		else this.elHostsMaskExclude.textContent = '';

		if( this.params.errors_forgive_time )
			this.elForgive.innerHTML = 'ErrForgive:<b>'+ cm_TimeStringFromSeconds( this.params.errors_forgive_time) + '</b>';
		else this.elForgive.textContent = '';

		if( this.params.jobs_life_time )
			this.elJobsLifeTime.innerHTML = 'JobsLife:<b>'+ cm_TimeStringFromSeconds( this.params.jobs_life_time) + '</b>';
		else this.elJobsLifeTime.textContent = '';

		var errstr = 'ErrSlv(';
		errstr += 'A:<b>' + this.params.errors_avoid_host + '</b>';
		errstr += ',T:<b>' + this.params.errors_task_same_host + '</b>';
		errstr += ',R:<b>' + this.params.errors_retries + '</b>';
		errstr += ')';
		this.elErrors.innerHTML = errstr;

		var jobs = 'Jobs:';
		if( this.params.jobs_num)
			jobs += '<b>' + this.params.jobs_num + '</b>';
		else
			jobs += '<b>0</b>';
		if( this.params.running_jobs_num )
			jobs += ' / <b>'+this.params.running_jobs_num + '</b>Run';
		this.elJobs.innerHTML = jobs;

		if( this.params.solve_parallel )
			this.elSolving.innerHTML = '<b>Parallel</b>';
		else
			this.elSolving.innerHTML = '<b>Ordered</b>';
	}
	else
	{
		this.elPriority.innerHTML = '-<b>' + this.params.priority + '</b>';

		if( this.params.host_name )
			this.elHostName.innerHTML = '<b>' + this.params.host_name + '</b>';
		else this.elHostName.textContent = '';

		if( this.params.max_running_tasks )
			this.elMaxRunningTasks.innerHTML = 'm<b>'+this.params.max_running_tasks+'</b>';
		else this.elMaxRunningTasks.textContent = '';

		if( this.params.hosts_mask )
			this.elHostsMask.innerHTML = 'h(<b>'+this.params.hosts_mask+')</b>';
		else this.elHostsMask.textContent = '';

		if( this.params.hosts_mask_exclude )
			this.elHostsMaskExclude.innerHTML = 'e(<b>'+this.params.hosts_mask_exclude+')</b>';
		else this.elHostsMaskExclude.textContent = '';

		if( this.params.errors_forgive_time )
			this.elForgive.innerHTML = 'f<b>'+ cm_TimeStringFromSeconds( this.params.errors_forgive_time) + '</b>';
		else this.elForgive.textContent = '';

		if( this.params.jobs_life_time )
			this.elJobsLifeTime.innerHTML = 'l<b>'+ cm_TimeStringFromSeconds( this.params.jobs_life_time) + '</b>';
		else this.elJobsLifeTime.textContent = '';

		var errstr = 'e:';
		errstr += '<b>' + this.params.errors_avoid_host + '</b>b';
		errstr += ',<b>' + this.params.errors_task_same_host + '</b>t';
		errstr += ',<b>' + this.params.errors_retries + '</b>r';
		this.elErrors.innerHTML = errstr;

		var jobs = 'j';
		if( this.params.jobs_num)
			jobs += '<b>' + this.params.jobs_num + '</b>';
		else
			jobs += '<b>0</b>';
		if( this.params.running_jobs_num )
			jobs += '/<b>'+this.params.running_jobs_num + '</b>r';
		this.elJobs.innerHTML = jobs;

		if( this.params.solve_parallel )
			this.elSolving.innerHTML = '<b>par</b>';
		else
			this.elSolving.innerHTML = '<b>ord</b>';
	}

	var solving = 'User jobs solving method:';
	if( this.params.solve_parallel )
		solving += '\nParallel: All together according to jobs priority.\n';
	else
		solving += '\nOrdered: Queued by jobs priority and order.\n';
	this.elSolving.title = solving;

	var errtit = 'Errors solving:';
	errtit += '\nJob blocks to avoid host: ' + this.params.errors_avoid_host;
	errtit += '\nJob tasks to avoid host: ' + this.params.errors_task_same_host;
	errtit += '\nError task retries: ' + this.params.errors_retries;
	this.elErrors.title = errtit;

	if( this.params.annotation )
		this.elAnnotation.innerHTML = '<b><i>' + this.params.annotation + '</i></b>';
	else
		this.elAnnotation.textContent = '';

	var title = '';
	title += 'Time Registered: ' + cm_DateTimeStrFromSec( this.params.time_register) + '\n';
	title += 'Last Acitvity: ' + cm_DateTimeStrFromSec( this.params.time_activity) + '\n';
	title += 'ID = ' + this.params.id + '\n';
	this.element.title = title;

	this.refresh();
}

UserNode.prototype.refresh = function()
{
	var percent = '';
	var label = '';
	if( this.params.running_tasks_num && ( this.monitor.max_tasks > 0 ))
	{
		percent = 100 * this.params.running_tasks_num/this.monitor.max_tasks;
		var capacity = cm_ToKMG( this.params.running_capacity_total);
		if( cm_IsPadawan())
			label = 'Running Tasks: <b>' + this.params.running_tasks_num + '</b> / Total Capacity: <b>' + capacity + '</b>';
		else if( cm_IsJedi())
			label = 'Tasks:<b>' + this.params.running_tasks_num + '</b> / Capacity:<b>' + capacity + '</b>';
		else
			label = 't<b>' + this.params.running_tasks_num + '</b>/c<b>' + capacity + '</b>';
	}
	else
		percent = '0';
	this.elBar.style.width = percent + '%';
	this.elBar.innerHTML = label;
}


UserNode.createPanels = function( i_monitor)
{
	// Jobs solving:
	var acts = {};
	acts.solve_ord = {'name':'solve_parallel','value':false,'label':'ORD','tooltip':'Solve jobs by order.','handle':'mh_Param'};
	acts.solve_par = {'name':'solve_parallel','value':true, 'label':'PAR','tooltip':'Solve jobs parallel.','handle':'mh_Param'};
	i_monitor.createCtrlBtns( acts);


	// Custom data:
	var acts = {};
	acts.custom_data = {'type':'json','handle':'mh_Dialog','label':'DAT','tooltip':'Set user custom data.'};
	i_monitor.createCtrlBtns( acts);
}


UserNode.prototype.updatePanels = function()
{
	// Info:
	var info = 'Registered:<br> ' + cm_DateTimeStrFromSec( this.params.time_register);
	if( this.params.time_activity )
		info += '<br>Last Activity:<br> ' + cm_DateTimeStrFromSec( this.params.time_activity);
	this.monitor.setPanelInfo( info);
}


UserNode.prototype.onDoubleClick = function( e) { g_ShowObject({"object":this.params},{"evt":e,"wnd":this.monitor.window});}


UserNode.params = {};
UserNode.params.priority              = {'type':'num','label':'Priority'};
UserNode.params.max_running_tasks     = {'type':'num','label':'Max Runnig Tasks'};
UserNode.params.hosts_mask            = {'type':'reg','label':'Hosts Mask'};
UserNode.params.hosts_mask_exclude    = {'type':'reg','label':'Exclude Hosts Mask'};
UserNode.params.errors_retries        = {'type':'num','label':'Errors Retries'};
UserNode.params.errors_avoid_host     = {'type':'num','label':'Errors Avoid Host'};
UserNode.params.errors_task_same_host = {'type':'num','label':'Errors Task Same Host'};
UserNode.params.errors_forgive_time   = {'type':'hrs','label':'Errors Forgive Time'};
UserNode.params.jobs_life_time        = {'type':'hrs','label':'Jobs Life Time'};
UserNode.params.annotation            = {'type':'str','label':'Annotation'};
//UserNode.params.custom_data           = {'type':'json','label':'Custom Data'};


UserNode.sort = ['priority','name','host_name'];
UserNode.filter = ['name','host_name'];

