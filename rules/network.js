function n_ReadDir( i_path)
{
	var path = i_path;
	if( RULES.root )
		path = RULES.root + path;

	var request = {};
	request.readdir = path;
	request.rules = RULES.rules;
	var data = n_Request( request);
	var response = c_Parse( data);

	if( response == null ) return null;
	if( response.readdir == null ) return null;

	return response.readdir;
}

function n_Request( i_obj)
{
	var obj_str = JSON.stringify( i_obj);

	var log = '<b><i>send:</i></b> '+ obj_str;

	var xhr = new XMLHttpRequest;
	xhr.overrideMimeType('application/json');
//	xhr.onerror = function() { g_Error(xhr.status + ':' + xhr.statusText); }
//	xhr.open('POST', 'server.php', true); 
	xhr.open('POST', 'rules.php', false); 
	xhr.send( obj_str);
//window.console.log('n_Requestr='+obj_str);

	log += '<br/><b><i>recv:</i></b> '+ xhr.responseText;
	c_Log( log);

//window.console.log('xhr.responseText='+xhr.responseText);
	return xhr.responseText;
/*	xhr.onreadystatechange = function()
	{
		if( xhr.readyState == 4 )
		{
			if( xhr.status == 200 )
			{
document.getElementById('recv').textContent='recv: ' + xhr.responseText;
				g_ProcessMsg( xhr.responseText);
				return;
			}
			g_ConnectionLost();
		}
	}
*/
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
