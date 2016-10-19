wt_windows = [];

wt_types  = ['TExec',    'TOutput','TLog','TErrorHosts','TListen'];
wt_labels = ['Executable','Output','Log', 'Error Hosts','Listen'];

function wt_same( i_a, i_b)
{
	if( i_a.pos.job   != i_b.pos.job   ) return false;
	if( i_a.pos.block != i_b.pos.block ) return false;
	if( i_a.pos.task  != i_b.pos.task  ) return false;

	return true;
}

function WndTaskOpen( i_args)
{
	for( var i = 0; i < wt_windows.length; i++)
		if( wt_same( wt_windows[i], i_args))
			return null;

	return new WndTask( i_args);
}

function WndTaskShow( i_obj)
{
	var task = i_obj.task;
	if( task == null )
	{
		g_Error('WndTaskShow: Task is NULL.');
	}

	for( var i = 0; i < wt_windows.length; i++)
		if( wt_same( wt_windows[i], task))
			wt_windows[i].show( task);
}

function WndTask( i_args)
{
	this.pos = i_args.pos;
	this.taskitem = i_args.taskitem;

	this.name = 'WndTask[' + this.pos.job + '][' + this.pos.block + '][' + this.pos.task + ']';

	i_args.title = this.name;
	this.wnd = g_OpenWindow( i_args);
	this.wnd.wndtask = this;
	this.wnd.onbeforeunload = function(e){e.currentTarget.wndtask.destroy()};
	this.wnd.onDestroy = function(i_wnd){ i_wnd.wndtask.destroy();};
	this.elContent = this.wnd.elContent;
	this.elContent.classList.add('wnd_task');

	// Progress:
	this.elProgress = document.createElement('div');
	this.elContent.appendChild( this.elProgress);
	this.elProgress.classList.add('progress');

	// Buttons:
	var divBtns = document.createElement('div');
	this.elContent.appendChild( divBtns);
	divBtns.classList.add('buttons_div');

	this.elBtnSkip = document.createElement('div');
	divBtns.appendChild( this.elBtnSkip);
	this.elBtnSkip.classList.add('button');
	this.elBtnSkip.textContent = 'Skip';
	this.elBtnSkip.title = 'Double click to skip a task';
	this.elBtnSkip.m_operation = 'skip';
	this.elBtnSkip.m_window= this;
	this.elBtnSkip.ondblclick = function(e){ var el = e.currentTarget; el.m_window.doOperation( el);}

	this.elBtnRestart = document.createElement('div');
	divBtns.appendChild( this.elBtnRestart);
	this.elBtnRestart.classList.add('button');
	this.elBtnRestart.textContent = 'Restart';
	this.elBtnRestart.title = 'Double click to restart a task';
	this.elBtnRestart.m_operation = 'restart';
	this.elBtnRestart.m_window= this;
	this.elBtnRestart.ondblclick = function(e){ var el = e.currentTarget; el.m_window.doOperation( el);}

	// Ouput:
	var elOutputDiv = document.createElement('div');
	divBtns.appendChild( elOutputDiv);
	elOutputDiv.classList.add('output_div');

	var elOutputLabel = document.createElement('div');
	elOutputDiv.appendChild( elOutputLabel);
	elOutputLabel.classList.add('output_label');
	elOutputLabel.textContent = 'Ouput Number:';

	this.elOutputNum = document.createElement('div');
	elOutputDiv.appendChild( this.elOutputNum);
	this.elOutputNum.classList.add('output_num');
	this.elOutputNum.contentEditable = true;
	this.elOutputNum.m_wnd_task = this;
	this.elOutputNum.onkeydown = function(e){ e.currentTarget.m_wnd_task.outNumPress(e);}
	this.elOutputNum.onblur = function(e){ e.currentTarget.m_wnd_task.outNumBlur(e);}

	// Tabs:
	var divTabs = document.createElement('div');
	this.elContent.appendChild( divTabs);
	divTabs.classList.add('tabs_div');

	var divTabBtns = document.createElement('div');
	divTabs.appendChild( divTabBtns);
	divTabBtns.classList.add('tabs_btns_div');

	var divTabPages = document.createElement('div');
	divTabs.appendChild( divTabPages);
	divTabPages.classList.add('tab_pages_div');

	this.elTabs = {};
	this.elPages = {};
	for( var i = 0; i < wt_types.length; i++)
	{
		var el = document.createElement('div');
		divTabBtns.appendChild( el);
		el.classList.add('button');
		el.classList.add('tab_btn');
		el.textContent = wt_labels[i];
		el.m_type = wt_types[i];
		el.m_wndtask = this;
		el.onclick = function(e){ e.currentTarget.m_wndtask.tabClicked( e.currentTarget.m_type); }
		this.elTabs[wt_types[i]] = el;

		var el = document.createElement('div');
		divTabPages.appendChild( el);
		el.classList.add('tab_page');
		this.elPages[wt_types[i]] = el;
	}

	wt_windows.push( this);

//	this.get('info');
	this.tabClicked('TExec');
}

