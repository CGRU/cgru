function BlockItem() {}
BlockItem.prototype.init = function() 
{
	this.name = document.createElement('span');
	this.element.appendChild( this.name);
	this.name.title = 'Block name';

	this.props = document.createElement('span');
	this.element.appendChild( this.props);
	this.props.style.cssFloat = 'right';
	this.props.title = 'Block properties';
}

BlockItem.prototype.update = function()
{
	if( this.params.running === true )
	{
		if( false == this.element.classList.contains('running'))
		this.element.classList.add('running');
	}
	else
		this.element.classList.remove('running');

	if( this.params.tasks )
		this.props.innerHTML = 'array';
	else
		this.props.innerHTML = 'numeric';

	this.name.innerHTML = this.params.name;
}

function TaskItem( i_task_num )
{
	this.task_num = i_task_num;
}

TaskItem.prototype.init = function() 
{
	this.name = document.createElement('span');
	this.element.appendChild( this.name);
	this.name.title = 'Task name';

	this.props = document.createElement('span');
	this.element.appendChild( this.props);
	this.props.style.cssFloat = 'right';
	this.props.title = 'Task properties';
}

TaskItem.prototype.update = function()
{
	var t = this.task_num;
	var name = 'task ' + t;
	var tasks_name = this.params.tasks_name;
	if( this.params.tasks )
	{
		this.props.innerHTML = 'array';
		var task_name = this.params.tasks[t].name;
		if( task_name )
		{
			if( tasks_name && ( tasks_name != '' ))
				name = tasks_name.replace('@#@', task_name);
			else
				name = task_name;
		}
		else if( tasks_name)
			name = tasks_name.replace('@#@', '' + t);
	}
	else
	{
		this.props.innerHTML = 'numeric';
		if( tasks_name && ( tasks_name != '' ))
		{
			name = tasks_name;
			name.replace('@#@', '' + t);
		}
		else
			name = 'frame ' + t;
	}
	this.name.innerHTML = name;

	if( this.params.running === true )
	{
		if( false == this.element.classList.contains('running'))
		this.element.classList.add('running');
	}
	else
		this.element.classList.remove('running');
}

