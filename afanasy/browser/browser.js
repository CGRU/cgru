g_cycle = 0;
g_last_msg_cycle = g_cycle;
g_id = 0;
g_uid = -1;
g_uid_orig = -1;
g_keysdown = '';
g_closing = false;

g_auth = {};
g_digest = null;

g_windows = [];
g_receivers = [];
g_refreshers = [];
g_monitors = [];
g_cur_monitor = null;
g_main_monitor = null;
g_main_monitor_type = 'jobs';
g_monitor_buttons = [];

g_TopWindow = null;

g_HeaderOpened = false;
g_FooterOpened = false;

g_Images = [];

function cgru_params_OnChange( i_param, i_value) { cm_ApplyStyles();}

function g_Init()
{
	g_Info('HTML body load.');
	cgru_Init();
	cm_Init();

	window.onbeforeunload = g_OnClose;
	document.body.onkeydown = g_OnKeyDown;

	$('platform').textContent = cgru_Platform;
	$('browser').textContent = cgru_Browser;

	if( localStorage.main_monitor )
		g_main_monitor_type = localStorage.main_monitor;

	var header = $('header');
	g_monitor_buttons = header.getElementsByClassName('mbutton');
	for( var i = 0; i < g_monitor_buttons.length; i++)
		g_monitor_buttons[i].onclick = function(e){return g_MButtonClicked(e.currentTarget.textContent,e);};

	g_GetConfig();
}

function g_GetConfig()
{
	var obj = {"get":{"type":"config"}};
	nw_request({"send":obj,"func":g_ConfigReceived});
}
function g_ConfigReceived( i_obj)
{
	if( i_obj.realm )
	{
		if( g_digest )
		{
			g_Error('Access denied.');
			g_DigestRemove();
			return;
		}
		g_DigestInit( i_obj);
		return;
	}

	if( i_obj.cgru_config )
	{
		if( false == cgru_ConfigLoad( i_obj.cgru_config))
		{
			g_Error('Invalid config recieved.');
			return;
		}

		if( cgru_Config.docs_url )
			$('docs_link').href = cgru_Config.docs_url + '/afanasy/gui#web';
		if( cgru_Config.forum_url )
			$('forum_link').href = cgru_Config.forum_url + '/viewforum.php?f=17';

		var title = 'CGRU version: ' + cgru_Environment.version;
		title += '\nBuild at: ' + cgru_Environment.builddate;
		$('version').textContent = cgru_Environment.version;
		$('version').title = title;

		var log = 'CGRU version: ' + cgru_Environment.version;
		log += '<br>Build at: ' + cgru_Environment.builddate;
		log += '<br>Build revision: ' + cgru_Environment.buildrevision;
		log += '<br>Server: ' + cgru_Environment.username + '@' + cgru_Environment.hostname + ':' + cgru_Environment.location;
		if( cgru_Environment.servedir && cgru_Environment.servedir.length )
			log += '<br>HTTP root override: ' + cgru_Environment.servedir;
		g_Log( log);
	}

	if( g_digest == null )
	{
		cgru_params.push(['user_name','User Name', 'coord', 'Enter user name<br/>Need restart (F5)']);
		$('auth_parameters').style.display = 'none';
	}
	cgru_params.push(['host_name','Host Name', 'pc','Enter host name<br/>Needed for logs only']);
	cgru_params.push(['run_symbol','Run Symbol', '★','Enter any <a href="http://en.wikipedia.org/wiki/Miscellaneous_Symbols" target="_blank">unicode</a><br/>You can copy&paste some:<br>★☀☢☠☣☮☯☼♚♛♜☹♿⚔☻⚓⚒⚛⚡⚑☭']);

	cgru_ConstructSettingsGUI();
	cgru_InitParameters();
	cgru_Info = g_Info;
	cgru_Error = g_Error;
	cm_ApplyStyles();

	nw_GetSoftwareIcons();
	g_RegisterSend();
	g_Refresh();
}

