function JobNode() {}

JobNode.prototype.init = function() 
{
	this.element.classList.add('job');

	this.name = document.createElement('span');
	this.element.appendChild( this.name);
	this.name.title = 'Job name';

	this.user_name = document.createElement('span');
	this.element.appendChild( this.user_name);
	this.user_name.style.cssFloat = 'right';
	this.user_name.title = 'User name'

	this.element.appendChild( document.createElement('br'));

	this.state = document.createElement('span');
	this.element.appendChild( this.state);
	this.state.title = 'Job state';

	this.time = document.createElement('span');
	this.element.appendChild( this.time);
	this.time.style.cssFloat = 'right';
	this.time.style.marginLeft = '4px';
	this.time.title = 'Running time';

	this.priority = document.createElement('span');
	this.element.appendChild( this.priority);
	this.priority.style.cssFloat = 'right';
	this.priority.style.marginLeft = '4px';
	this.priority.title = 'Priority';

	this.depend_mask = document.createElement('span');
	this.element.appendChild( this.depend_mask);
	this.depend_mask.style.cssFloat = 'right';
	this.depend_mask.style.marginLeft = '4px';
	this.depend_mask.title = 'Depend mask';

	this.depend_mask_global = document.createElement('span');
	this.element.appendChild( this.depend_mask_global);
	this.depend_mask_global.style.cssFloat = 'right';
	this.depend_mask_global.style.marginLeft = '4px';
	this.depend_mask_global.title = 'Global depend mask';

	this.hosts_mask = document.createElement('span');
	this.element.appendChild( this.hosts_mask);
	this.hosts_mask.style.cssFloat = 'right';
	this.hosts_mask.style.marginLeft = '4px';
	this.hosts_mask.title = 'Hosts mask';

	this.hosts_mask_exclude = document.createElement('span');
	this.element.appendChild( this.hosts_mask_exclude);
	this.hosts_mask_exclude.style.cssFloat = 'right';
	this.hosts_mask_exclude.style.marginLeft = '4px';
	this.hosts_mask_exclude.title = 'Hosts mask exclude';

	this.blocks = [];
	for( var b = 0; b < this.params.blocks.length; b++)
		this.blocks.push( new JobBlock( this.element, this.params.blocks[b]));

	this.annotation = document.createElement('div');
	this.element.appendChild( this.annotation);
	this.annotation.title = 'Annotation';
	this.annotation.style.textAlign = 'center';
}

JobNode.prototype.update = function()
{
	var state = cm_GetState( this.params.state, this.element);
	this.state.innerHTML = state.string;
	this.name.innerHTML = this.params.name;
	this.priority.innerHTML = ' p' + this.params.priority;
	this.user_name.innerHTML = this.params.user_name;

	var time = this.params.time_started;
	if( time )
		this.time.innerHTML = time;
	else
		this.time.innerHTML = '';

	if( this.params.depend_mask )
		this.depend_mask.innerHTML = ' d(' + this.params.depend_mask + ') ';
	else
		this.depend_mask.innerHTML = '';

	if( this.params.depend_mask_global )
		this.depend_mask_global.innerHTML = ' g(' + this.params.depend_mask_global + ') ';
	else
		this.depend_mask_global.innerHTML = '';

	if( this.params.hosts_mask )
		this.hosts_mask.innerHTML = ' h(' + this.params.hosts_mask + ') ';
	else
		this.hosts_mask.innerHTML = '';

	if( this.params.hosts_mask_exclude )
		this.hosts_mask_exclude.innerHTML = ' e(' + this.params.hosts_mask_exclude + ') ';
	else
		this.hosts_mask_exclude.innerHTML = '';

	if( this.params.annotation )
		this.annotation.innerHTML = this.params.annotation;
	else
		this.annotation.innerHTML = '';

	for( var b = 0; b < this.params.blocks.length; b++)
	{
		this.blocks[b].params = this.params.blocks[b];
		this.blocks[b].update();
	}
}

JobNode.prototype.onDoubleClick = function()
{
	g_OpenTasks( this.params.id );
}

function JobBlock( i_elParent, i_block)
{
	this.params = i_block;	
	this.element = document.createElement('div');	
	i_elParent.appendChild( this.element);
	this.element.classList.add('jobblock');

	this.tasks = document.createElement('span');
	this.element.appendChild( this.tasks);
	var tasks = 't' + this.params.tasks_num;
	var tasks_title = 'Block tasks:'
	if( this.params.numeric )
	{
		tasks_title += ' Numeric:';
		tasks += '(' + this.params.frame_first + '-' + this.params.frame_last;
		tasks_title += ' from ' + this.params.frame_first + ' to ' + this.params.frame_last;
		if( this.params.frames_per_task > 1 )
		{
			tasks += ':' + this.params.frames_per_task;
			tasks_title += ' per ' + this.params.frames_per_task;
		}
		if( this.params.frames_inc > 1 )
		{
			tasks += '/' + this.params.frames_inc;
			tasks_title += ' by ' + this.params.frames_inc;
		}
		tasks += ')';
		tasks_title += '.';
	}
	else
	{
		tasks_title += ' Not numeric.';
	}
	if( this.params.non_sequential )
	{
		tasks += '*';
		tasks_title += '\nNon-sequential solving.';
	}
	tasks += ': ';
	this.tasks.innerHTML = tasks;
	this.tasks.title = tasks_title;

	this.name = document.createElement('span');
	this.element.appendChild( this.name);
	this.name.innerHTML = this.params.name;
	this.name.title = 'Block name';

	this.depends = document.createElement('span');
	this.element.appendChild( this.depends);

	this.properties = document.createElement('span');
	this.element.appendChild( this.properties);
	this.properties.style.cssFloat = 'right';
}

JobBlock.prototype.update = function()
{
	var deps = '';
	var deps_title = ''
	if( this.params.depend_mask )
	{
		deps += ' [' + this.params.depend_mask + ']';
		if( deps_title.length ) deps_title += '\n';
		deps_title += 'Depend mask = \"' + this.params.depend_mask + '\".'
	}
	if( this.params.tasks_depend_mask )
	{
		deps += ' T[' + this.params.tasks_depend_mask + ']';
		if( deps_title.length ) deps_title += '\n';
		deps_title += 'Tasks depend mask = \"' + this.params.tasks_depend_mask + '\".'
	}
	if( this.params.depend_sub_task )
	{
		deps += ' [SUB]';
		if( deps_title.length ) deps_title += '\n';
		deps_title += 'Subtasks depend.'
	}
	this.depends.innerHTML = deps;
	this.depends.title = deps_title;

	var props = '';
	props += '[' + this.params.capacity + ']';
	this.properties.innerHTML = props;
}

