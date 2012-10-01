g_cycle = 0;
g_id = 0;
g_uids = [0];
g_name = 'web';
g_version = 'browser';
g_user_name = "jimmy";
g_host_name = "pc01";

g_mouse_down = false;
g_key_ctrl = false;
g_key_shift = false;

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

	if( obj.id )
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

	if( g_id == 0 )
		return;

	for( i = 0; i < g_recievers.length; i++)
	{
		g_recievers[i].processMsg( obj);
	}
}

function g_Refresh()
{
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
	cm_Init();
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
	g_id = 0;
	g_CloseAllMonitors();
	g_Register();
}

function g_ConnectionLost()
{
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

function g_OpenMonitor( i_type, i_id)
{
	for( var i = 0; i < g_monitor_buttons.length; i++)
		if( g_monitor_buttons[i].textContent == i_type )
			g_monitor_buttons[i].classList.add('pushed');

	new Monitor( document.getElementById('content'), i_type, i_id);
}

function g_OpenTasks( i_job_id)
{
	g_CloseAllMonitors();
	g_OpenMonitor('tasks', i_job_id);
}

function g_ShowMessage( msg)
{
	if( msg.list == null ) return;
	var wnd = window.open( null, 'Message', 'location=no,scrollbars=yes,resizable=yes,menubar=no');
	wnd.document.writeln('<!DOCTYPE html>');
	wnd.document.write('<html><head><title>'+msg.name+':'+msg.type+'</title></head>');
	wnd.document.write('<body style="font: 12px Arial; background: #CCC;">');

	for( i = 0; i < msg.list.length; i++)
		wnd.document.write('<div>'+msg.list[i]+'</div>');

	wnd.document.write('</body></html>');
	wnd.focus();
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
