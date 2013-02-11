n_sendCount = 0;
n_recvCount = 0;
n_server = 'rules.php';

function n_ReadConfig()
{
	var request = {};
	request.readconfig = 'config_default.json';
	return c_Parse( n_Request( request));
}

function n_WalkDir( i_paths, i_depth, i_rufolder, i_rufiles, i_lookahead)
{
	if( typeof( i_paths) != 'object')
	{
		c_Error('PATH "'+i_paths+'" not on object.');
		return null;
	}

	if( i_depth == null ) i_depth = 0;

	var paths = [];
	for( var i = 0; i < i_paths.length; i++)
		if( RULES.root )
			paths.push( RULES.root + i_paths[i]);
		else
			paths.push( i_paths[i]);

	var request = {};
	request.walkdir = paths;
	request.depth = i_depth;
	if( i_rufolder ) request.rufolder = i_rufolder;
	if( i_rufiles ) request.rufiles = i_rufiles;
	if( i_lookahead ) request.lookahead = i_lookahead;
	var data = n_Request( request);
	var response = c_Parse( data);

	if( response == null ) return null;
	if( response.walkdir == null ) return null;

	return response.walkdir;
}

function n_Request( i_obj, i_wait)
{
	if( i_wait == null) i_wait = true;
	var obj_str = JSON.stringify( i_obj);

	var log = '<b style="color:';
	if( i_wait ) log += '#040';
	else log += '#044';
	log += '"><i>send'+(n_sendCount++)+':</i></b> '+ obj_str;

	var xhr = new XMLHttpRequest;
	xhr.overrideMimeType('application/json');
//	xhr.onerror = function() { g_Error(xhr.status + ':' + xhr.statusText); }
//	xhr.open('POST', 'server.php', true); 
	xhr.open('POST', n_server, i_wait ? false : true); 
	xhr.send( obj_str);
//window.console.log('n_Requestr='+obj_str);

	if( i_wait )
		log += '<br/><b style="color:#040"><i>recv'+(n_recvCount++)+':</i></b> '+ xhr.responseText;

	c_Log( log);

//window.console.log('xhr.responseText='+xhr.responseText);
	if( i_wait )
	{
		if( xhr.getResponseHeader('WWW-Authenticate'))
		{
			c_Error('Authorization Required');
			g_GO('/');
//			window.location.reload();
			return null;
		}
		return xhr.responseText;
	}

	xhr.onreadystatechange = function()
	{
		if( xhr.readyState == 4 )
		{
			if( xhr.status == 200 )
			{
				c_Log('<b style="color:#044"><i>recv'+(n_recvCount++)+':</i></b> '+ xhr.responseText);
				if( window.n_MessageReceived )
					window.n_MessageReceived( c_Parse( xhr.responseText));
				return;
			}
		}
	}
}

function n_SendJob( job)
{
	if( job.user_name == null )
		job.user_name = localStorage.user_name;
	if( job.host_name == null )
		job.host_name = localStorage.host_name;

	var obj = {};
	obj.afanasy = 1;
	obj.job = job;
	obj.address = cgru_Config.af_servername;
	obj.port = cgru_Config.af_serverport;
	obj.sender_id = 0;
	obj.magick_number = cgru_Config.af_magic_number;
	
	n_Request( obj);
}
/*
function n_Get( i_file)
{
	var path = i_file;
	if( path == null ) return;

	if( RULES['root'])
		path = RULES['root'] + path;

	var log = '<b><i>send:</i></b> GET '+ path;

	var xhr = new XMLHttpRequest;
	xhr.open('GET', path, false); 
	xhr.setRequestHeader('Pragma','no-cache');
	xhr.setRequestHeader('Cache-Control','no-cache');
//	xhr.overrideMimeType('application/json');
	xhr.send( null);

	log += '<br/><b><i>recv:</i></b> '+ xhr.responseText;
	c_Log( log);

	return xhr.responseText;
}
*/

