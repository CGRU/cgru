function UserNode() {}

UserNode.prototype.init = function() 
{
	this.element.classList.add('job');

	this.name = document.createElement('span');
	this.name.classList.add('name');
	this.element.appendChild( this.name);
	this.name.title = 'User name';

	this.priority = document.createElement('span');
	this.element.appendChild( this.priority);
	this.priority.title = 'Priority';

	this.host_name = document.createElement('span');
	this.host_name.classList.add('name');
	this.element.appendChild( this.host_name);
	this.host_name.style.cssFloat = 'right';
	this.host_name.style.marginLeft = '4px';
	this.host_name.title = 'User host'

	this.errors = document.createElement('span');
	this.element.appendChild( this.errors);
	this.errors.style.cssFloat = 'right';
	this.errors.style.marginLeft = '4px';

	this.element.appendChild( document.createElement('br'));

	this.jobs = document.createElement('span');
	this.element.appendChild( this.jobs);
	this.jobs.style.cssFloat = 'left';
	this.jobs.title = 'Jobs, all/running';

	this.solving = document.createElement('span');
	this.element.appendChild( this.solving);
	this.solving.style.cssFloat = 'right';

	this.element.appendChild( document.createElement('br'));

	this.annotation = document.createElement('div');
	this.element.appendChild( this.annotation);
	this.annotation.title = 'Annotation';
	this.annotation.style.textAlign = 'center';

	this.bar = document.createElement('div');
	this.element.appendChild( this.bar);
	this.bar.classList.add('bar');
	this.bar.style.textAlign = 'center';
}

UserNode.prototype.update = function()
{
	this.name.innerHTML = this.params.name;

	this.priority.innerHTML = '-' + this.params.priority;

	if( this.params.host_name )
		this.host_name.innerHTML = this.params.host_name;
	else
		this.host_name.innerHTML = '';

	var errstr = 'err:';
	var errtit = 'Errors solving:';
	errstr += this.params.errors_avoid_host + 'j-';
	errtit += '\nJob blocks to avoid host: ' + this.params.errors_avoid_host;
	errstr += this.params.errors_task_same_host + 't-';
	errtit += '\nJob tasks to avoid host: ' + this.params.errors_task_same_host;
	errstr += this.params.errors_retries + 'r';
	errtit += '\nError task retries: ' + this.params.errors_retries;
	this.errors.innerHTML = errstr;
	this.errors.title = errtit;

	var jobs = 'j';
	if( this.params.jobs_num)
		jobs += this.params.jobs_num;
	else
		jobs += '0';
	jobs += '/';
	if( this.params.running_jobs_num )
		jobs += this.params.running_jobs_num;
	else
		jobs += '0';
	if( this.params.running_tasks_num )
		jobs += '-' + this.params.running_tasks_num;
	this.jobs.innerHTML = jobs;

	var solving = 'User jobs solving method:';
	if( this.params.solve_parallel )
	{
		this.solving.innerHTML = 'Par';
		solving += '\nParallel: All together accoring to jobs priority.\n';
	}
	else
	{
		this.solving.innerHTML = 'Ord';
		solving += '\nOrdered: One by one accoring to jobs priority and order.\n';
	}
	this.solving.title = solving;

	if( this.params.annotation )
		this.annotation.innerHTML = this.params.annotation;
	else
		this.annotation.innerHTML = '';

	this.refresh();
}

UserNode.prototype.refresh = function()
{
	var percent = '';
	if( this.params.running_tasks_num && ( this.monitor.max_tasks > 0 ))
		percent = 100 * this.params.running_tasks_num/this.monitor.max_tasks;
	else
		percent = '0';
	this.bar.style.width = percent + '%';
}

UserNode.prototype.onDoubleClick = function()
{
}

