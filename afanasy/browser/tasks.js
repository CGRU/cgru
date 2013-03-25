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

	this.state = {};
	this.thumbnail_http_base = cgru_Config['af_thumbnail_http'];
	this.thumbnail_http_naming = cgru_Config['af_thumbnail_naming'];
	
	this.thumbnail_http_path = this.thumbnail_http_base + this.thumbnail_http_naming;
	this.thumbnail_http_path = this.thumbnail_http_path.replace("%(job_id)d", this.params.job_id);
	this.thumbnail_http_path = this.thumbnail_http_path.replace("%(block_id)d", this.params.block_num);
}

BlockItem.prototype.update = function()
{
	this.elState.textContent = this.params.state;
	cm_GetState( this.params.state, this.state, this.element);

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

	this.elBar = this.monitor.document.createElement('div');
	this.element.appendChild( this.elBar);
	this.elBar.classList.add('bar');
//this.elBar.textContent='bar';

	this.elStar = this.monitor.document.createElement('div');
	this.element.appendChild( this.elStar);
	this.elStar.classList.add('star');
	this.elStar.textContent = localStorage.run_symbol;

	this.elBody = this.monitor.document.createElement('div');
	this.element.appendChild( this.elBody);
	this.elBody.classList.add('body');
//this.elBody.textContent='body';

	this.elName = this.monitor.document.createElement('span');
	this.elBody.appendChild( this.elName);
	this.elName.title = 'Task name';

	this.elPercent = this.monitor.document.createElement('span');
	this.elBody.appendChild( this.elPercent);

	this.elTime = cm_ElCreateFloatText( this.elBody, 'right', 'Run Time');
	this.elStarts = cm_ElCreateFloatText( this.elBody, 'right', 'Starts Count');
	this.elErrors = cm_ElCreateFloatText( this.elBody, 'right', 'Errors Count');
	this.elState = cm_ElCreateFloatText( this.elBody, 'right', 'Task State');
	this.elActivity = cm_ElCreateFloatText( this.elBody, 'right', 'Activity');
	this.elHost = cm_ElCreateFloatText( this.elBody, 'right', 'Last Running Host');

	this.params = {};
	this.params.order = this.task_num;
	this.percent = 0;
	this.state = {};
	
}

TaskItem.prototype.update = function()
{
	this.params.name = this.genName();
	this.elName.textContent = this.params.name;
}

TaskItem.prototype.updateProgress = function( i_progress)
{
	if( i_progress.per == null )
		i_progress.per = 0;

	for( var attr in i_progress )
		this.params[attr] = i_progress[attr];

	if( this.params.hst == null ) this.params.hst = '';
	if( this.params.act == null ) this.params.act = '';
	if( this.params.str == null ) this.params.str = 0;
	if( this.params.err == null ) this.params.err = 0;

	this.elState.textContent = this.params.state;
	cm_GetState( this.params.state, this.state, this.element);

	this.elStarts.textContent = 's' + this.params.str;
	this.elErrors.textContent = 'e' + this.params.err;
	this.elHost.textContent = this.params.hst;
	if( this.params.act != '' )
		this.elActivity.textContent = ': '+this.params.act;
	else
		this.elActivity.textContent = '';
//console.log( this.params.act)

	if( this.params.tst )
		if( this.params.tdn && ( this.state.RUN != true ))
			this.elTime.textContent = cm_TimeStringInterval( this.params.tst, this.params.tdn);
		else
			this.elTime.textContent = cm_TimeStringInterval( this.params.tst);

	this.percent = 0;

	if( this.state.RUN )
	{
		this.percent = this.params.per;
		if( this.percent < 0 ) this.percent = 0;
		else if( this.percent > 100 ) this.percent = 100;

		this.elPercent.textContent = ' ' + this.percent + '%';
		this.elStar.style.display = 'block';
	}
	else
	{
		this.elPercent.textContent = '';
		this.elStar.style.display = 'none';
	}
	this.elBar.style.width = ( this.percent + '%');
}

TaskItem.prototype.genName = function()
{
	var t = this.task_num;
	var name = 'task ' + t;
	var tasks_name = this.block.params.tasks_name;
	if( this.block.params.tasks )
	{
		var task_name = this.block.params.tasks[t].name;
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
	this.generateThumbnails();
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

TaskItem.prototype.generateThumbnails = function()
{
	if (this.block.params.files)
	{
		thumbnail_http_path = this.block.thumbnail_http_path.replace("%(task_id)d", this.task_num);
		//console.log(this);
		var files = cm_FillNumbers(this.block.params.files, this.frame_start);
		
		var files_list = files.split(";");
		for(i in files_list) {
			var filepath = files_list[i];
			var thumbnail_name = filepath.split("/").pop();
			var temp = thumbnail_name.split(".");
			temp.pop();
			thumbnail_name = temp.join(".")+".jpg";
			thumbnail_path = thumbnail_http_path.replace("%(thumbnail_filename)s", thumbnail_name);
		}
	}
}

TaskItem.prototype.genFrames = function()
{
	var p = this.block.params;

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
	var maxShownOutputs = 5;
	if( this.params.str && ( this.params.str > 1 ))
	{
		for( var i = this.params.str; i > 0; i--)
		{
			if( i <= this.params.str - maxShownOutputs) break;
			var num = i;
			if( i == this.params.str ) num = 0;
			i_menu.addItem('output', this, 'menuHandleOutput', 'Output '+i, true, num);
		}
		if( this.params.str > maxShownOutputs )
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
		new cgru_Dialog( this.monitor.window, this, 'menuHandleGet', 'output', 'num', null, this.job.name, 'Get Task Process Output', 'Enter Start Number');
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

TaskItem.sort = ['order','name','hst','str','err'];
TaskItem.filter = ['name','hst'];
