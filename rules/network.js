n_server = 'rules.php';

n_requests = [];
n_requests_count = 0;
n_conn_count = 0;

n_walks = {};

function n_WalkDir( i_paths, i_depth, i_rufolder, i_rufiles, i_lookahead, i_mtime)
{
	if( typeof( i_paths) != 'object')
	{
		c_Error('PATH "'+i_paths+'" not an object.');
		return null;
	}

	if( i_depth == null ) i_depth = 0;

	var paths = [];
	var cur_seconds = c_DT_CurSeconds();
	for( var i = 0; i < i_paths.length; i++)
	{
		if( i_mtime && n_walks[i_paths[i]] && ( cur_seconds - n_walks[i_paths[i]].walktime) < i_mtime )
			continue;
		else
			n_walks[i_paths[i]] = null

		if( RULES.root )
			paths.push( RULES.root + i_paths[i]);
		else
			paths.push( i_paths[i]);
	}

	if( paths.length )
	{
		var request = {};
		request.walkdir = paths;
		request.depth = i_depth;
		request.showhidden = ( localStorage.show_hidden == 'ON' );
		if( i_rufolder ) request.rufolder = i_rufolder;
		if( i_rufiles ) request.rufiles = i_rufiles;
		if( i_lookahead ) request.lookahead = i_lookahead;
		var data = n_Request({"send":request});
		var response = c_Parse( data);

		if( response == null ) return null;
		if( response.walkdir == null ) return null;
	}

	var o_walks = [];
	var w = 0;
	for( var i = 0; i < i_paths.length; i++ )
	{
		var walk = n_walks[i_paths[i]];
		if( walk == null )
		{
			walk = response.walkdir[w];
			walk.walktime = c_DT_CurSeconds();
			w++;
		}
		else 
			c_Log('Walk cached '+i_mtime+'s: '+i_paths[i]);
		o_walks.push( walk);
		if( walk == null ) continue;
//		if( walk.error ) continue;
		n_walks[i_paths[i]] = walk;
	}

	return o_walks;
//	return response.walkdir;
}

function n_Request_old( i_obj, i_wait, i_encode)
{
	return n_Request({"send":i_obj,"wait":i_wait,"encode":i_encode})
}
function n_Request( i_args)
{
	if( i_args.send == null )
	{
		c_Error('Network reqest: send object is null.');
		return;
	}

	i_args.id = n_requests_count++;
	if( p_PLAYER != true )
		i_args.path = g_CurPath();
	if( i_args.info == null ) i_args.info = '';

	if( i_args.wait == null)
		i_args.wait = true;
	else
		n_conn_count++;

	if( SERVER && SERVER.AUTH_RULES )
	{
		var digest = ad_ConstructDigest();
		if( digest ) i_args.send.digest = digest;
	}

	var send_str = JSON.stringify( i_args.send);
	if( i_args.encode == true )
		send_str = btoa( send_str);

	var log = '<b><i style="color:';
	if( i_args.wait )
		log += '#040">send '+i_args.id;
	else
		log += '#044">send '+i_args.id + '('+n_conn_count+')';
	log += '</i> '+i_args.info+':</b> '+ send_str;

	var xhr = new XMLHttpRequest;
	xhr.m_args = i_args;
//	n_requests.push( xhr);

	xhr.overrideMimeType('application/json');
//	xhr.onerror = function() { g_Error(xhr.status + ':' + xhr.statusText); }
//	xhr.open('POST', 'server.php', true); 
	xhr.open('POST', n_server, i_args.wait ? false : true); 
	xhr.send( send_str);
//window.console.log('n_Request_oldr='+send_str);

	if( i_args.wait )
	{
		log += '<br/><b><i style="color:#040">recv '+xhr.m_args.id+'</i> '+xhr.m_args.info+':</b> ';
		if( i_args.send.getfile )
			log += i_args.send.getfile;
		else
			log += xhr.responseText.replace(/[<>]/g,'*');
	}

	c_Log( log);

//window.console.log('xhr.responseText='+xhr.responseText);
	if( i_args.wait )
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

	u_el.cycle.classList.remove('timeout');
	u_el.cycle.style.opacity = '1';

	xhr.onreadystatechange = n_XHRHandler;
}

function n_XHRHandler()
{
//console.log( this);
//console.log( this.readyState);
	if( this.readyState == 4 )
	{
		n_conn_count--;
//		if( u_el && u_el.cycle ) setTimeout('u_el.cycle.classList.add("timeout");u_el.cycle.style.opacity = ".1";',1)
		if( n_conn_count < 0 ) n_conn_count = 0;
		if( n_conn_count == 0 )
		{
			u_el.cycle.classList.add("timeout");
			u_el.cycle.style.opacity = '.1';
		}

		if( this.status == 200 )
		{
			c_Log('<b><i style="color:#044">recv '+this.m_args.id+'('+n_conn_count+')</i> '+this.m_args.info+':</b> '+ this.responseText.replace(/[<>]/g,'*'));

			if( this.m_args.func )
			{
				if( p_PLAYER != true )
				if( this.m_args.local && ( this.m_args.path != g_CurPath() ))
					return;

				var data = this.responseText;
				if( this.m_args.parse )
					data = c_Parse( data);

				window[this.m_args.func]( data, this.m_args);
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
	
	n_Request({"send":obj});
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
	return n_Request({"send":{"getfile":c_GetRuFilePath( i_file)}});
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

	var result = c_Parse( n_Request({"send":{"sendmail":obj}}));
	if( result == null ) return false;
	if( result.error )
	{
		c_Error( result.error);
		return false;
	}

	return true;
}

