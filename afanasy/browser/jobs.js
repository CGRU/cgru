function JobNode() {}

JobNode.prototype.init = function() 
{
	this.name = document.createElement('span');
	this.element.appendChild( this.name);
	this.name.title = 'Job name';

	this.priority = document.createElement('span');
	this.element.appendChild( this.priority);
	this.priority.style.cssFloat = 'right';
	this.priority.title = 'Priority';

	this.user_name = document.createElement('span');
	this.element.appendChild( this.user_name);
	this.user_name.style.cssFloat = 'right';
	this.user_name.title = 'User name'
}

JobNode.prototype.update = function()
{
	if( this.params.offline === true )
	{
		if( false == this.element.classList.contains('offline'))
		this.element.classList.add('offline');
	}
	else
		this.element.classList.remove('offline');

	if( this.params.running === true )
	{
		if( false == this.element.classList.contains('running'))
		this.element.classList.add('running');
	}
	else
		this.element.classList.remove('running');

	this.name.innerHTML = this.params.name;

	this.priority.innerHTML = '-' + this.params.priority;

	this.user_name.innerHTML = this.params.user_name;

	if( this.params.offline === true )
	{
		this.center.innerHTML = 'offline';
		return;
	}
}

JobNode.prototype.onDoubleClick = function()
{
	g_OpenTasks( this.params.id );
}

