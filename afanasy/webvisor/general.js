var $ = function( id ) { return document.getElementById( id ); };

function g_Init()
{
	g_Log('init');
	g_Request({"send":'init',"func":g_Start});
}

function g_Start()
{
	g_Log('Started');
}

function g_Log( i_msg)
{
	$('info').innerHTML = i_msg;
	$('info').classList.remove('error');
}

function g_Error( i_msg)
{
	g_Log( i_msg);
	$('info').classList.add('error');
}

function g_Request( i_args)
{
	if( i_args.send == null )
	{
		g_Error('Network reqest: send object is null.');
		return;
	}

	if( i_args.func )
	{
		i_args.wait = false;
		if( i_args.parse == null )
			i_args.parse = true;
	}
	else if( i_args.wait == null )
		i_args.wait = true;

	var send_str = JSON.stringify( i_args.send);
	if( i_args.encode == true )
		send_str = btoa( send_str);

	var xhr = new XMLHttpRequest;
	xhr.m_args = i_args;

	xhr.overrideMimeType('application/json');
//	xhr.onerror = function() { g_Error(xhr.status + ':' + xhr.statusText); }
	xhr.open('POST', 'server.php', i_args.wait ? false : true); 
	xhr.send( send_str);

	if( i_args.wait )
	{
		if( xhr.getResponseHeader('WWW-Authenticate'))
		{
			g_Error('Authorization Required');
			return null;
		}
		return xhr.responseText;
	}

	xhr.onreadystatechange = n_XHRHandler;
}

function n_XHRHandler()
{
//console.log( this);
//console.log( this.readyState);
	if( this.readyState == 4 )
	{
		if( this.status == 200 )
		{
			if( this.m_args.func )
			{
				var data = this.responseText;
				if( this.m_args.parse )
					data = c_Parse( data);

				this.m_args.func( data, this.m_args);
			}
		}
	}
}