WndTask.prototype.destroy = function()
{
//console.log('Task window closed: ' + this.name);
	cm_ArrayRemove( wt_windows, this);

	if( this.listening )
		this.listen( false);

	this.taskitem.wndtask = null;
}

WndTask.prototype.close = function()
{
	if( this.wnd.destroy )
		this.wnd.destroy();
	else
		this.wnd.close();
}

WndTask.prototype.get = function( i_mode)
{
	var get = {"type":'jobs',"ids":[this.pos.job],"mode":i_mode}
	get.block_ids = [this.pos.block];
	get.task_ids = [this.pos.task];
	get.mon_id = g_id;
	if(( i_mode == 'output' ) && this.output_number )
		get.number = this.output_number;

	nw_request({"send":{"get":get},"func":WndTaskShow});
}

WndTask.prototype.tabClicked = function( i_type)
{
	for( var i = 0; i < wt_types.length; i++)
	{
		this.elTabs[wt_types[i]].classList.remove('active');
		this.elPages[wt_types[i]].classList.remove('active');
	}

	this.elTabs[i_type].classList.add('active');
	this.elPages[i_type].classList.add('active');

	if( i_type == 'TExec')
		this.get('info');
	else if( i_type == 'TOutput')
		this.get('output');
	else if( i_type == 'TLog')
		this.get('log');
	else if( i_type == 'TErrorHosts')
		this.get('error_hosts');
	else if( i_type == 'TListen')
		this.listen( true);
}

WndTask.prototype.show = function( i_obj)
{
//console.log('WndTask.prototype.show: ' + this.name);
//console.log( JSON.stringify( i_obj));

	// Update progress:
	this.updateProgress( i_obj.progress);

	//
	// Set window title
	//
	var title = 'Task[' + i_obj.job_name + '][' + i_obj.block_name + '][' + i_obj.task_name + ']';
	if( this.wnd.setTitle )
		this.wnd.setTitle( title); // It is a CGRU window.
	else
		this.wnd.document.title = title; // It is a separated browser window.

	//
	// Show data:
	//
	if( i_obj.type == 'TExec' )
		this.showExec( i_obj.exec);
	else if( i_obj.type == 'TOutput' )
		this.showOutput( i_obj.data);
	else if( i_obj.type == 'TLog' )
		this.showLog( i_obj.data);
	else if( i_obj.type == 'TErrorHosts' )
		this.showErrorHosts( i_obj.data);
	else if( i_obj.type == 'TListen' )
		this.showListen( i_obj.data);
}

WndTask.prototype.updateProgress = function( i_progress)
{
	//
	// Show progress info:
	//
	this.progress = i_progress;
	this.state = {};
	cm_GetState( this.progress.state, this.state);

	var info = ' State: <b>' + this.progress.state + '</b>';
	if( this.state.RUN && this.progress.per) info += ' <b>' + this.progress.per + '%</b>';
	if( this.progress.str ) info += ' Starts: <b>' + this.progress.str + '</b>';
	if( this.progress.err ) info += ' (<b>' + this.progress.err + '</b> errors)';
	if( this.progress.hst ) info += ' Last host: <b>' + this.progress.hst + '</b>';

	if( this.progress.tst && this.progress.tdn && ( ! this.state.RUN ))
		info += ' Time: <b>' + cm_TimeStringInterval( this.progress.tst, this.progress.tdn) + '</b>';
	this.elProgress.innerHTML = info;

	//
	// Update buttons state:
	//
	if( this.state.RDY || this.state.RUN )
		this.elBtnSkip.classList.add('active');
	else
		this.elBtnSkip.classList.remove('active');

	if( this.state.RUN || this.state.DON || this.state.ERR )
		this.elBtnRestart.classList.add('active');
	else
		this.elBtnRestart.classList.remove('active');
}

