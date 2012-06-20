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
g_updaters = [];
g_monitors = [];
g_cur_monitor = null;
g_monitor_buttons = [];

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
	}

	if( g_id == 0 )
		return;

	for( i = 0; i < g_recievers.length; i++)
	{
		g_recievers[i].processMsg( obj);
	}
}

function g_Update()
{
	g_cycle++;
	setTimeout("g_Update()", 1000);

	document.getElementById('id').innerHTML = 'ID = ' + g_id + ' c' + g_cycle;

	if( g_id == 0 )
		return;

	nw_GetEvents('monitors','events');

	for( i = 0; i < g_updaters.length; i++)
	{
		g_updaters[i].update();
	}
}

function g_Init()
{
	var header = document.getElementById('header');
	g_monitor_buttons = header.getElementsByClassName('mbutton');
	for( var i = 0; i < g_monitor_buttons.length; i++)
		g_monitor_buttons[i].onclick = g_MButtonClick;

	g_Register();
	g_Update();
	cm_Init();
}

function g_Registered()
{
//	new Monitor( document.getElementById('view'), 'renders');
	new Monitor( document.getElementById('view'), 'jobs');
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
	for( var i = 0; i < g_monitor_buttons.length; i++)
		g_monitor_buttons[i].classList.remove('pushed');

	while( g_monitors.length > 0 )
		g_monitors[0].destroy();
}

function g_OpenTasks( jobId)
{
	g_CloseAllMonitors();
	new Monitor( document.getElementById('view'), 'tasks', jobId);
//	new Monitor( document.getElementById('view'), 'renders');
//document.getElementById('test').innerHTML = jobId;
}

function g_MButtonClick( evt)
{
	if( evt == null ) return;
	var el = evt.currentTarget;
	if( el == null ) return;

	g_CloseAllMonitors();
	el.classList.add('pushed');

	new Monitor( document.getElementById('view'), el.innerHTML);
}

function g_Info( i_msg, i_elem)
{
	if( i_elem == null )
		i_elem = 'info';
	document.getElementById(i_elem).innerHTML=i_msg;
}

function g_Error( i_err)
{
	g_Info('Error: ' + i_err);
}
