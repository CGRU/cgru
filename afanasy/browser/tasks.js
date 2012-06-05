function BlockItem( i_block_num)
{
	this.block_num = i_block_num;
}

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
	this.element.classList.add('task');

	this.progress = document.createElement('span');
	this.element.appendChild( this.progress);
	this.progress.classList.add('progress');
	this.progress.style.zIndex = 1;
/*		this.progress_done = document.createElement('span');
		this.progress.appendChild( this.progress_done);
		this.progress_done.classList.add('progress_done');
*/
	this.name = document.createElement('span');
	this.element.appendChild( this.name);
	this.name.title = 'Task name';
	this.name.style.zIndex = 100;
	this.name.style.left = '1%';
	this.name.style.cssFloat = 'left';

	this.percent = document.createElement('span');
	this.element.appendChild( this.percent);

	this.props = document.createElement('span');
	this.element.appendChild( this.props);
	this.props.style.cssFloat = 'right';
	this.props.title = 'Task properties';
}

TaskItem.prototype.update = function()
{
	this.name.innerHTML = this.genName();

	if( this.params.tasks )
	{
		this.props.innerHTML = 'array';
	}
	else
	{
		this.props.innerHTML = 'numeric';
	}

	if( this.params.running === true )
	{
		if( false == this.element.classList.contains('running'))
		this.element.classList.add('running');
	}
	else
		this.element.classList.remove('running');
}

TaskItem.prototype.updateProgress = function( progress)
{
//	this.progress = progress;
	percent = progress.per;
	this.percent.innerHTML = ' ' + percent + '%';
}

TaskItem.prototype.genName = function()
{
	var t = this.task_num;
	var name = 'task ' + t;
	var tasks_name = this.params.tasks_name;
	if( this.params.tasks )
	{
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
		return name
	}

	this.genFrames();
	if( tasks_name && ( tasks_name != '' ))
	{
		name = tasks_name;
		name.replace('@#@', '' + this.frame_start);
	}
	else
	{
		if( this.frames_num > 1 )
			name = 'frame ' + this.frame_start + '-' + this.frame_end;
		else
			name = 'frame ' + this.frame_start;
	}

	return name;
}

TaskItem.prototype.genFrames = function()
{
	var p = this.params;

	var offset = this.task_num * p.frames_per_task * p.frames_inc;
	if( p.frames_inc > 1 )
		offset -= offset % p.frames_inc;
	this.frame_start = p.frame_first + offset;

	offset = p.frames_per_task * p.frames_inc - 1;
	if(( this.frame_start + offset ) > p.frame_last )
		offset = p.frame_last - this.frame_start;
	if( p.frames_inc > 1 )
		offset -= offset % p.frames_inc;
	this.frame_end = this.frame_start + offset;

	if( p.frames_inc > 1 )
		this.frames_num = ( this.frame_end - this.frame_start ) / p.frames_inc + 1;
	else
		this.frames_num = this.frame_end - this.frame_start + 1;
}
