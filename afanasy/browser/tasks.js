function BlockItem( i_block_num)
{
	this.block_num = i_block_num;
}

BlockItem.prototype.init = function() 
{
	this.element.classList.add('block');

	this.elName = cm_ElCreateText( this.element, 'Block Name');

	this.elState = cm_ElCreateFloatText( this.element, 'right', 'Block State');
	this.elProperties = cm_ElCreateFloatText( this.element, 'right', 'Block Properties');
}

BlockItem.prototype.update = function()
{
	cm_GetState( this.params.state, this.element, this.elState);

	if( this.params.name )
	{
		if( this.params.tasks )
			this.elProperties.textContent = 'array';
		else
			this.elProperties.textContent = 'numeric';

		this.elName.textContent = this.params.name;
	}
}

function TaskItem( i_job, i_block, i_task_num )
{
	this.job = i_job;
	this.block = i_block;
	this.task_num = i_task_num;
}

TaskItem.prototype.init = function() 
{
	this.element.classList.add('task');

	this.elProgress = this.monitor.document.createElement('span');
	this.element.appendChild( this.elProgress);
	this.elProgress.classList.add('bar');

	this.elName = this.monitor.document.createElement('span');
	this.element.appendChild( this.elName);
	this.elName.title = 'Task name';

	this.elPercent = this.monitor.document.createElement('span');
	this.element.appendChild( this.elPercent);

	this.elState = cm_ElCreateFloatText( this.element, 'right', 'Task State');
	this.elStarts = cm_ElCreateFloatText( this.element, 'right', 'Starts Count');
	this.elErrors = cm_ElCreateFloatText( this.element, 'right', 'Errors Count');
	this.elHost = cm_ElCreateFloatText( this.element, 'right', 'Last Running Host');
}

TaskItem.prototype.update = function()
{
	this.params.name = this.genName();
	this.elName.textContent = this.params.name;
/*	if( this.params.running === true )
	{
		if( false == this.element.classList.contains('running'))
		this.element.classList.add('running');
	}
	else
		this.element.classList.remove('running');*/
}

TaskItem.prototype.updateProgress = function( i_progress)
{
	for( var attr in i_progress )
		this.params[attr] = i_progress[attr];

	cm_GetState( this.params.state, this.element, this.elState);
//	this.elState.textContent = state.string;

	if( this.params.str ) this.elStarts.textContent = 's' + this.params.str;
	else this.elStarts.textContent = 's0';
	if( this.params.err ) this.elErrors.textContent = 'e' + this.params.err;
	else this.elErrors.textContent = 'e0';
	if( this.params.hst ) this.elHost.textContent = this.params.hst;

	var percent = 0;
	if( this.elState.RUN && this.params.per ) percent = this.params.per;
	if( this.elState.DON ) percent = 100;
	if( this.elState.SKP ) percent = 100;
	if( percent < 0 ) percent = 0;
	if( percent > 100 ) percent = 100;

	if( this.elState.RUN )
		this.elPercent.textContent = ' ' + percent + '%';
	else
		this.elPercent.textContent = '';
	this.elProgress.style.width = ( percent + '%');
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

TaskItem.prototype.getBlockTasksIds = function( o_bids, o_tids)
{
	var blocks = this.monitor.blocks;
	for( var b = 0; b < blocks.length; b++)
	{
		if( blocks[b].element.selected )
		{
			o_bids.push( b);
			break;
		}
		else
		{
			var tasks = blocks[b].tasks;
			for( var t = 0; t < tasks.length; t++)
			{
				if( tasks[t].element.selected )
				{
					o_tids.push(t);
				}
			}
			if( o_tids.length )
			{
				o_bids.push( b);
				break;
			}
		}
	}
//g_Info('bids='+o_bids+' tids='+o_tids);
}

TaskItem.prototype.onContextMenu = function( i_menu)
{
	if( this.params.str && ( this.params.str > 1 ))
	{
		for( var i = this.params.str; i > 0; i--)
		{
			if( i <= this.params.str - 3) break;
			i_menu.addItem('output', this, 'menuHandleOutput', 'Output '+i, true, i);
		}
		if( this.params.str > 3 )
			i_menu.addItem('output', this, 'menuHandleOutput', 'Output...', true, -1);
	}
	else
		i_menu.addItem('output', this, 'menuHandleOutput', 'Output');
	i_menu.addItem();
	i_menu.addItem('log',    this, 'menuHandleGet', 'Log');
	i_menu.addItem('info',   this, 'menuHandleGet', 'Info');
	i_menu.addItem();
	i_menu.addItem('restart', this, 'menuHandleOperation', 'Restart');
	i_menu.addItem('skip',    this, 'menuHandleOperation', 'Skip');
}

TaskItem.prototype.menuHandleOutput = function( i_number)
{
	if( i_number == -1 )
	{
		new cgru_Dialog( this.monitor.document, this.monitor.document.body, this, 'menuHandleGet', 'output', 'num', null, this.job.name, 'Get Task Process Output', 'Enter Start Number');
		return;
	}
//	this.menuHandleGet('output', i_number);
	nw_GetNodes('jobs', [this.job.id], 'output', [this.block.block_num], [this.task_num], i_number)
}

TaskItem.prototype.menuHandleGet = function( i_name, i_number)
{
	var bids = []; var tids = [];
	this.getBlockTasksIds( bids, tids);
	nw_GetNodes('jobs', [this.job.id], i_name, bids, tids, i_number)
}

TaskItem.prototype.menuHandleOperation = function( i_name, i_value)
{
	var operation = {};
	operation.type = i_name;
	var bids = []; var tids = [];
	this.getBlockTasksIds( bids, tids);
	if( tids.length ) operation.task_ids = tids;
	nw_Action('jobs', [this.job.id], operation, null, bids);
}

TaskItem.prototype.onDoubleClick = function()
{
g_Info( this.params.name);
	this.menuHandleGet('info');
}

/*
TaskItem.actions = [];

TaskItem.actions.push(['context', 'output',  null, 'menuHandleGet', 'Output']);
TaskItem.actions.push(['context', 'log',     null, 'menuHandleGet', 'Log']);
TaskItem.actions.push(['context', 'info',    null, 'menuHandleGet', 'Info']);

TaskItem.actions.push(['context', 'restart', null, 'menuHandleOperation', 'Restart']);
TaskItem.actions.push(['context', 'skip',    null, 'menuHandleOperation', 'Skip']);
*/

TaskItem.sort = ['name','host_name','str'];
TaskItem.filter = ['name','host_name'];
