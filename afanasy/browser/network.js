nw_connected = false;
nw_error_count = 0;
nw_error_count_max = 5;
nw_error_total = 0;

function nw_send( obj)
{
	nw_request({"send":obj});
}

function nw_request( i_args)
{
	if( g_closing )
		return;

	var obj = i_args.send;
	if( g_digest )
	{
		g_auth.nc++;
		g_auth.response = hex_md5( g_digest + ':' + g_auth.nonce + ':' + g_auth.nc);
		obj.auth = g_auth;
	}

	var obj_str = JSON.stringify( obj);

	var xhr = new XMLHttpRequest();
	xhr.overrideMimeType('application/json');
//	xhr.onerror = function() { g_Error(xhr.status + ':' + xhr.statusText); }
	xhr.open('POST', '/', true);

	xhr.setRequestHeader('AFANASY', obj_str.length);
	xhr.setRequestHeader('Connection','close');

	xhr.m_log = '<b><i>send:</i></b> '+ obj_str;
	xhr.m_args = i_args;

	xhr.send( obj_str);

	xhr.onreadystatechange = n_XHRHandler;
}

function n_XHRHandler()
{
	if( this.readyState == 4 )
	{
		if( this.status == 200 )
		{
			if( this.responseText.length )
				this.m_log += '<br><b><i>recv:</i></b> '+ this.responseText;
			g_Log( this.m_log, 'netlog');

			nw_error_count = 0;
			nw_connected = true;

			if( this.responseText.length )
			{
				var recv_obj = null;
				try { recv_obj = JSON.parse( this.responseText);}
				catch( err)
				{
					g_Error('JSON.parse:');
					g_Log( err.message+'<br>'+this.responseText);
					recv_obj = null;
				}

				if( recv_obj )
				{
					if( this.m_args.func )
						this.m_args.func( recv_obj, this.m_args);
					else
						g_ProcessMsg( recv_obj);
				}
			}
		}
		else
		{
			nw_error_count++;
			nw_error_total++;
			if(( nw_error_count > nw_error_count_max ) && nw_connected )
			{
				nw_connected = false;
				g_Error('Connection lost.');
				g_ConnectionLost();
			}
		}
	}
}

function nw_Subscribe( i_class, i_subscribe, i_ids)
{
	if( g_id == 0 ) return;

	var obj = nw_ConstructActionObject('monitors', [g_id]);
	obj.action.operation = {};
	obj.action.operation.type = 'watch';
	obj.action.operation.class = i_class;
	if( i_subscribe )
		obj.action.operation.status = 'subscribe';
	else
		obj.action.operation.status = 'unsubscribe';
	if( i_ids != null )
		obj.action.operation.ids = i_ids;
/*	if( i_class == 'jobs' )
	{
		var uid = g_uid;
		if( g_VISOR() || g_GOD() || ( uid < 0 ))
			uid = 0
		obj.action.operation.uids = [uid];
	}*/

	nw_send(obj);
}

function nw_GetEvents()
{
	if( g_id == 0 ) return;
//info('c' + g_cycle + ' getting events...');
	var obj = {};
	obj.get = {};
	obj.get.type = 'monitors';
	obj.get.ids = [g_id];
	obj.get.mode = 'events';

	nw_send(obj);
}

function nw_GetNodes( i_type, i_ids, i_mode, i_blocks, i_tasks, i_number, i_func)
{
	var obj = {};
	obj.get = {};
	obj.get.type = i_type;

	if(( i_ids != null ) && ( i_ids.length > 0 ))
		obj.get.ids = i_ids;
	else if(( i_type == 'jobs') && ( false == ( g_VISOR() || g_GOD() )))
		obj.get.uids = [g_uid];

	if( i_mode )
		obj.get.mode = i_mode;
	if( i_blocks )
		obj.get.block_ids = i_blocks;
	if( i_tasks )
		obj.get.task_ids = i_tasks;
	if( i_number )
		obj.get.number = i_number;

	nw_request({"send":obj,"func":i_func});
}

function nw_GetBlocks( i_job_id, i_blocks, i_modes)
{
	nw_GetNodes( 'jobs', [i_job_id], i_modes, i_blocks);
}

function nw_GetSoftwareIcons()
{
	var obj = {};
	obj.get = {};
	obj.get.type = 'files';
	obj.get.path = 'icons/software';

	nw_send(obj);
}

function nw_ConstructActionObject( i_type, i_ids)
{
	var obj = {};
	obj.action = {};
	obj.action.user_name = localStorage['user_name'];
	obj.action.host_name = localStorage['host_name'];
	obj.action.type = i_type;
	obj.action.ids = i_ids;

	return obj;
}

function nw_Action( i_type, i_ids, i_operation, i_params, i_block_ids)
{
	if( i_ids.length == 0 )
	{
		g_Error( i_type + ' Action: IDs are empty.');
		return;
	}

	var obj = nw_ConstructActionObject( i_type, i_ids);

	if( i_params    ) obj.action.params = i_params;
	if( i_operation ) obj.action.operation = i_operation;
	if( i_block_ids ) obj.action.block_ids = i_block_ids;

	nw_send( obj);
}

