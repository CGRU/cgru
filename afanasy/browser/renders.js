function RenderNode() {}

RenderNode.prototype.init = function()
{
	this.name = document.createElement('span');
	this.name.classList.add('name');
	this.element.appendChild( this.name);
	this.name.title = 'Client host name';

	this.version = document.createElement('span');
	this.element.appendChild( this.version);
	this.version.title = 'Client version';

	this.priority = document.createElement('span');
	this.element.appendChild( this.priority);
	this.priority.style.cssFloat = 'right';
	this.priority.title = 'Priority';

	this.user_name = document.createElement('span');
	this.element.appendChild( this.user_name);
	this.user_name.style.cssFloat = 'right';
	this.user_name.title = 'User name and "Nimby" status'

	this.center = document.createElement('span');
	this.element.appendChild( this.center);
	this.center.className = 'resources';

	this.element.appendChild( document.createElement('br'));

	this.capacity = document.createElement('span');
	this.element.appendChild( this.capacity);
	this.capacity.title = 'Capacity used / total'

	this.max_tasks = document.createElement('span');
	this.element.appendChild( this.max_tasks);
	this.max_tasks.title = 'Running tasks / maximum'

	this.stateTime = document.createElement('span');
	this.element.appendChild( this.stateTime);
	this.stateTime.style.cssFloat = 'right';
	this.stateTime.title = 'Busy / free status and time';

	this.annotation = document.createElement('div');
	this.element.appendChild( this.annotation);
	this.annotation.title = 'Annotation';
	this.annotation.style.textAlign = 'center';

	this.state = {};
}

RenderNode.prototype.update = function()
{
	var user = this.params.user_name;
	cm_GetState( this.params.state, this.element, this.state);

	this.name.innerHTML = this.params.name;
	if( this.params.version != null )
		this.version.innerHTML = ' ' + this.params.version;
	else
		this.version.innerHTML = '';

	this.priority.innerHTML = '-' + this.params.priority;

	this.user_name.innerHTML = user;

	if( this.params.annotation )
		this.annotation.innerHTML = this.params.annotation;
	else
		this.annotation.innerHTML = '';

	if( this.state.OFF == true )
	{
		this.center.innerHTML = 'offline';
		this.capacity.innerHTML = '';
		this.max_tasks.innerHTML = '';
		this.state.innerHTML = '';
		this.refresh();
		return;
	}
	this.center.innerHTML = '.';

	var capacity = this.params.capacity;
	if( capacity == null )
		capacity = this.params.host.capacity;
	capacity = this.params.capacity_used + '/' + capacity;
	this.capacity.innerHTML = capacity;

	var max_tasks = this.params.max_tasks;
	if( max_tasks == null )
		max_tasks = this.params.host.max_tasks;
	if( this.params.busy === true )
		max_tasks = '(' + this.params.tasks.length + '/' + max_tasks + ')';
	else
		max_tasks = '(0/' + max_tasks + ')';
	max_tasks = ' ' + max_tasks;
	this.max_tasks.innerHTML = max_tasks;

	this.refresh();
}

RenderNode.prototype.refresh = function()
{
	var stateTime = 'NEW';
	if(( this.params.task_start_finish_time != null ) && ( this.params.task_start_finish_time > 0 ))
	{
		stateTime = cm_TimeStringInterval( this.params.task_start_finish_time);
		if( this.state.RUN == true )
			stateTime += ' busy';
		else
			stateTime += ' free';
	}
	this.stateTime.innerHTML = stateTime;
}

RenderNode.prototype.onDoubleClick = function()
{
}
