nw_connected = false;
nw_error_count = 0;
nw_error_count_max = 5;
nw_error_total = 0;

function nw_Send( obj)
{
	var obj_str = JSON.stringify(obj);

	var log = '<b><i>send:</i></b> '+ obj_str;

	var xhr = new XMLHttpRequest();
	xhr.overrideMimeType('application/json');
//	xhr.onerror = function() { g_Error(xhr.status + ':' + xhr.statusText); }
	xhr.open('POST', '/', true);

	xhr.setRequestHeader('AFANASY', '1 0 '+obj_str.length);

	xhr.send( obj_str);

	xhr.onreadystatechange = function()
	{
		if( xhr.readyState == 4 )
		{
			if(( xhr.status == 200 ) && xhr.responseText.length )
			{
				log += '<br/><b><i>recv:</i></b> '+ xhr.responseText;

				nw_error_count = 0;
				nw_connected = true;
//				g_ProcessMsg( eval('('+xhr.responseText+')'));
//				g_ProcessMsg( JSON.parse( xhr.responseText));

				var newobj = null;
				try { newobj = JSON.parse( xhr.responseText);}
				catch( err)
				{
					g_Log(err.message+'\n\n'+xhr.responseText);
					newobj = null;
				}

				if( newobj )
					g_ProcessMsg( newobj);
			}
			else
			{
				nw_error_count++;
				nw_error_total++;
//document.getElementById("status").textContent=nw_error_count+': Status number = ' + xhr.status;
//document.getElementById("statustext").textContent='Status text: ' + xhr.statusText;
				if(( nw_error_count > nw_error_count_max ) && nw_connected )
				{
					nw_connected = false;
					g_Error('Connection lost.');
					g_ConnectionLost();
				}
			}
			g_Log( log, 'netlog');
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
	if( i_class == 'jobs' )
	{
		var uid = g_uid;
		if( g_VISOR() || g_GOD() || ( uid < 0 ))
			uid = 0
		obj.action.operation.uids = [uid];
	}

	nw_Send(obj);
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

	nw_Send(obj);
}

function nw_GetNodes( i_type, i_ids, i_mode, i_blocks, i_tasks, i_number)
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

	nw_Send(obj);
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

	nw_Send(obj);
}

function nw_GetCGRUConfig()
{
	var obj = {};
	obj.get = {};
	obj.get.type = 'config';

	nw_Send(obj);
}

function nw_ReqestRendersResources()
{
	nw_GetNodes('renders', null, 'resources');
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

	nw_Send( obj);
}

