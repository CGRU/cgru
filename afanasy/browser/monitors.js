function MonitorNode() {}

MonitorNode.prototype.init = function() 
{
	this.element.classList.add('mnode');

	cm_CreateStart( this);

	this.elName = document.createElement('span');
	this.elName.classList.add('name');
	this.element.appendChild( this.elName);
	this.elName.title = 'User Name';
	this.elName.classList.add('prestar');

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

	this.elForgive = cm_ElCreateText( this.elCenter, 'Errors Forgive Time');
	this.elJobsLifeTime = cm_ElCreateText( this.elCenter, 'Jobs Life Time');

	this.element.appendChild( document.createElement('br'));

	this.elAddress = cm_ElCreateFloatText( this.element, 'left', 'Jobs: All/Running');
	this.elAddress.classList.add('prestar');

	this.elIP = cm_ElCreateFloatText( this.element, 'right');

	this.element.appendChild( document.createElement('br'));

	this.elAnnotation = document.createElement('div');
	this.element.appendChild( this.elAnnotation);
	this.elAnnotation.title = 'Annotation';
	this.elAnnotation.style.textAlign = 'center';
	this.elAnnotation.classList.add('prestar');

	this.elBarParent = document.createElement('div');
	this.element.appendChild( this.elBarParent);
	this.elBarParent.style.position = 'absolute';
	this.elBarParent.style.left = '120px';
	this.elBarParent.style.right = '50px';
	this.elBarParent.style.top = '18px';
	this.elBarParent.style.height = '12px';

	this.elBar = document.createElement('div');
	this.elBarParent.appendChild( this.elBar);
	this.elBar.classList.add('bar');
//	this.elBar.style.textAlign = 'right';
}

MonitorNode.prototype.update = function( i_obj)
{
	if( i_obj ) this.params = i_obj;

	this.elName.textContent = this.params.name.substr( 0, this.params.name.indexOf(':'));

	if( this.params.host_name ) this.elHostName.textContent = this.params.host_name;
	else this.elHostName.textContent = '';

	if( this.params.errors_forgive_time ) this.elForgive.textContent = 'F'+ cm_TimeStringFromSeconds( this.params.errors_forgive_time);
	else this.elForgive.textContent = '';

	if( this.params.jobs_life_time ) this.elJobsLifeTime.textContent = 'L'+ cm_TimeStringFromSeconds( this.params.jobs_life_time);
	else this.elJobsLifeTime.textContent = '';

	this.elAddress.textContent = this.params.address.ip + ':' + this.params.address.port;

	this.elIP.textContent = 'IP=' + this.params.address.ip;

	if( this.params.annotation )
		this.elAnnotation.textContent = this.params.annotation;
	else
		this.elAnnotation.textContent = '';

	var title = '';
	title += 'Launched at: ' + cm_DateTimeStrFromSec( this.params.time_launch) + '\n';
	title += 'Registered at: ' + cm_DateTimeStrFromSec( this.params.time_register) + '\n';
	title += 'Last acitvity at: ' + cm_DateTimeStrFromSec( this.params.time_activity) + '\n';
	title += 'ID = ' + this.params.id + '\n';
//	this.elName.title = title;
	this.element.title = title;

	this.refresh();
}

MonitorNode.prototype.refresh = function() {}

MonitorNode.prototype.onDoubleClick = function( e) { g_ShowObject({"object":this.params},{"evt":e,"wnd":this.monitor.window});}

MonitorNode.actions = [];

MonitorNode.actions.push({"mode":'context', "name":'log', "handle":'mh_Get', "label":'Show Log'});
MonitorNode.actions.push({"mode":'context'});
MonitorNode.actions.push({"mode":'context', "name":'priority',              "type":'num', "handle":'mh_Dialog', "label":'Priority'});
MonitorNode.actions.push({"mode":'context', "name":'max_running_tasks',     "type":'num', "handle":'mh_Dialog', "label":'Max Runnig Tasks'});
MonitorNode.actions.push({"mode":'context', "name":'hosts_mask',            "type":'reg', "handle":'mh_Dialog', "label":'Hosts Mask'});
MonitorNode.actions.push({"mode":'context', "name":'hosts_mask_exclude',    "type":'reg', "handle":'mh_Dialog', "label":'Exclude Hosts Mask'});
MonitorNode.actions.push({"mode":'context'});
MonitorNode.actions.push({"mode":'context', "name":'errors_retries',        "type":'num', "handle":'mh_Dialog', "label":'Errors Retries'});
MonitorNode.actions.push({"mode":'context', "name":'errors_avoid_host',     "type":'num', "handle":'mh_Dialog', "label":'Errors Avoid Host'});
MonitorNode.actions.push({"mode":'context', "name":'errors_task_same_host', "type":'num', "handle":'mh_Dialog', "label":'Errors Task Same Host'});
MonitorNode.actions.push({"mode":'context', "name":'errors_forgive_time',   "type":'hrs', "handle":'mh_Dialog', "label":'Errors Forgive Time'});

MonitorNode.actions.push({"mode":'set', "name":'solve_parallel', "type":'bl1',  "handle":'mh_Dialog', "label":'Solve Jobs Parallel'});
MonitorNode.actions.push({"mode":'set'});
MonitorNode.actions.push({"mode":'set', "name":'jobs_life_time', "type":'hrs',  "handle":'mh_Dialog', "label":'Jobs Life Time'});
MonitorNode.actions.push({"mode":'set'});
MonitorNode.actions.push({"mode":'set', "name":'permanent',      "type":'bl1',  "handle":'mh_Dialog', "label":'Set Permanent'});
MonitorNode.actions.push({"mode":'set'});
MonitorNode.actions.push({"mode":'set', "name":'annotation',     "type":'str',  "handle":'mh_Dialog', "label":'Annotate'});
MonitorNode.actions.push({"mode":'set'});
MonitorNode.actions.push({"mode":'set', "name":'custom_data',    "type":'json', "handle":'mh_Dialog', "label":'Custom Data'});

MonitorNode.sort = ['priority','name','host_name'];
MonitorNode.filter = ['name','host_name'];

