g_cycle = 0;
g_last_msg_cycle = g_cycle;
g_id = 0;
g_uid = -1;
g_keysdown = '';

g_windows = [];
g_recievers = [];
g_refreshers = [];
g_monitors = [];
g_cur_monitor = null;
g_main_monitor = null;
g_main_monitor_type = 'jobs';
g_monitor_buttons = [];

g_HeaderOpened = false;
g_FooterOpened = false;

g_Images = [];

function g_RegisterSend()
{
	if( g_id != 0)
		return;

	var obj = {};
	obj.monitor = {};
	obj.monitor.gui_name = localStorage['gui_name'];
	obj.monitor.user_name = localStorage['user_name'];
	obj.monitor.host_name = localStorage['host_name'];
	obj.monitor.engine = navigator.userAgent;
	nw_Send(obj);

	setTimeout('g_RegisterSend()', 5000);
}

function g_ProcessMsg( obj)
{
//g_Info( g_cycle+' Progessing '+g_recievers.length+' recieves');
	g_last_msg_cycle = g_cycle;

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

	if( obj.monitor )
	{
		if(( g_id == 0 ) && ( obj.monitor.id > 0 ))
		{
			// Monitor is not registered and recieved an ID:
			g_RegisterRecieved( obj.monitor);
		}
		else if( obj.monitor.id != g_id )
		{
			// Recieved ID does not match:
			g_Info('This ID = '+g_id+' != '+obj.monitor.id+' recieved.');
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

	for( var i = 0; i < g_recievers.length; i++)
	{
		g_recievers[i].processMsg( obj);
	}
}

function g_Refresh()
{
	if(( g_last_msg_cycle != null ) && ( g_last_msg_cycle < g_cycle - 10 ))
		g_ConnectionLost();

	g_cycle++;
	setTimeout("g_Refresh()", 1000);

	if( g_id == 0 )
		return;

	nw_GetEvents('monitors','events');

	for( var i = 0; i < g_refreshers.length; i++)
	{
		g_refreshers[i].refresh();
	}
}

function g_Init()
{
	window.onbeforeunload = g_OnClose;
	document.body.onkeydown = g_OnKeyDown;

	nw_GetSoftwareIcons();

	g_ConstructSettingsGUI();
	g_InitSettings();

	var header = document.getElementById('header');
	g_monitor_buttons = header.getElementsByClassName('mbutton');
	for( var i = 0; i < g_monitor_buttons.length; i++)
		g_monitor_buttons[i].onclick = function(e){return g_MButtonClicked(e.currentTarget.textContent,e);};

	g_RegisterSend();
	g_Refresh();
}

function g_RegisterRecieved( i_obj)
{
	g_id = i_obj.id;
	if( i_obj.uid && ( i_obj.uid > 0 ))
		g_uid = i_obj.uid;

	g_Info('Registed: ID = '+g_id+' User = "'+localStorage['user_name']+'"['+g_uid+"]");
	document.getElementById('registered').textContent = 'Registered';
	document.getElementById('id').textContent = g_id;
	document.getElementById('uid').textContent = g_uid;

	g_MButtonClicked( g_main_monitor_type);

	g_SuperUserProcessGUI();
}

function g_Deregistered()
{
	if( g_id == 0 )
		return;

	g_id = 0;
	g_uid = -1;
	g_Info('Deregistered.');
	document.getElementById('registered').textContent = 'Deregistered';
	document.getElementById('id').textContent = g_id;
	document.getElementById('uid').textContent = g_uid;
	g_CloseAllMonitors();
	g_RegisterSend();
}

function g_ConnectionLost()
{
	if( g_id == 0 )
		return;

	g_Info('Connection Lost.');
	g_Deregistered();
}

function g_MButtonClicked( i_type, i_evt)
{
	for( var i = 0; i < g_monitor_buttons.length; i++)
		if( g_monitor_buttons[i].textContent == i_type )
			if( g_monitor_buttons[i].classList.contains('pushed'))
				return;
			else
				g_monitor_buttons[i].classList.add('pushed');

	var new_wnd = false;
	if( i_evt )
	{
		if( i_evt.shiftKey ) new_wnd = true;
		if( i_evt.ctrlKey ) new_wnd = true;
		if( i_evt.altKey ) new_wnd = true;
	}

	g_OpenMonitor( i_type, new_wnd);
}

function g_MonitorClosed( i_monitor)
{
	for( var i = 0; i < g_monitor_buttons.length; i++)
		if( g_monitor_buttons[i].textContent == i_monitor.name )
			g_monitor_buttons[i].classList.remove('pushed');
	if( g_main_monitor == i_monitor )
		g_main_monitor = null;
}

function g_OpenMonitor( i_type, i_new_wnd, i_id, i_name)
{
	if( i_name == null )
		i_name = i_type;

	for( var i = 0; i < g_monitors.length; i++)
		if( g_monitors[i].name == i_name )
		{
			g_Error('Monitor "'+i_name+'" already opened.');
			return;
		}

	var elParent = document.getElementById('content');
	var wnd = window;
	if( i_new_wnd )
	{
		wnd = g_OpenWindow( i_name);
		if( wnd == null ) return;
		elParent = wnd.document.body;
	}
	else if( g_main_monitor )
		g_main_monitor.destroy();

	var monitor = new Monitor( wnd, elParent, i_type, i_id, i_name);

	if( i_new_wnd )
	{
		wnd.monitor = monitor;
		wnd.onbeforeunload = function(e){e.currentTarget.monitor.destroy()};
	}
	else
	{
		g_main_monitor = monitor;
		g_main_monitor_type = i_type;
		localStorage.main_monitor = i_type;
	}

	return monitor;
}
function g_OpenTasks( i_job_name, i_job_id) { g_OpenMonitor('tasks', true, i_job_id, i_job_name);}

function g_CloseAllMonitors()
{
	cgru_ClosePopus();

//	for( var i = 0; i < g_monitor_buttons.length; i++)
//		g_monitor_buttons[i].classList.remove('pushed');

	while( g_monitors.length > 0 )
		g_monitors[0].destroy();
}

function g_ShowMessage( msg)
{
	if( msg.list == null ) return;
	var name = msg.name+'_'+msg.type;
	var title = msg.name+':'+msg.type;
	var wnd = g_OpenWindow( name, title);
	if( wnd == null ) return;
	for( var i = 0; i < msg.list.length; i++)
		wnd.document.write('<div>'+((msg.list[i]).replace(/\n/g,'<br/>'))+'</div>');
}

function g_ShowObject( obj)
{
	var title = 'Object';
	if( obj.name ) title = obj.name;
	var wnd = g_OpenWindow( title, title);
	if( wnd == null ) return;
	var obj_str = JSON.stringify( obj, null, '&nbsp&nbsp&nbsp&nbsp');
	wnd.document.write( obj_str.replace(/\n/g,'<br/>'));
}

function g_OpenWindow( i_name, i_title, i_focus )
{
	if( i_title == null )
		i_title = i_name;

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
	if( wnd == null )
	{
		g_Error('Can`t open new browser window.');
		return;
	}

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

function g_HeaderButtonClicked()
{
	var header = document.getElementById('header');
	var button = document.getElementById('headeropenbutton');
	if( g_HeaderOpened )
	{
		header.style.top = '-200px';
		button.innerHTML = '&darr;';
		g_HeaderOpened = false;
	}
	else
	{
		header.style.top = '0px';
		button.innerHTML = '&uarr;';
		g_HeaderOpened = true;
	}
}
function g_FooterButtonClicked()
{
	var footer = document.getElementById('footer');
	var button = document.getElementById('footeropenbutton');
	if( g_FooterOpened )
	{
		footer.style.height = '26px';
		button.innerHTML = '&uarr;';
		document.getElementById('log_btn').classList.remove('pushed');
		document.getElementById('netlog_btn').classList.remove('pushed');
		document.getElementById('log').style.display = 'none';
		document.getElementById('netlog').style.display = 'none';
		document.getElementById('log_btn').style.display = 'none';
		document.getElementById('netlog_btn').style.display = 'none';
		g_FooterOpened = false;
	}
	else
	{
		footer.style.height = '226px';
		button.innerHTML = '&darr;';
		document.getElementById('log_btn').classList.add('pushed');
		document.getElementById('log').style.display = 'block';
		document.getElementById('log_btn').style.display = 'block';
		document.getElementById('netlog_btn').style.display = 'block';
		g_FooterOpened = true;
	}
}
function g_LogButtonClicked( i_type)
{
	var btn_log = document.getElementById('log_btn');
	var btn_net = document.getElementById('netlog_btn');
	var log = document.getElementById('log');
	var netlog = document.getElementById('netlog');
	if( i_type == 'log' )
	{
		btn_log.classList.add('pushed');
		btn_net.classList.remove('pushed');
		log.style.display = 'block';
		netlog.style.display = 'none';
	}
	else
	{
		btn_net.classList.add('pushed');
		btn_log.classList.remove('pushed');
		netlog.style.display = 'block';
		log.style.display = 'none';
	}
}
function g_Log( i_msg, i_log)
{
	if( i_log == null ) i_log = 'log';
	var log = document.getElementById( i_log);
	var lines = log.getElementsByTagName('div');
	if( lines.length && ( i_msg == lines[0].msg ))
	{
		var count = lines[0].msg_count + 1;
		var msg = '<i>'+g_cycle+' x'+count+':</i> '+i_msg;
		lines[0].innerHTML = msg;
		lines[0].msg_count = count;
		return;
	}

	var line = document.createElement('div');
	line.msg = i_msg;
	line.msg_count = 1;
	var msg = '<i>'+g_cycle+':</i> '+i_msg;
	line.innerHTML = msg;
	log.insertBefore( line, lines[0]);
	if( lines.length > 100 )
		log.removeChild( lines[100]);
}
function g_Info( i_msg, i_log)
{
	document.getElementById('info').textContent=i_msg;
	if( i_log == null || i_log == true )
		g_Log( i_msg);
}
function g_Error( i_err)
{
	g_Info('Error: ' + i_err);
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

	g_keysdown += String.fromCharCode( e.keyCode);
	if( g_keysdown.length > 5 )
		g_keysdown = g_keysdown.slice( g_keysdown.length - 5, g_keysdown.length);
//g_Info( g_keysdown );
	g_CheckSequence();
}

g_params = [];
g_params.push(['user_name','User Name', null,'Enter User Name<br/>Need Restart (F5)']);
g_params.push(['host_name','Host Name', 'pc','Enter Host Name']);
g_params.push(['gui_name','GUI Name', 'webgui','Enter GUI Name<br/>Needed For Logs Only']);
g_params.push(['run_symbol','Run Symbol', '★','Enter Any <a href="http://en.wikipedia.org/wiki/Miscellaneous_Symbols" target="_blank">Unicode<a/><br/>You Should Copy&Paste<br/>★☀☢☠☣☮☯☼♚♛♜☹♿⚔☻⚓⚒⚛⚡⚑☭']);
function g_ConstructSettingsGUI()
{
	var elParams = document.getElementById('parameters');
	for( var i = 0; i < g_params.length; i++)
	{
		var elParam = document.createElement('div');
		elParam.classList.add('parameter');
		elParam.style.top = (i*30)+'px';
		elParams.appendChild( elParam);

		var elLabel = document.createElement('div');
		elParam.appendChild( elLabel);
		elLabel.classList.add('label');
		elLabel.textContent = g_params[i][1];

		var elVariable = document.createElement('div');
		elParam.appendChild( elVariable);
		elVariable.classList.add('variable');

		var elButton = document.createElement('div');
		elParam.appendChild( elButton);
		elButton.classList.add('button');
		elButton.textContent = 'Edit';
		elButton.param = g_params[i][0];
		elButton.onclick = function(e){g_SetParameterDialog(e.currentTarget.param);}

		g_params[i].splice( 0, 0, elVariable);
	}
}
function g_InitSettings()
{
	if( localStorage.main_monitor == null ) localStorage.main_monitor = g_main_monitor_type;
	else g_main_monitor_type = localStorage.main_monitor;

	for( var i = 0; i < g_params.length; i++)
		g_SetParameter( g_params[i][1]);
}
function g_SetParameter( i_param, i_value)
{
	var initial = null;
	var title = null;
	var pos;
	for( pos = 0; pos < g_params.length; pos++)
		if( i_param == g_params[pos][1] )
		{
			title = g_params[pos][2];
			initial = g_params[pos][3];
			break;
		}

	if( i_value == null )
		i_value = localStorage[i_param];
	if( i_value == null )
		i_value = initial;
//window.console.log(i_param+'='+i_value);
	if( i_value == null )
	{
		g_SetParameterDialog( i_param)
		return;
	}

	localStorage[i_param] = i_value;

	g_params[pos][0].innerHTML = i_value;
}
function g_SetParameterDialog( i_param)
{
	var value = '';
	var title = 'Set Parameter';
	var info = null; 
	for( var i = 0; i < g_params.length; i++)
		if( i_param == g_params[i][1] )
		{
			title = 'Set '+g_params[i][2];
			value = g_params[i][3];
			info = g_params[i][4];
		}
	if( localStorage[i_param])
		value = localStorage[i_param];
	new cgru_Dialog( window, window, 'g_SetParameter', i_param, 'str', value, 'settings', title, info);
}

function g_LocalStorageShow() { g_ShowObject( localStorage);}
function g_LocalStorageClearClicked()
{
	new cgru_Dialog( window, window, 'g_LocalStorageClear', 'local_storage_clear', 'str', '', 'settings', 'Clear Local Storage', 'Are You Sure?<br/>Type "yes".');
}
function g_LocalStorageClear( i_name, i_value)
{
	if( i_name && i_value )
		if( i_value != 'yes' )
			return;
	localStorage.clear();
	g_Info('Local Storage Cleared.');
	g_InitSettings();
}

function g_CheckSequence()
{
	var god = ( g_keysdown == 'IDDQD' );
	var visor = false;
	if( god ) visor = true;
	else visor = ( g_keysdown == 'IDKFA' );

	if(( visor == false ) && ( god == false ))
		return;

	if( localStorage['visor'] )
	{
		localStorage.removeItem('visor');
		localStorage.removeItem('god');
		g_Info('USER MODE');
	}
	else
	{
		if( visor )
		{
			localStorage['visor'] = true;
			g_Info('VISOR MODE');
		}
		if( god )
		{
			localStorage['god'] = true;
			g_Info('GOD MODE');
		}
	}
	g_SuperUserProcessGUI();
}
function g_VISOR()
{
	if( localStorage['visor'] ) return true;
	if( g_uid < 1 ) return true;
	return false;
}
function g_GOD()
{
	if( localStorage['god'] ) return true;
	if( g_uid < 1 ) return true;
	return false;
}
function g_SuperUserProcessGUI()
{
//g_Info('g_SuperUserProcessGUI()')
	if( g_GOD())
	{
		document.getElementById('header').classList.add('su_god');
		document.getElementById('footer').classList.add('su_god');
	}
	else if( g_VISOR())
	{
		document.getElementById('header').classList.add('su_visor');
		document.getElementById('footer').classList.add('su_visor');
	}
	else
	{
		document.getElementById('header').classList.remove('su_visor');
		document.getElementById('header').classList.remove('su_god');
		document.getElementById('footer').classList.remove('su_visor');
		document.getElementById('footer').classList.remove('su_god');
	}
}

