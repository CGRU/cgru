n_sendCount = 0;
n_recvCount = 0;
n_server = 'rules.php';

function n_WalkDir( i_paths, i_depth, i_rufolder, i_rufiles, i_lookahead)
{
	if( typeof( i_paths) != 'object')
	{
		c_Error('PATH "'+i_paths+'" not an object.');
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
	request.showhidden = ( localStorage.show_hidden == 'ON' );
	if( i_rufolder ) request.rufolder = i_rufolder;
	if( i_rufiles ) request.rufiles = i_rufiles;
	if( i_lookahead ) request.lookahead = i_lookahead;
	var data = n_Request( request);
	var response = c_Parse( data);

	if( response == null ) return null;
	if( response.walkdir == null ) return null;

	return response.walkdir;
}

function n_Request( i_obj, i_wait, i_encode)
{
	if( i_wait == null) i_wait = true;

	if( SERVER && SERVER.AUTH_RULES )
	{
		var digest = ad_ConstructDigest();
		if( digest ) i_obj.digest = digest;
	}

	var obj_str = JSON.stringify( i_obj);
	if( i_encode == true )
		obj_str = btoa( obj_str);

	var log = '<b style="color:';
	if( i_wait ) log += '#040';
	else log += '#044';
	log += '"><i>send:</i></b> '+ obj_str;
//	log += '"><i>send'+(n_sendCount++)+':</i></b> '+ obj_str;

	var xhr = new XMLHttpRequest;
	xhr.overrideMimeType('application/json');
//	xhr.onerror = function() { g_Error(xhr.status + ':' + xhr.statusText); }
//	xhr.open('POST', 'server.php', true); 
	xhr.open('POST', n_server, i_wait ? false : true); 
	xhr.send( obj_str);
//window.console.log('n_Requestr='+obj_str);

	if( i_wait )
	{
		log += '<br/><b style="color:#040"><i>recv:</i></b> ';
//		log += '<br/><b style="color:#040"><i>recv'+(n_recvCount++)+':</i></b> ';
		if( i_obj.getfile )
			log += i_obj.getfile;
		else
			log += xhr.responseText;
	}

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
	if( g_auth_user == null )
	{
		c_Error('Guests can`t send jobs.');
		return;
	}

	if( job.user_name == null )
		job.user_name = g_auth_user.id;
	if( job.host_name == null )
		job.host_name = cgru_Browser;

	var obj = {};
	obj.afanasy = 1;
	obj.job = job;
	obj.address = cgru_Config.af_servername;
	obj.port = cgru_Config.af_serverport;
	obj.sender_id = 0;
	obj.magick_number = cgru_Config.af_magic_number;
	
	n_Request( obj);
}

function n_Get( i_path)
{
	var log = '<b><i>get:</i></b> '+ i_path;
	var xhr = new XMLHttpRequest;
	xhr.open('GET', i_path, false); 
	xhr.setRequestHeader('Pragma','no-cache');
	xhr.setRequestHeader('Cache-Control','no-cache');
//	xhr.overrideMimeType('application/json');
	xhr.send( null);
//	log += '<br/><b><i>recv:</i></b> '+ xhr.responseText;
	c_Log( log);
	return xhr.responseText;
}


function n_GetRuFile( i_file, i_nockeck )
{
	if( i_nockeck != true )
		if( false == c_RuFileExists( i_file)) return null;
	return n_Request({"getfile":c_GetRuFilePath( i_file)});
//	return n_Get( c_GetRuFilePath( i_file));
}

function n_SendMail( i_address, i_subject, i_body)
{
	var obj = {};
	obj.address = c_EmailEncode( i_address);
	obj.subject = i_subject;

	obj.headers = '';
	obj.headers += 'MIME-Version: 1.0\r\n';
	obj.headers += 'Content-type: text/html; charset=utf-8\r\n';
//	obj.headers += 'To: <'+i_address+'>\r\n';
	obj.headers += 'From: CGRU <noreply@cgru.info>\r\n';

	obj.body = '<html><body>';
	obj.body += '<div style="background:#DFA; color:#020; margin:8px; padding:8px; border:2px solid #070; border-radius:9px;">';
	obj.body += i_subject;
	obj.body += '<div style="background:#FFF; color:#000; margin:8px; padding:8px; border:2px solid #070; border-radius:9px;">';
	obj.body += i_body;
	obj.body += '</div><a href="cgru.info" style="padding:10px;margin:10px;" target="_blank">CGRU</a>';
	obj.body += '</div></body></html>';

	var result = c_Parse( n_Request({"sendmail":obj}));
//	var result = c_Parse( n_Request({"sendmail":obj}, true, true));
	if( result == null ) return false;
	if( result.error )
	{
		c_Error( result.error);
		return false;
	}

	return true;
}

