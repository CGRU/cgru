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
/*
	this.thumbnail_http_base = cgru_Config['af_thumbnail_http'];
	this.thumbnail_http_naming = cgru_Config['af_thumbnail_naming'];
	
	this.thumbnail_http_path = this.thumbnail_http_base + this.thumbnail_http_naming;
	this.thumbnail_http_path = this.thumbnail_http_path.replace("%(job_id)d", this.params.job_id);
	this.thumbnail_http_path = this.thumbnail_http_path.replace("%(block_id)d", this.params.block_num);
*/
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
//	this.generateThumbnails();

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
/*
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
*/
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
			i_menu.addItem({"name":'output', "receiver":this, "handle":'mh_Output', "label":'Output '+i, "param":num});
		}
		if( this.params.str > maxShownOutputs )
			i_menu.addItem({"name":'output', "receiver":this, "handle":'mh_Output', "label":'Output...', "param":-1});
	}
	else
		i_menu.addItem({"name":'output', "receiver":this, "handle":'mh_Output', "label":'Output'});
}

TaskItem.prototype.mh_Output = function( i_number, i_evt)
{
	if( i_number == -1 )
	{
		new cgru_Dialog({"wnd":this.monitor.window,"receiver":this,"handle":'menuHandleGetOutput',"param":i_evt,
			"type":'num',"name":this.job.name,"title":'Get Task Process Output',"info":'Enter Start Number'});
		return;
	}
	this.mh_Get('output', i_evt, i_number);
}

TaskItem.prototype.menuHandleGetOutput = function( i_number, i_evt){ this.mh_Get('output', i_evt, i_number)}

