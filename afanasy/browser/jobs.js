function JobNode() {}

JobNode.prototype.init = function() 
{
	this.element.classList.add('job');

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

	this.state = document.createElement('span');
	this.element.appendChild( this.state);
//	this.state.style.cssFloat = 'right';
	this.state.style.position = 'absolute';
	this.state.style.left = '40%';
	this.state.title = 'Job state';
}

JobNode.prototype.update = function()
{
	var state = cm_GetState( this.params.state, this.element);
	this.state.innerHTML = state.string;
	this.name.innerHTML = this.params.name;
	this.priority.innerHTML = '-' + this.params.priority;
	this.user_name.innerHTML = this.params.user_name;
}

JobNode.prototype.onDoubleClick = function()
{
	g_OpenTasks( this.params.id );
}

