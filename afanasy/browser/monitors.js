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

	this.elEvents = document.createElement('div');
	this.element.appendChild( this.elEvents);

	this.elAnnotation = document.createElement('div');
	this.element.appendChild( this.elAnnotation);
	this.elAnnotation.title = 'Annotation';
	this.elAnnotation.style.textAlign = 'center';
	this.elAnnotation.classList.add('prestar');
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

	var events = '';
	if( this.params.users_ids && this.params.users_ids.length )
		events += ' UIDs:' + JSON.stringify( this.params.users_ids);
	if( this.params.jobs_ids && this.params.jobs_ids.length )
		events += ' JIDs:' + JSON.stringify( this.params.jobs_ids);
	if( this.params.events && this.params.events.length )
		events += ' EVTs:' + JSON.stringify( this.params.events);
	this.elEvents.textContent = events;

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

MonitorNode.prototype.updatePanels = function()
{
	// Info:
	var info = '';

	info += 'User name: ' + this.params.user_name;

	if( this.params.address )
	{
		info += '<br>IP: ' + this.params.address.ip;
		if( this.params.address.port )
			info += ' Port: ' + this.params.address.port;
	}

	info += '<br>';

	info += '<br>Launched at:<br>' + cm_DateTimeStrFromSec( this.params.time_launch);
	info += '<br>Registered at:<br>' + cm_DateTimeStrFromSec( this.params.time_register);
	info += '<br>Last acitvity at:<br>' + cm_DateTimeStrFromSec( this.params.time_activity);

	this.monitor.setPanelInfo( info);
}


MonitorNode.sort = ['name'];
MonitorNode.filter = ['name'];

