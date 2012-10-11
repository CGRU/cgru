g_cycle = 0;
g_last_msg_cycle = g_cycle;
g_id = 0;
g_uids = [0];
g_name = 'web';
g_version = 'browser';
g_user_name = "jimmy";
g_host_name = "pc01";

g_windows = [];
g_recievers = [];
g_refreshers = [];
g_monitors = [];
g_cur_monitor = null;
g_monitor_buttons = [];

g_Images = [];

if( localStorage['user_name'] == null )
	localStorage['user_name'] = 0;
localStorage['user_name']++;

function g_Register()
{
	if( g_id != 0)
		return;

	var obj = {};
	obj.monitor = {};
	obj.monitor.name = g_name;
	obj.monitor.version = g_version;
	nw_Send(obj);

	setTimeout("g_Register()", 5000);
}

function g_ProcessMsg( obj)
{
g_last_msg_cycle = g_cycle;
//g_Info( g_cycle+' Progessing '+g_recievers.length+' recieves');
	if( obj.files && obj.path )
	{
		for( var i = 0; i < obj.files.length; i++)
		{
			var img = new Image();
			img.src = obj.path + "/" + obj.files[i];
			g_Images.push( img);
		}
		return;
	}

	if( obj.id != null )
	{
		if(( g_id == 0 ) && ( obj.id > 0 ))
		{
			// Monitor is not registered and recieved an ID:
			g_id = obj.id;
			g_Registered();
		}
		else if( obj.id != g_id )
		{
			// Recieved ID does not match:
			g_Deregistered();
		}
		return;
	}

	if( obj.message )
	{
		g_ShowMessage( obj.message);
		return;
	}
	if( obj.object )
	{
		g_ShowObject( obj.object);
		return;
	}
	if( obj.task_exec )
	{
		g_ShowObject( obj.task_exec);
		return;
	}

	if( g_id == 0 )
		return;

	for( i = 0; i < g_recievers.length; i++)
	{
		g_recievers[i].processMsg( obj);
	}
}

function g_Refresh()
{
	if(( g_last_msg_cycle != null ) && ( g_last_msg_cycle < g_cycle - 10 ))
	{
g_Info( g_last_msg_cycle + ' < ' + g_cycle );
		g_ConnectionLost();
	}

	g_cycle++;
	setTimeout("g_Refresh()", 1000);

//	document.getElementById('id').textContent = 'ID = ' + g_id + ' c' + g_cycle;

	if( g_id == 0 )
		return;

	nw_GetEvents('monitors','events');

	for( i = 0; i < g_refreshers.length; i++)
	{
		g_refreshers[i].refresh();
	}
}

function g_Init()
{
	var header = document.getElementById('header');
	g_monitor_buttons = header.getElementsByClassName('mbutton');
	for( var i = 0; i < g_monitor_buttons.length; i++)
		g_monitor_buttons[i].onclick = g_MButtonClick;

	nw_GetSoftwareIcons();
	g_Register();
	g_Refresh();

	window.onbeforeunload = g_OnClose;
	document.body.onkeydown = g_OnKeyDown;
}

function g_OnClose()
{
	var operation = {};
	operation.type = 'deregister';
	if( g_id)
		nw_Action('monitors', [g_id], operation);
	g_CloseAllWindows();
	g_CloseAllMonitors();
}

function g_Registered()
{
	g_Info('Registed: ID = ' + g_id + ' User = ' + localStorage['user_name']);
	g_OpenMonitor('jobs');
//	g_OpenMonitor('renders');
//	g_OpenMonitor('users');
}

function g_Deregistered()
{
//g_Info('Deregistered.');
	g_id = 0;
	g_CloseAllMonitors();
	g_Register();
}

function g_ConnectionLost()
{
	g_Info('Connection Lost.');
	g_Deregistered();
}

function g_CloseAllMonitors()
{
	cgru_ClosePopus();

	for( var i = 0; i < g_monitor_buttons.length; i++)
		g_monitor_buttons[i].classList.remove('pushed');

	while( g_monitors.length > 0 )
		g_monitors[0].destroy();
}