WndTask.prototype.outNumPress = function( i_evt)
{
	if( i_evt.key == 'Enter' )
	{
		i_evt.preventDefault();
		this.outNumProcess();
	}
}
WndTask.prototype.outNumBlur = function( i_evt) { this.outNumProcess(); }
WndTask.prototype.outNumProcess = function()
{
	var str = this.elOutputNum.textContent;
	var num = parseInt( str);

	if( isNaN( num))
	{
		this.output_number = 0;
		this.elOutputNum.textContent = this.output_number;
	}
	else if( this.output_number != num )
	{
		this.output_number = num;
		this.tabClicked('TOutput');
	}
}

WndTask.prototype.showOutput = function( i_data)
{
	this.elPages.TOutput.textContent = '';
	var el = document.createElement('pre');
	el.textContent = i_data;
	this.elPages.TOutput.appendChild( el);
}

WndTask.prototype.showLog = function( i_data)
{
	if( i_data.length == 0 )
	{
		this.elPages.TLog.innerHTML = '<i>Task log is empty.</i>';
		return;
	}

	this.elPages.TLog.textContent = '';

	var lines = i_data.split('\n');
	for( var i = 0; i < lines.length; i++)
	{
		var el = document.createElement('div');
		el.textContent = lines[i];
		this.elPages.TLog.appendChild( el);
	}
}

WndTask.prototype.showErrorHosts = function( i_data)
{
	if( i_data.length == 0 )
	{
		this.elPages.TErrorHosts.innerHTML = '<i>Task has no error hosts.</i>';
		return;
	}

	this.elPages.TErrorHosts.textContent = '';

	var lines = i_data.split('\n');
	for( var i = 0; i < lines.length; i++)
	{
		var el = document.createElement('div');
		el.textContent = lines[i];
		this.elPages.TErrorHosts.appendChild( el);
	}
}

WndTask.prototype.listen_Start = function( i_args)
{
	var name = 'Listen Job';
	if( i_args.task != null )
		name = 'Listen Task';

	wnd = new cgru_Window({"name":name,"wnd":i_args.parent_window});
	wnd.elContent.classList.add('listen');

	wnd.listen = i_args;
	wnd.processMsg = listen_ProcessMsg;
	wnd.onDestroy = listenSubscribe;

	listenSubscribe( wnd);
}

WndTask.prototype.listen = function( i_subscribe)
{
	if( this.listening == i_subscribe )
		return;

	var op = {};
	op.type = 'watch';
	op.class = 'listen';
	op.job = this.pos.job;
	op.block = this.pos.block;
	op.task = this.pos.task;
	op.status = i_subscribe ? 'subscribe' : 'unsubscribe';

	nw_Action('monitors', [g_id], op);

	this.listening = i_subscribe;
}

WndTask.prototype.showListen = function( i_data)
{
	var el = document.createElement('pre');
	el.textContent = i_data;
	this.elPages.TListen.appendChild( el);
	el.scrollIntoView( false);
}

WndTask.prototype.showExec = function( i_obj)
{
	var elParent = this.elPages.TExec;
	elParent.textContent = '';

	var t_attrs = {};
	t_attrs.name = {};
	t_attrs.capacity = {"float":'left',"width":'24%'};
	t_attrs.service = {"float":'left',"width":'38%'};
	t_attrs.parser = {"float":'left',"width":'38%'};
	t_attrs.command = {"pathmap":true};
	t_attrs.working_directory = {"label":'Directory',"pathmap":true};
	t_attrs.environment = {"label":'Environ'};

	var attrs = document.createElement('div');
	elParent.appendChild( attrs);
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

		var elValue = document.createElement('div');
		div.appendChild( elValue);
		elValue.classList.add('value');
		var value = i_obj[attr];
		if( attr == 'environment' )
		{
			var env = value;
			value = '';
			for( name in env )
			{
				if( value.length )
					value += ',';
				value += name + '=' + env[name];
			}
		}
		if( t_attrs[attr].pathmap )
			elValue.textContent = cgru_PM( value);
		else
			elValue.textContent = value;
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
		elParent.appendChild( elFilesDiv);
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
			elFile.onclick = wt_FileOpen;
		}
	}

	var elRawDiv = document.createElement('div');
	elParent.appendChild( elRawDiv);
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

function wt_FileOpen( i_evt)
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
		var cmd = cgru_Config.previewcmds[c].replace('@ARG@', file);
		cgru_CmdExecCreate({"parent":elCmds,"cmd":cmd,"label":cmd});
	}

	return false;
}

WndTask.prototype.doOperation = function( i_el)
{
	if( false == i_el.classList.contains('active'))
		return;

	var operation = {};
	operation.type = i_el.m_operation;
	operation.task_ids = [this.pos.task];

	nw_Action('jobs', [this.pos.job], operation, null,[this.pos.block]);

	this.elPages.TListen.textContent = '';
}

