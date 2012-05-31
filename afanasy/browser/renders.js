function RenderNode() {}

RenderNode.prototype.init = function()
{
	this.name = document.createElement('span');
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

	this.state = document.createElement('span');
	this.element.appendChild( this.state);
	this.state.style.cssFloat = 'right';
	this.state.title = 'Busy / free status and time';
}

RenderNode.prototype.update = function()
{
	var user = this.params.user_name;

	if( this.params.offline === true )
	{
		if( false == this.element.classList.contains('offline'))
		this.element.classList.add('offline');
	}
	else
		this.element.classList.remove('offline');

	if( this.params.busy === true )
	{
		if( false == this.element.classList.contains('running'))
		this.element.classList.add('running');
	}
	else
		this.element.classList.remove('running');

	if( this.params.NIMBY === true )
	{
		if( false == this.element.classList.contains('nimby'))
			this.element.classList.add('nimby');
		user = '(' + user + ')N';
	}
	else if( this.params.nimby === true )
	{
		if( false == this.element.classList.contains('nimby'))
			this.element.classList.add('nimby');
		user = '(' + user + ')n';
	}
	else
		this.element.classList.remove('nimby');

	this.name.innerHTML = this.params.name;
	if( this.params.version != null )
		this.version.innerHTML = ' ' + this.params.version;
	else
		this.version.innerHTML = '';

	this.priority.innerHTML = '-' + this.params.priority;

	this.user_name.innerHTML = user;

	if( this.params.offline === true )
	{
		this.center.innerHTML = 'offline';
		this.capacity.innerHTML = '';
		this.max_tasks.innerHTML = '';
		this.state.innerHTML = '';
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

	var state = 'NEW';
	if(( this.params.task_start_finish_time != null ) && ( this.params.task_start_finish_time > 0 ))
	{
		state = cm_TimeStringFromNow( this.params.task_start_finish_time);
		if( this.params.busy === true )
			state += ' busy';
		else
			state += ' free';
	}
	this.state.innerHTML = state;
}