function g_MButtonClick( evt)
{
	if( evt == null ) return;
	var el = evt.currentTarget;
	if( el == null ) return;

	g_CloseAllMonitors();

	g_OpenMonitor( el.textContent);
}

function g_OpenMonitor( i_type, i_document, i_id, i_name)
{
	var elParent = document.getElementById('content');
	if( i_document == null )
		i_document = document;
	else
		elParent = i_document.body;

	for( var i = 0; i < g_monitor_buttons.length; i++)
		if( g_monitor_buttons[i].textContent == i_type )
			g_monitor_buttons[i].classList.add('pushed');

	return new Monitor( i_document, elParent, i_type, i_id, i_name);
}

function g_OpenTasks( i_job_name, i_job_id)
{
//g_CloseAllMonitors();g_OpenMonitor('tasks', null, i_job_id);return;
	var wnd = g_OpenWindow( i_job_name, i_job_name);
	var monitor = g_OpenMonitor('tasks', wnd.document, i_job_id, i_job_name);
	wnd.monitor = monitor;
	wnd.onbeforeunload = function(e){e.currentTarget.monitor.destroy()};
}

function g_ShowMessage( msg)
{
	if( msg.list == null ) return;
	var name = msg.name+'_'+msg.type;
	var title = msg.name+':'+msg.type;
	var wnd = g_OpenWindow( name, title);
	for( i = 0; i < msg.list.length; i++)
		wnd.document.write('<div>'+((msg.list[i]).replace(/\n/g,'<br/>'))+'</div>');
}

function g_ShowObject( obj)
{
	var title = 'Object';
	if( obj.name ) title = obj.name;
	var wnd = g_OpenWindow( title, title);
	var obj_str = JSON.stringify( obj, null, '&nbsp&nbsp&nbsp&nbsp');
	wnd.document.write( obj_str.replace(/\n/g,'<br/>'));
}

function g_OpenWindow( i_name, i_title, i_focus )
{
	for( var i = 0; i < g_windows.length; i++)
		if( g_windows[i].name == i_name )
			if( i_focus )
			{
				g_windows[i].focus();
				return;
			}
			else
			{
				g_windows[i].close();
			}

	var wnd = window.open( null, i_name, 'location=no,scrollbars=yes,resizable=yes,menubar=no');
	g_windows.push( wnd);
	wnd.name = i_name;

	wnd.document.writeln('<!DOCTYPE html>');
	wnd.document.write('<html><head><title>'+i_title+'</title>');
	wnd.document.write('<link type="text/css" rel="stylesheet" href="lib/styles.css">');
	wnd.document.write('<link type="text/css" rel="stylesheet" href="afanasy/browser/style.css">');
	wnd.document.write('</head><body></body></html>');
	wnd.document.body.onkeydown = g_OnKeyDown;
	wnd.focus();

	return wnd;
}

function g_CloseAllWindows()
{
	for( var i = 0; i < g_windows.length; i++)
		g_windows[i].close();
}

function g_Info( i_msg, i_elem)
{
	if( i_elem == null )
		i_elem = 'info';
	document.getElementById(i_elem).textContent=i_msg;
}

function g_Error( i_err)
{
	g_Info('Error: ' + i_err);
}

function g_OnKeyDown(e)
{
	if(!e) return;
	if(e.keyCode==27) // ESC
	{
		for( var i = 0; i < g_monitors.length; i++)
		{
			g_monitors[i].selectAll( false);
		}
		cgru_ClosePopus();
		return;
	}

	if( cgru_DialogsAll.length || cgru_MenusAll.length ) return;

	if(e.keyCode==65 && e.ctrlKey) // CTRL+A
	{
		if( g_cur_monitor) g_cur_monitor.selectAll( true);
		return false;
	}
	else if((e.keyCode==38) && g_cur_monitor) g_cur_monitor.selectNext( e, true ); // UP
	else if((e.keyCode==40) && g_cur_monitor) g_cur_monitor.selectNext( e, false); // DOWN
//	else if(evt.keyCode==116) return false; // F5
//document.getElementById('test').textContent='key down: ' + e.keyCode;
//	return true; 
}