function g_DigestInit( i_obj)
{
	g_digest = null;
	g_auth.nc = 0;

	if( i_obj )
	{
		g_auth.nonce = i_obj.nonce;

		if(( localStorage.digest == null )
		|| ( localStorage.realm  == null )
		|| ( localStorage.realm != i_obj.realm )
		|| ( localStorage.user_name == null)
		|| ( localStorage.user_name.length < 1 ))
		{
			g_DigestRemove();
			localStorage.realm = i_obj.realm;
			g_DigestAsk();
			return;
		}
	}

	g_digest = localStorage.digest;
	g_auth.user_name = localStorage.user_name;
	$('auth_user').textContent = localStorage.user_name;
	g_GetConfig();
}
function g_DigestAsk()
{
	new cgru_Dialog({"handle":'g_DigestAskPasswd',"type":'str',"name":'settings',"title":'Login',"info":'Enter User Name'});
//	new cgru_Dialog( window, window, 'g_DigestAskPasswd', null, 'str', '', 'settings', 'Login', 'Enter User Name');
}
function g_DigestAskPasswd( i_value)
{
	localStorage.user_name = i_value;
	new cgru_Dialog({"handle":'g_DigestConstruct',"type":'str',"name":'settings',"title":'Login',"info":'Enter Password'});
//	new cgru_Dialog( window, window, 'g_DigestConstruct', null, 'str', '', 'settings', 'Login', 'Enter Password');
}
function g_DigestConstruct( i_value)
{
	localStorage.digest = hex_md5( localStorage.user_name + ':' + localStorage.realm + ':' + i_value);
	g_DigestInit();
}
function g_DigestRemove()
{
	localStorage.removeItem('digest');
	localStorage.removeItem('realm');
}
function g_Logout()
{
	g_DigestRemove();
	window.location.reload();
}

function g_RegisterSend()
{
	if( g_id != 0)
		return;

	g_Info('Sending register request.');

	var obj = {};
	obj.monitor = {};
	obj.monitor.user_name = localStorage['user_name'];
	obj.monitor.host_name = localStorage['host_name'];
	obj.monitor.engine = cgru_Browser;
	nw_send(obj);

	setTimeout('g_RegisterSend()', 5000);
}

function g_ProcessMsg( i_obj)
{
//g_Info( g_cycle+' Progessing '+g_receivers.length+' recieves');
	g_last_msg_cycle = g_cycle;

	// Realm is sended if message not authorized
	if( i_obj.realm )
	{
		g_Error('Authentication problems...');
		return;
	}

	// Preload images (service icons):
	if( i_obj.files && i_obj.path )
	{
		for( var i = 0; i < i_obj.files.length; i++)
		{
			var img = new Image();
			img.src = i_obj.path + "/" + i_obj.files[i];
			g_Images.push( img);
		}
		return;
	}

	if( i_obj.monitor )
	{
		if(( g_id == 0 ) && ( i_obj.monitor.id > 0 ))
		{
			// Monitor is not registered and recieved an ID:
			g_RegisterRecieved( i_obj.monitor);
		}
		else if( i_obj.monitor.id != g_id )
		{
			// Recieved ID does not match:
			g_Info('This ID = '+g_id+' != '+i_obj.monitor.id+' recieved.');
			g_Deregistered();
		}
		return;
	}

	if( i_obj.message || i_obj.info || i_obj.object )
	{
		g_ShowObject( i_obj);
		return;
	}

	if( i_obj.events && i_obj.events.tasks_outputs && i_obj.events.tasks_outputs.length )
		for( var i = 0; i < i_obj.events.tasks_outputs.length; i++)
			WndTaskShow({'task':i_obj.events.tasks_outputs[i]});
	if( i_obj.events && i_obj.events.tasks_listens && i_obj.events.tasks_listens.length )
		for( var i = 0; i < i_obj.events.tasks_listens.length; i++)
			WndTaskShow({'task':i_obj.events.tasks_listens[i]});

	if( g_id == 0 )
		return;

	for( var i = 0; i < g_receivers.length; i++)
	{
		g_receivers[i].processMsg( i_obj);
	}
}

