g_cycle = 0;
g_id = 0;
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

function register()
{
	if( g_id != 0)
		return;

	var obj = {};
	obj.monitor = {};
	obj.monitor.name = g_name;
	obj.monitor.version = g_version;
	send(obj);

	setTimeout("register()", 5000);
}

function processMsg( obj)
{
document.getElementById('recv').innerHTML='c' + g_cycle + ' recv: ' + JSON.stringify(obj);

	if( obj.id != null )
	{
		if(( g_id == 0 ) && ( obj.id > 0 ))
		{
			// Monitor is not registered and recieved an ID:
			g_id = obj.id;
			registered();
		}
		else if( obj.id != g_id )
		{
			// Recieved ID does not match:
			g_id = 0;
			deregistered();
			register();
		}
	}

	if( g_id == 0 )
		return;

	for( i = 0; i < g_recievers.length; i++)
	{
		g_recievers[i].processMsg( obj);
	}
}

function update()
{
	g_cycle++;
	setTimeout("update()", 1000);

	document.getElementById('id').innerHTML = 'ID = ' + g_id + ' c' + g_cycle;

	if( g_id == 0 )
		return;

	getEvents('monitors','events');

	for( i = 0; i < g_updaters.length; i++)
	{
		g_updaters[i].update();
	}
}

function init()
{
//	document.getElementById('id').innerHTML='launching...';

	register();
	update();
	cm_Init();
}

function registered()
{
	renders = new RendersList(document.getElementById('view'));
}

function deregistered()
{
	for( var i = 0; i < g_monitors.length; i++)
		g_monitors[i].destroy();
}
