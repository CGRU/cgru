function UserNode() {}

UserNode.prototype.init = function() 
{
	this.element.classList.add('job');

	this.name = document.createElement('span');
	this.element.appendChild( this.name);
	this.name.title = 'User name';

	this.priority = document.createElement('span');
	this.element.appendChild( this.priority);
	this.priority.title = 'Priority';

	this.host_name = document.createElement('span');
	this.element.appendChild( this.host_name);
	this.host_name.style.cssFloat = 'right';
	this.host_name.title = 'User host'

	this.element.appendChild( document.createElement('br'));

	this.jobs = document.createElement('span');
	this.element.appendChild( this.jobs);
	this.jobs.title = 'Jobs, all and running';

	this.solving = document.createElement('span');
	this.element.appendChild( this.solving);
	this.solving.style.cssFloat = 'right';
	this.solving.title = 'Solving';

	this.element.appendChild( document.createElement('br'));

	this.annotation = document.createElement('div');
	this.element.appendChild( this.annotation);
	this.annotation.title = 'Annotation';
	this.annotation.style.textAlign = 'center';
}

UserNode.prototype.update = function()
{
	this.name.innerHTML = this.params.name;

	this.priority.innerHTML = '-' + this.params.priority;

	if( this.params.host_name )
		this.host_name.innerHTML = this.params.host_name;
	else
		this.host_name.innerHTML = '';

	var jobs = 'j';
	if( this.params.jobs_num)
		jobs += this.params.jobs_num;
	else
		jobs += '0';
	this.jobs.innerHTML = jobs;

	if( this.params.solve_parallel )
		this.solving.innerHTML = 'Par';
	else
		this.solving.innerHTML = 'Ord';

	if( this.params.annotation )
		this.annotation.innerHTML = this.params.annotation;
	else
		this.annotation.innerHTML = '';
}

UserNode.prototype.onDoubleClick = function()
{
	g_OpenTasks( this.params.id );
}