function g_ReceiverAdd( i_obj)
{
	if( g_ReceiverExist( i_obj))
		g_Error('g_ReceiverAdd: Receiver "' + i_obj.name + '" already exists.');
	else
		g_receivers.push( i_obj);

}
function g_ReceiverRemove( i_obj)
{
	if( g_ReceiverExist( i_obj ))
		cm_ArrayRemove( g_receivers, i_obj);
	else
		g_Error('g_ReceiverRemove: Receiver "' + i_obj.name + '" does not exist.');
}
function g_ReceiverExist( i_obj)
{
	for( var i = 0; i < g_receivers.length; i++)
		if( g_receivers[i] == i_obj )
			return true;
	return false;
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

function g_RegisterRecieved( i_obj)
{
	g_id = i_obj.id;
	if( i_obj.uid && ( i_obj.uid > 0 ))
	{
		g_uid_orig = i_obj.uid;
		if( g_uid == -1 )
			g_uid = g_uid_orig;
	}

	this.document.title = 'AF';
	g_Info('Registed: ID = '+g_id+' User = "'+localStorage['user_name']+'"['+g_uid+"]");
	$('registered').textContent = 'Registered';
	$('id').textContent = g_id;
	$('uid').textContent = g_uid;

	g_MButtonClicked( g_main_monitor_type);

	g_SuperUserProcessGUI();
}

function g_Deregistered()
{
	if( g_id == 0 )
		return;

	this.document.title = 'AF (deregistered)';
	g_id = 0;
	g_uid = -1;
	g_uid_orig = -1;
	g_Info('Deregistered.');
	$('registered').textContent = 'Deregistered';
	$('id').textContent = g_id;
	$('uid').textContent = g_uid;
	g_CloseAllWindows();
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


	g_OpenMonitor({"type":i_type,"evt":i_evt});
}

function g_MonitorClosed( i_monitor)
{
	for( var i = 0; i < g_monitor_buttons.length; i++)
		if( g_monitor_buttons[i].textContent == i_monitor.name )
			g_monitor_buttons[i].classList.remove('pushed');
	if( g_main_monitor == i_monitor )
		g_main_monitor = null;
}

//function g_OpenMonitor( i_type, i_evt, i_id, i_name)
function g_OpenMonitor( i_args)
{
	if( i_args.name == null )
		i_args.name = i_args.type;

	if( i_args.wnd == null )
		i_args.wnd = window;

	var new_wnd = false;
	if( i_args.evt )
	{
		if( i_args.evt.shiftKey ) new_wnd = true;
		if( i_args.evt.ctrlKey ) new_wnd = true;
		if( i_args.evt.altKey ) new_wnd = true;
	}

	for( var i = 0; i < g_monitors.length; i++)
		if( g_monitors[i].name == i_args.name )
		{
			g_Info('Monitor "'+i_args.name+'" already opened.', false);
			g_monitors[i].window.focus();
			return;
		}

	i_args.elParent = $('content');
	if(( i_args.type == 'tasks' ) && ( new_wnd == false ))
	{
		if( g_TopWindow )
		{
			g_TopWindow.destroy();
		}

		g_TopWindow = new cgru_Window({"name":'tasks',"title":i_args.name,"wnd":i_args.wnd,"closeOnEsc":false,"addClasses":["cgru_absolute","tasks"]});
		g_TopWindow.closeOnEsc = false;
		g_TopWindow.onDestroy = function(){ g_TopWindow.monitor.destroy(); g_TopWindow = null;};

		i_args.elParent = g_TopWindow.elContent;
	}
	else if( new_wnd )
	{
		i_args.wnd = g_OpenWindowWrite( i_args.name);
		if( i_args.wnd == null ) return;
		i_args.elParent = i_args.wnd.document.body;
	}
	else if( g_main_monitor )
		g_main_monitor.destroy();

	var monitor = new Monitor( i_args);

	if( new_wnd )
	{
		i_args.wnd.monitor = monitor;
		i_args.wnd.onbeforeunload = function(e){e.currentTarget.monitor.destroy()};
	}
	else if( i_args.type == 'tasks')
	{
		g_TopWindow.monitor = monitor;
	}
	else
	{
		g_main_monitor = monitor;
		g_main_monitor_type = i_args.type;
		localStorage.main_monitor = i_args.type;
	}

	return monitor;
}

function g_CloseAllMonitors()
{
	cgru_ClosePopus();

//	for( var i = 0; i < g_monitor_buttons.length; i++)
//		g_monitor_buttons[i].classList.remove('pushed');

	while( g_monitors.length > 0 )
		g_monitors[0].destroy();
}


function g_ShowObject( i_data, i_args)
{
	if( i_data.info )
	{
		if( i_data.info.kind == 'log')
			g_Log( i_data.info.text);
		else if( i_data.info.kind == 'error')
			g_Error( i_data.info.text);
		else
			g_Info( i_data.info.text);
			return;
	}

	var object = i_data;
	var type = 'object';
	if( i_data.object )
		object = i_data.object;
	else if( i_data.message )
	{
		object = i_data.message;
		type = 'message';
		g_Info('Message received.');
	}

	if( i_args == null )
	{
		i_args = {};
	}

	var title = 'Object';
	if( object.name ) title = object.name;
	if( i_args.name ) title = i_args.name;
	if( object.type ) title += ' ' + object.type;
	i_args.title = title;

	var wnd = g_OpenWindow( i_args);

	if( type == 'message')
	{
		for( var i = 0; i < object.list.length; i++)
		{
			var el = document.createElement('p');
			el.innerHTML = object.list[i].replace(/\n/g,'<br/>');
			wnd.elContent.appendChild(el);
		}
	}
	else
	{
		var el = document.createElement('p');
		el.innerHTML = JSON.stringify( object, null, '&nbsp&nbsp&nbsp&nbsp').replace(/\n/g,'<br/>');
		wnd.elContent.appendChild(el);
	}
}
function g_OpenWindow( i_args)
{
	var new_wnd = false;
	var wnd = window;
	if( i_args.wnd )
	{
		wnd = i_args.wnd;
	}
	if( i_args.evt )
	{
		if( i_args.evt.shiftKey ) new_wnd = true;
		if( i_args.evt.ctrlKey ) new_wnd = true;
		if( i_args.evt.altKey ) new_wnd = true;
	}

	var elContent = null;
	if( new_wnd )
	{
		wnd = g_OpenWindowWrite( i_args.title);
		if( wnd == null )
			return;

		wnd.elContent = wnd.document.body;
		wnd.document.title = i_args.title;
	}
	else
	{
		wnd = new cgru_Window({"name":i_args.title,"wnd":wnd});
	}

	return wnd;
}
function g_OpenWindowLoad( i_file, i_name)
{
	for( var i = 0; i < g_windows.length; i++)
		if( g_windows[i].name == i_name )
			g_windows[i].close();

	var wnd = window.open('afanasy/browser/' + i_file, name, 'location=no,scrollbars=yes,resizable=yes,menubar=no');
	if( wnd == null )
	{
		g_Error('Can`t open window "'+i_file+'"');
		return;
	}
	g_windows.push( wnd);
	wnd.name = i_name;
	wnd.focus();
	return wnd;
}

function g_OpenWindowWrite( i_name, i_title, i_notFinishWrite )
{
	if( i_title == null )
		i_title = i_name;

	for( var i = 0; i < g_windows.length; i++)
		if( g_windows[i].name == i_name )
			g_windows[i].close();

	var wnd = window.open( null, i_name, 'location=no,scrollbars=yes,resizable=yes,menubar=no');
	if( wnd == null )
	{
		g_Error('Can`t open new browser window.');
		return;
	}

	g_windows.push( wnd);
	wnd.name = i_name;

	wnd.document.writeln('<!DOCTYPE html>');
	wnd.document.writeln('<html><head><title>'+i_title+'</title>');
	wnd.document.writeln('<link type="text/css" rel="stylesheet" href="lib/styles.css">');
	wnd.document.writeln('<link type="text/css" rel="stylesheet" href="afanasy/browser/style.css">');
	if(( i_notFinishWrite == null ) || ( i_notFinishWrite == false ))
	{
		wnd.document.writeln('</head><body></body></html>');
		wnd.document.body.onkeydown = g_OnKeyDown;
	}
	if( wnd.document.body )
	{
		if( localStorage.background ) wnd.document.body.style.background = localStorage.background;
		if( localStorage.text_color ) wnd.document.body.style.color = localStorage.text_color;
	}
	wnd.focus();
	wnd.document.close();

	return wnd;
}

function g_CloseAllWindows()
{
	for( var i = 0; i < g_windows.length; i++)
		g_windows[i].close();
}

function g_HeaderButtonClicked()
{
	var header = $('header');
	var button = $('headeropenbutton');
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
	var footer = $('footer');
	var button = $('footeropenbutton');
	if( g_FooterOpened )
	{
		footer.style.height = '26px';
		button.innerHTML = '&uarr;';
		$('log_btn').classList.remove('pushed');
		$('netlog_btn').classList.remove('pushed');
		$('log').style.display = 'none';
		$('netlog').style.display = 'none';
		$('log_btn').style.display = 'none';
		$('netlog_btn').style.display = 'none';
		g_FooterOpened = false;
	}
	else
	{
		footer.style.height = '226px';
		button.innerHTML = '&darr;';
		$('log_btn').classList.add('pushed');
		$('log').style.display = 'block';
		$('log_btn').style.display = 'block';
		$('netlog_btn').style.display = 'block';
		g_FooterOpened = true;
	}
}
function g_LogButtonClicked( i_type)
{
	var btn_log = $('log_btn');
	var btn_net = $('netlog_btn');
	var log = $('log');
	var netlog = $('netlog');
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
	var log = $( i_log);
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
	$('info').textContent=i_msg;
	if( i_log == null || i_log == true )
		g_Log( i_msg);
}
function g_Error( i_err, i_log)
{
	g_Info('Error: ' + i_err, i_log);
}

function g_OnClose()
{
	localStorage.main_monitor = g_main_monitor_type;

	var operation = {};
	operation.type = 'deregister';
	if( g_id)
		nw_Action('monitors', [g_id], operation);

	g_closing = true;

	g_CloseAllWindows();
	g_CloseAllMonitors();
}

function g_OnKeyDown(e)
{
	if( ! e ) return;
	if( e.keyCode == 27 ) // ESC
	{
		if( cgru_EscapePopus())
			return;
		for( var i = 0; i < g_monitors.length; i++)
			g_monitors[i].selectAll( false);
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
//$('test').textContent='key down: ' + e.keyCode;
//	return true;

	g_keysdown += String.fromCharCode( e.keyCode);
	if( g_keysdown.length > 5 )
		g_keysdown = g_keysdown.slice( g_keysdown.length - 5, g_keysdown.length);
//g_Info( g_keysdown );
	g_CheckSequence();
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
		g_uid = g_uid_orig;
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
		g_uid = 0;
		$('header').classList.add('su_god');
		$('footer').classList.add('su_god');
	}
	else if( g_VISOR())
	{
		g_uid = 0;
		$('header').classList.add('su_visor');
		$('footer').classList.add('su_visor');
	}
	else
	{
		g_uid = g_uid_orig;
		$('header').classList.remove('su_visor');
		$('header').classList.remove('su_god');
		$('footer').classList.remove('su_visor');
		$('footer').classList.remove('su_god');
	}

	if( g_uid == 0 )
	{
		var obj = nw_ConstructActionObject('monitors', [g_id]);
		obj.action.operation = {};
		obj.action.operation.type = 'watch';
		obj.action.operation.class = 'perm';
		obj.action.operation.uid = 0;

		nw_send(obj);
	}
}
