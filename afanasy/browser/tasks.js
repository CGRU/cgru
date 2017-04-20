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

	this.elThumbBtn = this.monitor.document.createElement('div');
	this.element.appendChild( this.elThumbBtn);
	this.elThumbBtn.classList.add('thumbs_btn');
	this.elThumbBtn.textContent = 'T';
	this.elThumbBtn.m_task = this;
	this.elThumbBtn.onclick = function(e){ e.stopPropagation(); e.currentTarget.m_task.showTumbs();};

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

	if( this.params.str == null ) this.params.str = 0;
	if( this.params.err == null ) this.params.err = 0;

	this.elState.textContent = this.params.state;
	cm_GetState( this.params.state, this.state, this.element);

	this.elStarts.textContent = 's' + this.params.str;
	this.elErrors.textContent = 'e' + this.params.err;
	if( this.params.hst )
		this.elHost.textContent = this.params.hst;
	if( this.params.act != null )
		this.elActivity.textContent = ': '+this.params.act;
	else
		this.elActivity.textContent = '';
//console.log( this.params.act)

	if( this.params.tst )
		if( this.params.tdn && ( this.state.RUN != true ))
		{
			this.elTime.textContent = cm_TimeStringInterval( this.params.tst, this.params.tdn);
			this.elTime.title = 'Started at: ' + cm_DateTimeStrFromSec( this.params.tst)
				+ '\nFinished at: ' + cm_DateTimeStrFromSec( this.params.tdn);
		}
		else
		{
			this.elTime.textContent = cm_TimeStringInterval( this.params.tst);
			this.elTime.title = 'Started at: ' + cm_DateTimeStrFromSec( this.params.tst);
		}

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

	if( this.wndtask )
		this.wndtask.updateProgress( i_progress);
}

TaskItem.prototype.genName = function()
{
	var t = this.task_num;
	var name = 'task ' + t; //< This is the default task name

	var tasks_name = this.block.params.tasks_name;

	// If block is not numeric:
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

	// The block is numeric:
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
		if( blocks[b].selected )
		{
			o_bids.push( b);
			break;
		}
		else
		{
			var tasks = blocks[b].tasks;
			for( var t = 0; t < tasks.length; t++)
			{
				if( tasks[t].selected )
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
//console.log('bids='+o_bids+' tids='+o_tids);
}

TaskItem.mh_Oper = function( i_param)
{
	var task = i_param.monitor.cur_item;
	if( task == null )
	{
		g_Error('No tasks selected.');
		return;
	}
	if( task.task_num == null )
		return;

	var operation = {};
	operation.type = i_param.name;

	var bids = []; var tids = [];
	task.getBlockTasksIds( bids, tids);

	if( tids.length )
		operation.task_ids = tids;

	nw_Action('jobs', [task.job.id], operation, null, bids);
}

TaskItem.prototype.onDoubleClick = function( i_evt)
{
	if( this.wndtask )
		return;

	var args = {};

	args.pos = {};
	args.pos.job   = this.job.id;
	args.pos.block = this.block.block_num;
	args.pos.task  = this.task_num;

	args.wnd = this.monitor.window;
	args.evt = i_evt;
	args.taskitem = this;

	this.wndtask = WndTaskOpen( args);
}

TaskItem.prototype.monitorDestroy = function()
{
	if( this.wndtask )
		this.wndtask.close();
}

TaskItem.prototype.showTumbs = function()
{
	if( this.elThumbs )
	{
		this.element.removeChild( this.elThumbs);
		this.elThumbs = null;
		return;
	}

	var bids = []; var tids = [];
	this.getBlockTasksIds( bids, tids);

	var get = {"type":'jobs',"ids":[this.job.id],"mode":'files',"block_ids":[bids[0]],"task_ids":[tids[0]]};

	nw_request({"send":{"get":get},"task":this,"func":function( i_obj, i_args){i_args.task.thumbsReceived( i_obj)}});
}

TaskItem.prototype.thumbsReceived = function( i_obj)
{
	if( i_obj == null ) return;
	if( i_obj.task_files == null ) return;
	if( i_obj.task_files.files == null ) return;

	var files = i_obj.task_files.files;

	this.elThumbs = document.createElement('div');
	this.element.appendChild( this.elThumbs);
	this.elThumbs.classList.add('thumbs');

	for( var i = 0; i < files.length; i++ )
	{
		elImg = this.monitor.document.createElement('img');
		this.elThumbs.appendChild( elImg);
		elImg.src = '@TMP@' + files[i].name;
	}
}

TaskItem.createPanels = function( i_monitor)
{
	var acts = {};
	acts.skip    = {'handle':'mh_Oper','label':'SKIP','tooltip':'Double click to skip selected task(s).','ondblclick':true};
	acts.restart = {'handle':'mh_Oper','label':'RES', 'tooltip':'Double click to restart selected task(s).','ondblclick':true};
	i_monitor.createCtrlBtns( acts);
}

TaskItem.prototype.updatePanels = function()
{
	// Info:
	var info = '';

	if( this.params.str )
		info += 'Starts ' + this.params.str + ', last on ' + this.params.hst + ' at:<br> ' + cm_DateTimeStrFromSec( this.params.tst) + '<br>';

	if( this.params.tdn && ( this.state.RUN != true ))
	{
		info += 'Done at:<br> ' + cm_DateTimeStrFromSec( this.params.tdn) + '<br>';
		info += 'Running time: ' + cm_TimeStringInterval( this.params.tst, this.params.tdn) + '<br>';
	}

	if( this.params.err )
		info += 'Errors count: '  + this.params.err + '<br>';

	this.monitor.setPanelInfo( info);
}


TaskItem.sort = ['order','name','hst','str','err'];
TaskItem.filter = ['name','hst'];