TaskItem.prototype.mh_Get = function( i_mode, i_evt, i_number)
{
	var get = {"type":'jobs',"ids":[this.job.id],"mode":i_mode,"number":i_number};
	get.block_ids = [this.block.block_num];
	get.task_ids = [this.task_num];
	get.mon_id = g_id;

	nw_request({"send":{"get":get},"func":g_ShowObject,"evt":i_evt,"wnd":this.monitor.window});
}
TaskItem.mh_Get = function( i_param, i_evt)
{
	var task = i_param.monitor.cur_item;
	if( task == null )
	{
		g_Error('No tasks selected.');
		return;
	}
	if( task.task_num == null )
		return;

	var get = {'type':'jobs','ids':[task.job.id],'mode':i_param.name,'number':i_param.number};
	get.block_ids = [task.block.block_num];
	get.task_ids = [task.task_num];

	nw_request({'send':{'get':get},'func':g_ShowObject,'evt':i_evt,'wnd':task.monitor.window});
}
/*
TaskItem.prototype.mh_Oper = function( i_name, i_value)
{
console.log('TaskItem.prototype.mh_Oper: ' + i_name);
	var operation = {};
	operation.type = i_name;
	var bids = []; var tids = [];
	this.getBlockTasksIds( bids, tids);
	if( tids.length ) operation.task_ids = tids;
	nw_Action('jobs', [this.job.id], operation, null, bids);
}
*/
TaskItem.mh_Lis = function( i_param)
{
	var task = i_param.monitor.cur_item;
	if( task == null )
	{
		g_Error('No task selected.');
		return;
	}
	if( task.task_num == null )
		return;

	var bids = []; var tids = [];
	task.getBlockTasksIds( bids, tids);
	if(( tids.length == 0 ) || ( bids.length == 0 ))
	{
		g_Error('No task selected to listen.');
		return;
	}

	var args = {};
	args.job   = task.job.id;
	args.block = bids[0];
	args.task  = tids[0];
	args.parent_window = task.monitor.window;

	listen_Start( args);
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
	this.mh_Get('info', i_evt);
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
	acts.info    = {'handle':'mh_Get', 'label':'INFO','tooltip':'Get task full info.'};
	acts.skip    = {'handle':'mh_Oper','label':'SKIP','tooltip':'Double click to skip selected task(s).','ondblclick':true};
	acts.restart = {'handle':'mh_Oper','label':'RES', 'tooltip':'Double click to restart selected task(s).','ondblclick':true};
	acts.listen  = {'handle':'mh_Lis', 'label':'LIS', 'tooltip':'Double click to listen task.','ondblclick':true};
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

t_attrs = {};
t_attrs.name = {};
t_attrs.capacity = {"float":'left',"width":'24%'};
t_attrs.service = {"float":'left',"width":'38%'};
t_attrs.parser = {"float":'left',"width":'38%'};
t_attrs.command = {"pathmap":true};
t_attrs.working_directory = {"label":'Directory',"pathmap":true};

function t_ShowExec( i_obj, i_elParent)
{
	i_elParent.classList.add('task_exec');

	var attrs = document.createElement('div');
	i_elParent.appendChild( attrs);
	attrs.classList.add('attrs');

	for( attr in t_attrs )
	{
		if( i_obj[attr] == null )
			continue;

		var div = document.createElement('div');
		attrs.appendChild( div);
		div.classList.add('attr');
		if( t_attrs[attr].float )
			div.style.cssFloat = t_attrs[attr].float;
		else
			div.style.clear = 'both';
		if( t_attrs[attr].width )
			div.style.width = t_attrs[attr].width;

		var label = document.createElement('div');
		div.appendChild( label);
		label.classList.add('label');
		if( t_attrs[attr].label )
			label.textContent = t_attrs[attr].label + ':';
		else
		{
			label.textContent = attr + ':';
			label.style.textTransform = 'capitalize';
		}

		var value = document.createElement('div');
		div.appendChild( value);
		value.classList.add('value');
		if( t_attrs[attr].pathmap )
			value.textContent = cgru_PM( i_obj[attr]);
		else
			value.textContent = i_obj[attr];
	}

	var files = [];
	var label = 'Files:';
	if( i_obj.files ) files = i_obj.files;
	if( i_obj.parsed_files && i_obj.parsed_files.length )
	{
		files = i_obj.parsed_files;
		label = 'Files (parsed):';
	}
	
	var dir_pm = cgru_PM( i_obj.working_directory);
	if( files.length )
	{
		var elFilesDiv = document.createElement('div');
		i_elParent.appendChild( elFilesDiv);
		elFilesDiv.classList.add('files_div');

		var elFilesLabel = document.createElement('div');
		elFilesDiv.appendChild( elFilesLabel);
		elFilesLabel.textContent = label;

		var elFiles = document.createElement('div');
		elFilesDiv.appendChild( elFiles);
		elFiles.classList.add('files');

		for( var f = 0; f < files.length; f++)
		{
			var file = cgru_PM( files[f]);

			var elFile = document.createElement('div');
			elFiles.appendChild( elFile);
			elFile.textContent = cm_PathBase( file);
			elFile.title = file;
			elFile.m_file = file;
			elFile.m_dir = dir_pm;
			elFile.onclick = t_FileOpen;
		}
	}

	var elRawDiv = document.createElement('div');
	i_elParent.appendChild( elRawDiv);
	elRawDiv.classList.add('raw');

	var elRawLabel = document.createElement('div');
	elRawDiv.appendChild( elRawLabel);
	elRawLabel.classList.add('label');
	elRawLabel.textContent = 'Raw Object:';

	var elRawObj = document.createElement('div');
	elRawDiv.appendChild( elRawObj);
	elRawObj.classList.add('object');
	elRawObj.innerHTML = JSON.stringify( i_obj, null, '&nbsp&nbsp&nbsp&nbsp').replace(/\n/g,'<br/>');
}

function t_FileOpen( i_evt)
{
	elFile = i_evt.currentTarget;

	if( elFile.m_opened )
	{
		elFile.m_opened = false;
		elFile.removeChild( elFile.m_elCmds);
		elFile.classList.remove('opened');
		return;
	}

	var file = cgru_PathJoin( elFile.m_dir, elFile.m_file);

	elFile.m_opened = true;
	elFile.classList.add('opened');

	var elCmds = document.createElement('div');
	elFile.appendChild( elCmds);
	elFile.m_elCmds = elCmds;

	for( var c = 0; c < cgru_Config.previewcmds.length; c++ )
	{
		var elCmd = document.createElement('div');
		elCmds.appendChild( elCmd);
		elCmd.classList.add('cmdexec');
		elCmd.textContent = cgru_Config.previewcmds[c].replace('@ARG@', file);
	}

	return false;
}

