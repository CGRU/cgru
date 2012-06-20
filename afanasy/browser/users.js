function UserNode() {}

UserNode.prototype.init = function() 
{
	this.element.classList.add('job');

	this.name = document.createElement('span');
	this.element.appendChild( this.name);
	this.name.title = 'Job name';

	this.priority = document.createElement('span');
	this.element.appendChild( this.priority);
//	this.priority.style.cssFloat = 'right';
	this.priority.title = 'Priority';

	this.host_name = document.createElement('span');
	this.element.appendChild( this.host_name);
	this.host_name.style.cssFloat = 'right';
	this.host_name.title = 'User name'
}

UserNode.prototype.update = function()
{
	this.name.innerHTML = this.params.name;

	this.priority.innerHTML = '-' + this.params.priority;

	if( this.params.host_name )
		this.host_name.innerHTML = this.params.host_name;
	else
		this.host_name.innerHTML = '';
}

UserNode.prototype.onDoubleClick = function()
{
	g_OpenTasks( this.params.id );
}

